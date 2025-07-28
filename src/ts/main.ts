window.addEventListener('DOMContentLoaded', async () => {
	const replayFilePath = '../misc/replay_latest.json';
	const { setupReplayLoader } = await import('./replay_loader/replayLoader.js');
	await setupReplayLoader(replayFilePath);

	const { setupTimeManager } = await import('./time_manager/timeManager.js');
	await setupTimeManager();

	const { setupRenderer } = await import('./renderer/renderer.js');
	await setupRenderer();
});
