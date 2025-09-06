export interface BaseAction {
	type: string;
}
export interface MoveAction extends BaseAction {
	type: "move";
	unit_id: number;
	x: number;
	y: number;
}
export interface TransferGemsAction extends BaseAction {
	type: "transfer_gems";
	source_id: number;
	x: number;
	y: number;
	amount: number;
}
export interface BuildAction extends BaseAction {
	type: "build";
	unit_id: number;
	x: number;
	y: number;
}
export interface CreateAction extends BaseAction {
	type: "create";
	unit_type: number;
}
export interface AttackAction extends BaseAction {
	type: "attack";
	unit_id: number;
	x: number;
	y: number;
	dmg: number;
}
export type TickAction =
	| MoveAction
	| TransferGemsAction
	| BuildAction
	| CreateAction
	| AttackAction;
