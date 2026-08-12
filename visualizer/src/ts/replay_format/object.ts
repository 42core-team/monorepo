export interface BaseObject {
	id: number;
	type: number;
	x: number;
	y: number;
	hp: number;
	state?: string;
	debug_info?: string;
}

export interface CoreObject extends BaseObject {
	type: 0;
	teamId: number;
	gems: number;
	SpawnCooldown: number;
	SpawnCooldownLastResetTo: number;
}

export interface UnitObject extends BaseObject {
	type: 1;
	name: string;
	components: string[];
	properties: UnitProperties;
	teamId: number;
	gems: number;
	ActionCooldown: number;
	debug_path?: { x: number; y: number }[];
}

export interface DepositObject extends BaseObject {
	type: 2;
	gems: number;
}

export interface WallObject extends BaseObject {
	type: 3;
}

export interface GemPileObject extends BaseObject {
	type: 4;
	gems: number;
}

export type TickObject =
	| CoreObject
	| UnitObject
	| DepositObject
	| WallObject
	| GemPileObject;

export interface UnitProperties {
	hp: number;
	baseActionCooldown: number;
	gemsPerCooldownStep: number;
	maxGems: number;
	damageReductionPercent: number;
	damageCore: number;
	damageUnit: number;
	damageObject: number;
	postSpawnCoreCooldown: number;
}
