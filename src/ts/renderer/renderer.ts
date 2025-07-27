import type { TickObject } from '../replay_loader/object.js';
import { getGameConfig, getNameOfUnitType, getStateAt } from '../replay_loader/replayLoader.js';
import { getCurrentTickData } from '../time_manager/timeManager.js';

const svgNS = 'http://www.w3.org/2000/svg';
const xlinkNS = 'http://www.w3.org/1999/xlink';

export type AssetTeam = 0 | 1;
const svgAssets = {
	0: {
		0: 'cores/1.svg',
		1: 'cores/2.svg',
	},
	1: {
		Warrior: {
			0: 'units/Warrior/1.svg',
			1: 'units/Warrior/2.svg',
		},
		Miner: {
			0: 'units/Miner/1.svg',
			1: 'units/Miner/2.svg',
		},
		Builder: {
			0: 'units/Builder/1.svg',
			1: 'units/Builder/2.svg',
		},
		Carrier: {
			0: 'units/Carrier/1.svg',
			1: 'units/Carrier/2.svg',
		},
		Bomberman: {
			0: 'units/Bomberman/1.svg',
			1: 'units/Bomberman/2.svg',
		},
	},
	2: 'resource.svg',
	3: 'wall.svg',
	4: 'money.svg',
	5: 'core.svg',
} as const;

const svgCanvas = document.getElementById('svg-canvas') as HTMLElement;
const gameConfig = getGameConfig();

function drawObject(obj: TickObject): void {
	let path: string;
	const teamIndex = ((obj.teamId ?? 1) - 1) as AssetTeam;

	switch (obj.type) {
		case 0: {
			// Core
			const cores = svgAssets[0];
			path = cores[teamIndex];
			break;
		}
		case 1: {
			// Unit
			const units = svgAssets[1];
			const unitType = obj.unit_type;
			if (obj.unit_type === undefined) {
				throw new Error(`Unit object ${obj.id} missing unit_type 🤯`);
			}
			const unitName = getNameOfUnitType(unitType);
			path = units[unitName as keyof typeof units][teamIndex];
			break;
		}
		case 2:
		case 3:
		case 4:
		case 5: {
			path = svgAssets[obj.type as 2 | 3 | 4 | 5];
			break;
		}
	}

	const img = document.createElementNS(svgNS, 'image');
	img.setAttributeNS(xlinkNS, 'href', `/assets/object-svgs/${path}`);
	img.setAttribute('x', `${obj.x}`);
	img.setAttribute('y', `${obj.y}`);
	img.setAttribute('width', '1');
	img.setAttribute('height', '1');
	svgCanvas.appendChild(img);
}

function cleanGrid(): void {
	const nonPersistentElements = svgCanvas.querySelectorAll(':not(.persistent)');

	for (const element of nonPersistentElements) {
		svgCanvas.removeChild(element);
	}
}

function drawFrame(_timestamp: number): void {
	const currentTickData = getCurrentTickData();
	const replayData = getStateAt(currentTickData.tick);
	if (!replayData) {
		console.warn('No replay data available for the current tick.');
		window.requestAnimationFrame(drawFrame);
		return;
	}

	cleanGrid();

	for (const obj of replayData.objects) {
		drawObject(obj);
	}

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
	for (let y = 0; y < gridSize; y++) {
		for (let x = 0; x < gridSize; x++) {
			const rect = document.createElementNS(svgNS, 'rect');
			rect.setAttribute('class', 'persistent');
			rect.setAttribute('x', x.toString());
			rect.setAttribute('y', y.toString());
			rect.setAttribute('width', '1');
			rect.setAttribute('height', '1');
			rect.setAttribute('fill', (x + y) % 2 === 0 ? '#f0f0f0' : '#d0d0d0');
			svgCanvas.appendChild(rect);
		}
	}
}
