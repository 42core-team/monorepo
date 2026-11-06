// Package game defines the CORE game state as seen by a bot.
//
// A [Game] holds the configuration sent by the server at the start of a
// match and the [Object] values currently on the board. Objects carry
// type-specific data that is accessed through [Object.UnitData],
// [Object.CoreData] and [Object.DepositData].
//
// Bots normally do not create these values themselves. The core package
// keeps a Game up to date and exposes it as core.Game.
package game
