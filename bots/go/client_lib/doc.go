// Package core implements a client for writing CORE game bots.
//
// The API mirrors the C client: it is a set of package-level functions that
// operate on a single, process-wide game. [StartGame] connects to the server
// and calls a tick callback once per game tick, after [Game] has been updated
// with the latest state. Inside the callback, bots read the state through
// [Game] and the getters, and queue actions and debug output, which are sent
// to the server once the callback returns.
//
// A minimal bot looks like this:
//
//	func tick(elapsedTicks uint) {
//		if err := core.CreateUnit("Warrior", "combat"); err != nil {
//			core.Log("create unit: %v", err)
//		}
//	}
//
//	func main() {
//		config := core.DefaultBotConfig(teamID, "My CORE Bot")
//		if err := core.StartGame(config, tick); err != nil {
//			log.Fatal(err)
//		}
//	}
//
// # Getters
//
// Getters such as [ObjectByID], [ObjectAtPos], [ObjectsFilter] and
// [NearestObject] look up objects in [Game]. They are shorthands for the
// methods of the same name on [game.Game]. The returned objects are views
// into the current state and must not be modified.
//
// # Actions
//
// Actions such as [CreateUnit], [Move], [Attack], [TransferGems] and
// [Travel] are queued locally and sent to the server at the end of the tick.
// The client only rejects calls that are malformed, such as a nil unit or a
// position outside the board. All gameplay rules, including cooldowns,
// ownership, range and costs, are checked by the server, so a nil error does
// not mean that an action will succeed. Errors reported by the server are
// printed to standard output on the next tick.
//
// # Debugging
//
// [AddObjectInfo] and [AddObjectPathStep] attach text and paths to objects.
// They are sent alongside the actions of the current tick and shown in the
// visualizer. The print helpers, such as [PrintField] and [PrintObject],
// write a human-readable representation of the state to standard output.
//
// # Concurrency
//
// The API is not safe for concurrent use. [Game], the getters, the actions
// and the debug helpers must only be used from the goroutine that runs the
// tick callback. The exceptions are [Close], which may be called from any
// goroutine, and [StartGame], which rejects concurrent calls with
// [ErrGameAlreadyRunning].
package core
