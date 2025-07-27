import { getStateAt, getTotalReplayTicks } from '../replay_loader/replayLoader.js';

const playButton = document.getElementById('play-button') as HTMLButtonElement;
const playRevButton = document.getElementById('play-rev-button') as HTMLButtonElement;
const pauseButton = document.getElementById('pause-button') as HTMLButtonElement;

const nextActionButton = document.getElementById('next-action-button') as HTMLButtonElement;
const prevActionButton = document.getElementById('prev-action-button') as HTMLButtonElement;

const nextTickButton = document.getElementById('next-tick-button') as HTMLButtonElement;
const prevTickButton = document.getElementById('prev-tick-button') as HTMLButtonElement;

const skipStartButton = document.getElementById('skip-start-button') as HTMLButtonElement;
const skipEndButton = document.getElementById('skip-end-button') as HTMLButtonElement;

const tickTimelineSlider = document.getElementById('tick-timeline-slider') as HTMLInputElement;
const tickTimelineNumberInput = document.getElementById('tick-timeline-number-input') as HTMLInputElement;

const actionTimelineSlider = document.getElementById('action-timeline-slider') as HTMLInputElement;
const actionTimelineNumberInput = document.getElementById('action-timeline-number-input') as HTMLInputElement;

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
	action: number;
	actionProgress: number; // Fractional progress through current action (0–1)
};

let playing: PlayingStates = PlayingStates.NotPlaying;
let action: number = 0;
let tick: number = 0;
let speedApS: number = 1; // Actions per Second

// Live playback state
let lastTimestamp: number | null = null;
let actionProgress: number = 0; // Fractional progress through current action (0–1)

// Helper functions

function getTotalTicks(): number {
	return getTotalReplayTicks();
}
function getTotalActionsOfCurrentTick(): number {
	const state = getStateAt(tick);
	return state ? state.actions.length : 0;
}

function setTick(tickValue: number) {
	tick = tickValue;
	tickTimelineSlider.value = tick.toString();
	tickTimelineNumberInput.value = tick.toString();
	setAction(0);

	const total = getTotalActionsOfCurrentTick();
	const maxIndex = total > 0 ? total - 1 : 0;
	actionTimelineSlider.max = maxIndex.toString();
	actionTimelineNumberInput.max = maxIndex.toString();
	actionTimelineSlider.disabled = total === 0;
	actionTimelineNumberInput.disabled = total === 0;
}
function setAction(actionValue: number) {
	const total = getTotalActionsOfCurrentTick();
	const maxIndex = total > 0 ? total - 1 : 0;
	action = Math.min(Math.max(actionValue, 0), maxIndex);

	actionTimelineSlider.value = action.toString();
	actionTimelineNumberInput.value = action.toString();
	actionTimelineSlider.max = maxIndex.toString();
	actionTimelineNumberInput.max = maxIndex.toString();
	actionTimelineSlider.disabled = total === 0;
	actionTimelineNumberInput.disabled = total === 0;
}

// External Functions

export async function setupTimeManager() {
	tickTimelineSlider.max = (getTotalTicks() - 1).toString();
	tickTimelineNumberInput.max = (getTotalTicks() - 1).toString();
	setTick(0);
	setAction(0);

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

	nextActionButton.addEventListener('click', () => {
		if (action < getTotalActionsOfCurrentTick() - 1) {
			setAction(action + 1);
		} else {
			let nextTick = tick + 1;
			while (nextTick < getTotalTicks() && getStateAt(nextTick)?.actions.length === 0) {
				nextTick++;
			}
			if (nextTick < getTotalTicks()) {
				setTick(nextTick);
				setAction(0);
			}
		}
	});
	prevActionButton.addEventListener('click', () => {
		if (action > 0) {
			setAction(action - 1);
		} else {
			let prevTick = tick - 1;
			while (prevTick >= 0 && getStateAt(prevTick)?.actions.length === 0) {
				prevTick--;
			}
			if (prevTick >= 0) {
				setTick(prevTick);
				const lastActionIndex = getTotalActionsOfCurrentTick() - 1;
				setAction(lastActionIndex);
			}
		}
	});

	nextTickButton.addEventListener('click', () => {
		if (tick < getTotalTicks() - 1) {
			setTick(tick + 1);
			setAction(0);
		}
	});
	prevTickButton.addEventListener('click', () => {
		if (tick > 0) {
			setTick(tick - 1);
			setAction(0);
		}
	});

	skipStartButton.addEventListener('click', () => {
		setTick(0);
		setAction(0);
	});
	skipEndButton.addEventListener('click', () => {
		const totalTicks = getTotalTicks();
		if (totalTicks > 0) {
			setTick(totalTicks - 1);
			setAction(getTotalActionsOfCurrentTick() - 1);
		}
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

	actionTimelineSlider.addEventListener('input', () => {
		setAction(parseInt(actionTimelineSlider.value, 10));
	});
	actionTimelineNumberInput.addEventListener('input', () => {
		const value = parseInt(actionTimelineNumberInput.value, 10);
		console.log(`Action value input: ${value}`);
		if (!Number.isNaN(value) && value >= 0 && value < getTotalActionsOfCurrentTick()) {
			setAction(value);
		} else {
			console.warn(`Invalid action value: ${value}. Must be between 0 and ${getTotalActionsOfCurrentTick() - 1}`);
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
}

export function getCurrentTckData(): tickData {
	const now = Date.now();
	if (playing === PlayingStates.NotPlaying) {
		lastTimestamp = null;
		return { tick, action, actionProgress: 0 };
	}
	if (lastTimestamp === null) {
		lastTimestamp = now;
		return { tick, action, actionProgress };
	}
	const dt = (now - lastTimestamp) / 1000; // seconds elapsed
	lastTimestamp = now;
	const delta = dt * speedApS * (playing as number);
	actionProgress += delta;

	// Advance forward through actions/ticks
	while (actionProgress >= 1) {
		actionProgress -= 1;
		if (action < getTotalActionsOfCurrentTick() - 1) {
			action++;
		} else {
			let nextTick = tick + 1;
			while (nextTick < getTotalTicks() && getStateAt(nextTick)?.actions.length === 0) {
				nextTick++;
			}
			if (nextTick < getTotalTicks()) {
				setTick(nextTick);
				action = 0;
			} else {
				playing = PlayingStates.NotPlaying;
				actionProgress = 0;
				break;
			}
		}
		setAction(action);
	}

	// Reverse playback through actions/ticks
	while (actionProgress < 0) {
		actionProgress += 1;
		if (action > 0) {
			action--;
		} else {
			let prevTick = tick - 1;
			while (prevTick >= 0 && getStateAt(prevTick)?.actions.length === 0) {
				prevTick--;
			}
			if (prevTick >= 0) {
				setTick(prevTick);
				action = getTotalActionsOfCurrentTick() - 1;
			} else {
				playing = PlayingStates.NotPlaying;
				actionProgress = 0;
				break;
			}
		}
		setAction(action);
	}
	actionProgress = Math.max(0, Math.min(1, actionProgress));

	return { tick, action, actionProgress };
}
