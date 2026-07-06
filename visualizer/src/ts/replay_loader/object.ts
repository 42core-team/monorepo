import { getGameConfig, getGameMisc } from "./replayLoader";

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
	type: 0; // Core
	teamId: number;
	gems: number;
	SpawnCooldown: number;
	SpawnCooldownLastResetTo: number;
}
export interface UnitObject extends BaseObject {
	type: 1; // Unit
	name: string;
	components: string[];
	properties: UnitProperties;
	teamId: number;
	gems: number;
	ActionCooldown: number;
	debug_path?: { x: number; y: number }[];
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
export type TickObject =
	| CoreObject
	| UnitObject
	| DepositObject
	| WallObject
	| GemPileObject;

export interface UnitProperties {
	hp: number;
	baseActionCooldown: number;
	balancePerCooldownStep: number;
	maxBalance: number;
	damageReductionPercent: number;
	damageCore: number;
	damageUnit: number;
	damageObject: number;
	postSpawnCoreCooldown: number;
}

const FALLBACK_UNIT_ASSET_PATH = "fallback";

function escapeHtml(unsafe: string): string {
	return unsafe
		.replace(/&/g, "&amp;")
		.replace(/</g, "&lt;")
		.replace(/>/g, "&gt;")
		.replace(/"/g, "&quot;")
		.replace(/'/g, "&#039;")
		.replace(/\//g, "&#x2F;");
}

type UnitComponentEntry = {
	id: string;
	count: number;
	assetPath: string;
	prioritized: boolean;
};
function getSortedUnitComponentEntries(unit: UnitObject): UnitComponentEntry[] {
	const counts = new Map<string, number>();
	for (const componentId of unit.components ?? []) {
		counts.set(componentId, (counts.get(componentId) ?? 0) + 1);
	}

	return (getGameConfig()?.components.components ?? [])
		.map((component) => ({
			id: component.id,
			count: counts.get(component.id) ?? 0,
			assetPath: component.visualizer_asset_path,
			prioritized: component.visualizer_asset_prioritized,
		}))
		.filter((entry) => entry.count > 0 && entry.assetPath)
		.sort((a, b) => b.count - a.count || a.id.localeCompare(b.id));
}
export function getDominantUnitAssetPath(unit: UnitObject): string {
	const entries = getSortedUnitComponentEntries(unit);
	const prioritized = entries.filter((entry) => entry.prioritized);

	return (
		(prioritized.length > 0 ? prioritized : entries)[0]?.assetPath ??
		FALLBACK_UNIT_ASSET_PATH
	);
}
function renderUnitComponentSummary(unit: UnitObject): string {
	const entries = getSortedUnitComponentEntries(unit);
	if (entries.length === 0) return "";

	return entries
		.map(({ id, count, assetPath }) => {
			const symbolId = escapeHtml(`icon-unit-${assetPath}-1`);

			return `<span title="${escapeHtml(id)}" style="
				display:inline-flex;
				align-items:center;
				gap:0.25rem;
				white-space:nowrap;
				color:var(--text);
				text-shadow:
					0 0 2px var(--tooltip-shadow-contrast),
					0 0 4px var(--tooltip-shadow-contrast);
			">
				<svg viewBox="0 0 1 1" width="16" height="16" aria-hidden="true" style="
					display:block;
					overflow:visible;
					color:inherit;
					fill:currentColor;
					filter:drop-shadow(0 0 2px var(--tooltip-shadow-contrast));
				">
					<use href="#${symbolId}" xlink:href="#${symbolId}"></use>
				</svg>
				<span style="color:inherit;">${count}</span>
			</span>`;
		})
		.join("");
}

export function formatObjectData(obj: TickObject): string {
	const num = (v: unknown) =>
		Number.isFinite(Number(v)) ? String(Number(v)) : "NaN"; // XSS prevention
	const title = (obj: TickObject) => {
		switch (obj.type) {
			case 0:
				return `Core (ID: ${num(obj.id)})`;
			case 1:
				return `${obj.name ? `${obj.name} Unit` : "Unit"} (ID: ${num(obj.id)})`;
			case 2:
				return `Gem Deposit (ID: ${num(obj.id)})`;
			case 3:
				return `Wall (ID: ${num(obj.id)})`;
			case 4:
				return `Gem Pile (ID: ${num(obj.id)})`;
		}
	};

	const lines: { line: string; priority: number; color: string }[] = [];

	lines.push({
		line: escapeHtml(title(obj)),
		priority: -1,
		color: "var(--text)",
	});
	lines.push({
		line: `📍 Position: [x: ${num(obj.x)}, y: ${num(obj.y)}]`,
		priority: 2,
		color: "var(--text)",
	});

	if (obj.type === 1) {
		const componentSummary = renderUnitComponentSummary(obj);
		if (componentSummary) {
			lines.push({
				line: `<span style="display:inline-flex;align-items:center;flex-wrap:wrap;gap:0.5rem;">${componentSummary}</span>`,
				priority: 2.5,
				color: "var(--text)",
			});
		}
	}

	lines.push({
		line: `❤️ HP: ${num(obj.hp)}`,
		priority: 3,
		color: "var(--hp-color)",
	});

	switch (obj.type) {
		case 0:
			lines.push({
				line: `🏁 Team ID: ${num(obj.teamId)} (${getGameMisc()?.team_results.find((team) => team.id === obj.teamId)?.name || "Unknown Name"})`,
				priority: 0.5,
				color: "var(--text)",
			});
			lines.push({
				line: `💎 Gems: ${num(obj.gems)}`,
				priority: 4,
				color: "var(--gems-color)",
			});
			lines.push({
				line: `🔢 Spawn Cooldown: ${num(obj.SpawnCooldown)}`,
				priority: 5,
				color: "var(--cooldown-color)",
			});
			break;
		case 1:
			lines.push({
				line: `🏁 Team ID: ${num(obj.teamId)} (${getGameMisc()?.team_results.find((team) => team.id === obj.teamId)?.name || "Unknown Name"})`,
				priority: 0.5,
				color: "var(--text)",
			});
			lines.push({
				line: `💎 Gems: ${num(obj.gems)}`,
				priority: 4,
				color: "var(--gems-color)",
			});
			lines.push({
				line: `🔢 Action Cooldown: ${num(obj.ActionCooldown)}`,
				priority: 5,
				color: "var(--cooldown-color)",
			});
			break;
		case 4:
		case 2:
			lines.push({
				line: `💎 Gems: ${num(obj.gems)}`,
				priority: 4,
				color: "var(--gems-color)",
			});
			break;
	}

	lines.sort((a, b) => a.priority - b.priority);

	const result: string[] = [];
	for (const { line, priority, color } of lines) {
		if ([3, 2.5].includes(priority)) result.push(""); // add visual separator
		result.push(`<span style="color: ${color}">${line}</span>`);
	}

	let objectData = result.join("<br>");

	if (obj.type === 1) {
		const properties = Object.entries(obj.properties)
			.map(
				([key, value]) =>
					`<span style="color: var(--text)">${escapeHtml(key)}: ${num(value)}</span>`,
			)
			.join("<br>");

		if (properties) {
			objectData += `<br><br><span style="color: var(--text)">⚙️ Properties: ⤵</span><br>${properties}`;
		}
	}

	if (obj.debug_info) {
		// split debug info string by "[begin_errs]"
		const debugInfo = obj.debug_info.split("[begin_errs]");

		if (debugInfo[0].trim() !== "") {
			objectData += `<br><br>🐞 Debug Info: ⤵<br>`;
			objectData += escapeHtml(debugInfo[0].trim()).replace(
				/\r\n|\r|\n/g,
				"<br>",
			);
		}
		if (debugInfo.length > 1 && debugInfo[1].trim() !== "") {
			objectData += `<br><br>⚠️ Errors: ⤵<br>`;
			objectData += escapeHtml(debugInfo[1].trim()).replace(
				/\r\n|\r|\n/g,
				"<br>",
			);
		}
	}

	return objectData;
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
			maxHP = obj.properties.hp;
			break;
		case 2: // Deposit
			maxHP = getGameConfig()?.depositHp || -1;
			break;
		case 3: // Wall
			maxHP = getGameConfig()?.wallHp || -1;
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
		let maxBalance = Math.max(500, obj.gems);
		if (obj.type === 1) maxBalance = obj.properties.maxBalance;
		metrics.push({
			key: "gems",
			percentage: (obj.gems / maxBalance) * 100,
		});
	}

	// Action Cooldown / Spawn Cooldown
	if (obj.type === 1) {
		const cfg = getGameConfig();
		if (!cfg) return metrics;

		const step = Math.max(1, obj.properties.balancePerCooldownStep);
		const base = obj.properties.baseActionCooldown;

		let calc = base + Math.floor(obj.gems / step);
		calc = Math.max(1, calc);
		const denom = Math.max(calc, obj.ActionCooldown);
		metrics.push({
			key: "cooldown",
			percentage: (Math.max(0, obj.ActionCooldown) / denom) * 100,
		});
	} else if (obj.type === 0) {
		const cfg = getGameConfig();
		if (!cfg) return metrics;
		const total = obj.SpawnCooldownLastResetTo;
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
