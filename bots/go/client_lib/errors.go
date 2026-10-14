package coregame

import (
	"errors"
	"fmt"

	"github.com/42core-team/go-client-lib/game"
)

var (
	ErrClientUnavailable = errors.New("client is not connected or is closed")
	ErrInvalidPosition   = errors.New("position is outside the board")
)

func (b *Bot) checkClient() error {
	if b == nil || b.connection == nil || b.connection.IsClosed() || b.game == nil {
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

func (b *Bot) checkPosition(pos game.Position) error {
	if !b.game.IsPosValid(pos) {
		return fmt.Errorf("position (%d, %d): %w", pos.X, pos.Y, ErrInvalidPosition)
	}
	return nil
}
