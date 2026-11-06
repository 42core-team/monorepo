package core

import (
	"errors"
	"fmt"

	"github.com/42core-team/go-client-lib/game"
)

// Errors returned by the actions and [Close]. They are usually wrapped with
// context and can be tested for using [errors.Is]. Actions also return
// [game.ErrNilObject] and [game.ErrWrongObjectType] for invalid objects.
var (
	ErrClientUnavailable = errors.New("client is not connected or is closed") // no game is running
	ErrInvalidPosition   = errors.New("position is outside the board")        // see IsPosValid
)

func checkClient() error {
	conn := connection.Load()
	if conn == nil || conn.IsClosed() || Game == nil {
		return ErrClientUnavailable
	}
	return nil
}

func checkUnit(unit *game.Object) error {
	if unit == nil {
		return game.ErrNilObject
	}
	if unit.Type != game.ObjectUnit {
		return fmt.Errorf("object %d is not a unit: %w", unit.ID, game.ErrWrongObjectType)
	}
	return nil
}

func checkPosition(pos game.Position) error {
	if !Game.IsPosValid(pos) {
		return fmt.Errorf("position (%d, %d): %w", pos.X, pos.Y, ErrInvalidPosition)
	}
	return nil
}
