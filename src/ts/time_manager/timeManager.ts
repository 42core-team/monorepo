import { getTotalReplayTicks } from '../replay_loader/replayLoader.js';

const playButton = document.getElementById('play-button') as HTMLButtonElement;
const playRevButton = document.getElementById('play-rev-button') as HTMLButtonElement;
const pauseButton = document.getElementById('pause-button') as HTMLButtonElement;

const nextTickButton = document.getElementById('next-tick-button') as HTMLButtonElement;
const prevTickButton = document.getElementById('prev-tick-button') as HTMLButtonElement;

const skipStartButton = document.getElementById('skip-start-button') as HTMLButtonElement;
const skipEndButton = document.getElementById('skip-end-button') as HTMLButtonElement;

const tickTimelineSlider = document.getElementById('tick-timeline-slider') as HTMLInputElement;
const tickTimelineNumberInput = document.getElementById('tick-timeline-number-input') as HTMLInputElement;

const speedSlider = document.getElementById('speed-slider') as HTMLInputElement;
const speedNumberInput = document.getElementById('speed-number-input') as HTMLInputElement;

// Time Tracking Variables

enum PlayingStates {
	Playing = 1,
	NotPlaying = 0,
	Reverse = -1,
}
export type tickData = {
	tick: number;
	tickProgress: number; // Fractional progress through current action (0–1)
};

let playing: PlayingStates = PlayingStates.NotPlaying;
let tick: number = 0;
let speedApS: number = 3; // Actions per Second

// Live playback state
let lastTimestamp: number | null = null;
let tickProgress: number = 0; // Fractional progress through current action (0–1)

// Helper functions

function getTotalTicks(): number {
	return getTotalReplayTicks();
}

function setTick(tickValue: number) {
	tick = tickValue;
	tickTimelineSlider.value = tick.toString();
	tickTimelineNumberInput.value = tick.toString();
}

// External Functions

export async function setupTimeManager() {
	tickTimelineSlider.max = (getTotalTicks() - 1).toString();
	tickTimelineNumberInput.max = (getTotalTicks() - 1).toString();
	setTick(0);

	// Setup event listeners

	playButton.addEventListener('click', () => {
		playing = PlayingStates.Playing;
		lastTimestamp = Date.now(); // Start timing immediately
	});
	playRevButton.addEventListener('click', () => {
		playing = PlayingStates.Reverse;
		lastTimestamp = Date.now(); // Start timing immediately in reverse
	});
	pauseButton.addEventListener('click', () => {
		playing = PlayingStates.NotPlaying;
		lastTimestamp = null;
	});

	nextTickButton.addEventListener('click', () => {
		if (tick < getTotalTicks() - 1) {
			setTick(tick + 1);
		}
	});
	prevTickButton.addEventListener('click', () => {
		if (tick > 0) {
			setTick(tick - 1);
		}
	});

	skipStartButton.addEventListener('click', () => {
		setTick(0);
		tickProgress = 0;
	});
	skipEndButton.addEventListener('click', () => {
		const totalTicks = getTotalTicks();
		if (totalTicks > 0) {
			setTick(totalTicks - 1);
		}
		tickProgress = 1;
	});

	tickTimelineSlider.addEventListener('input', () => {
		setTick(parseInt(tickTimelineSlider.value, 10));
	});
	tickTimelineNumberInput.addEventListener('input', () => {
		const value = parseInt(tickTimelineNumberInput.value, 10);
		if (!Number.isNaN(value) && value >= 0 && value < getTotalTicks()) {
			setTick(value);
		}
	});

	speedSlider.addEventListener('input', () => {
		const value = parseFloat(speedSlider.value);
		if (!Number.isNaN(value) && value > 0) {
			speedApS = value;
			speedNumberInput.value = speedApS.toString();
		}
	});
	speedNumberInput.addEventListener('input', () => {
		const value = parseFloat(speedNumberInput.value);
		if (!Number.isNaN(value) && value > 0) {
			speedApS = value;
			speedSlider.value = speedApS.toString();
		}
	});

	window.addEventListener('keydown', (event) => {
		switch (event.key) {
			case ' ':
				if (playing === PlayingStates.NotPlaying) {
					playButton.click();
				} else {
					pauseButton.click();
				}
			case 'r':
			case 's':
				skipStartButton.click();
				break;
			case 'e':
				skipEndButton.click();
				break;
			case 'ArrowRight':
				nextTickButton.click();
				break;
			case 'ArrowLeft':
				prevTickButton.click();
				break;
			case 'ArrowUp':
				if (speedApS >= 20) break;
				speedNumberInput.value = (parseFloat(speedNumberInput.value) + 0.5).toString();
				speedSlider.value = speedNumberInput.value;
				speedApS = parseFloat(speedNumberInput.value);
				break;
			case 'ArrowDown':
				if (speedApS < 1) break;
				speedNumberInput.value = (parseFloat(speedNumberInput.value) - 0.5).toString();
				speedSlider.value = speedNumberInput.value;
				speedApS = parseFloat(speedNumberInput.value);
				break;
		}
	});
}

export function getCurrentTickData(): tickData {
	const now = Date.now();
	if (playing === PlayingStates.NotPlaying) {
		lastTimestamp = null;
		return { tick, tickProgress: 0 };
	}
	if (lastTimestamp === null) {
		lastTimestamp = now;
		return { tick, tickProgress };
	}
	const dt = (now - lastTimestamp) / 1000; // seconds elapsed
	lastTimestamp = now;
	const delta = dt * speedApS * (playing as number);
	tickProgress += delta;

	if (tickProgress > 1) {
		if (tick < getTotalTicks() - 1) {
			const newTick = Math.min(tick + Math.floor(tickProgress), getTotalTicks() - 1);
			setTick(newTick);
			tickProgress = tickProgress % 1;
		} else {
			playing = PlayingStates.NotPlaying;
			tickProgress = 1;
		}
	}
	// TODO: this is buggy reversing will loop first tick and never hit else
	if (tickProgress < 0) {
		if (tick > 0) {
			const newTick = Math.max(tick + Math.ceil(tickProgress), 0);
			setTick(newTick);
			tickProgress = (tickProgress % 1) + 1;
		} else {
			playing = PlayingStates.NotPlaying;
			tickProgress = 0;
		}
	}

	return { tick, tickProgress };
}

export function resetTimeManager() {
	setTick(0);
	lastTimestamp = null;
	tickProgress = 0;
	tickTimelineSlider.max = (getTotalTicks() - 1).toString();
	tickTimelineSlider.value = '0';
	tickTimelineNumberInput.max = (getTotalTicks() - 1).toString();
	tickTimelineNumberInput.value = '0';
}
