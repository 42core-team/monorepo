package game

import "errors"

var (
	ErrNilObject         = errors.New("object is nil")
	ErrWrongObjectType   = errors.New("wrong object type")
	ErrInvalidObjectData = errors.New("invalid object data")
)

// ActionRejection is a server-reported rejection received with a tick.
// The protocol supplies only a message, not a reliable action ID or error code.
type ActionRejection struct {
	Tick    uint
	Message string
}

func (e ActionRejection) Error() string { return e.Message }
