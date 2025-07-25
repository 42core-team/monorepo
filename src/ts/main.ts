import { getStateAt, loadReplayAndKeepUpdated } from './replay_loader/replayLoader.js';

console.log('Debug Visualizer is running!');

await loadReplayAndKeepUpdated('../misc/replay.json');
setInterval(async () => {
	const state = getStateAt(0);
	if (state) {
		console.log('State at tick 0:', state);
	} else {
		console.log('No state found at tick 0');
	}
}, 10000);
