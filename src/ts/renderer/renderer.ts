import { GameConfig } from '../replay_loader/config.js';
import { formatObjectData, getBarMetrics, type TickObject } from '../replay_loader/object.js';
import { getGameConfig, getGridMessageOverride, getNameOfUnitType, getStateAt } from '../replay_loader/replayLoader.js';
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
			0: 'units/warrior/1.svg',
			1: 'units/warrior/2.svg',
		},
		Miner: {
			0: 'units/miner/1.svg',
			1: 'units/miner/2.svg',
		},
		Builder: {
			0: 'units/builder/1.svg',
			1: 'units/builder/2.svg',
		},
		Carrier: {
			0: 'units/carrier/1.svg',
			1: 'units/carrier/2.svg',
		},
		Bomberman: {
			0: 'units/bomberman/1.svg',
			1: 'units/bomberman/2.svg',
		},
	},
	2: 'resource.svg',
	3: 'wall.svg',
	4: 'money.svg',
	5: 'core.svg',
} as const;

const svgCanvasElement = document.getElementById('svg-canvas');
if (!svgCanvasElement || !(svgCanvasElement instanceof SVGSVGElement)) {
	throw new Error('SVG canvas element not found or is not an SVG element');
}
const svgCanvas = svgCanvasElement as SVGSVGElement;
const tooltipElement = document.getElementById('tooltip') as HTMLDivElement;
let gameConfig: GameConfig | undefined;

function drawObject(obj: TickObject, xOffset: number = 0, yOffset: number = 0, scaleFactor: number = 1): void {
	if (!gameConfig) {
		throw new Error('Game configuration not found. Cannot draw objects.');
	}

	// metric bars background

	const metrics = getBarMetrics(obj);
	metrics.forEach(({ key, percentage }, i) => {
		const height = 1 / metrics.length;
		const yPosition = yOffset + i * height;

		const color = key === 'hp' ? 'var(--hp-color)' : key === 'balance' ? 'var(--balance-color)' : 'var(--cooldown-color)';

		const bg = document.createElementNS(svgNS, 'rect');
		bg.setAttribute('x', xOffset.toString());
		bg.setAttribute('y', yPosition.toString());
		bg.setAttribute('width', '1');
		bg.setAttribute('height', height.toString());
		bg.setAttribute('fill', color);
		bg.setAttribute('fill-opacity', `${0.3 * scaleFactor}`);
		svgCanvas.appendChild(bg);

		const fg = document.createElementNS(svgNS, 'rect');
		fg.setAttribute('x', xOffset.toString());
		fg.setAttribute('y', yPosition.toString());
		fg.setAttribute('width', String(percentage / 100));
		fg.setAttribute('height', height.toString());
		fg.setAttribute('fill-opacity', `${0.7 * scaleFactor}`);
		fg.setAttribute('fill', color);
		svgCanvas.appendChild(fg);
	});

	// object icon

	let path: string;

	switch (obj.type) {
		case 0: {
			// Core
			const cores = svgAssets[0];
			const teamIndex = ((obj.teamId ?? 1) - 1) as AssetTeam;
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
			const teamIndex = ((obj.teamId ?? 1) - 1) as AssetTeam;
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

	let scale = 0.8;
	if (obj.type === 2) {
		scale = 0.95; // Resource
	} else if (obj.type === 3) {
		scale = 1; // Wall
	} else if (obj.type === 4) {
		scale = 0.6; // Money
	}

	const offset = (1 - scale * scaleFactor) / 2;
	img.removeAttribute('x');
	img.removeAttribute('y');
	img.setAttribute('width', '1');
	img.setAttribute('height', '1');
	img.setAttribute('transform', `translate(${xOffset + offset},${yOffset + offset}) scale(${scale * scaleFactor})`);
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
	console.log(`Rendering tick: ${currentTickData.tick}, progress: ${currentTickData.tickProgress}`);
	const replayData = getStateAt(currentTickData.tick);
	if (!replayData) {
		console.warn('No replay data available for the current tick.');
		window.requestAnimationFrame(drawFrame);
		return;
	}

	cleanGrid();

	let gridMessageOverride: string | undefined = getGridMessageOverride();

	if (gridMessageOverride) {
		const gridSize = gameConfig!.gridSize;

		const text = document.createElementNS(svgNS, 'text');
		text.textContent = gridMessageOverride;
		text.setAttribute('x', (gridSize / 2).toString());
		text.setAttribute('y', (gridSize / 2).toString());
		text.setAttribute('font-family', 'sans-serif');
		text.setAttribute('font-size', (gridSize / 14).toString());
		text.setAttribute('font-weight', 'bold');
		text.setAttribute('fill', 'rgba(0, 0, 0, 0.75)');
		text.setAttribute('text-anchor', 'middle');
		text.setAttribute('dominant-baseline', 'middle');
		svgCanvas.appendChild(text);

		window.requestAnimationFrame(drawFrame);
		return;
	}

	for (const currObj of replayData.objects) {
		let scale = 1;
		let x = currObj.x;
		let y = currObj.y;

		let prevObj = null;
		try {
			if (currentTickData.tick - 1 >= 0) prevObj = getStateAt(currentTickData.tick - 1)?.objects.find((o) => o.id === currObj.id);
		} catch {}
		let nextObj = null;
		try {
			nextObj = getStateAt(currentTickData.tick + 1)?.objects.find((o) => o.id === currObj.id);
		} catch {}

		const sineProgress = Math.sin((currentTickData.tickProgress * Math.PI) / 2);

		if (!prevObj || prevObj.state === 'dead') {
			scale = sineProgress;
		}
		if (!nextObj || nextObj.state === 'dead') {
			scale = 1 - sineProgress;
		}
		// check movement
		if (nextObj) {
			x = currObj.x + (nextObj.x - currObj.x) * sineProgress;
			y = currObj.y + (nextObj.y - currObj.y) * sineProgress;
		}

		drawObject(currObj, x, y, scale);
	}

	window.requestAnimationFrame(drawFrame);
}
window.requestAnimationFrame(drawFrame);

export async function setupRenderer(): Promise<void> {
	gameConfig = getGameConfig();
	if (!gameConfig) {
		throw new Error('Game configuration not found. Cannot set up renderer.');
	}

	const gridSize = gameConfig.gridSize;
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
			rect.setAttribute('fill', '#f7f7f7');
			rect.setAttribute('stroke', 'black');
			rect.setAttribute('stroke-width', '0.01');
			svgCanvas.appendChild(rect);
		}
	}

	svgCanvas.addEventListener('mousemove', (e) => {
		const pt = svgCanvas.createSVGPoint();
		pt.x = e.clientX;
		pt.y = e.clientY;

		const ctm = svgCanvas.getScreenCTM();
		if (!ctm) {
			tooltipElement.style.display = 'none';
			return;
		}
		const svgP = pt.matrixTransform(ctm.inverse());

		const tx = Math.floor(svgP.x),
			ty = Math.floor(svgP.y);

		const currentObjects = getStateAt(getCurrentTickData().tick)?.objects || [];
		const obj = currentObjects.find((o: TickObject) => o.x === tx && o.y === ty);

		if (obj) {
			const offsetX = 10;
			const offsetY = e.clientY > window.innerHeight / 2 ? -tooltipElement.offsetHeight - 10 : 10;
			tooltipElement.style.left = `${e.pageX + offsetX}px`;
			tooltipElement.style.top = `${e.pageY + offsetY}px`;
			tooltipElement.style.borderRadius = e.clientY > window.innerHeight / 2 ? '15px 15px 15px 0' : '0 15px 15px 15px';
			tooltipElement.style.display = 'block';
			tooltipElement.innerHTML = formatObjectData(obj);
		} else {
			tooltipElement.style.display = 'none';
		}
	});
	const hideIfOutside = (e: MouseEvent) => {
		const rect = svgCanvas.getBoundingClientRect();
		if (e.clientX < rect.left || e.clientX > rect.right || e.clientY < rect.top || e.clientY > rect.bottom) {
			tooltipElement.style.display = 'none';
		}
	};
	document.addEventListener('mousemove', hideIfOutside);
	window.addEventListener('blur', () => (tooltipElement.style.display = 'none'));
}
