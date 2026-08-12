import { setRenderFireworks } from "../renderer/fireworksRenderer";
import {
	getLastReplayTick,
	getLiveTickDuration,
	getReplayEndState,
} from "../replay_loader/replayLoader";
import { initDoubleSpeedHandler } from "./spaceHandler";
import { toggleTheme } from "./themeManager";

const playButton = document.getElementById(
	"play-pause-button",
) as HTMLButtonElement;
const nextTickButton = document.getElementById(
	"next-tick-button",
) as HTMLButtonElement;
const prevTickButton = document.getElementById(
	"prev-tick-button",
) as HTMLButtonElement;
const skipStartButton = document.getElementById(
	"skip-start-button",
) as HTMLButtonElement;
const skipEndButton = document.getElementById(
	"skip-end-button",
) as HTMLButtonElement;
const tickTimelineSlider = document.getElementById(
	"tick-timeline-slider",
) as HTMLInputElement;
const tickTimelineNumberInput = document.getElementById(
	"tick-timeline-number-input",
) as HTMLInputElement;
const speedSlider = document.getElementById("speed-slider") as HTMLInputElement;
const speedNumberInput = document.getElementById(
	"speed-number-input",
) as HTMLInputElement;
const speedDownButton = document.getElementById(
	"speed-down-button",
) as HTMLButtonElement;
const speedUpButton = document.getElementById(
	"speed-up-button",
) as HTMLButtonElement;
const winnerDisplay = document.getElementById(
	"win-display-box",
) as HTMLDivElement;
const fullscreenToggleButton = document.getElementById(
	"fullscreen-toggle-button",
) as HTMLButtonElement;
const liveIndicator = document.getElementById(
	"live-indicator",
) as HTMLButtonElement;

const minSpeed = 0.5;
export const maxSpeed = 50;
const speedIncrement = 0.5;
const fallbackLiveTickMs = 250;

export type tickData = {
	tick: number;
	tickProgress: number;
};

let playing = false;
let tick = 0;
let speedApS = 5;
let renderDirty = true;
let lastTimestamp: number | null = null;
let tickProgress = 0;
let liveAvailable = false;
let liveMode = false;

function getLastTick(): number {
	return getLastReplayTick();
}

function setTick(value: number): void {
	tick = value;
	const displayed = liveMode ? getLastTick() : tick;
	tickTimelineSlider.value = String(displayed);
	tickTimelineNumberInput.value = String(displayed);
	renderDirty = true;
}

function updateDisplayedTick(): void {
	const displayed = liveMode
		? getLastTick()
		: Math.min(getLastTick(), tick + (tickProgress > 0.5 ? 1 : 0));
	tickTimelineSlider.value = String(displayed);
	tickTimelineNumberInput.value = String(displayed);
}

function setPlaying(value: boolean): void {
	playing = value;
	const icon = document.getElementById("playPauseIcon") as HTMLImageElement;
	icon.src = playing ? "/assets/ui-svgs/pause.svg" : "/assets/ui-svgs/play.svg";
}

function setLiveMode(enabled: boolean, jumpToEnd = true): void {
	liveMode = enabled && liveAvailable;
	liveIndicator.classList.toggle("active", liveMode);
	liveIndicator.setAttribute("aria-pressed", String(liveMode));
	for (const control of [
		speedSlider,
		speedNumberInput,
		speedDownButton,
		speedUpButton,
	]) {
		control.disabled = liveMode;
	}
	if (liveMode && jumpToEnd) {
		setTick(getLastTick());
		tickProgress = 1;
		setPlaying(true);
	}
	lastTimestamp = performance.now();
	renderDirty = true;
	updateDisplayedTick();
}

function leaveLiveMode(): void {
	if (liveMode) setLiveMode(false, false);
}

export function setLiveAvailable(available: boolean): void {
	const wasLive = liveMode;
	liveAvailable = available;
	liveIndicator.hidden = !available;
	if (!available) {
		setLiveMode(false, false);
		if (wasLive) setPlaying(false);
	}
}

export function finishLiveReplay(): void {
	liveAvailable = false;
	liveIndicator.hidden = true;
	if (!liveMode) updateDisplayedTick();
}

export function isAtEnd(): boolean {
	return (
		getReplayEndState() !== null && tick === getLastTick() && tickProgress >= 1
	);
}

export function startPlayback(): void {
	if (isAtEnd()) resetTimeManager();
	setPlaying(true);
	if (liveAvailable && tick === getLastTick()) setLiveMode(true);
	lastTimestamp = performance.now();
}

export function pausePlayback(): void {
	setPlaying(false);
	leaveLiveMode();
}

export function isPlaying(): boolean {
	return playing;
}

export function setPlaybackSpeed(newSpeed: number): void {
	const stepped = Math.round(newSpeed / speedIncrement) * speedIncrement;
	speedApS = Math.min(maxSpeed, Math.max(minSpeed, stepped));
	speedSlider.value = String(speedApS);
	speedNumberInput.value = String(speedApS);
	localStorage.setItem("tm.speed", String(speedApS));
	renderDirty = true;
}

export function getPlaybackSpeed(): number {
	return speedApS;
}

function isFullscreen(): boolean {
	return Boolean(
		document.fullscreenElement || document.webkitFullscreenElement,
	);
}

async function enterFullscreen(): Promise<void> {
	const request =
		document.documentElement.requestFullscreen ||
		document.documentElement.webkitRequestFullscreen;
	try {
		await request?.call(document.documentElement);
	} catch {}
}

async function exitFullscreen(): Promise<void> {
	const exit = document.exitFullscreen || document.webkitExitFullscreen;
	try {
		await exit?.call(document);
	} catch {}
}

function updateFullscreenUI(): void {
	const icon = document.getElementById(
		"fullscreen-icon",
	) as HTMLImageElement | null;
	const active = isFullscreen();
	if (icon) {
		icon.src = active
			? "/assets/ui-svgs/fullscreen-close.svg"
			: "/assets/ui-svgs/fullscreen-open.svg";
		icon.alt = active ? "Exit Fullscreen" : "Enter Fullscreen";
	}
	fullscreenToggleButton?.setAttribute("aria-pressed", String(active));
}

function toggleFullscreen(): void {
	if (isFullscreen()) void exitFullscreen();
	else void enterFullscreen();
}

export async function setupTimeManager(): Promise<void> {
	const savedSpeed = parseFloat(localStorage.getItem("tm.speed") || "");
	if (!Number.isNaN(savedSpeed)) setPlaybackSpeed(savedSpeed);
	resetTimeManager();

	const setSpeed = (value: number) => setPlaybackSpeed(value);

	playButton.addEventListener("click", () => {
		if (Math.floor(Math.random() * 420) === 0) {
			window.location.href =
				"https://www.youtube.com/embed/dQw4w9WgXcQ?autoplay=1&mute=1&controls=0&loop=1&playlist=dQw4w9WgXcQ&rel=0&modestbranding=1&playsinline=1";
			return;
		}
		if (playing) pausePlayback();
		else startPlayback();
	});

	nextTickButton.addEventListener("click", () => {
		leaveLiveMode();
		if (tick < getLastTick()) {
			setTick(tick + 1);
			tickProgress = 0;
		}
	});
	prevTickButton.addEventListener("click", () => {
		leaveLiveMode();
		if (tick > 0) {
			setTick(tick - 1);
			tickProgress = 0;
		}
	});
	skipStartButton.addEventListener("click", () => {
		leaveLiveMode();
		setTick(0);
		tickProgress = 0;
	});
	skipEndButton.addEventListener("click", () => {
		leaveLiveMode();
		setTick(getLastTick());
		tickProgress = 1;
	});

	const seek = (value: string) => {
		leaveLiveMode();
		const parsed = parseInt(value, 10);
		if (!Number.isNaN(parsed)) {
			setTick(Math.min(getLastTick(), Math.max(0, parsed)));
			tickProgress = 0;
		}
	};
	tickTimelineSlider.addEventListener("input", () =>
		seek(tickTimelineSlider.value),
	);
	tickTimelineNumberInput.addEventListener("input", () =>
		seek(tickTimelineNumberInput.value),
	);

	speedSlider.addEventListener("input", () => {
		const value = parseFloat(speedSlider.value);
		if (!Number.isNaN(value)) setSpeed(value);
	});
	speedNumberInput.addEventListener("input", () => {
		const value = parseFloat(speedNumberInput.value);
		if (!Number.isNaN(value)) setSpeed(value);
	});
	speedUpButton.addEventListener("click", () =>
		setSpeed(speedApS + speedIncrement),
	);
	speedDownButton.addEventListener("click", () =>
		setSpeed(speedApS - speedIncrement),
	);

	const keyBindings: Record<
		string,
		{ action: () => void; button?: HTMLButtonElement }
	> = {
		r: { action: () => skipStartButton.click(), button: skipStartButton },
		s: { action: () => skipStartButton.click(), button: skipStartButton },
		e: { action: () => skipEndButton.click(), button: skipEndButton },
		ArrowRight: {
			action: () => nextTickButton.click(),
			button: nextTickButton,
		},
		ArrowLeft: {
			action: () => prevTickButton.click(),
			button: prevTickButton,
		},
		ArrowUp: {
			action: () => setSpeed(speedApS + speedIncrement),
			button: speedUpButton,
		},
		ArrowDown: {
			action: () => setSpeed(speedApS - speedIncrement),
			button: speedDownButton,
		},
		f: { action: toggleFullscreen, button: fullscreenToggleButton },
		t: { action: toggleTheme },
		g: {
			action: () => document.getElementById("gridlines-toggle-button")?.click(),
		},
	};

	window.addEventListener("keydown", (event) => {
		if (event.ctrlKey || event.metaKey || event.altKey) return;
		if (
			["INPUT", "TEXTAREA", "SELECT"].includes(
				(event.target as HTMLElement).tagName,
			)
		) {
			return;
		}
		const binding = keyBindings[event.key];
		if (!binding) return;
		binding.button?.classList.add("active");
		window.setTimeout(() => binding.button?.classList.remove("active"), 100);
		binding.action();
		event.preventDefault();
	});

	window.addEventListener("pageshow", () => updateReplayBounds(getLastTick()));
	fullscreenToggleButton?.addEventListener("click", toggleFullscreen);
	document.addEventListener("fullscreenchange", updateFullscreenUI);
	document.addEventListener("webkitfullscreenchange", updateFullscreenUI);
	updateFullscreenUI();
	initDoubleSpeedHandler();
	liveIndicator.addEventListener("click", () => {
		if (liveMode) pausePlayback();
		else setLiveMode(true);
	});
}

function advanceLive(elapsedMs: number): void {
	let remainingMs = elapsedMs;
	while (tick < getLastTick() && remainingMs > 0) {
		const duration = getLiveTickDuration(tick + 1) ?? fallbackLiveTickMs;
		const needed = (1 - tickProgress) * duration;
		if (remainingMs < needed) {
			tickProgress += remainingMs / duration;
			remainingMs = 0;
		} else {
			remainingMs -= needed;
			setTick(tick + 1);
			tickProgress = tick === getLastTick() ? 1 : 0;
		}
	}
}

export function getCurrentTickData(): tickData {
	const endState = getReplayEndState();
	const atEnd =
		endState !== null && tick === getLastTick() && tickProgress >= 1;
	if (winnerDisplay) {
		document
			.querySelectorAll<HTMLElement>(".win-display")
			.forEach((element) => {
				element.style.display = atEnd ? "block" : "none";
			});
		setRenderFireworks(atEnd && endState === "complete");
	}

	const now = performance.now();
	if (!playing) {
		lastTimestamp = null;
		updateDisplayedTick();
		return { tick, tickProgress };
	}
	if (lastTimestamp === null) {
		lastTimestamp = now;
		updateDisplayedTick();
		return { tick, tickProgress };
	}

	const elapsedMs = now - lastTimestamp;
	lastTimestamp = now;
	renderDirty = true;
	if (liveMode) {
		advanceLive(elapsedMs);
		if (!liveAvailable && tick === getLastTick() && tickProgress >= 1) {
			setLiveMode(false, false);
			setPlaying(false);
		}
	} else {
		tickProgress += (elapsedMs / 1000) * speedApS;
		while (tickProgress >= 1) {
			if (tick < getLastTick()) {
				setTick(tick + 1);
				tickProgress -= 1;
			} else if (liveAvailable) {
				setLiveMode(true, false);
				tickProgress = 1;
				break;
			} else {
				setPlaying(false);
				tickProgress = 1;
				break;
			}
		}
		if (tick === getLastTick() && liveAvailable && playing && !liveMode) {
			setLiveMode(true, false);
			tickProgress = 1;
		}
	}

	updateDisplayedTick();
	return { tick, tickProgress };
}

export function isDirty(): boolean {
	return renderDirty;
}

export function updateReplayBounds(previousLastTick: number): void {
	const lastTick = getLastTick();
	tickTimelineSlider.max = String(lastTick);
	tickTimelineNumberInput.max = String(lastTick);
	if (tick > lastTick) {
		setTick(lastTick);
		tickProgress = 1;
	}
	if (liveMode && lastTick > previousLastTick && tick >= previousLastTick) {
		setTick(Math.max(0, previousLastTick));
		tickProgress = 0;
		lastTimestamp = performance.now();
	}
	renderDirty = true;
}

export function resetTimeManager(): void {
	setPlaying(false);
	setLiveMode(false, false);
	setTick(0);
	lastTimestamp = null;
	tickProgress = 0;
	tickTimelineSlider.max = String(getLastTick());
	tickTimelineNumberInput.max = String(getLastTick());
}
