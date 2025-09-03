import { setupInfoPopupManager } from "./input_manager/infoPopupManager.js";
import { loadSavedTheme } from "./input_manager/themeManager.js";
import { setupRainbowMode } from "./input_manager/rainbowMode.js";

const svgCanvas = document.getElementById("svg-canvas") as HTMLElement;

window.addEventListener("DOMContentLoaded", async () => {
	// load url parameters

	let replayFilePath = "/replays/replay_latest.json";
	const urlParams = new URLSearchParams(window.location.search);
	if (urlParams.has("replay")) {
		replayFilePath = urlParams.get("replay") || replayFilePath;
	}

	const speedParam = urlParams.get("speed");
	if (speedParam) {
		const v = parseFloat(speedParam);
		if (!Number.isNaN(v)) localStorage.setItem("tm.speed", String(v));
	}

	const autoplayRaw = urlParams.getAll("autoplay");
	const autoplayList = autoplayRaw
		.flatMap((s) => s.split(","))
		.map((s) => s.trim())
		.filter(Boolean);

	// project imports

	const { setupReplayLoader } = await import("./replay_loader/replayLoader.js");
	const { setupTimeManager, startPlayback, isAtEnd } = await import(
		"./input_manager/timeManager.js"
	);
	const { setupRenderer } = await import("./renderer/renderer.js");

	// autoplay enabled

	if (autoplayList.length > 0) {
		await setupReplayLoader(autoplayList[0]);
		await setupTimeManager();
		await setupRenderer();
		startPlayback();

		let idx = 0;
		const watchAndAdvance = () => {
			const timer = setInterval(() => {
				if (isAtEnd()) {
					clearInterval(timer);
					setTimeout(async () => {
						idx = (idx + 1) % autoplayList.length;
						await setupReplayLoader(autoplayList[idx]);
						startPlayback();
						watchAndAdvance();
					}, 5000);
				}
			}, 400);
		};
		watchAndAdvance();
	} else {
		// normal, manual playback

		await setupReplayLoader(replayFilePath);
		await setupTimeManager();
		await setupRenderer();
	}

	// svg layout height renderer

	function updateSvgSize() {
		const svgTop = svgCanvas.getBoundingClientRect().top;
		const raw = window.innerHeight - svgTop - 10; // 10px as arbitrary padding to bottom

		const gridSize = Number(getComputedStyle(document.documentElement)
			.getPropertyValue("--grid-size").trim()) || 1;

		// set size to a multiple of gridSize to avoid tiny gaps between object through breaking seams because of floating point calculation imprecision
		const cell = Math.max(1, Math.floor(raw / gridSize));
		const snapped = cell * gridSize;

		document.documentElement.style.setProperty("--svg-canvas-scale", `${snapped}px`);
	}
	window.addEventListener("resize", updateSvgSize);
	window.addEventListener("load", updateSvgSize);
	updateSvgSize();

	// theme
	loadSavedTheme();

	// info popup manager
	setupInfoPopupManager();

	// setup rainbow mode
	setupRainbowMode();
});
