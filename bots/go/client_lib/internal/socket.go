package internal

import (
	"bufio"
	"context"
	"fmt"
	"net"

	"github.com/42core-team/go-client-lib/actions"
	"github.com/42core-team/go-client-lib/shared"
	"github.com/42core-team/go-client-lib/shared/schmeas"
	schema_action "github.com/42core-team/go-client-lib/shared/schmeas/actions"
)

type Connection struct {
	socket         *net.Conn
	reader         *bufio.Reader
	Game           *shared.Game
	onTickCallback func(*shared.Game)
	actionQueue    *actions.ActionQueue
	ctx            context.Context
	timeout        context.CancelFunc
}

const (
	// BufferSize defines the size of the read buffer for incoming messages
	// Should be larger than your maximum expected message size
	BufferSize = 65536 // 64KB - can handle most game server messages
)

func NewConnection(serverAddr string, selfTeamId uint) (*Connection, error) {
	conn, err := net.Dial("tcp", serverAddr)
	if err != nil {
		return nil, fmt.Errorf("error connecting to server: %v", err)
	}

	return &Connection{
		socket:         &conn,
		reader:         bufio.NewReader(conn),
		Game:           &shared.Game{MyTeamId: selfTeamId},
		onTickCallback: nil,
		actionQueue:    actions.NewActionQueue(100),
	}, nil
}

func (connection *Connection) Start(teamId uint, teamName string) error {
	defer func() {
		if err := connection.Close(); err != nil {
			fmt.Printf("Error closing connection: %v\n", err)
			return
		}
	}()

	if err := connection.sendLoginPacket(teamId, teamName); err != nil {
		return fmt.Errorf("Failed to send login packet: %v\n", err)
	}

	buffer := make([]byte, BufferSize)
	for {
		n, err := connection.reader.Read(buffer)
		if err != nil {
			panic(err)
		}
		if err := connection.handleReceive(buffer[:n]); err != nil {
			return fmt.Errorf("failed to handle the latest socket message")
		}
	}
	return nil
}

func (connection *Connection) sendLoginPacket(teamId uint, teamName string) error {
	loginPacket, err := schmeas.NewLoginRequest(teamId, teamName).Marshal()
	if err != nil {
		return fmt.Errorf("Error marshaling login request: %v\n", err)
	}

	if err := connection.Send(loginPacket); err != nil {
		return fmt.Errorf("Error sending login request: %v\n", err)
	}
	return nil
}

func (connection *Connection) SendString(message string) error {
	if _, err := (*connection.socket).Write([]byte(message)); err != nil {
		return fmt.Errorf("error sending data to server: %v", err)
	}
	return nil
}

func (connection *Connection) Send(buffer []byte) error {
	if _, err := (*connection.socket).Write(buffer); err != nil {
		return fmt.Errorf("error sending data to server: %v", err)
	}
	return nil
}

func (connection *Connection) SendActions(plannedActions []schema_action.Action) error {
	clientPacket, err := schmeas.NewClientPacket(plannedActions).Marshal()
	if err != nil {
		return fmt.Errorf("error marshaling client packet: %v", err)
	}

	if err := connection.Send(clientPacket); err != nil {
		return fmt.Errorf("error sending client packet: %v", err)
	}
	return nil
}

func (connection *Connection) handleReceive(buffer []byte) error {

	tick, err := NewGameTick(buffer)
	if err != nil {
		return fmt.Errorf("error parsing game tick: %v", err)
	}
	tick.UpdateGame(connection.Game)

	queue := connection.GetActionQueue()
	plannedActions := queue.GetAll()
	// send all the actions to the server
	if err := connection.SendActions(plannedActions); err != nil {
		return fmt.Errorf("error sending actions to server: %v", err)
	}

	if connection.onTickCallback != nil {
		connection.onTickCallback(connection.Game)
	}

	return nil
}

func (connection *Connection) Close() error {
	return (*connection.socket).Close()
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
