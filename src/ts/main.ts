import { getStateAt, loadReplay } from './replay_loader/replayLoader.js';

console.log('Debug Visualizer is running!');

setInterval(async () => {
	await loadReplay('../misc/replay.json');
	const state = getStateAt(0);
	if (state) {
		console.log('State at tick 0:', state);
	} else {
		console.log('No state found at tick 0');
	}
}, 1000);
