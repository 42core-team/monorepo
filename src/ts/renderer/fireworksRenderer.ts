let fireworks: any | null = null;
let renderFireworks = false;
let fireworkStrength = 1;
let fireworkDelay = 2000;
let isActive = false;

function isWindowMinimized(): boolean {
	return window.outerWidth === 0 && window.outerHeight === 0;
}

function shouldRun(): boolean {
	const visible = document.visibilityState === "visible" && !document.hidden;
	return visible && !isWindowMinimized() && renderFireworks;
}

function ensureFireworks(): void {
	if (fireworks) return;
	const globalFW = (window as any).Fireworks;
	const Ctor = globalFW?.default ?? globalFW;
	const container = document.querySelector(".fireworks") as HTMLElement | null;
	if (!Ctor || !container) return;
	fireworks = new Ctor(container, {
		autoresize: true,
		opacity: 0.6,
		acceleration: 1.02,
		friction: 0.98,
		gravity: 0,
		particles: 40,
		traceLength: 2,
		traceSpeed: 6,
		explosion: 10,
		intensity: 60,
		flickering: 30,
		rocketsPoint: { min: 25, max: 75 },
		delay: { min: 1000000000, max: 1000000000 },
		brightness: { min: 50, max: 100 },
		decay: { min: 0.015, max: 0.025 },
	});
}

function applyActivity(): void {
	const nextActive = shouldRun();
	if (nextActive && !isActive) {
		isActive = true;
		ensureFireworks();
		fireworks?.start?.();
	} else if (!nextActive && isActive) {
		isActive = false;
		fireworks?.clear?.();
		fireworks?.stop?.();
	}
}

document.addEventListener("visibilitychange", applyActivity);
window.addEventListener("focus", applyActivity);
window.addEventListener("blur", applyActivity);
window.addEventListener("resize", applyActivity);
window.addEventListener("pageshow", applyActivity);
window.addEventListener("load", applyActivity);
window.addEventListener("pagehide", () => {
	fireworks?.clear?.();
	fireworks?.stop?.();
	isActive = false;
});
window.addEventListener("beforeunload", () => {
	fireworks?.clear?.();
	fireworks?.stop?.();
});

(function loop() {
	if (isActive) {
		ensureFireworks();
		fireworks?.launch?.(fireworkStrength);
		if (fireworkStrength > 1) fireworkStrength--;
	}
	setTimeout(loop, isActive ? fireworkDelay : 1500);
})();

type Mode = "SWISS" | "ELIMINATION" | "QUEUE";

function getFireworkStrengthFromUrlParameters(): number {
	const p = new URLSearchParams(window.location.search);
	const mode = (p.get("mode") || "QUEUE").toUpperCase() as Mode;
	const round = Number(p.get("round"));
	const maxRounds = Number(p.get("maxRounds"));
	if (mode === "QUEUE" || !mode) return 1;
	fireworkDelay = 750;
	if (mode === "SWISS") return 2;
	fireworkDelay = 250;
	if (mode === "ELIMINATION" && round >= maxRounds - 1) return 30;
	fireworkDelay = 500;
	if (mode === "ELIMINATION" && round >= maxRounds - 3) return 5;
	fireworkDelay = 625;
	if (mode === "ELIMINATION") return 3;
	fireworkDelay = 2000;
	return 1;
}

export function setRenderFireworks(render: boolean): void {
	if (!renderFireworks && render) {
		fireworkStrength = getFireworkStrengthFromUrlParameters();
		ensureFireworks();
		fireworks?.clear?.();
	}
	renderFireworks = render;
	applyActivity();
}
