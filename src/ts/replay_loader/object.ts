import { getGameConfig } from './replayLoader.js';

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
	const lines: { line: string; priority: number; color: string }[] = [];

	lines.push({ line: `❤️ HP: ${obj.hp}`, priority: 0, color: 'var(--hp-color)' });
	lines.push({ line: `❓ Object Type: ${objectTypeNames[obj.type] || 'Unknown'}`, priority: 4, color: 'black' });
	lines.push({ line: `#️⃣ ID: ${obj.id}`, priority: 5, color: 'black' });
	lines.push({ line: `📍 Position: [${obj.x}, ${obj.y}]`, priority: 6, color: 'black' });

	switch (obj.type) {
		case 0:
			lines.push({ line: `🏁 Team ID: ${obj.teamId}`, priority: 5, color: 'black' });
			lines.push({ line: `💰 Balance: ${obj.balance}`, priority: 1, color: 'var(--balance-color)' });
			break;
		case 1:
			lines.push({ line: `🏁 Team ID: ${obj.teamId}`, priority: 5, color: 'black' });
			lines.push({ line: `💰 Balance: ${obj.balance}`, priority: 1, color: 'var(--balance-color)' });
			lines.push({ line: `🔢 Move Cooldown: ${obj.moveCooldown}`, priority: 2, color: 'var(--cooldown-color)' });
			break;
		case 4:
		case 2:
			lines.push({ line: `💰 Balance: ${obj.balance}`, priority: 1, color: 'var(--balance-color)' });
			break;
		case 5:
			lines.push({ line: `💣 Explosion Countdown: ${obj.countdown}`, priority: -1, color: 'black' });
			break;
	}

	lines.sort((a, b) => a.priority - b.priority);

	const result: string[] = [];
	let prevPriority = -Infinity;
	for (const { line, priority, color } of lines) {
		if (prevPriority <= 2 && priority >= 3) {
			result.push('');
		}
		result.push(`<span style="color: ${color}">${line}</span>`);
		prevPriority = priority;
	}

	return result.join('<br>');
}

export function getBarMetrics(obj: TickObject): { key: string; percentage: number }[] {
	const metrics: { key: string; percentage: number }[] = [];

	// HP

	let maxHP: number = -1;
	switch (obj.type) {
		case 0: // Core
			maxHP = getGameConfig()?.coreHp || -1;
			break;
		case 1: // Unit
			maxHP = getGameConfig()?.units?.[obj.unit_type]?.hp || -1;
			break;
		case 2: // Resource
			maxHP = getGameConfig()?.resourceHp || -1;
			break;
		case 3: // Wall
			maxHP = getGameConfig()?.wallHp || -1;
			break;
		case 5: // Bomb
			maxHP = getGameConfig()?.bombHp || -1;
			break;
	}
	if (obj.hp < maxHP && maxHP !== -1) {
		metrics.push({
			key: 'hp',
			percentage: (obj.hp / maxHP) * 100,
		});
	}

	// Balance
	if ((obj.type === 0 || obj.type === 1) && obj.balance > 0) {
		// resources and moneys holding money doesnt actually contain any info
		let maxBalance = (getGameConfig()?.resourceIncome || 175) * 3;
		if (obj.balance > maxBalance) {
			maxBalance = obj.balance;
		}
		metrics.push({
			key: 'balance',
			percentage: (obj.balance / maxBalance) * 100,
		});
	}

	// Move Cooldown
	if (obj.type === 1 && obj.moveCooldown > 0) {
		const cfg = getGameConfig();
		if (!cfg) return metrics;
		const baseSpeed = cfg.units[obj.unit_type].baseMoveCooldown;
		const minSpeed = cfg.units[obj.unit_type].maxMoveCooldown;
		let resourcePart = obj.balance / (cfg.resourceIncome / 4);
		if (resourcePart < 1) resourcePart = 1;
		const defaultCd = Math.min(baseSpeed * resourcePart, minSpeed);

		metrics.push({
			key: 'moveCooldown',
			percentage: (obj.moveCooldown / defaultCd) * 100,
		});
	}

	return metrics;
}
