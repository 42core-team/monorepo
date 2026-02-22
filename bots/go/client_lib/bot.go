package goConn

import (
	"fmt"
	"os"
	"strconv"

	"github.com/42core-team/go-client-lib/internal"
	"github.com/42core-team/go-client-lib/shared"
	"github.com/42core-team/go-client-lib/shared/schmeas/actions"
)

// CoreGameBot represents a game bot instance that connects to the game server.
// It provides a clean API for controlling units and interacting with the game.
type CoreGameBot struct {
	conn     *internal.Connection
	teamName string
	teamId   uint
}

// NewCoreGameBot creates a new game bot with the given team name.
// It reads the team ID from the first command line argument and server
// connection details from environment variables (SERVER_IP and SERVER_PORT).
//
// Args:
//   - teamName: The name of your bot/team
//
// Returns a new CoreGameBot instance or an error if connection fails.
//
// Environment Variables:
//   - SERVER_IP: Server IP address (default: 127.0.0.1)
//   - SERVER_PORT: Server port (default: 4444)
//
// Command Line Args:
//   - args[1]: Team ID (required)
//
// Example:
//
//	bot, err := bot.NewCoreGameBot("MyBot")
//	if err != nil {
//	    panic(err)
//	}
func NewCoreGameBot(teamName string) (*CoreGameBot, error) {

	envIp, exists := os.LookupEnv("SERVER_IP")
	if !exists {
		fmt.Println("Environment variable SERVER_IP not set")
		envIp = "127.0.0.1"
	}

	envPort, exists := os.LookupEnv("SERVER_PORT")
	if !exists {
		fmt.Println("Environment variable SERVER_PORT not set")
		envPort = "4444"
	}

	if len(os.Args) < 2 {
		return nil, fmt.Errorf("team id not provided as first argument")
	}

	id := os.Args[1]
	if id == "" {
		return nil, fmt.Errorf("team id not provided as first argument")
	}
	teamId, err := strconv.Atoi(id)
	if err != nil {
		return nil, fmt.Errorf("team id not a number")
	}

	serverAddr := fmt.Sprintf("%s:%s", envIp, envPort)

	conn, err := internal.NewConnection(serverAddr, uint(teamId))
	if err != nil {
		return nil, fmt.Errorf("could not create bot: %w", err)
	}

	return &CoreGameBot{conn: conn, teamName: teamName, teamId: uint(teamId)}, nil
}

func (bot *CoreGameBot) GetGame() *shared.Game {
	return bot.conn.GetGame()
}

func (bot *CoreGameBot) CreateUnit(unitType shared.UnitType) error {
	// TODO: check if spawning is possible
	bot.conn.GetActionQueue().Add(actions.NewActionCreate(unitType))
	return nil
}

func (bot *CoreGameBot) Move(object *shared.Object, pos shared.Position) error {
	if !object.IsReadyForAction() {
		return fmt.Errorf("unit %d is on cooldown for %d more ticks", object.Id, object.ObjectData.(shared.UnitData).ActionCooldown)
	}
	bot.conn.GetActionQueue().Add(actions.NewActionMove(object.Id, pos.X, pos.Y))
	return nil
}

func (bot *CoreGameBot) SimplePathfind(object *shared.Object, pos shared.Position) {
	panic("unimplemented")
}

func (bot *CoreGameBot) Attack(object *shared.Object, target *shared.Object) error {
	if !object.IsReadyForAction() {
		return fmt.Errorf("unit %d is on cooldown for %d more ticks", object.Id, object.ObjectData.(shared.UnitData).ActionCooldown)
	}
	bot.conn.GetActionQueue().Add(actions.NewActionAttack(object.Id, target.Id))
	return nil
}

func (bot *CoreGameBot) TransferGems(source *shared.Object, targetPos shared.Position, amount uint) error {
	if !source.IsReadyForAction() {
		return fmt.Errorf("unit %d is on cooldown for %d more ticks", source.Id, source.ObjectData.(shared.UnitData).ActionCooldown)
	}

	bot.conn.GetActionQueue().Add(actions.NewActionTransferGems(source.Id, targetPos.X, targetPos.Y, amount))
	return nil
}

func (bot *CoreGameBot) Build(builder *shared.Object, pos shared.Position) error {
	if !builder.IsReadyForAction() {
		return fmt.Errorf("unit %d is on cooldown for %d more ticks", builder.Id, builder.ObjectData.(shared.UnitData).ActionCooldown)
	}
	bot.conn.GetActionQueue().Add(actions.NewActionBuild(builder.Id, pos.X, pos.Y))
	return nil
}

func (bot *CoreGameBot) Run(callback func(game *shared.Game)) error {
	bot.conn.SetTickCallback(callback)
	if err := bot.conn.Start(bot.teamId, bot.teamName); err != nil {
		return err
	}
	return nil
}
