const svgCanvas = document.getElementById('svg-canvas') as HTMLElement;

window.addEventListener('DOMContentLoaded', async () => {
	let replayFilePath = '/replays/replay_latest.json';
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

	function updateSvgSize() {
		const svgHeight = window.innerHeight - svgCanvas.getBoundingClientRect().top;
		document.documentElement.style.setProperty('--svg-canvas-scale', `${svgHeight - 24}px`);
	}
	window.addEventListener('resize', updateSvgSize);
	window.addEventListener('load', updateSvgSize);
	updateSvgSize();
});
