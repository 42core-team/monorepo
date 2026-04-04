package coregame

import (
	"github.com/42core-team/go-client-lib/game"
)

func (b *Bot) AddObjectInfo(obj *game.Object, info string) {
	if obj == nil {
		return
	}
	b.connection.DebugData().AddObjectInfo(obj.ID, info)
}

func (b *Bot) AddObjectPathStep(unit *game.Object, pos game.Position) {
	if unit == nil {
		return
	}
	b.connection.DebugData().AddObjectPathStep(unit.ID, pos)
}
