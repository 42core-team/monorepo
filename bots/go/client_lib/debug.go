package core

import (
	"fmt"
	"github.com/42core-team/go-client-lib/game"
)

func AddObjectInfo(obj *game.Object, format string, args ...any) {
	if obj == nil || checkClient() != nil {
		return
	}
	connection.Load().DebugData().AddObjectInfo(obj.ID, fmt.Sprintf(format, args...))
}

func AddObjectPathStep(unit *game.Object, pos game.Position) {
	if unit == nil || checkClient() != nil {
		return
	}
	connection.Load().DebugData().AddObjectPathStep(unit.ID, pos)
}
