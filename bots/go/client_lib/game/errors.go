package game

import "errors"

var (
	ErrNilObject         = errors.New("object is nil")
	ErrWrongObjectType   = errors.New("wrong object type")
	ErrInvalidObjectData = errors.New("invalid object data")
)
