package coregame

import (
	"fmt"
	"os"

	"github.com/42core-team/go-client-lib/game"
	"github.com/42core-team/go-client-lib/internal"
)

type BotConfig struct {
	TeamID     uint
	TeamName   string
	ServerAddr string
	Debug      bool
}

type Bot struct {
	config     BotConfig
	connection *internal.Connection
	game       *game.Game
}

func DefaultBotConfig(teamID int, teamName string) BotConfig {
	envIP := os.Getenv("SERVER_IP")
	envPort := os.Getenv("SERVER_PORT")
	port := "4444"
	if envPort != "" {
		port = envPort
	}

	serverAddr := "127.0.0.1:" + port
	if envIP != "" {
		serverAddr = envIP + ":" + port
	}

	return BotConfig{
		TeamID:     uint(teamID),
		TeamName:   teamName,
		ServerAddr: serverAddr,
		Debug:      false,
	}
}

func NewBot(config BotConfig) (*Bot, error) {
	conn, err := internal.NewConnection(config.ServerAddr, config.TeamID)
	if err != nil {
		return nil, fmt.Errorf("failed to create connection: %v", err)
	}

	return &Bot{
		config:     config,
		connection: conn,
		game:       conn.Game(),
	}, nil
}

func (b *Bot) Run(tick func(*game.Game, *Bot)) error {
	b.connection.SetTickCallback(func(g *game.Game) {
		tick(g, b)
	})
	return b.connection.Start(b.config.TeamID, b.config.TeamName)
}

func (b *Bot) Close() error {
	return b.connection.Close()
}

func (b *Bot) Game() *game.Game {
	return b.game
}
