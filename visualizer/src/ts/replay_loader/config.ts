export interface UnitConfig {
	name: string;
	visualizer_asset_path: string;
	cost: number;
	hp: number;
	baseActionCooldown: number;
	maxActionCooldown: number;
	balancePerCooldownStep: number;
	damageCore: number;
	damageUnit: number;
	damageDeposit: number;
	damageWall: number;
	buildType: number;
}
export interface GameConfig {
	gridSize: number;
	idleIncome: number;
	idleIncomeTimeOut: number;
	depositHp: number;
	depositIncome: number;
	gemPileIncome: number;
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
