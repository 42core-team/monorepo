export interface UnitConfig {
	name: string;
	cost: number;
	hp: number;
	speed: number;
	minSpeed: number;
	damageCore: number;
	damageUnit: number;
	damageResource: number;
	damageWall: number;
	buildType: number;
}
export interface GameConfig {
	width: number;
	height: number;
	tickRate: number;
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
	corePositions: { x: number; y: number }[]; // 💥 included in JSON
}
