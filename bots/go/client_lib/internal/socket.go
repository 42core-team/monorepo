package internal

import (
	"bufio"
	"encoding/json"
	"fmt"
	"net"
	"strings"
	"time"

	"github.com/42core-team/go-client-lib/game"
)

type loginRequest struct {
	ID       uint   `json:"id"`
	Password string `json:"password"`
	TeamName string `json:"name"`
}

type clientPacket struct {
	Actions []Action     `json:"actions"`
	Debug   []DebugEntry `json:"debug_data"`
}

type Connection struct {
	socket         net.Conn
	reader         *bufio.Reader
	game           *game.Game
	actionQueue    *ActionQueue
	debugData      *DebugData
	onTickCallback func(*game.Game)
	debug          bool
}

const socketReadTimeout = 42 * time.Second

func NewConnection(serverAddr string, selfTeamID uint, debug bool) (*Connection, error) {
	fmt.Println("Connecting to server")
	var conn net.Conn
	var err error
	for {
		conn, err = net.Dial("tcp", serverAddr)
		if err == nil {
			break
		}
		fmt.Print(".")
		time.Sleep(350 * time.Millisecond)
	}
	fmt.Println("Connected!")

	return &Connection{
		socket:      conn,
		reader:      bufio.NewReader(conn),
		game:        &game.Game{MyTeamID: selfTeamID},
		actionQueue: NewActionQueue(100),
		debugData:   NewDebugData(100),
		debug:       debug,
	}, nil
}

func (c *Connection) readLine() (string, error) {
	if err := c.socket.SetReadDeadline(time.Now().Add(socketReadTimeout)); err != nil {
		return "", fmt.Errorf("failed to set socket read deadline: %v", err)
	}
	line, err := c.reader.ReadString('\n')
	if len(line) > 0 {
		return strings.TrimSuffix(line, "\n"), nil
	}
	if err != nil {
		return "", fmt.Errorf("error reading from socket: %v", err)
	}
	return "", fmt.Errorf("connection closed by server")
}

func (c *Connection) Start(teamID uint, teamName string) error {
	defer func() {
		if err := c.Close(); err != nil {
			fmt.Printf("Error closing connection: %v\n", err)
		}
	}()

	if err := c.sendLoginPacket(teamID, teamName); err != nil {
		return fmt.Errorf("failed to send login packet: %v", err)
	}

	configLine, err := c.readLine()
	if err != nil {
		return fmt.Errorf("failed to receive config: %v", err)
	}
	if err := json.Unmarshal([]byte(configLine), &c.game.Config); err != nil {
		return fmt.Errorf("failed to parse config: %v", err)
	}
	if c.debug {
		fmt.Printf("Received config: %s\n", configLine)
	}

	for {
		actions := c.actionQueue.Drain()
		debugEntries := make([]DebugEntry, 0)
		if c.debugData.HasData() {
			debugEntries = c.debugData.GetEntries()
		}
		c.debugData.Reset()

		if err := c.sendPacket(actions, debugEntries); err != nil {
			return fmt.Errorf("error sending packet: %v", err)
		}

		line, err := c.readLine()
		if err != nil {
			fmt.Println("The connection was closed by the server. Bye, bye!")
			break
		}
		if c.debug {
			fmt.Printf("Received state: %s\n", line)
		}

		tick, err := parseGameTick(line)
		if err != nil {
			return fmt.Errorf("error parsing game tick: %v", err)
		}
		tick.applyTo(c.game)

		if c.onTickCallback != nil {
			c.onTickCallback(c.game)
		}
		if core := c.game.MyCore(); core == nil || !core.IsAlive() {
			break
		}
	}
	c.printGameResult()

	return nil
}

func (c *Connection) printGameResult() {
	myCore := c.game.MyCore()
	livingOpponentCores := c.game.ObjectsFilterCount(func(object *game.Object) bool {
		data := object.GetCoreData()
		return data != nil && data.TeamID != c.game.MyTeamID && object.IsAlive()
	})
	if myCore != nil && myCore.IsAlive() && livingOpponentCores == 0 {
		fmt.Println("Game over! You won!")
	} else {
		fmt.Println("Game over! You lost!")
	}
}

func (c *Connection) sendLoginPacket(teamID uint, teamName string) error {
	login := loginRequest{ID: teamID, Password: "42", TeamName: teamName}
	data, err := json.Marshal(login)
	if err != nil {
		return fmt.Errorf("error marshaling login request: %v", err)
	}
	return c.send(data)
}

func (c *Connection) sendPacket(actions []Action, debugEntries []DebugEntry) error {
	packet := clientPacket{
		Actions: actions,
		Debug:   debugEntries,
	}
	data, err := json.Marshal(packet)
	if err != nil {
		return fmt.Errorf("error marshaling client packet: %v", err)
	}
	if c.debug {
		fmt.Printf("Actions: %s\n", data)
	}
	return c.send(data)
}

func (c *Connection) send(buffer []byte) error {
	buffer = append(buffer, '\n')
	if _, err := c.socket.Write(buffer); err != nil {
		return fmt.Errorf("error sending data to server: %v", err)
	}
	return nil
}

func (c *Connection) Close() error {
	return c.socket.Close()
}

func (c *Connection) Game() *game.Game {
	return c.game
}

func (c *Connection) ActionQueue() *ActionQueue {
	return c.actionQueue
}

func (c *Connection) DebugData() *DebugData {
	return c.debugData
}

func (c *Connection) SetTickCallback(callback func(*game.Game)) {
	c.onTickCallback = callback
}
