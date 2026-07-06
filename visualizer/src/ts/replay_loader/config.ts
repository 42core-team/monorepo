import type { UnitProperties } from "./object";

export interface ComponentConfigProperty {
	name: string;
	modification: number;
}
export interface ComponentConfig {
	id: string;
	properties: ComponentConfigProperty[];
	cost: number;
	visualizer_asset_prioritized: boolean;
	visualizer_asset_path: string;
}
export interface ComponentsConfig {
	maxComponentsPerUnit: number;
	unitDefaultProperties: UnitProperties;
	components: ComponentConfig[];
}
export interface GameConfig {
	gridSize: number;
	seed: number;
	idleIncome: number;
	idleIncomeTimeOut: number;
	maxUnitStandingCooldown: number;
	depositHp: number;
	depositIncome: number;
	gemPileIncome: number;
	coreHp: number;
	initialBalance: number;
	wallHp: number;
	components: ComponentsConfig;
	corePositions: { x: number; y: number }[];
}
