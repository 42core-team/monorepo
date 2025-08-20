export interface UnitConfig {
	name: string;
	cost: number;
	hp: number;
	baseMoveCooldown: number;
	maxMoveCooldown: number;
	balancePerCooldownStep: number;
	damageCore: number;
	damageUnit: number;
	damageResource: number;
	damageWall: number;
	buildType: number;
}
export interface GameConfig {
	gridSize: number;
	idleIncome: number;
	idleIncomeTimeOut: number;
	resourceHp: number;
	resourceIncome: number;
	moneyObjIncome: number;
	coreHp: number;
	initialBalance: number;
	wallHp: number;
	wallBuildCost: number;
	bombHp: number;
	bombCountdown: number;
	bombThrowCost: number;
	bombReach: number;
	bombDamage: number;
	units: UnitConfig[];
	corePositions: { x: number; y: number }[];
}
