import { parseReplayData, type ReplayData } from "../replay_format/replay";

export type ReplayFileCheck = {
	gameId: string | null;
	replay?: ReplayData;
};

function gameIdFromPrefix(text: string): string | null {
	const match = text.match(/"game_id"\s*:\s*("(?:\\.|[^"\\])*")/);
	if (!match) return null;
	try {
		const gameId = JSON.parse(match[1]);
		return typeof gameId === "string" && gameId ? gameId : null;
	} catch {
		return null;
	}
}

export async function fetchReplayFile(filePath: string): Promise<ReplayData> {
	const response = await fetch(filePath, { cache: "no-store" });
	if (!response.ok) {
		throw new Error(
			`Failed to fetch replay file: ${response.status} ${response.statusText}`,
		);
	}
	return parseReplayData(
		JSON.parse(await response.text()),
		`legacy-file:${filePath}`,
	);
}

export async function checkReplayFile(
	filePath: string,
): Promise<ReplayFileCheck> {
	const response = await fetch(filePath, {
		cache: "no-store",
		headers: { Range: "bytes=0-4095" },
	});
	if (!response.ok) {
		throw new Error(
			`Failed to check replay file: ${response.status} ${response.statusText}`,
		);
	}
	const text = await response.text();
	const gameId = gameIdFromPrefix(text);
	if (response.status !== 206) {
		return {
			gameId,
			replay: parseReplayData(JSON.parse(text), `legacy-file:${filePath}`),
		};
	}
	return { gameId };
}

export function parseDroppedReplay(contents: string): ReplayData {
	return parseReplayData(
		JSON.parse(contents),
		`dropped-file:${crypto.randomUUID()}`,
	);
}
