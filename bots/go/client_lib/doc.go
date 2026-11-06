// Package core provides the CORE bot client with a C-style, process-wide API.
// StartGame manages the connection and invokes a tick callback after updating
// Game. Inside the callback, use package-level getters, actions and debug helpers.
// Game and its objects are read-only views for bot code. Except for Close and
// rejection of concurrent StartGame calls, this API is not goroutine-safe.
package core
