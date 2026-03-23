package internal

import (
	"bufio"
	"encoding/json"
	"fmt"
	"net"
	"time"

	"github.com/42core-team/go-client-lib/actions"
	"github.com/42core-team/go-client-lib/shared"
	"github.com/42core-team/go-client-lib/shared/schemas"
	schema_action "github.com/42core-team/go-client-lib/shared/schemas/actions"
)

type Connection struct {
	socket         net.Conn
	scanner        *bufio.Scanner
	Game           *shared.Game
	onTickCallback func(*shared.Game)
	actionQueue    *actions.ActionQueue
}

func NewConnection(serverAddr string, selfTeamId uint) (*Connection, error) {
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
		socket:         conn,
		scanner:        scanner,
		Game:           &shared.Game{MyTeamId: selfTeamId},
		onTickCallback: nil,
		actionQueue:    actions.NewActionQueue(100),
	}, nil
}

func (connection *Connection) readLine() (string, error) {
	if connection.scanner.Scan() {
		return connection.scanner.Text(), nil
	}
	if err := connection.scanner.Err(); err != nil {
		return "", fmt.Errorf("error reading from socket: %v", err)
	}
	return "", fmt.Errorf("connection closed by server")
}

func (connection *Connection) Start(teamId uint, teamName string) error {
	defer func() {
		if err := connection.Close(); err != nil {
			fmt.Printf("Error closing connection: %v\n", err)
		}
	}()

	// Send login
	if err := connection.sendLoginPacket(teamId, teamName); err != nil {
		return fmt.Errorf("failed to send login packet: %v", err)
	}

	// Receive config
	configLine, err := connection.readLine()
	if err != nil {
		return fmt.Errorf("failed to receive config: %v", err)
	}
	if err := json.Unmarshal([]byte(configLine), &connection.Game.Config); err != nil {
		return fmt.Errorf("failed to parse config: %v", err)
	}
	fmt.Println("Config received")

	// Game loop: send actions -> receive state -> callback
	for {
		// Send queued actions
		plannedActions := connection.actionQueue.GetAll()
		if err := connection.SendActions(plannedActions); err != nil {
			return fmt.Errorf("error sending actions: %v", err)
		}

		// Receive game state
		line, err := connection.readLine()
		if err != nil {
			fmt.Println("The connection was closed by the server. Bye, bye!")
			break
		}

		tick, err := NewGameTick([]byte(line))
		if err != nil {
			return fmt.Errorf("error parsing game tick: %v", err)
		}
		tick.UpdateGame(connection.Game)

		// Execute user callback
		if connection.onTickCallback != nil {
			connection.onTickCallback(connection.Game)
		}
	}

	return nil
}

func (connection *Connection) sendLoginPacket(teamId uint, teamName string) error {
	loginPacket, err := schemas.NewLoginRequest(teamId, teamName).Marshal()
	if err != nil {
		return fmt.Errorf("error marshaling login request: %v", err)
	}

	if err := connection.Send(loginPacket); err != nil {
		return fmt.Errorf("error sending login request: %v", err)
	}
	return nil
}

func (connection *Connection) Send(buffer []byte) error {
	buffer = append(buffer, '\n')
	if _, err := connection.socket.Write(buffer); err != nil {
		return fmt.Errorf("error sending data to server: %v", err)
	}
	return nil
}

func (connection *Connection) SendActions(plannedActions []schema_action.Action) error {
	clientPacket, err := schemas.NewClientPacket(plannedActions).Marshal()
	if err != nil {
		return fmt.Errorf("error marshaling client packet: %v", err)
	}

	if err := connection.Send(clientPacket); err != nil {
		return fmt.Errorf("error sending client packet: %v", err)
	}
	return nil
}

func (connection *Connection) Close() error {
	return connection.socket.Close()
}

func (connection *Connection) GetGame() *shared.Game {
	return connection.Game
}

func (connection *Connection) GetActionQueue() *actions.ActionQueue {
	return connection.actionQueue
}

func (connection *Connection) SetTickCallback(callback func(*shared.Game)) {
	connection.onTickCallback = callback
}
