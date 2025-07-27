export interface BaseAction {
	type: string;
}
export interface MoveAction extends BaseAction {
	type: 'move';
	targetX: number;
	targetY: number;
}
export interface TransferMoneyAction extends BaseAction {
	type: 'transfer_money';
	source_id: number;
	x: number;
	y: number;
	amount: number;
}
export interface BuildAction extends BaseAction {
	type: 'build';
	builder_id: number;
	x: number;
	y: number;
}
export interface CreateAction extends BaseAction {
	type: 'create';
	unit_type: number;
}
export interface AttackAction extends BaseAction {
	type: 'attack';
	unit_id: number;
	target_pos_x: number;
	target_pos_y: number;
	damage: number;
}
export type TickAction = MoveAction | TransferMoneyAction | BuildAction | CreateAction | AttackAction;
