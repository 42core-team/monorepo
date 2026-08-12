import { ensureIcons } from "../renderer/iconManager";
import { getTeamIndex } from "../renderer/objectRenderer";
import { refreshTeamDisplay, setupRenderer } from "../renderer/renderer";
import type { TickAction } from "../replay_format/action";
import type { GameConfig } from "../replay_format/config";
import type { TickObject } from "../replay_format/object";
import type {
	ReplayData,
	ReplayMisc,
	ReplayTick,
} from "../replay_format/replay";
import {
	checkReplayFile,
	fetchReplayFile,
	parseDroppedReplay,
} from "./replayFileInput";
import { watchReplayWebSocket } from "./websocketInput";

const EXPECTED_REPLAY_VERSION = "3.0.0";
const WEBSOCKET_RETRY_MS = 500;
const endTitleElement = document.getElementById(
	"end-title",
) as HTMLHeadingElement;
const winReasonElement = document.getElementById(
	"winreason",
) as HTMLSpanElement;
const timeControlsElement = document.getElementById(
	"time-controls",
) as HTMLDivElement;
const deathReasons: Record<number, string> = {
	0: "Survived",
	1: "Core destruction",
	2: "Unexpectedly disconnected",
	3: "Kicked for spamming actions or debug data",
	4: "Did not connect to the game server",
	5: "Timeout while sending data",
	6: "Game timed out - decision via Core HP",
	7: "Game timed out - decision via Unit HP",
	8: "Game timed out - random decision",
};

type State = Record<number, TickObject>;
type ReplaySource = "file" | "websocket";
type ReplayControls = {
	finishLiveReplay: () => void;
	resetTimeManager: () => void;
	setLiveAvailable: (available: boolean) => void;
	setPlaybackSpeed: (speed: number) => void;
	updateReplayBounds: (previousLastTick: number) => void;
};

let replayControls: ReplayControls;

export function setReplayControls(controls: ReplayControls): void {
	replayControls = controls;
}

function clone<T>(value: T): T {
	return structuredClone(value);
}

function initializeDerivedObjectFields(obj: TickObject): TickObject {
	if (
		obj.type === 0 &&
		typeof obj.SpawnCooldown === "number" &&
		typeof obj.SpawnCooldownLastResetTo !== "number"
	) {
		obj.SpawnCooldownLastResetTo = obj.SpawnCooldown;
	}
	return obj;
}

function isDynamicSpeedEnabled(): boolean {
	const params = new URLSearchParams(window.location.search);
	return (
		(params.get("dynamicSpeed") || "off").toLowerCase() === "on" &&
		!params.has("speed")
	);
}

function computeDynamicSpeed(ticks: number): number {
	const desiredSeconds = 20 + Math.abs(ticks - 500) * (40 / 1500);
	const speed = Math.max(5, Math.min(25, ticks / desiredSeconds));
	return Math.round(speed / 0.5) * 0.5;
}

function warnAboutReplayVersion(misc?: ReplayMisc): void {
	if (!misc?.version || misc.version === EXPECTED_REPLAY_VERSION) return;
	const suppress =
		new URLSearchParams(window.location.search).get(
			"suppress_version_warning",
		) === "true";
	if (!suppress) {
		alert(
			`Unsupported replay version. Expected ${EXPECTED_REPLAY_VERSION}, but got ${misc.version}.`,
		);
	}
	console.error(
		`Expected replay version ${EXPECTED_REPLAY_VERSION}, but got ${misc.version}.`,
	);
}

class ReplayLoader {
	private replayData: ReplayData;
	private readonly snapshots = new Map<number, State>();
	private readonly liveDurations = new Map<number, number>();
	private latestState: State = {};
	private lastTick = -1;
	private finished = false;
	private interrupted = false;

	constructor(
		replayData: ReplayData,
		private readonly cacheInterval = 25,
		finished = true,
	) {
		this.replayData = replayData;
		this.finished = finished;
		this.rebuildSnapshots();
	}

	static live(
		gameId: string,
		config: GameConfig,
		misc: ReplayMisc,
		cacheInterval: number,
	): ReplayLoader {
		return new ReplayLoader(
			{
				game_id: gameId,
				config,
				misc,
				ticks: {},
				full_tick_amount: -1,
			},
			cacheInterval,
			false,
		);
	}

	private applyDiff(state: State, tickData: ReplayTick): void {
		for (const diffObj of tickData.objects ?? []) {
			const existing = state[diffObj.id];
			if (existing) {
				Object.assign(existing, diffObj);
				if (
					existing.type === 0 &&
					"SpawnCooldown" in diffObj &&
					typeof diffObj.SpawnCooldown === "number"
				) {
					existing.SpawnCooldownLastResetTo = diffObj.SpawnCooldown;
				}
				if (diffObj.state === "dead") delete state[diffObj.id];
			} else if (diffObj.state !== "dead") {
				state[diffObj.id] = initializeDerivedObjectFields(clone(diffObj));
			}
		}
	}

	private rebuildSnapshots(): void {
		this.snapshots.clear();
		this.lastTick = Math.max(
			this.replayData.full_tick_amount,
			...Object.keys(this.replayData.ticks).map(Number),
			-1,
		);
		const state: State = {};
		for (let tick = 0; tick <= this.lastTick; tick++) {
			this.applyDiff(state, this.replayData.ticks[String(tick)] ?? {});
			if (tick === 0 || tick % this.cacheInterval === 0) {
				this.snapshots.set(tick, clone(state));
			}
		}
		this.latestState = clone(state);
	}

	appendTick(tick: number, data: ReplayTick, durationMs?: number): boolean {
		if (this.replayData.ticks[String(tick)]) return false;
		const previousLastTick = this.lastTick;
		this.replayData.ticks[String(tick)] = data;
		this.replayData.full_tick_amount = Math.max(
			this.replayData.full_tick_amount,
			tick,
		);
		this.lastTick = Math.max(this.lastTick, tick);
		if (durationMs !== undefined && durationMs > 0) {
			this.liveDurations.set(tick, durationMs);
		}

		if (tick === previousLastTick + 1) {
			this.applyDiff(this.latestState, data);
			if (tick === 0 || tick % this.cacheInterval === 0) {
				this.snapshots.set(tick, clone(this.latestState));
			}
		} else {
			this.rebuildSnapshots();
		}
		return true;
	}

	finish(misc: ReplayMisc): void {
		this.replayData.misc = misc;
		this.finished = true;
		this.interrupted = false;
		warnAboutReplayVersion(misc);
	}

	markInterrupted(): void {
		if (!this.finished) this.interrupted = true;
	}

	clearInterrupted(): void {
		this.interrupted = false;
	}

	updateMisc(misc: ReplayMisc): void {
		this.replayData.misc = misc;
	}

	getStateAt(tick: number): ReplayTick | null {
		if (tick < 0 || tick > this.lastTick) return null;
		let snapshotTick = -1;
		for (const candidate of this.snapshots.keys()) {
			if (candidate <= tick && candidate > snapshotTick) {
				snapshotTick = candidate;
			}
		}
		if (snapshotTick < 0) return null;

		const state = clone(this.snapshots.get(snapshotTick) ?? {});
		for (let current = snapshotTick + 1; current <= tick; current++) {
			this.applyDiff(state, this.replayData.ticks[String(current)] ?? {});
		}
		return {
			objects: Object.values(state),
			actions: clone(this.replayData.ticks[String(tick)]?.actions ?? []),
		};
	}

	getActionsByExecutor(tick: number): Record<number, TickAction[]> {
		return (this.replayData.ticks[String(tick)]?.actions ?? []).reduce(
			(map, action) => {
				const executor =
					"unit_id" in action
						? action.unit_id
						: "source_id" in action
							? action.source_id
							: undefined;
				if (executor !== undefined) {
					if (!map[executor]) map[executor] = [];
					map[executor].push(action);
				}
				return map;
			},
			{} as Record<number, TickAction[]>,
		);
	}

	getGameId(): string {
		return this.replayData.game_id;
	}

	getConfig(): GameConfig {
		return this.replayData.config;
	}

	getMisc(): ReplayMisc | undefined {
		return this.replayData.misc;
	}

	getLastTick(): number {
		return this.lastTick;
	}

	getLiveDuration(targetTick: number): number | undefined {
		return this.liveDurations.get(targetTick);
	}

	getEndState(): "complete" | "interrupted" | null {
		if (this.interrupted) return "interrupted";
		return this.finished ? "complete" : null;
	}

	toJSON(): string {
		return JSON.stringify(this.replayData);
	}
}

let replayLoader: ReplayLoader | null = null;
let activeSource: ReplaySource | null = null;
let remoteFileGameId: string | null = null;
let websocketGameId: string | null = null;
let websocketLive = false;
let cacheInterval = 25;
let filePoll: number | null = null;
let stopWebSocket: (() => void) | null = null;
let renderedStates = new Map<number, ReplayTick>();

function setWaitingForGame(waiting: boolean): void {
	document.documentElement.dataset.replayState = waiting ? "waiting" : "ready";
	timeControlsElement.toggleAttribute("inert", waiting);
	timeControlsElement.setAttribute("aria-disabled", String(waiting));
}

function updateEndDisplay(): void {
	const endState = replayLoader?.getEndState();
	if (endState === null || endState === undefined) {
		endTitleElement.textContent = "";
		winReasonElement.textContent = "";
		return;
	}
	if (endState === "interrupted") {
		endTitleElement.textContent = "Game was manually interrupted";
		winReasonElement.textContent = "";
		return;
	}

	const results = replayLoader?.getMisc()?.team_results ?? [];
	const winner = results.find((team) => team.place === 0);
	endTitleElement.textContent = winner
		? `🎖️🎉 Winner: ${getTeamIndex(winner.id) === 0 ? "🟣" : "🟠"} ${winner.name || "Unknown"} 🎈🏁`
		: "";
	winReasonElement.textContent = results
		.filter((team) => team.place !== undefined && team.place !== 0)
		.map((team) => {
			const emoji = getTeamIndex(team.id) === 0 ? "🟣" : "🟠";
			const reason =
				team.death_reason === undefined
					? "Unknown"
					: (deathReasons[team.death_reason] ?? "Unknown");
			return `Place ${(team.place ?? 0) + 1}: ${emoji} ${team.name || `Team ${team.id}`} (Death reason: ${reason})`;
		})
		.join("\n");
}

async function refreshReplay(
	gameChanged: boolean,
	previousLastTick: number,
	refreshRenderer: boolean,
): Promise<void> {
	renderedStates = new Map();
	if (gameChanged) replayControls.resetTimeManager();
	if (refreshRenderer) {
		await setupRenderer();
		await ensureIcons();
	}
	replayControls.updateReplayBounds(previousLastTick);
	replayControls.setLiveAvailable(
		activeSource === "websocket" && websocketLive,
	);
	updateEndDisplay();
}

function useReplayFile(replay: ReplayData): void {
	const previousGameId = replayLoader?.getGameId();
	const previousLastTick = replayLoader?.getLastTick() ?? -1;
	const gameChanged = previousGameId !== replay.game_id;
	replayLoader = new ReplayLoader(replay, cacheInterval);
	activeSource = "file";
	setWaitingForGame(false);
	warnAboutReplayVersion(replay.misc);
	void refreshReplay(gameChanged, previousLastTick, true);
	if (gameChanged && isDynamicSpeedEnabled()) {
		replayControls.setPlaybackSpeed(
			computeDynamicSpeed(replayLoader.getLastTick() + 1),
		);
	}
}

function useReplayWebSocket(
	gameId: string,
	config: GameConfig,
	misc: ReplayMisc,
): void {
	const previousGameId = replayLoader?.getGameId();
	const previousLastTick = replayLoader?.getLastTick() ?? -1;
	const gameChanged = previousGameId !== gameId;
	const replaceLoader =
		gameChanged || !replayLoader || activeSource !== "websocket";
	if (replaceLoader) {
		replayLoader = ReplayLoader.live(gameId, config, misc, cacheInterval);
		activeSource = "websocket";
		setWaitingForGame(false);
		warnAboutReplayVersion(misc);
	} else {
		replayLoader.updateMisc(misc);
		replayLoader.clearInterrupted();
	}
	void refreshReplay(gameChanged, previousLastTick, replaceLoader);
}

async function loadRemoteFile(filePath: string): Promise<ReplayData> {
	const replay = await fetchReplayFile(filePath);
	remoteFileGameId = replay.game_id;
	return replay;
}

async function pollReplayFile(filePath: string): Promise<boolean> {
	try {
		const check = await checkReplayFile(filePath);
		const gameId = check.gameId ?? check.replay?.game_id ?? null;
		if (!gameId) return false;
		const inputChanged = remoteFileGameId !== gameId;
		remoteFileGameId = gameId;
		const replacesLiveGame =
			activeSource === "websocket" && replayLoader?.getGameId() === gameId;
		if (!inputChanged && !replacesLiveGame) return false;
		useReplayFile(check.replay ?? (await loadRemoteFile(filePath)));
		return true;
	} catch (error) {
		console.debug("Replay file is not available.", error);
		return false;
	}
}

export async function setupReplayLoader(
	filePath: string,
	websocketUrl: string,
	newCacheInterval = 25,
	updateInterval = 3000,
): Promise<void> {
	cacheInterval = newCacheInterval;
	remoteFileGameId = null;
	websocketGameId = null;
	websocketLive = false;
	if (filePoll !== null) window.clearInterval(filePoll);
	stopWebSocket?.();
	setWaitingForGame(replayLoader === null);

	let ready = false;
	let resolveReady = () => {};
	const readyPromise = new Promise<void>((resolve) => {
		resolveReady = resolve;
	});
	const markReady = () => {
		if (!ready) {
			ready = true;
			resolveReady();
		}
	};

	try {
		useReplayFile(await loadRemoteFile(filePath));
		markReady();
	} catch (error) {
		console.debug("Initial replay file is not available.", error);
	}

	stopWebSocket = watchReplayWebSocket(
		websocketUrl,
		{
			onConfig(gameId, config, misc) {
				websocketGameId = gameId;
				const alreadyFinished =
					replayLoader?.getGameId() === gameId &&
					replayLoader.getEndState() === "complete";
				websocketLive = !alreadyFinished;
				if (
					!replayLoader ||
					replayLoader.getGameId() !== gameId ||
					(activeSource === "websocket" && replayLoader.getGameId() === gameId)
				) {
					useReplayWebSocket(gameId, config, misc);
				}
				markReady();
			},
			onTick(gameId, tick, data, durationMs) {
				if (
					activeSource !== "websocket" ||
					replayLoader?.getGameId() !== gameId
				) {
					return;
				}
				const previousLastTick = replayLoader.getLastTick();
				if (replayLoader.appendTick(tick, data, durationMs)) {
					renderedStates = new Map();
					if (tick === 0) refreshTeamDisplay();
					replayControls.updateReplayBounds(previousLastTick);
				}
			},
			onEnd(gameId, misc) {
				if (
					activeSource === "websocket" &&
					replayLoader?.getGameId() === gameId
				) {
					websocketLive = false;
					replayLoader.finish(misc);
					replayControls.finishLiveReplay();
					refreshTeamDisplay();
					updateEndDisplay();
				}
			},
			onDisconnect(gameId, ended) {
				if (websocketGameId === gameId) websocketLive = false;
				if (
					activeSource === "websocket" &&
					replayLoader?.getGameId() === gameId
				) {
					if (!ended) {
						replayLoader.markInterrupted();
						replayControls.setLiveAvailable(false);
						updateEndDisplay();
					}
				}
			},
		},
		WEBSOCKET_RETRY_MS,
	);

	let checkingFile = false;
	filePoll = window.setInterval(async () => {
		if (checkingFile) return;
		checkingFile = true;
		try {
			if (await pollReplayFile(filePath)) markReady();
		} finally {
			checkingFile = false;
		}
	}, updateInterval);

	await readyPromise;
}

export function getStateAt(tick: number): ReplayTick | null {
	if (!replayLoader) {
		throw new Error("Replay is not loaded.");
	}
	if (!renderedStates.has(tick)) {
		const state = replayLoader.getStateAt(tick);
		if (!state) return null;
		renderedStates.set(tick, state);
		if (renderedStates.size > 4) {
			renderedStates.delete(renderedStates.keys().next().value as number);
		}
	}
	return renderedStates.get(tick) ?? null;
}

export function getActionsByExecutor(
	tick: number,
): Record<number, TickAction[]> {
	if (!replayLoader) throw new Error("Replay is not loaded.");
	return replayLoader.getActionsByExecutor(tick);
}

export function getLastReplayTick(): number {
	if (!replayLoader) throw new Error("Replay is not loaded.");
	return Math.max(0, replayLoader.getLastTick());
}

export function getTotalReplayTicks(): number {
	return getLastReplayTick() + 1;
}

export function getLiveTickDuration(targetTick: number): number | undefined {
	return replayLoader?.getLiveDuration(targetTick);
}

export function getReplayEndState(): "complete" | "interrupted" | null {
	return replayLoader?.getEndState() ?? null;
}

export function getGameConfig(): GameConfig | undefined {
	return replayLoader?.getConfig();
}

export function getGameMisc(): ReplayMisc | undefined {
	return replayLoader?.getMisc();
}

export function getReplayJSON(): string {
	if (!replayLoader) throw new Error("Replay is not loaded.");
	return replayLoader.toJSON();
}

export function getWinningTeamFormatted(): string {
	const winner = replayLoader
		?.getMisc()
		?.team_results.find((team) => team.place === 0);
	return winner ? `${winner.name} (ID: ${winner.id})` : "No winning team found";
}

window.addEventListener("drop", (event) => {
	event.preventDefault();
	const file = event.dataTransfer?.files[0];
	if (!file) return;
	const reader = new FileReader();
	reader.readAsText(file);
	reader.addEventListener("load", () => {
		try {
			const replay = parseDroppedReplay(String(reader.result));
			useReplayFile(replay);
		} catch (error) {
			console.error("Could not load dropped replay file.", error);
		}
	});
});

window.addEventListener("dragover", (event) => event.preventDefault());
