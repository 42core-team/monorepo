package internal

import (
	"encoding/json"

	"github.com/42core-team/go-client-lib/game"
)

// A DebugPathNode is one step of a debug path, stored as a singly linked list.
type DebugPathNode struct {
	Pos  game.Position
	Next *DebugPathNode
}

// A DebugEntry holds the debug text and path attached to one object.
type DebugEntry struct {
	ObjectID uint           `json:"object_id"`
	Info     string         `json:"object_info"`
	Path     *DebugPathNode `json:"-"`
}

type debugPathPos struct {
	X uint `json:"x"`
	Y uint `json:"y"`
}

// MarshalJSON implements [json.Marshaler]. It encodes Path as an array of
// positions and omits it if it is empty.
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

// DebugData collects the debug entries created during one tick, with at
// most one entry per object.
type DebugData struct {
	entries  []DebugEntry
	Capacity int // number of entries preallocated after each Reset
}

// NewDebugData returns an empty DebugData with room for capacity entries.
func NewDebugData(capacity int) *DebugData {
	return &DebugData{
		entries:  make([]DebugEntry, 0, capacity),
		Capacity: capacity,
	}
}

func (d *DebugData) findOrCreateEntry(objectID uint) *DebugEntry {
	for i := range d.entries {
		if d.entries[i].ObjectID == objectID {
			return &d.entries[i]
		}
	}

	d.entries = append(d.entries, DebugEntry{
		ObjectID: objectID,
		Info:     "",
		Path:     nil,
	})
	return &d.entries[len(d.entries)-1]
}

// AddObjectInfo appends info to the debug text of the object with the given ID.
func (d *DebugData) AddObjectInfo(objectID uint, info string) {
	entry := d.findOrCreateEntry(objectID)
	if entry.Info == "" {
		entry.Info = info
	} else {
		entry.Info += info
	}
}

// AddObjectPathStep appends pos to the debug path of the object with the
// given ID.
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

// Reset discards all entries.
func (d *DebugData) Reset() {
	d.entries = make([]DebugEntry, 0, d.Capacity)
}

// HasData reports whether any entry has been added since the last Reset.
func (d *DebugData) HasData() bool {
	return len(d.entries) > 0
}

// Entries returns the collected entries. The slice is owned by d and is
// valid until the next call to Reset.
func (d *DebugData) Entries() []DebugEntry {
	return d.entries
}
