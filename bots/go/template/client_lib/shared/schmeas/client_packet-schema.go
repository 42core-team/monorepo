package schmeas

import (
	"encoding/json"

	"github.com/42core-team/go-client-lib/shared/schmeas/actions"
)

type ClientPacket struct {
	Actions []actions.Action `json:"actions"`
	// DebugData string           `json:"debugData,omitempty"`
}

func NewClientPacket(actions []actions.Action) ClientPacket {
	return ClientPacket{
		Actions: actions,
	}
}

func (cp ClientPacket) Marshal() ([]byte, error) {
	return json.Marshal(cp)
}
