import { resetTimeManager } from '../time_manager/timeManager.js';
import type { TickAction } from './action.js';
import type { GameConfig } from './config.js';
import type { TickObject } from './object.js';

const winnerNameElement = document.getElementById('winnername') as HTMLSpanElement;
const winReasonElement = document.getElementById('winreason') as HTMLSpanElement;
const gameEndReasons: Record<number, string> = {
	0: 'Core Destruction',
	1: 'Game Timed Out - Decision via Core HP',
	2: 'Game Timed Out - Decision via Total Unit HP',
	3: 'Game Timed Out - Random Decision',
};

export interface ReplayTick {
	objects: TickObject[];
	actions: TickAction[];
}

export interface ReplayData {
	misc: {
		team_results: { id: number; name: string; place: number }[];
		game_end_reason: number;
	};
	ticks: { [tick: string]: ReplayTick };
	full_tick_amount: number;
	config?: GameConfig;
}

type State = Record<number, TickObject>;
type ReplayMisc = {
	team_results: { id: number; name: string; place: number }[];
	game_end_reason: number;
};

export let totalReplayTicks = 0;

let replayDataOverride: string | null = null; // if a file was dropped into the window, display this instead of the auto fetched replay data

function deepClone<T>(obj: T): T {
	return JSON.parse(JSON.stringify(obj));
}

class ReplayLoader {
	private replayData: ReplayData = { misc: { team_results: [], game_end_reason: 0 }, ticks: {}, full_tick_amount: 0 };
	private cache: Map<number, State> = new Map<number, State>();
	private cacheInterval: number;

	constructor(cacheInterval = 25) {
		this.cacheInterval = cacheInterval;
	}

	public async loadReplay(filePath: string): Promise<void> {
		let fileData: string | null = replayDataOverride;
		if (!fileData) {
			await fetch(filePath, { cache: 'no-cache' })
				.then((response) => {
					if (!response.ok) {
						throw new Error(`Failed to fetch replay file: ${response.statusText}`);
					}
					return response.text();
				})
				.then((data) => {
					fileData = data;
				})
				.catch((err) => {
					console.error('Error fetching replay file:', err);
				});
		}

		if (!fileData) {
			throw new Error('No replay data available to load.');
		}

		this.replayData = JSON.parse(fileData) as ReplayData;
		if (!this.replayData.ticks || !this.replayData.full_tick_amount) {
			throw new Error('Invalid replay data format: missing ticks or full_tick_amount');
		}
		totalReplayTicks = this.replayData.full_tick_amount;

		winnerNameElement.innerHTML = this.replayData.misc.team_results.find((team) => team.place === 0)?.name || 'Unknown';
		winReasonElement.innerHTML = gameEndReasons[this.replayData.misc.game_end_reason] || 'Unknown Reason';

		const fullState: State = {};
		const tick0 = this.replayData.ticks['0'];
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
			if (t % this.cacheInterval === 0) {
				this.cache.set(t, deepClone(fullState));
			}
		}
		console.log(`💫 Replay loaded successfully! ✨ (${this.replayData.misc.team_results[0].name} 🤜 vs 🤛 ${this.replayData.misc.team_results[1].name} for ${totalReplayTicks} ticks)`);
	}

	private applyDiff(state: State, tickData: ReplayTick): void {
		for (const diffObj of tickData.objects) {
			const id = diffObj.id;
			if (state[id]) {
				Object.assign(state[id], diffObj);
				if (diffObj.state === 'dead') {
					delete state[id];
				}
			} else {
				state[id] = deepClone(diffObj);
			}
		}
	}

	public getStateAt(tick: number): ReplayTick {
		if (tick < 0 || tick > totalReplayTicks) {
			throw new Error('Tick out of range');
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
			throw new Error('No snapshot found');
		}
		const cachedState = this.cache.get(snapshotTick);
		if (!cachedState) {
			throw new Error('Cached state not found');
		}
		const state: State = deepClone(cachedState);
		for (let t = snapshotTick + 1; t <= tick; t++) {
			const tickData = this.replayData.ticks[t.toString()];
			if (tickData?.objects) {
				this.applyDiff(state, tickData);
			}
			for (const obj of Object.values(state)) {
				if (!('moveCooldown' in obj)) {
					continue;
				}
				if (obj.moveCooldown > 0) {
					obj.moveCooldown--;
				}
			}
		}

		const resultTickData = this.replayData.ticks[tick.toString()];
		const actions: TickAction[] = resultTickData?.actions ? deepClone(resultTickData.actions) : [];

		return { objects: Object.values(state), actions } as ReplayTick;
	}

	public getActionsByExecutor(tick: number): Record<number, TickAction[]> {
		const tickActions = this.replayData.ticks[tick]?.actions ?? [];
		return tickActions.reduce(
			(map, action) => {
				const exec = 'unit_id' in action ? action.unit_id : 'source_id' in action ? action.source_id : undefined;
				if (exec !== undefined) {
					if (!map[exec]) map[exec] = [];
					map[exec].push(action);
				}
				return map;
			},
			{} as Record<number, TickAction[]>
		);
	}

	public resetReplayData() {
		this.replayData = { misc: { team_results: [], game_end_reason: 0 }, ticks: {}, full_tick_amount: 0 };
	}

	public getGameConfig(): GameConfig | undefined {
		return this.replayData.config;
	}

	public getGameMisc(): ReplayMisc {
		return this.replayData.misc;
	}
}

let replayLoader: ReplayLoader | null = null;
let replayInterval: ReturnType<typeof setInterval> | null = null;

// loads replay and sets up periodic updates
export async function setupReplayLoader(filePath: string, cacheInterval = 25, updateInterval = 3000): Promise<void> {
	replayLoader = new ReplayLoader(cacheInterval);

	// initial load
	await replayLoader.loadReplay(filePath);

	// grab initial ETag
	let lastEtag: string | null = null;
	try {
		const headRes = await fetch(filePath, { method: 'HEAD', cache: 'no-cache' });
		lastEtag = headRes.headers.get('ETag');
	} catch (err) {
		console.warn('Failed to fetch initial ETag:', err);
	}

	// schedule updates
	if (replayInterval) {
		clearInterval(replayInterval);
	}
	replayInterval = setInterval(async () => {
		if (replayDataOverride) return;
		try {
			const head = await fetch(filePath, { method: 'HEAD', cache: 'no-cache' });

			if (!head.ok) {
				throw 'catch block time - lets reset the replayLoader';
			}

			const etag = head.headers.get('ETag');
			if (!etag) {
				console.warn('No ETag header present. This is a web server issue.');
				return;
			}

			if (etag !== lastEtag) {
				lastEtag = etag;
				const newReplayLoader = new ReplayLoader(cacheInterval);
				await newReplayLoader.loadReplay(filePath);
				replayLoader = newReplayLoader;
				resetTimeManager();
			}
		} catch (err) {
			console.error('Error checking for updates:', err);
			replayLoader = null;
		}
	}, updateInterval);
}

let tempStateCache: { tick: number; state: ReplayTick; lastAccess: number }[] | null = null;
const lastAccessThreshold = 10;
export function getStateAt(tick: number): ReplayTick | null {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
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
export function getActionsByExecutor(tick: number): Record<number, TickAction[]> {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
	}

	return replayLoader.getActionsByExecutor(tick);
}
export function getTotalReplayTicks(): number {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
	}

	return totalReplayTicks;
}
export function getNameOfUnitType(unitType: number): string {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
	}
	const cfg = replayLoader.getGameConfig();
	if (!cfg) {
		throw new Error('GameConfig is missing! Did loadReplay parse config?');
	}
	if (!Array.isArray(cfg.units) || unitType < 0 || unitType >= cfg.units.length) {
		console.error(`Invalid unitType index: ${unitType}. Available unit indices: 0-${cfg.units.length - 1}`);
		console.error('Available units:', cfg.units.map((u, i) => `${i}: ${u.name}`));
		throw new Error(`Invalid unitType index: ${unitType}`);
	}
	return cfg.units[unitType].name;
}

export function getGameConfig(): GameConfig | undefined {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
	}

	return replayLoader.getGameConfig();
}
export function getGameMisc(): ReplayMisc | undefined {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
	}

	return replayLoader.getGameMisc();
}

export function getWinningTeamFormatted(): string {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
	}

	const winningTeam = replayLoader.getGameMisc().team_results.find((team) => team.place === 0);
	if (!winningTeam) {
		return 'No winning team found';
	}

	return `${winningTeam.name} (ID: ${winningTeam.id})`;
}

window.addEventListener('drop', (e) => {
	e.preventDefault();
	if (!e.dataTransfer || !e.dataTransfer.files || e.dataTransfer.files.length === 0) {
		console.error('No file dropped but drop event triggered.');
		return;
	}
	const file = e.dataTransfer.files[0];
	const reader = new FileReader();
	reader.readAsText(file);
	reader.onload = async () => {
		const contents = reader.result as string;
		if (contents) {
			console.log('File loaded successfully');
			replayDataOverride = contents;
			const newReplayLoader = new ReplayLoader(25);
			await newReplayLoader.loadReplay('dummy.json');
			replayLoader = newReplayLoader;
			resetTimeManager();
		}
	};
	alert('File loaded successfully, please refresh the page to see the changes.');
});
window.addEventListener('dragover', (e) => {
	e.preventDefault();
});
