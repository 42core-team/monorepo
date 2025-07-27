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

const objectTypeNames = {
	0: 'Core',
	1: 'Unit',
	2: 'Resource',
	3: 'Wall',
	4: 'Money',
	5: 'Bomb',
};

export function formatObjectData(obj: TickObject): string {
	const lines: { line: string; priority: number }[] = [];

	lines.push({ line: `❤️ HP: ${obj.hp}`, priority: 0 });
	lines.push({ line: `❓ Object Type: ${objectTypeNames[obj.type] || 'Unknown'}`, priority: 4 });
	lines.push({ line: `#️⃣ ID: ${obj.id}`, priority: 5 });
	lines.push({ line: `📍 Position: [${obj.x}, ${obj.y}]`, priority: 6 });

	switch (obj.type) {
		case 0:
			lines.push({ line: `🏁 Team ID: ${obj.teamId}`, priority: 5 });
			lines.push({ line: `💰 Balance: ${obj.balance}`, priority: 1 });
			break;
		case 1:
			lines.push({ line: `🏁 Team ID: ${obj.teamId}`, priority: 5 });
			lines.push({ line: `💰 Balance: ${obj.balance}`, priority: 1 });
			lines.push({ line: `🔢 Move Cooldown: ${obj.moveCooldown}`, priority: 2 });
			break;
		case 4:
		case 2:
			lines.push({ line: `💰 Balance: ${obj.balance}`, priority: 1 });
			break;
		case 5:
			lines.push({ line: `💣 Explosion Countdown: ${obj.countdown}`, priority: -1 });
			break;
	}

	lines.sort((a, b) => a.priority - b.priority);
	return lines.map((line) => line.line).join('<br>');
}
