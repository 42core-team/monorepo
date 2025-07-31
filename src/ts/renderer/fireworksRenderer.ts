declare const Fireworks: any;

console.log('loading fireworks renderer');

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
	explosion: 4,
	intensity: 25,
	flickering: 30,
	delay: { min: 60, max: 90 }, // control spawn rate
	brightness: { min: 50, max: 70 },
	decay: { min: 0.015, max: 0.025 },
});
fireworks.start();

(function loop() {
	console.group('fireworks loop');
	fireworks.launch(1);
	setTimeout(loop, 250);
})();
