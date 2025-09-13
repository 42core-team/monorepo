import { getGameConfig, getGameMisc } from "./replayLoader";

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
	gems: number;
	SpawnCooldown: number;
}
export interface UnitObject extends BaseObject {
	type: 1; // Unit
	unit_type: number;
	teamId: number;
	gems: number;
	ActionCooldown: number;
}
export interface DepositObject extends BaseObject {
	type: 2; // Deposit
	gems: number;
}
export interface WallObject extends BaseObject {
	type: 3; // Wall
}
export interface GemPileObject extends BaseObject {
	type: 4; // Gems
	gems: number;
}
export interface BombObject extends BaseObject {
	type: 5; // Bomb
	countdown: number;
	explosionTiles?: { x: number; y: number }[];
}
export type TickObject =
	| CoreObject
	| UnitObject
	| DepositObject
	| WallObject
	| GemPileObject
	| BombObject;

const objectTypeNames = {
	0: "Core",
	1: "Unit",
	2: "Gem Deposit",
	3: "Wall",
	4: "Gem Pile",
	5: "Bomb",
};

export function formatObjectData(obj: TickObject): string {
	const num = (v: unknown) =>
		Number.isFinite(Number(v)) ? String(Number(v)) : "NaN"; // XSS prevention

	const lines: { line: string; priority: number; color: string }[] = [];

	lines.push({
		line: `❤️ HP: ${num(obj.hp)}`,
		priority: 0,
		color: "var(--hp-color)",
	});
	lines.push({
		line: `❓ Object Type: ${objectTypeNames[obj.type] || "Unknown"}`,
		priority: 4,
		color: "var(--text)",
	});
	lines.push({
		line: `#️⃣ ID: ${num(obj.id)}`,
		priority: 7,
		color: "var(--text)",
	});
	lines.push({
		line: `📍 Position: [x: ${num(obj.x)}, y: ${num(obj.y)}]`,
		priority: 8,
		color: "var(--text)",
	});

	switch (obj.type) {
		case 0:
			lines.push({
				line: `🏁 Team ID: ${num(obj.teamId)} (${getGameMisc()?.team_results.find((team) => team.id === obj.teamId)?.name || "Unknown Name"})`,
				priority: 6,
				color: "var(--text)",
			});
			lines.push({
				line: `💎 Gems: ${num(obj.gems)}`,
				priority: 1,
				color: "var(--gems-color)",
			});
			lines.push({
				line: `🔢 Spawn Cooldown: ${num(obj.SpawnCooldown)}`,
				priority: 2,
				color: "var(--cooldown-color)",
			});
			break;
		case 1:
			lines.push({
				line: `⚔️ Unit Type: ${getGameConfig()?.units?.[obj.unit_type]?.name || num(obj.unit_type)}`,
				priority: 5,
				color: "var(--text)",
			});
			lines.push({
				line: `🏁 Team ID: ${num(obj.teamId)} (${getGameMisc()?.team_results.find((team) => team.id === obj.teamId)?.name || "Unknown Name"})`,
				priority: 6,
				color: "var(--text)",
			});
			lines.push({
				line: `💎 Gems: ${num(obj.gems)}`,
				priority: 1,
				color: "var(--gems-color)",
			});
			lines.push({
				line: `🔢 Action Cooldown: ${num(obj.ActionCooldown)}`,
				priority: 2,
				color: "var(--cooldown-color)",
			});
			break;
		case 4:
		case 2:
			lines.push({
				line: `💎 Gems: ${num(obj.gems)}`,
				priority: 1,
				color: "var(--gems-color)",
			});
			break;
		case 5:
			lines.push({
				line: `💣 Explosion Countdown: ${num(obj.countdown)}`,
				priority: -1,
				color: "var(--text)",
			});
			break;
	}

	lines.sort((a, b) => a.priority - b.priority);

	const result: string[] = [];
	let prevPriority = -Infinity;
	for (const { line, priority, color } of lines) {
		if (prevPriority <= 2 && priority >= 3) {
			result.push(""); // add visual separator
		}
		result.push(`<span style="color: ${color}">${line}</span>`);
		prevPriority = priority;
	}

	return result.join("<br>");
}

export function getBarMetrics(
	obj: TickObject,
): { key: string; percentage: number }[] {
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
		case 2: // Deposit
			maxHP = getGameConfig()?.depositHp || -1;
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
			key: "hp",
			percentage: (obj.hp / maxHP) * 100,
		});
	}

	// gems
	if ((obj.type === 0 || obj.type === 1) && obj.gems > 0) {
		// deposits and gem piles holding gems doesnt actually contain any info
		const maxBalance = Math.max(500, obj.gems);
		metrics.push({
			key: "gems",
			percentage: (obj.gems / maxBalance) * 100,
		});
	}

	// Action Cooldown / Spawn Cooldown
	if (obj.type === 1) {
		const cfg = getGameConfig();
		if (!cfg) return metrics;
		const u = cfg.units[obj.unit_type];
		const step = Math.max(1, u.balancePerCooldownStep);
		let calc = u.baseActionCooldown + Math.floor(obj.gems / step);
		if (u.maxActionCooldown > 0 && calc > u.maxActionCooldown)
			calc = u.maxActionCooldown;
		calc = Math.max(1, calc);
		const denom = Math.max(calc, obj.ActionCooldown);
		metrics.push({
			key: "cooldown",
			percentage: (obj.ActionCooldown / denom) * 100,
		});
	} else if (obj.type === 0) {
		const cfg = getGameConfig();
		if (!cfg) return metrics;
		const total = cfg.coreSpawnCooldown;
		if (total > 0 && obj.SpawnCooldown > 0) {
			const denom = Math.max(total, obj.SpawnCooldown);
			metrics.push({
				key: "cooldown",
				percentage: (obj.SpawnCooldown / denom) * 100,
			});
		}
	}

	return metrics;
}
