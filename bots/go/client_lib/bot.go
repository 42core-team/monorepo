package core

import (
	"errors"
	"fmt"
	"os"
	"sync/atomic"

	"github.com/42core-team/go-client-lib/game"
	"github.com/42core-team/go-client-lib/internal"
)

// A BotConfig describes how a bot connects to the server.
type BotConfig struct {
	TeamID     uint   // team ID assigned by the server operator
	TeamName   string // name shown in the visualizer
	ServerAddr string // server address in "host:port" form
	Debug      bool   // log raw configuration, state and action packets
}

// Game is the current game state. [StartGame] replaces it with a fresh state
// and updates it before each call to the tick callback.
// Bots must treat Game and the objects it contains as read-only, and must
// only access it from the tick callback.
var Game = &game.Game{}

var running atomic.Bool
var connection atomic.Pointer[internal.Connection]

// ErrGameAlreadyRunning is returned by [StartGame] if a game is already
// running in this process.
var ErrGameAlreadyRunning = errors.New("a game is already running")

// DefaultBotConfig returns a [BotConfig] for the given team.
// The server address is read from the SERVER_IP and SERVER_PORT environment
// variables, which default to 127.0.0.1 and 4444. Debug output is disabled.
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

// StartGame connects to the server described by config and plays one game.
// Until the server accepts the connection, StartGame retries every few
// hundred milliseconds. It then logs in, receives the game configuration
// and calls tick once per game tick with the number of elapsed ticks,
// after [Game] has been updated. Actions and debug data queued during
// tick are sent to the server when tick returns.
//
// StartGame blocks until the game ends, the connection fails or [Close] is
// called, and closes the connection before it returns. It returns nil once
// the own core or all opposing cores have been destroyed.
//
// Only one game may run per process at a time; StartGame returns
// [ErrGameAlreadyRunning] if another call is still in progress.
// A later call starts with a fresh [Game].
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

// Close closes the connection of the running game, which causes [StartGame]
// to return with an error. It returns [ErrClientUnavailable] if no game is
// running. Close is safe to call from any goroutine.
func Close() error {
	conn := connection.Load()
	if conn == nil {
		return ErrClientUnavailable
	}
	return conn.Close()
}
