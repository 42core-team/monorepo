import { setColorSwitchPreview } from "./infoPopupManager.ts";

let rafId: number | null = null;
let startTs = 0;
let active = false;

function step(timestamp: number) {
	if (!active) return;
	if (!startTs) startTs = timestamp;

	const t = (timestamp - startTs) / 1000;
	const hue = (t * 100) % 360;
	const color = `hsl(${hue} 100% 75%)`;
	document.documentElement.style.setProperty("--theme-color", color);
	localStorage.setItem("ui.themeColor", color);
	setColorSwitchPreview(color);

	rafId = requestAnimationFrame(step);
}

export function setupRainbowMode(): void {
	const sequence = ["KeyC", "KeyO", "KeyR", "KeyE"];
	let nextKeyToBeTyped = 0;
	let lastKeyHitTimestamp = 0;

	const TIMEOUT_MS = 800; // reset if pause too long between keys

	function reset() {
		nextKeyToBeTyped = 0;
		lastKeyHitTimestamp = 0;
	}

	window.addEventListener("keydown", (e) => {
		if (e.repeat) return;
		if (!e.code) return;

		const now = performance.now();
		if (nextKeyToBeTyped > 0 && now - lastKeyHitTimestamp > TIMEOUT_MS) reset();
		lastKeyHitTimestamp = now;

		if (e.code === sequence[nextKeyToBeTyped]) {
			nextKeyToBeTyped += 1;
			if (nextKeyToBeTyped === sequence.length) {
				reset();

				if (active) {
					disableRainbowIfActive();
				} else {
					active = true;
					startTs = 0;
					rafId = requestAnimationFrame(step);
				}
			}
			return;
		}

		reset();
	});

	window.addEventListener("blur", reset);
	document.addEventListener("visibilitychange", () => {
		if (document.visibilityState !== "visible") reset();
	});
}

export function disableRainbowIfActive(): void {
	if (active) {
		active = false;
		if (rafId != null) {
			cancelAnimationFrame(rafId);
			rafId = null;
		}
	}
}

export function isRainbowActive(): boolean {
	return active;
}
