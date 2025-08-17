import { getTotalReplayTicks } from '../replay_loader/replayLoader.js';
import { setRenderFireworks } from '../renderer/fireworksRenderer.js';

const playButton = document.getElementById('play-pause-button') as HTMLButtonElement;

const nextTickButton = document.getElementById('next-tick-button') as HTMLButtonElement;
const prevTickButton = document.getElementById('prev-tick-button') as HTMLButtonElement;

const skipStartButton = document.getElementById('skip-start-button') as HTMLButtonElement;
const skipEndButton = document.getElementById('skip-end-button') as HTMLButtonElement;

const tickTimelineSlider = document.getElementById('tick-timeline-slider') as HTMLInputElement;
const tickTimelineNumberInput = document.getElementById('tick-timeline-number-input') as HTMLInputElement;

const speedSlider = document.getElementById('speed-slider') as HTMLInputElement;
const speedNumberInput = document.getElementById('speed-number-input') as HTMLInputElement;
const speedDownButton = document.getElementById('speed-down-button') as HTMLButtonElement;
const speedUpButton = document.getElementById('speed-up-button') as HTMLButtonElement;

const winnerDisplay = document.getElementById('win-display-box') as HTMLDivElement;

// consts

const minSpeed = 0.5;
const maxSpeed = 20;
const speedIncrement = 0.5;

// Time Tracking Variables

export type tickData = {
	tick: number;
	tickProgress: number; // Fractional progress through current action (0–1)
};

let playing: boolean = false;
let tick: number = 0;
let speedApS: number = 5; // Actions per Second
let renderDirty: boolean = true; // true when a rendering-update worthy change has occurred

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
	renderDirty = true;
}
function setPlaying(isPlaying: boolean) {
	playing = isPlaying;
	const playPauseIcon = document.getElementById('playPauseIcon') as HTMLImageElement;
	if (playPauseIcon) {
		playPauseIcon.src = playing ? '/assets/ui-svgs/pause.svg' : '/assets/ui-svgs/play.svg';
	}
}

// External Functions

export async function setupTimeManager() {
	const total = Math.max(1, getTotalTicks());
	const s = parseFloat(localStorage.getItem('tm.speed') || '');
	if (!Number.isNaN(s)) {
		const stepped = Math.round(s / speedIncrement) * speedIncrement;
		speedApS = Math.min(maxSpeed, Math.max(minSpeed, stepped));
	}
	tick = 0;
	tickTimelineSlider.max = String(total - 1);
	tickTimelineNumberInput.max = String(total - 1);
	tickTimelineSlider.value = '0';
	tickTimelineNumberInput.value = '0';
	speedSlider.value = String(speedApS);
	speedNumberInput.value = String(speedApS);

	function setSpeedLocal(v: number) {
		const stepped = Math.round(v / speedIncrement) * speedIncrement;
		speedApS = Math.min(maxSpeed, Math.max(minSpeed, stepped));
		speedSlider.value = String(speedApS);
		speedNumberInput.value = String(speedApS);
		localStorage.setItem('tm.speed', String(speedApS));
		renderDirty = true;
	}

	playButton.addEventListener('click', () => {
		if (Math.floor(Math.random() * 100) === 0) {
			window.location.href = 'https://www.youtube.com/embed/dQw4w9WgXcQ?autoplay=1&mute=1&controls=0&loop=1&playlist=dQw4w9WgXcQ&rel=0&modestbranding=1&playsinline=1';
			return;
		}

		playing = !playing;
		const playPauseIcon = document.getElementById('playPauseIcon') as HTMLImageElement;
		if (playPauseIcon) playPauseIcon.src = playing ? '/assets/ui-svgs/pause.svg' : '/assets/ui-svgs/play.svg';
		lastTimestamp = playing ? Date.now() : null;
	});

	nextTickButton.addEventListener('click', () => {
		if (tick < total - 1) {
			tick += 1;
			tickProgress = 0;
			tickTimelineSlider.value = String(tick);
			tickTimelineNumberInput.value = String(tick);
			renderDirty = true;
		}
	});
	prevTickButton.addEventListener('click', () => {
		if (tick > 0) {
			tick -= 1;
			tickProgress = 0;
			tickTimelineSlider.value = String(tick);
			tickTimelineNumberInput.value = String(tick);
			renderDirty = true;
		}
	});
	skipStartButton.addEventListener('click', () => {
		tick = 0;
		tickProgress = 0;
		tickTimelineSlider.value = '0';
		tickTimelineNumberInput.value = '0';
		renderDirty = true;
	});
	skipEndButton.addEventListener('click', () => {
		tick = total - 1;
		tickProgress = 1;
		tickTimelineSlider.value = String(tick);
		tickTimelineNumberInput.value = String(tick);
		renderDirty = true;
	});

	tickTimelineSlider.addEventListener('input', () => {
		const v = Math.min(total - 1, Math.max(0, parseInt(tickTimelineSlider.value, 10)));
		if (!Number.isNaN(v)) {
			tick = v;
			tickTimelineSlider.value = String(tick);
			tickTimelineNumberInput.value = String(tick);
			tickProgress = 0;
			renderDirty = true;
		}
	});
	tickTimelineNumberInput.addEventListener('input', () => {
		const v = Math.min(total - 1, Math.max(0, parseInt(tickTimelineNumberInput.value, 10)));
		if (!Number.isNaN(v)) {
			tick = v;
			tickTimelineSlider.value = String(tick);
			tickTimelineNumberInput.value = String(tick);
			tickProgress = 0;
			renderDirty = true;
		}
	});

	speedSlider.addEventListener('input', () => {
		const v = parseFloat(speedSlider.value);
		if (!Number.isNaN(v)) setSpeedLocal(v);
	});
	speedNumberInput.addEventListener('input', () => {
		const v = parseFloat(speedNumberInput.value);
		if (!Number.isNaN(v)) setSpeedLocal(v);
	});
	speedUpButton.addEventListener('click', () => setSpeedLocal(speedApS + speedIncrement));
	speedDownButton.addEventListener('click', () => setSpeedLocal(speedApS - speedIncrement));

	const keyBindings: Record<string, { action: () => void; button?: HTMLButtonElement }> = {
		' ': { action: () => playButton.click(), button: playButton },
		r: { action: () => skipStartButton.click(), button: skipStartButton },
		s: { action: () => skipStartButton.click(), button: skipStartButton },
		e: { action: () => skipEndButton.click(), button: skipEndButton },
		ArrowRight: { action: () => nextTickButton.click(), button: nextTickButton },
		ArrowLeft: { action: () => prevTickButton.click(), button: prevTickButton },
		ArrowUp: { action: () => setSpeedLocal(speedApS + speedIncrement), button: speedUpButton },
		ArrowDown: { action: () => setSpeedLocal(speedApS - speedIncrement), button: speedDownButton },
	};

	window.addEventListener('keydown', (event) => {
		if (event.ctrlKey || event.metaKey || event.altKey) return;
		if (['INPUT', 'TEXTAREA', 'SELECT'].includes((event.target as HTMLElement).tagName)) return;
		const binding = keyBindings[event.key];
		if (!binding) return;
		if (binding.button) {
			binding.button.classList.add('active');
			setTimeout(() => {
				if (binding.button) binding.button.classList.remove('active');
			}, 100);
		}
		binding.action();
		event.preventDefault();
	});

	window.addEventListener('pageshow', () => {
		const max = Math.max(1, getTotalTicks()) - 1;
		if (tick > max) tick = 0;
		tickTimelineSlider.max = String(max);
		tickTimelineNumberInput.max = String(max);
		tickTimelineSlider.value = String(tick);
		tickTimelineNumberInput.value = String(tick);
		speedSlider.value = String(speedApS);
		speedNumberInput.value = String(speedApS);
	});
}

export function getCurrentTickData(): tickData {
	if (winnerDisplay) {
		const total = getTotalReplayTicks();
		const atEnd = total === 0 || tick === total - 1;

		document.querySelectorAll<HTMLElement>('.win-display').forEach((elem) => {
			elem.style.display = atEnd ? 'block' : 'none';
		});

		setRenderFireworks(atEnd);
	}

	const now = Date.now();
	if (!playing) {
		lastTimestamp = null;
		return { tick, tickProgress };
	}
	if (lastTimestamp === null) {
		lastTimestamp = now;
		return { tick, tickProgress };
	}
	const dt = (now - lastTimestamp) / 1000; // seconds elapsed
	lastTimestamp = now;
	const delta = dt * speedApS * (playing ? 1 : 0);
	if (delta !== 0) renderDirty = true;
	tickProgress += delta;

	if (tickProgress > 1) {
		if (tick < getTotalTicks() - 1) {
			const newTick = Math.min(tick + Math.floor(tickProgress), getTotalTicks() - 1);
			setTick(newTick);
			tickProgress = tickProgress % 1;
		} else {
			setPlaying(false);
			tickProgress = 1;
		}
	}

	return { tick, tickProgress };
}

export function isDirty(): boolean {
	return renderDirty;
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
