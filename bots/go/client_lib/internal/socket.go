package internal

import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net"
	"strings"
	"sync/atomic"
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
	closed         atomic.Bool
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
		return "", fmt.Errorf("set socket read deadline: %w", err)
	}
	line, err := c.reader.ReadString('\n')
	if err != nil {
		return "", fmt.Errorf("read from socket: %w", err)
	}
	return strings.TrimSuffix(line, "\n"), nil
}

func (c *Connection) Start(teamID uint, teamName string) (runErr error) {
	defer func() {
		if err := c.Close(); err != nil {
			runErr = errors.Join(runErr, fmt.Errorf("close connection: %w", err))
		}
	}()

	if err := c.sendLoginPacket(teamID, teamName); err != nil {
		return fmt.Errorf("send login packet: %w", err)
	}

	configLine, err := c.readLine()
	if err != nil {
		return fmt.Errorf("receive config: %w", err)
	}
	if err := json.Unmarshal([]byte(configLine), &c.game.Config); err != nil {
		return fmt.Errorf("parse config: %w", err)
	}
	if c.debug {
		fmt.Printf("Received config: %s\n", configLine)
	}

	for {
		actions := c.actionQueue.Drain()
		debugEntries := make([]DebugEntry, 0)
		if c.debugData.HasData() {
			debugEntries = c.debugData.Entries()
		}
		c.debugData.Reset()

		if err := c.sendPacket(actions, debugEntries); err != nil {
			return fmt.Errorf("send packet: %w", err)
		}

		line, err := c.readLine()
		if err != nil {
			return fmt.Errorf("receive game tick: %w", err)
		}
		if c.debug {
			fmt.Printf("Received state: %s\n", line)
		}

		tick, err := parseGameTick(line)
		if err != nil {
			return fmt.Errorf("parse game tick: %w", err)
		}
		if err := tick.applyTo(c.game); err != nil {
			return err
		}

		if c.onTickCallback != nil {
			c.onTickCallback(c.game)
		}
		if core := c.ownCore(); core == nil || !core.IsAlive() || !c.hasLivingOpponentCore() {
			break
		}
	}
	c.printGameResult()

	return nil
}

func (c *Connection) printGameResult() {
	myCore := c.ownCore()
	if myCore != nil && myCore.IsAlive() && !c.hasLivingOpponentCore() {
		fmt.Println("Game over! You won!")
	} else {
		fmt.Println("Game over! You lost!")
	}
}

func (c *Connection) ownCore() *game.Object {
	return c.game.NearestObject(game.Position{}, func(object *game.Object) bool {
		data, err := object.CoreData()
		return err == nil && data.TeamID == c.game.MyTeamID
	})
}

func (c *Connection) hasLivingOpponentCore() bool {
	return c.game.ObjectsFilterCount(func(object *game.Object) bool {
		if object.Type != game.ObjectCore {
			return false
		}
		data, err := object.CoreData()
		return err == nil && data.TeamID != c.game.MyTeamID && object.IsAlive()
	}) > 0
}

func (c *Connection) sendLoginPacket(teamID uint, teamName string) error {
	login := loginRequest{ID: teamID, Password: "42", TeamName: teamName}
	data, err := json.Marshal(login)
	if err != nil {
		return fmt.Errorf("marshal login request: %w", err)
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
		return fmt.Errorf("marshal client packet: %w", err)
	}
	if c.debug {
		fmt.Printf("Actions: %s\n", data)
	}
	return c.send(data)
}

func (c *Connection) send(buffer []byte) error {
	buffer = append(buffer, '\n')
	if n, err := c.socket.Write(buffer); err != nil {
		return fmt.Errorf("send data to server: %w", err)
	} else if n != len(buffer) {
		return fmt.Errorf("send data to server: %w", io.ErrShortWrite)
	}
	return nil
}

func (c *Connection) Close() error {
	if c.closed.Swap(true) || c.socket == nil {
		return nil
	}
	return c.socket.Close()
}

func (c *Connection) IsClosed() bool { return c.closed.Load() || c.socket == nil }

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
