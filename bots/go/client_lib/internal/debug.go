package internal

import (
	"encoding/json"

	"github.com/42core-team/go-client-lib/game"
)

type DebugPathNode struct {
	Pos  game.Position
	Next *DebugPathNode
}

type DebugEntry struct {
	ObjectID uint           `json:"object_id"`
	Info     string         `json:"object_info"`
	Path     *DebugPathNode `json:"-"`
}

type debugPathPos struct {
	X uint `json:"x"`
	Y uint `json:"y"`
}

func (e DebugEntry) MarshalJSON() ([]byte, error) {
	type debugEntryJSON struct {
		ObjectID uint           `json:"object_id"`
		Info     string         `json:"object_info"`
		Path     []debugPathPos `json:"object_path,omitempty"`
	}

	entry := debugEntryJSON{
		ObjectID: e.ObjectID,
		Info:     e.Info,
	}

	if e.Path != nil {
		var path []debugPathPos
		current := e.Path
		for current != nil {
			path = append(path, debugPathPos{X: current.Pos.X, Y: current.Pos.Y})
			current = current.Next
		}
		entry.Path = path
	}

	return json.Marshal(entry)
}

type DebugData struct {
	Entries  []DebugEntry
	Capacity int
}

func NewDebugData(capacity int) *DebugData {
	return &DebugData{
		Entries:  make([]DebugEntry, 0, capacity),
		Capacity: capacity,
	}
}

func (d *DebugData) findOrCreateEntry(objectID uint) *DebugEntry {
	for i := range d.Entries {
		if d.Entries[i].ObjectID == objectID {
			return &d.Entries[i]
		}
	}

	d.Entries = append(d.Entries, DebugEntry{
		ObjectID: objectID,
		Info:     "",
		Path:     nil,
	})
	return &d.Entries[len(d.Entries)-1]
}

func (d *DebugData) AddObjectInfo(objectID uint, info string) {
	entry := d.findOrCreateEntry(objectID)
	if entry.Info == "" {
		entry.Info = info
	} else {
		entry.Info += info
	}
}

func (d *DebugData) AddObjectPathStep(objectID uint, pos game.Position) {
	entry := d.findOrCreateEntry(objectID)

	newNode := &DebugPathNode{
		Pos:  pos,
		Next: nil,
	}

	if entry.Path == nil {
		entry.Path = newNode
	} else {
		current := entry.Path
		for current.Next != nil {
			current = current.Next
		}
		current.Next = newNode
	}
}

func (d *DebugData) Reset() {
	d.Entries = make([]DebugEntry, 0, d.Capacity)
}

func (d *DebugData) HasData() bool {
	return len(d.Entries) > 0
}

func (d *DebugData) GetEntries() []DebugEntry {
	return d.Entries
}
