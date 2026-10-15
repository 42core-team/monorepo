package core

import (
	"fmt"
	"github.com/42core-team/go-client-lib/game"
)

// AddObjectInfo formats according to a format specifier, like [fmt.Sprintf],
// and appends the result to the debug text of obj for the current tick.
// The text is shown in the visualizer. Repeated calls for the same object
// concatenate their text, so callers should end each entry with a newline.
//
// AddObjectInfo does nothing if obj is nil or no game is running.
func AddObjectInfo(obj *game.Object, format string, args ...any) {
	if obj == nil || checkClient() != nil {
		return
	}
	connection.Load().DebugData().AddObjectInfo(obj.ID, fmt.Sprintf(format, args...))
}

// AddObjectPathStep appends pos to the debug path of unit for the current
// tick. The path is drawn in the visualizer. [Travel] adds its chosen route
// automatically.
//
// AddObjectPathStep does nothing if unit is nil or no game is running.
func AddObjectPathStep(unit *game.Object, pos game.Position) {
	if unit == nil || checkClient() != nil {
		return
	}
	connection.Load().DebugData().AddObjectPathStep(unit.ID, pos)
}
