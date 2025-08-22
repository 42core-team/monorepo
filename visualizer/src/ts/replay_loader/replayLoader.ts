import { resetTimeManager } from "../input_manager/timeManager";
import { setupRenderer } from "../renderer/renderer";
import type { TickAction } from "./action";
import type { GameConfig } from "./config";
import type { TickObject } from "./object";

const expectedReplayVersion = "1.1.1";
const winnerNameElement = document.getElementById(
	"winnername",
) as HTMLSpanElement;
const winReasonElement = document.getElementById(
	"winreason",
) as HTMLSpanElement;
const deathReasons: Record<number, string> = {
	0: "Survived",
	1: "Core destruction",
	2: "Unexpectedly Disconnected",
	3: "Did not connect to gameserver",
	4: "Timeout while sending data",
	5: "Game timed out - Decision via Core HP",
	6: "Game timed out - Decision via Unit HP",
	7: "Game timed out - Random Decision",
};

export interface ReplayTick {
	objects: TickObject[];
	actions: TickAction[];
}

export interface ReplayData {
	misc: {
		team_results: {
			id: number;
			name: string;
			place: number;
			death_reason: number;
		}[];
		version: string;
		worldGeneratorSeed: number;
	};
	ticks: { [tick: string]: ReplayTick };
	full_tick_amount: number;
	config?: GameConfig;
}
const emptyReplayData: ReplayData = {
	misc: {
		team_results: [],
		version: "",
		worldGeneratorSeed: 0,
	},
	ticks: {},
	full_tick_amount: 0,
};

type State = Record<number, TickObject>;
type ReplayMisc = {
	team_results: {
		id: number;
		name: string;
		place: number;
		death_reason: number;
	}[];
	version: string;
	worldGeneratorSeed: number;
};

export let totalReplayTicks = 0;

let replayDataOverride: string | null = null; // if a file was dropped into the window, display this instead of the auto fetched replay data

function deepClone<T>(obj: T): T {
	return JSON.parse(JSON.stringify(obj));
}

function forceHttps(url: string): string {
	const u = new URL(url, location.href);
	if (location.protocol === "https:" && u.protocol !== "https:") {
		u.protocol = "https:";
	}
	return u.toString();
}

class ReplayLoader {
	private replayData: ReplayData = emptyReplayData;
	private cache: Map<number, State> = new Map<number, State>();
	private cacheInterval: number;

	constructor(cacheInterval = 25) {
		this.cacheInterval = cacheInterval;
	}

	public async loadReplay(filePath: string): Promise<void> {
		let fileData: string | null = replayDataOverride;
		if (!fileData) {
			await fetch(forceHttps(filePath), { cache: "no-cache" })
				.then((response) => {
					if (!response.ok) {
						throw new Error(
							`Failed to fetch replay file: ${response.statusText}`,
						);
					}
					return response.text();
				})
				.then((data) => {
					fileData = data;
				})
				.catch((err) => {
					console.error("Error fetching replay file:", err);
				});
		}

		if (!fileData) {
			throw new Error("No replay data available to load.");
		}

		this.replayData = JSON.parse(fileData) as ReplayData;
		if (
			!this.replayData.ticks ||
			typeof this.replayData.full_tick_amount !== "number"
		) {
			throw new Error(
				"Invalid replay data format: missing ticks or full_tick_amount",
			);
		}
		if (this.replayData.misc.version !== expectedReplayVersion) {
			alert(
				"Unsupported replay version. Things might stop working unexpectedly.",
			);
			console.error(
				`Expected version: ${expectedReplayVersion}, but got: ${this.replayData.misc.version}`,
			);
		}
		totalReplayTicks = this.replayData.full_tick_amount;

		winnerNameElement.textContent =
			this.replayData.misc.team_results.find((team) => team.place === 0)
				?.name || "Unknown";
		winReasonElement.textContent = "";
		for (const team of this.replayData.misc.team_results) {
			if (team.place === 0) continue;
			const teamName = team.name || `Team ${team.id}`;
			winReasonElement.textContent += `Place ${team.place + 1}: ${teamName} (Death Reason: ${deathReasons[team.death_reason]})\n`;
		}

		const fullState: State = {};
		const tick0 = this.replayData.ticks["0"];
		if (tick0?.objects) {
			for (const obj of tick0.objects) {
				fullState[obj.id] = deepClone(obj);
			}
		}
		this.cache.set(0, deepClone(fullState));

		for (let t = 1; t <= totalReplayTicks; t++) {
			const tickData = this.replayData.ticks[t.toString()];
			if (tickData?.objects) {
				this.applyDiff(fullState, tickData);
			}
			for (const obj of Object.values(fullState)) {
				if ("ActionCooldown" in obj && obj.ActionCooldown > 0) obj.ActionCooldown--;
			}
			if (t % this.cacheInterval === 0) {
				this.cache.set(t, deepClone(fullState));
			}
		}
		console.log(
			`💫 Replay loaded successfully! ✨ (${this.replayData.misc.team_results[0].name} 🤜 vs 🤛 ${this.replayData.misc.team_results[1].name} for ${totalReplayTicks} ticks)`,
		);
	}

	private applyDiff(state: State, tickData: ReplayTick): void {
		for (const diffObj of tickData.objects) {
			const id = diffObj.id;
			if (state[id]) {
				Object.assign(state[id], diffObj);
				if (diffObj.state === "dead") {
					delete state[id];
				}
			} else {
				state[id] = deepClone(diffObj);
			}
		}
	}

	public getStateAt(tick: number): ReplayTick {
		if (tick < 0 || tick > totalReplayTicks) {
			throw new Error("Tick out of range");
		}
		if (tick === totalReplayTicks) {
			return { objects: [], actions: [] }; // empty state for ending animation
		}

		let snapshotTick = -1;
		for (const key of this.cache.keys()) {
			if (key <= tick && key > snapshotTick) {
				snapshotTick = key;
			}
		}
		if (snapshotTick === -1) {
			throw new Error("No snapshot found");
		}
		const cachedState = this.cache.get(snapshotTick);
		if (!cachedState) {
			throw new Error("Cached state not found");
		}
		const state: State = deepClone(cachedState);
		for (let t = snapshotTick + 1; t <= tick; t++) {
			const tickData = this.replayData.ticks[t.toString()];
			if (tickData?.objects) {
				this.applyDiff(state, tickData);
			}
			for (const obj of Object.values(state)) {
				if (!("ActionCooldown" in obj)) {
					continue;
				}
				if (obj.ActionCooldown > 0) {
					obj.ActionCooldown--;
				}
			}
		}

		const resultTickData = this.replayData.ticks[tick.toString()];
		const actions: TickAction[] = resultTickData?.actions
			? deepClone(resultTickData.actions)
			: [];

		return { objects: Object.values(state), actions } as ReplayTick;
	}

	public getActionsByExecutor(tick: number): Record<number, TickAction[]> {
		const tickActions = this.replayData.ticks[tick]?.actions ?? [];
		return tickActions.reduce(
			(map, action) => {
				const exec =
					"unit_id" in action
						? action.unit_id
						: "source_id" in action
							? action.source_id
							: undefined;
				if (exec !== undefined) {
					if (!map[exec]) map[exec] = [];
					map[exec].push(action);
				}
				return map;
			},
			{} as Record<number, TickAction[]>,
		);
	}

	public resetReplayData() {
		this.replayData = emptyReplayData;
	}

	public getGameConfig(): GameConfig | undefined {
		return this.replayData.config;
	}

	public getGameMisc(): ReplayMisc {
		return this.replayData.misc;
	}

	public getReplayJSON(): string {
		return JSON.stringify(this.replayData);
	}
}

let replayLoader: ReplayLoader | null = null;
let replayInterval: ReturnType<typeof setInterval> | null = null;

let currentFilePath: string | null = null;
let currentCacheInterval = 25;
let lastEtag: string | null = null;

async function resetReplay(reason: string = "reset"): Promise<void> {
	if (!currentFilePath) {
		throw new Error("No file path set for replay.");
	}
	const newReplayLoader = new ReplayLoader(currentCacheInterval);
	await newReplayLoader.loadReplay(currentFilePath);
	replayLoader = newReplayLoader;
	tempStateCache = null;
	resetTimeManager();
	setupRenderer();
	console.debug(
		`Replay reset (${reason}). override=${Boolean(replayDataOverride)} etag=${lastEtag}`,
	);
}

export async function setupReplayLoader(
	filePath: string,
	cacheInterval = 25,
	updateInterval = 3000,
): Promise<void> {
	currentFilePath = filePath;
	currentCacheInterval = cacheInterval;

	// initial load (via central reset)
	await resetReplay("initial");

	// grab initial ETag
	lastEtag = null;
	try {
		const headRes = await fetch(forceHttps(filePath), {
			method: "HEAD",
			cache: "no-cache",
		});
		if (headRes.ok) {
			lastEtag = headRes.headers.get("ETag");
		} else {
			console.warn(
				"Failed to fetch initial ETag:",
				headRes.status,
				headRes.statusText,
			);
		}
	} catch (err) {
		console.warn("Failed to fetch initial ETag:", err);
	}

	// schedule updates
	if (replayInterval) {
		clearInterval(replayInterval);
	}
	replayInterval = setInterval(async () => {
		if (!currentFilePath) return;
		try {
			const head = await fetch(forceHttps(currentFilePath), {
				method: "HEAD",
				cache: "no-cache",
			});

			if (!head.ok) {
				console.error("Couldnt fetch current replay etag.");
			}

			const etag = head.headers.get("ETag");
			if (!etag) {
				console.warn("No ETag header present. This is a web server issue.");
				return;
			}

			// if ETag changes, clear local override and reset via the single path
			if (etag !== lastEtag) {
				lastEtag = etag;
				// make override non-permanent: clear it on remote update
				if (replayDataOverride) {
					replayDataOverride = null;
				}
				await resetReplay("etag-change");
			}
		} catch (err) {
			console.error("Error checking for updates:", err);
		}
	}, updateInterval);
}

let tempStateCache:
	| { tick: number; state: ReplayTick; lastAccess: number }[]
	| null = null;
const lastAccessThreshold = 10;
export function getStateAt(tick: number): ReplayTick | null {
	if (!replayLoader) {
		throw new Error("Replay not loaded. Please call loadReplay first.");
	}

	if (tempStateCache) {
		for (let i = 0; i < (tempStateCache?.length ?? 0); i++) {
			tempStateCache[i].lastAccess++;
		}
		for (let i = tempStateCache.length - 1; i >= 0; i--) {
			if (tempStateCache[i].lastAccess > lastAccessThreshold) {
				tempStateCache.splice(i, 1);
			}
		}
	}

	// attempt to find in cache
	if (tempStateCache) {
		for (const cached of tempStateCache) {
			if (cached.tick === tick) {
				cached.lastAccess = 0;
				return cached.state;
			}
		}
	}

	let result: ReplayTick | null = null;

	try {
		result = replayLoader.getStateAt(tick);
		if (!result) {
			throw new Error(`No state found for tick ${tick}`);
		}
		if (tempStateCache) {
			tempStateCache.push({ tick, state: result, lastAccess: 0 });
		} else {
			tempStateCache = [{ tick, state: result, lastAccess: 0 }];
		}
	} catch (err) {
		console.log(err);
		return null;
	}

	return result;
}
export function getActionsByExecutor(
	tick: number,
): Record<number, TickAction[]> {
	if (!replayLoader) {
		throw new Error("Replay not loaded. Please call loadReplay first.");
	}

	return replayLoader.getActionsByExecutor(tick);
}
export function getTotalReplayTicks(): number {
	if (!replayLoader) {
		throw new Error("Replay not loaded. Please call loadReplay first.");
	}

	return totalReplayTicks;
}
export function getNameOfUnitType(unitType: number): string {
	if (!replayLoader) {
		throw new Error("Replay not loaded. Please call loadReplay first.");
	}
	const cfg = replayLoader.getGameConfig();
	if (!cfg) {
		throw new Error("GameConfig is missing! Did loadReplay parse config?");
	}
	if (
		!Array.isArray(cfg.units) ||
		unitType < 0 ||
		unitType >= cfg.units.length
	) {
		throw new Error(`Invalid unitType index: ${unitType}`);
	}
	return cfg.units[unitType].name;
}

export function getGameConfig(): GameConfig | undefined {
	if (!replayLoader) {
		throw new Error("Replay not loaded. Please call loadReplay first.");
	}

	return replayLoader.getGameConfig();
}
export function getGameMisc(): ReplayMisc | undefined {
	if (!replayLoader) {
		throw new Error("Replay not loaded. Please call loadReplay first.");
	}

	return replayLoader.getGameMisc();
}
export function getReplayJSON(): string {
	if (!replayLoader) {
		throw new Error("Replay not loaded. Please call loadReplay first.");
	}
	return replayLoader.getReplayJSON();
}

export function getWinningTeamFormatted(): string {
	if (!replayLoader) {
		throw new Error("Replay not loaded. Please call loadReplay first.");
	}

	const winningTeam = replayLoader
		.getGameMisc()
		.team_results.find((team) => team.place === 0);
	if (!winningTeam) {
		return "No winning team found";
	}

	return `${winningTeam.name} (ID: ${winningTeam.id})`;
}

window.addEventListener("drop", (e) => {
	e.preventDefault();
	if (
		!e.dataTransfer ||
		!e.dataTransfer.files ||
		e.dataTransfer.files.length === 0
	) {
		console.error("No file dropped but drop event triggered.");
		return;
	}
	const file = e.dataTransfer.files[0];
	const reader = new FileReader();
	reader.readAsText(file);
	reader.onload = async () => {
		const contents = reader.result as string;
		if (contents) {
			console.log("File loaded successfully");
			replayDataOverride = contents;
			await resetReplay("file-drop");
		}
	};
	alert(
		"File loaded successfully, please refresh the page to see the changes.",
	);
});
window.addEventListener("dragover", (e) => {
	e.preventDefault();
});
