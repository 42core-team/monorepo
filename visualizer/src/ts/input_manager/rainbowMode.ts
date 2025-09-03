import { setColorSwitchPreview } from './infoPopupManager.ts';

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
	localStorage.setItem('ui.themeColor', color);
	setColorSwitchPreview(color);

	rafId = requestAnimationFrame(step);
}

export function setupRainbowMode(): void {
	const down = new Set<string>();
	const secret = new Set(["c","o","r","e"]);

	function checkCombo() {
		for (const k of secret) if (!down.has(k)) return false;
		return true;
	}

	window.addEventListener("keydown", (e) => {
		const k = e.key?.toLowerCase();
		down.add(k);
		if (checkCombo()) {
			if (active) {
				disableRainbowIfActive();
			} else {
				active = true;
				startTs = 0;
				rafId = requestAnimationFrame(step);
			}
		}
	});
	window.addEventListener("keyup", (e) => {
		down.delete(e.key?.toLowerCase());
	});

	window.addEventListener("blur", () => down.clear());
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
