window.addEventListener('DOMContentLoaded', async () => {
	const { setupReplayLoader } = await import('./replay_loader/replayLoader.js');
	const { setupTimeManager } = await import('./time_manager/timeManager.js');

	const replayFilePath = '../misc/replay.json';

	await setupReplayLoader(replayFilePath);
	await setupTimeManager();
});
