import { getGameConfig } from '../replay_loader/replayLoader.js';
import { getCurrentTickData } from '../time_manager/timeManager.js';

const svgCanvas = document.getElementById('svg-canvas') as HTMLElement;
const gameConfig = getGameConfig();

function drawFrame(timestamp: number): void {
	window.requestAnimationFrame(drawFrame);
}
window.requestAnimationFrame(drawFrame);

export async function setupRenderer(): Promise<void> {
	if (!gameConfig) {
		throw new Error('Game configuration not found. Cannot set up renderer.');
	}

	const gridSize = gameConfig.width;
	svgCanvas.setAttribute('width', gridSize.toString());
	svgCanvas.setAttribute('height', gridSize.toString());
	svgCanvas.setAttribute('viewBox', `0 0 ${gridSize} ${gridSize}`);

	// populate with chess tiles
	for (let x = 0; x < gridSize; x++) {
		for (let y = 0; y < gridSize; y++) {
			const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
			rect.setAttribute('x', x.toString());
			rect.setAttribute('y', y.toString());
			rect.setAttribute('width', '1');
			rect.setAttribute('height', '1');
			rect.setAttribute('fill', (x + y) % 2 === 0 ? '#f0f0f0' : '#d0d0d0');
			svgCanvas.appendChild(rect);
		}
	}
}
