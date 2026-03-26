package internal

import (
	"bufio"
	"encoding/json"
	"fmt"
	"net"
	"time"

	"github.com/42core-team/go-client-lib/actions"
	"github.com/42core-team/go-client-lib/game"
)

type loginRequest struct {
	Id       uint   `json:"id"`
	Password string `json:"password"`
	TeamName string `json:"name"`
}

type clientPacket struct {
	Actions []actions.Action `json:"actions"`
}

type Connection struct {
	socket         net.Conn
	scanner        *bufio.Scanner
	Game           *game.Game
	onTickCallback func(*game.Game, *actions.ActionQueue)
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
		Game:           &game.Game{MyTeamId: selfTeamId},
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

	if err := connection.sendLoginPacket(teamId, teamName); err != nil {
		return fmt.Errorf("failed to send login packet: %v", err)
	}

	configLine, err := connection.readLine()
	if err != nil {
		return fmt.Errorf("failed to receive config: %v", err)
	}
	if err := json.Unmarshal([]byte(configLine), &connection.Game.Config); err != nil {
		return fmt.Errorf("failed to parse config: %v", err)
	}

	for {
		plannedActions := connection.actionQueue.GetAll()
		if err := connection.SendActions(plannedActions); err != nil {
			return fmt.Errorf("error sending actions: %v", err)
		}

		line, err := connection.readLine()
		if err != nil {
			fmt.Println("The connection was closed by the server. Bye, bye!")
			break
		}

		tick, err := NewGameTick(line)
		if err != nil {
			return fmt.Errorf("error parsing game tick: %v", err)
		}
		tick.UpdateGame(connection.Game)

		if connection.onTickCallback != nil {
			connection.onTickCallback(connection.Game, connection.actionQueue)
		}
	}

	return nil
}

func (connection *Connection) sendLoginPacket(teamId uint, teamName string) error {
	login := loginRequest{
		Id:       teamId,
		Password: "42",
		TeamName: teamName,
	}
	data, err := json.Marshal(login)
	if err != nil {
		return fmt.Errorf("error marshaling login request: %v", err)
	}
	if err := connection.Send(data); err != nil {
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

func (connection *Connection) SendActions(plannedActions []actions.Action) error {
	packet := clientPacket{Actions: plannedActions}
	data, err := json.Marshal(packet)
	if err != nil {
		return fmt.Errorf("error marshaling client packet: %v", err)
	}
	if err := connection.Send(data); err != nil {
		return fmt.Errorf("error sending client packet: %v", err)
	}
	return nil
}

func (connection *Connection) Close() error {
	return connection.socket.Close()
}

func (connection *Connection) GetGame() *game.Game {
	return connection.Game
}

func (connection *Connection) GetActionQueue() *actions.ActionQueue {
	return connection.actionQueue
}

func (connection *Connection) SetTickCallback(callback func(*game.Game, *actions.ActionQueue)) {
	connection.onTickCallback = callback
}
