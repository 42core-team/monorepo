package internal

import (
	"bufio"
	"encoding/json"
	"fmt"
	"net"
	"time"

	"github.com/42core-team/go-client-lib/game"
)

type loginRequest struct {
	ID       uint   `json:"id"`
	Password string `json:"password"`
	TeamName string `json:"name"`
}

type clientPacket struct {
	Actions []Action `json:"actions"`
}

// Connection manages the TCP connection to the game server.
type Connection struct {
	socket         net.Conn
	scanner        *bufio.Scanner
	game           *game.Game
	onTickCallback func(*game.Game, *ActionQueue)
	actionQueue    *ActionQueue
}

// NewConnection dials the server, retrying until it is reachable.
func NewConnection(serverAddr string, selfTeamID uint) (*Connection, error) {
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

	scanner := bufio.NewScanner(conn)
	scanner.Buffer(make([]byte, 1024*1024), 1024*1024)

	return &Connection{
		socket:      conn,
		scanner:     scanner,
		game:        &game.Game{MyTeamID: selfTeamID},
		actionQueue: newActionQueue(100),
	}, nil
}

func (c *Connection) readLine() (string, error) {
	if c.scanner.Scan() {
		return c.scanner.Text(), nil
	}
	if err := c.scanner.Err(); err != nil {
		return "", fmt.Errorf("error reading from socket: %v", err)
	}
	return "", fmt.Errorf("connection closed by server")
}

// Start sends the login packet, receives the config, then runs the game loop
// until the server closes the connection.
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

	for {
		if err := c.sendActions(c.actionQueue.drain()); err != nil {
			return fmt.Errorf("error sending actions: %v", err)
		}

		line, err := c.readLine()
		if err != nil {
			fmt.Println("The connection was closed by the server. Bye, bye!")
			break
		}

		tick, err := parseGameTick(line)
		if err != nil {
			return fmt.Errorf("error parsing game tick: %v", err)
		}
		tick.applyTo(c.game)

		if c.onTickCallback != nil {
			c.onTickCallback(c.game, c.actionQueue)
		}
	}

	return nil
}

func (c *Connection) sendLoginPacket(teamID uint, teamName string) error {
	login := loginRequest{ID: teamID, Password: "42", TeamName: teamName}
	data, err := json.Marshal(login)
	if err != nil {
		return fmt.Errorf("error marshaling login request: %v", err)
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

func (c *Connection) sendActions(planned []Action) error {
	packet := clientPacket{Actions: planned}
	data, err := json.Marshal(packet)
	if err != nil {
		return fmt.Errorf("error marshaling client packet: %v", err)
	}
	return c.send(data)
}

// Close shuts down the TCP connection.
func (c *Connection) Close() error {
	return c.socket.Close()
}

// Game returns the current game state.
func (c *Connection) Game() *game.Game {
	return c.game
}

// ActionQueue returns the bot's action queue.
func (c *Connection) ActionQueue() *ActionQueue {
	return c.actionQueue
}

// SetTickCallback registers the function called after each game tick.
func (c *Connection) SetTickCallback(callback func(*game.Game, *ActionQueue)) {
	c.onTickCallback = callback
}
