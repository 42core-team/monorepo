export interface TickObject {
	id: number;
	type?: number;
	x?: number;
	y?: number;
	dir?: number;
	hp?: number;
	teamId?: number;
	balance?: number;
	state?: string;
}

export interface TickAction {
	type: string;
	unit_type?: number;
	unit_id?: number;
	dir?: number;
}

export interface ReplayTick {
	objects: TickObject[];
	actions: TickAction[];
}

export interface ReplayData {
	ticks: { [tick: string]: ReplayTick };
	full_tick_amount: number;
	config?: GameConfig;
}

export interface GameConfig {
	width: number;
	height: number;
	tickRate: number;
	idleIncome: number;
	idleIncomeTimeOut: number;
	resourceHp: number;
	resourceIncome: number;
	coreHp: number;
	initialBalance: number;
	wallHp: number;
	wallBuildCost: number;
}

type State = { [id: string]: unknown };

export let totalReplayTicks = 0;

function deepClone<T>(obj: T): T {
	return JSON.parse(JSON.stringify(obj));
}

class ReplayLoader {
	private replayData: ReplayData = { ticks: {}, full_tick_amount: 0 };
	private cache: Map<number, State>;
	private cacheInterval: number;

	constructor(cacheInterval = 25) {
		this.cacheInterval = cacheInterval;
		this.cache = new Map<number, State>();
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

				const totalTicks = this.replayData.full_tick_amount;
				for (let t = 1; t <= totalTicks; t++) {
					const tickData = this.replayData.ticks[t.toString()];
					if (tickData?.objects) {
						this.applyDiff(fullState, tickData);
					}
					if (t % this.cacheInterval === 0) {
						this.cache.set(t, deepClone(fullState));
					}
				}
				console.log('Replay loaded with snapshots at intervals:', Array.from(this.cache.keys()));
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
				if (diffObj.state == 'dead') {
					// maybe some death animations through progressive state change
					delete state[id]; // maybe cores should not disappear on death
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
		return { objects: Object.values(state), actions: [] } as ReplayTick;
	}

	public getGameConfig(): GameConfig | undefined {
		return this.replayData.config;
	}
}

let replayLoader: ReplayLoader | null = null;
let replayInterval: ReturnType<typeof setInterval> | null = null;

export async function loadReplayAndKeepUpdated(filePath: string, cacheInterval = 25, updateInterval = 1000): Promise<void> {
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
				await replayLoader!.loadReplay(filePath);
			}
		} catch (err) {
			console.error('Error checking for updates:', err);
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

export function getGameConfig(): GameConfig | undefined {
	if (!replayLoader) {
		throw new Error('Replay not loaded. Please call loadReplay first.');
	}

	return replayLoader.getGameConfig();
}
