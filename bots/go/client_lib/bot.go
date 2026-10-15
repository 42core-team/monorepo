package core

import (
	"errors"
	"fmt"
	"os"
	"sync/atomic"

	"github.com/42core-team/go-client-lib/game"
	"github.com/42core-team/go-client-lib/internal"
)

type BotConfig struct {
	TeamID     uint
	TeamName   string
	ServerAddr string
	Debug      bool
}

// Game is the current game state, updated before each tick callback.
// Treat it as read-only. Access state, getters, actions and debug helpers only
// from the callback; they are not safe for concurrent use from goroutines.
var Game = &game.Game{}

var running atomic.Bool
var connection atomic.Pointer[internal.Connection]

var ErrGameAlreadyRunning = errors.New("a game is already running")

func DefaultBotConfig(teamID int, teamName string) BotConfig {
	port := os.Getenv("SERVER_PORT")
	if port == "" {
		port = "4444"
	}
	host := os.Getenv("SERVER_IP")
	if host == "" {
		host = "127.0.0.1"
	}
	return BotConfig{TeamID: uint(teamID), TeamName: teamName, ServerAddr: host + ":" + port}
}

// StartGame connects, runs tick after each state update, and closes the connection
// on return. Only one game may run per process. A later call starts fresh state.
func StartGame(config BotConfig, tick func(uint)) error {
	if tick == nil {
		return fmt.Errorf("tick callback must not be nil")
	}
	if !running.CompareAndSwap(false, true) {
		return ErrGameAlreadyRunning
	}
	defer running.Store(false)
	conn, err := internal.NewConnection(config.ServerAddr, config.TeamID, config.Debug)
	if err != nil {
		return fmt.Errorf("create connection: %w", err)
	}
	Game = conn.Game()
	connection.Store(conn)
	defer connection.Store(nil)
	conn.SetTickCallback(func(g *game.Game) { tick(g.ElapsedTicks) })
	return conn.Start(config.TeamID, config.TeamName)
}

// Close stops the current connection. It may be called from another goroutine.
func Close() error {
	conn := connection.Load()
	if conn == nil {
		return ErrClientUnavailable
	}
	return conn.Close()
}
