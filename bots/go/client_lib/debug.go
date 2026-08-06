package coregame

import (
	"fmt"
	"github.com/42core-team/go-client-lib/game"
)

func (b *Bot) AddObjectInfo(obj *game.Object, format string, args ...any) {
	if obj == nil {
		return
	}
	b.connection.DebugData().AddObjectInfo(obj.ID, fmt.Sprintf(format, args...))
}

func (b *Bot) AddObjectPathStep(unit *game.Object, pos game.Position) {
	if unit == nil {
		return
	}
	b.connection.DebugData().AddObjectPathStep(unit.ID, pos)
}
