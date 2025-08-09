declare const Fireworks: any;

const container = document.querySelector('.fireworks');
const fireworks = new Fireworks.default(container, {
	autoresize: true,
	opacity: 0.6,
	acceleration: 1.02,
	friction: 0.98,
	gravity: 0.8,
	particles: 40,
	traceLength: 2,
	traceSpeed: 6,
	explosion: 10,
	intensity: 60,
	flickering: 30,
	rocketsPoint: { min: 25, max: 75 },
	delay: { min: 60, max: 90 }, // control spawn rate
	brightness: { min: 50, max: 100 },
	decay: { min: 0.015, max: 0.025 },
});
fireworks.start();
let renderFireworks = false;
let fireworkStrength = 1;
let isActive = true;

// Helpers
function isWindowMinimized(): boolean {
	return window.outerWidth === 0 && window.outerHeight === 0;
}
function shouldRun(): boolean {
	const visible = document.visibilityState === 'visible' && !document.hidden;
	return visible && !isWindowMinimized() && renderFireworks;
}
function applyActivity(): void {
	const nextActive = shouldRun();

	if (nextActive && !isActive) {
		isActive = true;
		fireworks.start?.();
	} else if (!nextActive && isActive) {
		isActive = false;
		fireworks.clear();
		fireworks.stop?.();
	}
}

// hooks
document.addEventListener('visibilitychange', applyActivity);
window.addEventListener('focus', applyActivity);
window.addEventListener('blur', applyActivity);
window.addEventListener('resize', applyActivity);
window.addEventListener('pagehide', () => {
	fireworks.clear();
	fireworks.stop?.();
	isActive = false;
});
window.addEventListener('beforeunload', () => {
	fireworks.clear();
	fireworks.stop?.();
});

// start setup
(function loop() {
	if (isActive) {
		fireworks.launch(fireworkStrength);
		if (fireworkStrength > 1) fireworkStrength--;
	}
	setTimeout(loop, isActive ? 500 : 1500);
})();

export function setRenderFireworks(render: boolean): void {
	if (renderFireworks == false && render == true) {
		fireworkStrength = 10;
		fireworks.clear();
	}
	renderFireworks = render;
	applyActivity();
}
