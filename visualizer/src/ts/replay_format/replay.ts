import type { TickAction } from "./action";
import type { GameConfig } from "./config";
import type { TickObject } from "./object";

export interface ReplayTick {
	objects?: TickObject[];
	actions?: TickAction[];
}

export interface TeamResult {
	id: number;
	name: string;
	place?: number;
	death_reason?: number;
}

export interface ReplayMisc {
	team_results: TeamResult[];
	version: string;
	game_id?: string;
	worldGeneratorSeed?: string | number;
}

export interface ReplayData {
	game_id: string;
	misc?: ReplayMisc;
	ticks: Record<string, ReplayTick>;
	full_tick_amount: number;
	config: GameConfig;
}

export function parseReplayData(
	value: unknown,
	fallbackGameId: string,
): ReplayData {
	if (!value || typeof value !== "object") {
		throw new Error("Replay data must be an object.");
	}
	const replay = value as Partial<ReplayData> & {
		misc?: ReplayMisc & { game_id?: string };
	};
	if (
		!replay.ticks ||
		typeof replay.ticks !== "object" ||
		typeof replay.full_tick_amount !== "number" ||
		!replay.config ||
		typeof replay.config !== "object" ||
		Array.isArray(replay.config)
	) {
		throw new Error(
			"Replay data is missing config, ticks, or full_tick_amount.",
		);
	}
	const gameId =
		(typeof replay.game_id === "string" && replay.game_id) ||
		(typeof replay.misc?.game_id === "string" && replay.misc.game_id) ||
		fallbackGameId;
	return { ...replay, game_id: gameId } as ReplayData;
}
