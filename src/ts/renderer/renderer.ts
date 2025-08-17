import type { GameConfig } from '../replay_loader/config.js';
import { formatObjectData, type TickObject } from '../replay_loader/object.js';
import { getGameConfig, getGameMisc, getStateAt } from '../replay_loader/replayLoader.js';
import { getCurrentTickData, isDirty } from '../time_manager/timeManager.js';
import { calcAndDrawObject, initializeTeamMapping } from './objectRenderer.js';

const svgNS = 'http://www.w3.org/2000/svg';

const svgCanvasElement = document.getElementById('svg-canvas');
if (!svgCanvasElement || !(svgCanvasElement instanceof SVGSVGElement)) {
	throw new Error('SVG canvas element not found or is not an SVG element');
}
const svgCanvas = svgCanvasElement as SVGSVGElement;
const tooltipElement = document.getElementById('tooltip') as HTMLDivElement;
const teamOneElement = document.getElementById('team-one-name') as HTMLDivElement;
const teamTwoElement = document.getElementById('team-two-name') as HTMLDivElement;

let gameConfig: GameConfig | undefined;

let lastRenderTime = 0;
const FPS = 30;
const MIN_FRAME_INTERVAL = 1000 / FPS; // 30 FPS
function scheduleNextFrame(): void {
	const now = performance.now();
	const elapsed = now - lastRenderTime;
	const delay = Math.max(0, MIN_FRAME_INTERVAL - elapsed);
	if (delay > 0) {
		setTimeout(() => {
			window.requestAnimationFrame(drawFrame);
		}, delay);
	} else {
		window.requestAnimationFrame(drawFrame);
	}
}
let isInitialRender = true;
function drawFrame(timestamp: number): void {
	lastRenderTime = timestamp;

	if (!isDirty() && !isInitialRender) {
		scheduleNextFrame();
		return;
	}
	isInitialRender = false;

	const currentTickData = getCurrentTickData();
	const replayData = getStateAt(currentTickData.tick);
	if (!replayData) {
		console.warn('No replay data available for the current tick.');
		window.requestAnimationFrame(drawFrame);
		return;
	}

	const nonPersistentElements = svgCanvas.querySelectorAll(':not(.persistent)');

	for (const element of nonPersistentElements) {
		element.classList.add('not-touched');
	}

	for (const currObj of replayData.objects) {
		calcAndDrawObject(currObj, svgCanvas, currentTickData);
	}

	if (tooltipElement.style.display === 'block' && lastSVGPoint) {
		refreshTooltipFromSVGPoint(lastSVGPoint, lastClientX, lastClientY);
	}

	for (const element of svgCanvas.querySelectorAll('.not-touched')) {
		element.remove();
	}

	scheduleNextFrame();
}

let lastSVGPoint: DOMPoint | null = null;
let lastClientX = 0;
let lastClientY = 0;

function refreshTooltipFromSVGPoint(svgP: DOMPoint, clientX: number, clientY: number): void {
	const tx = Math.floor(svgP.x);
	const ty = Math.floor(svgP.y);
	const currentObjects = getStateAt(getCurrentTickData().tick)?.objects || [];
	const obj = currentObjects.find((o: TickObject) => o.x === tx && o.y === ty);

	const offsetX = 10;
	const offsetY = clientY > window.innerHeight / 2 ? -tooltipElement.offsetHeight - 10 : 10;
	tooltipElement.style.left = `${clientX + offsetX}px`;
	tooltipElement.style.top = `${clientY + offsetY}px`;
	tooltipElement.style.borderRadius = clientY > window.innerHeight / 2 ? '15px 15px 15px 0' : '0 15px 15px 15px';
	tooltipElement.style.display = 'block';
	if (obj) {
		tooltipElement.innerHTML = formatObjectData(obj);
	} else {
		tooltipElement.innerHTML = `📍 Position: [${tx}, ${ty}]`;
	}
}
export async function setupRenderer(): Promise<void> {
	gameConfig = getGameConfig();
	if (!gameConfig) {
		throw new Error('Game configuration not found. Cannot set up renderer.');
	}

	if (!(svgCanvas as any).dataset.renderLoopStarted) {
		scheduleNextFrame();
		(svgCanvas as any).dataset.renderLoopStarted = '1';
	}

	teamOneElement.textContent = '';
	teamTwoElement.textContent = '';

	if ((getStateAt(0)?.objects ?? []).some((o) => o.type === 0)) {
		for (const team of getGameMisc()?.team_results ?? []) {
			for (const obj of getStateAt(0)?.objects ?? []) {
				if (obj.type === 0 && obj.teamId === team.id) {
					if (obj.x === 0) teamOneElement.textContent = `${team.name}(${team.id})`;
					else if (obj.x === gameConfig.gridSize - 1) teamTwoElement.textContent = `${team.name}(${team.id})`;
				}
			}
		}
	} else {
		const misc = getGameMisc();
		if (misc?.team_results?.[0]) teamOneElement.textContent = `${misc.team_results[0].name}(${misc.team_results[0].id})`;
		if (misc?.team_results?.[1]) teamTwoElement.textContent = `${misc.team_results[1].name}(${misc.team_results[1].id})`;
	}

	svgCanvas.querySelectorAll('.persistent').forEach((el) => el.remove());
	svgCanvas.querySelectorAll(':not(.persistent)').forEach((el) => el.remove());

	const gridSize = gameConfig.gridSize;
	svgCanvas.setAttribute('width', gridSize.toString());
	svgCanvas.setAttribute('height', gridSize.toString());
	svgCanvas.setAttribute('viewBox', `0 0 ${gridSize} ${gridSize}`);

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

	initializeTeamMapping();

	if (!(svgCanvas as any).dataset.listenersBound) {
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
			lastSVGPoint = svgP;
			lastClientX = e.pageX;
			lastClientY = e.pageY;
			refreshTooltipFromSVGPoint(svgP, e.clientX, e.clientY);
		});
		const hideIfOutside = (e: MouseEvent) => {
			const rect = svgCanvas.getBoundingClientRect();
			if (e.clientX < rect.left || e.clientX > rect.right || e.clientY < rect.top || e.clientY > rect.bottom) {
				tooltipElement.style.display = 'none';
			}
		};
		document.addEventListener('mousemove', hideIfOutside);
		window.addEventListener('blur', () => {
			tooltipElement.style.display = 'none';
		});
		(svgCanvas as any).dataset.listenersBound = '1';
	}

	isInitialRender = true;
}
