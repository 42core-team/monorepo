import type { TickAction } from './action';
import type { GameConfig } from './config';
import type { TickObject } from './object';

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

export let totalReplayTicks = 0;

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
		await fetch(filePath)
			.then((response) => response.json())
			.then((json) => {
				this.replayData = json as ReplayData;
				totalReplayTicks = this.replayData.full_tick_amount;
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
			})
			.catch((err) => {
				console.log('Failed to load Replay: ', err);
			});
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
		if (tick < 0 || tick > this.replayData.full_tick_amount) {
			throw new Error('Tick out of range');
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
		}

		const resultTickData = this.replayData.ticks[tick.toString()];
		const actions: TickAction[] = resultTickData?.actions ? deepClone(resultTickData.actions) : [];

		return { objects: Object.values(state), actions } as ReplayTick;
	}

	public getGameConfig(): GameConfig | undefined {
		return this.replayData.config;
	}
}

let replayLoader: ReplayLoader | null = null;
let replayInterval: ReturnType<typeof setInterval> | null = null;

// loads replay and sets up periodic updates
export async function setupReplayLoader(filePath: string, cacheInterval = 25, updateInterval = 1000): Promise<void> {
	replayLoader = new ReplayLoader(cacheInterval);

	// initial load
	await replayLoader.loadReplay(filePath);

	// grab initial ETag
	let lastEtag: string | null = null;
	try {
		const headRes = await fetch(filePath, { method: 'HEAD' });
		lastEtag = headRes.headers.get('ETag');
	} catch (err) {
		console.warn('Failed to fetch initial ETag:', err);
	}

	// schedule updates
	if (replayInterval) {
		clearInterval(replayInterval);
	}
	replayInterval = setInterval(async () => {
		try {
			const head = await fetch(filePath, { method: 'HEAD' });
			const etag = head.headers.get('ETag');
			if (!etag) {
				console.warn('No ETag header present. This is a web server issue.');
				return;
			}

			if (etag !== lastEtag) {
				lastEtag = etag;
				console.log('🔄 Detected changes via ETag, reloading replay…');
				if (replayLoader) {
					await replayLoader.loadReplay(filePath);
				} else {
					console.error('ReplayLoader is not initialized.');
				}
			}
		} catch (err) {
			console.error('Error checking for updates:', err);
			alert("Couldn't fetch replay updates. There is an issue with the webserver, please reopen your VSCode devcontainer.");
		}
	}, updateInterval);
}

export function getStateAt(tick: number): ReplayTick | null {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
	}

	try {
		return replayLoader.getStateAt(tick);
	} catch (err) {
		console.log(err);
		return null;
	}
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
