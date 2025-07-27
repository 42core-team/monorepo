export interface BaseObject {
	id: number;
	type: number;
	x: number;
	y: number;
	hp: number;
	state?: string;
}
export interface CoreObject extends BaseObject {
	type: 0; // Core
	teamId: number;
	balance: number;
}
export interface UnitObject extends BaseObject {
	type: 1; // Unit
	unit_type: number;
	teamId: number;
	balance: number;
	moveCooldown: number;
}
export interface ResourceObject extends BaseObject {
	type: 2; // Resource
	balance: number;
}
export interface WallObject extends BaseObject {
	type: 3; // Wall
}
export interface MoneyObject extends BaseObject {
	type: 4; // Money
	balance: number;
}
export interface BombObject extends BaseObject {
	type: 5; // Bomb
	countdown: number;
}
export type TickObject = CoreObject | UnitObject | ResourceObject | WallObject | MoneyObject | BombObject;
