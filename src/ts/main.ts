window.addEventListener('DOMContentLoaded', async () => {
	let replayFilePath = '../misc/replay_latest.json';
	const urlParams = new URLSearchParams(window.location.search);
	if (urlParams.has('replay')) {
		replayFilePath = urlParams.get('replay') || replayFilePath;
	}

	const { setupReplayLoader } = await import('./replay_loader/replayLoader.js');
	await setupReplayLoader(replayFilePath);

	const { setupTimeManager } = await import('./time_manager/timeManager.js');
	await setupTimeManager();

	const { setupRenderer } = await import('./renderer/renderer.js');
	await setupRenderer();
});
