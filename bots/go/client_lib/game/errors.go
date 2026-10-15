package game

import "errors"

// Errors returned by the [Object] data accessors. They are usually wrapped
// with context and can be tested for using [errors.Is].
var (
	ErrNilObject         = errors.New("object is nil")
	ErrWrongObjectType   = errors.New("wrong object type")   // object is of a type without the requested data
	ErrInvalidObjectData = errors.New("invalid object data") // object data is missing or does not match its type
)
