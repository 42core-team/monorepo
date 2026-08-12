import type { GameConfig } from "../replay_format/config";
import type { ReplayMisc, ReplayTick } from "../replay_format/replay";

type WebSocketHandlers = {
	onConfig: (gameId: string, config: GameConfig, misc: ReplayMisc) => void;
	onTick: (
		gameId: string,
		tick: number,
		data: ReplayTick,
		durationMs?: number,
	) => void;
	onEnd: (gameId: string, misc: ReplayMisc) => void;
	onDisconnect: (gameId: string, ended: boolean) => void;
};

type ConfigMessage = {
	type: "config";
	config: GameConfig;
	misc: ReplayMisc;
	last_tick: number;
};

type TickMessage = {
	type: "tick";
	tick: number;
	data: ReplayTick;
};

type EndMessage = {
	type: "end";
	misc: ReplayMisc;
};

export function watchReplayWebSocket(
	url: string,
	handlers: WebSocketHandlers,
	retryIntervalMs: number,
): () => void {
	let stopped = false;
	let socket: WebSocket | null = null;
	let retryTimer: number | null = null;

	const scheduleReconnect = () => {
		if (!stopped && retryTimer === null) {
			retryTimer = window.setTimeout(() => {
				retryTimer = null;
				connect();
			}, retryIntervalMs);
		}
	};

	const connect = () => {
		if (
			stopped ||
			socket?.readyState === WebSocket.OPEN ||
			socket?.readyState === WebSocket.CONNECTING
		) {
			return;
		}

		let gameId = "";
		let ended = false;
		let initialLastTick = -1;
		let lastArrival: number | null = null;
		socket = new WebSocket(url);
		const connectionTimeout = window.setTimeout(() => socket?.close(), 5000);

		socket.addEventListener("open", () => {
			window.clearTimeout(connectionTimeout);
		});
		socket.addEventListener("message", (event) => {
			try {
				const message = JSON.parse(String(event.data)) as
					| ConfigMessage
					| TickMessage
					| EndMessage;
				if (
					message.type === "config" &&
					message.config !== null &&
					typeof message.config === "object" &&
					!Array.isArray(message.config) &&
					message.misc !== null &&
					typeof message.misc === "object" &&
					!Array.isArray(message.misc) &&
					typeof message.misc.game_id === "string" &&
					message.misc.game_id &&
					Number.isInteger(message.last_tick)
				) {
					gameId = message.misc.game_id;
					ended = false;
					initialLastTick = message.last_tick;
					lastArrival = initialLastTick < 0 ? performance.now() : null;
					handlers.onConfig(gameId, message.config, message.misc);
				} else if (
					message.type === "tick" &&
					gameId &&
					Number.isInteger(message.tick) &&
					message.tick >= 0 &&
					message.data !== null &&
					typeof message.data === "object" &&
					!Array.isArray(message.data)
				) {
					const now = performance.now();
					const durationMs =
						message.tick > initialLastTick && lastArrival !== null
							? now - lastArrival
							: undefined;
					if (message.tick >= initialLastTick) lastArrival = now;
					handlers.onTick(gameId, message.tick, message.data, durationMs);
				} else if (
					message.type === "end" &&
					gameId &&
					message.misc !== null &&
					typeof message.misc === "object" &&
					!Array.isArray(message.misc)
				) {
					ended = true;
					handlers.onEnd(gameId, message.misc);
				}
			} catch (error) {
				console.warn("Ignored invalid replay WebSocket message.", error);
			}
		});
		socket.addEventListener("error", () => socket?.close());
		socket.addEventListener("close", () => {
			window.clearTimeout(connectionTimeout);
			socket = null;
			if (!stopped && gameId) handlers.onDisconnect(gameId, ended);
			scheduleReconnect();
		});
	};

	connect();
	return () => {
		stopped = true;
		if (retryTimer !== null) window.clearTimeout(retryTimer);
		socket?.close();
	};
}
