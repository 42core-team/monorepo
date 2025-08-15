import type { GameConfig } from '../replay_loader/config.js';
import { formatObjectData, getBarMetrics, type TickObject } from '../replay_loader/object.js';
import { getActionsByExecutor, getGameConfig, getGameMisc, getNameOfUnitType, getStateAt } from '../replay_loader/replayLoader.js';
import { getCurrentTickData, isDirty } from '../time_manager/timeManager.js';

const svgNS = 'http://www.w3.org/2000/svg';
const xlinkNS = 'http://www.w3.org/1999/xlink';

export type AssetTeam = 0 | 1;
const svgAssets = {
	0: {
		0: 'cores/1.svg',
		1: 'cores/2.svg',
	},
	1: {},
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
const teamOneElement = document.getElementById('team-one-name') as HTMLDivElement;
const teamTwoElement = document.getElementById('team-two-name') as HTMLDivElement;

let gameConfig: GameConfig | undefined;
const teamIdMapping: Map<number, AssetTeam> = new Map(); // number / asset id

function initializeTeamMapping(): void {
	teamIdMapping.clear();
	const objs = getStateAt(0)?.objects ?? [];
	const cores = objs.filter((o) => o.type === 0);
	if (cores.length < 2) return;

	cores.sort((a, b) => a.x - b.x || a.y - b.y);
	const leftCore = cores[0];
	const rightCore = cores[1];

	const flip = Math.random() < 0.5;
	const teamForLight = flip ? leftCore.teamId : rightCore.teamId;
	const teamForDark = flip ? rightCore.teamId : leftCore.teamId;

	if (teamForLight !== undefined) teamIdMapping.set(teamForLight, 1);
	if (teamForDark !== undefined) teamIdMapping.set(teamForDark, 0);
}

function getTeamIndex(teamId: number | undefined): AssetTeam {
	if (teamId === undefined) {
		return 0;
	}
	return teamIdMapping.get(teamId) ?? 0;
}

function drawObject(obj: TickObject, xOffset: number = 0, yOffset: number = 0, scaleFactor: number = 1): void {
	if (!gameConfig) {
		throw new Error('Game configuration not found. Cannot draw objects.');
	}

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
			const teamIndex = getTeamIndex(obj.teamId);
			path = cores[teamIndex];
			break;
		}
		case 1: {
			const unitType = obj.unit_type;
			if (obj.unit_type === undefined) {
				throw new Error(`Unit object ${obj.id} missing unit_type 🤯`);
			}
			const unitName = getNameOfUnitType(unitType);
			const teamIndex = getTeamIndex(obj.teamId);
			const unitNameLower = unitName.toLowerCase();
			path = `units/${unitNameLower}/${teamIndex + 1}.svg`;
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

	let img = document.querySelector(`image[data-obj-id="${obj.id}"]`) as SVGImageElement | null;

	if (!img) {
		img = document.createElementNS(svgNS, 'image');
		img.setAttribute('data-obj-id', obj.id.toString());
	}

	img.classList.remove('not-touched');
	img.classList.remove('team-0', 'team-1');
	if (obj.type === 0 || obj.type === 1) {
		img.classList.add(`team-${getTeamIndex(obj.teamId)}`);
	}
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
	const actionsByExec = getActionsByExecutor(currentTickData.tick + 1);

	const nonPersistentElements = svgCanvas.querySelectorAll(':not(.persistent)');

	for (const element of nonPersistentElements) {
		element.classList.add('not-touched');
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

		// check attacks / build / transfer money
		if (currObj.type === 1) {
			const actions = actionsByExec[currObj.id] || [];
			for (const action of actions) {
				if (action.type === 'attack' || action.type === 'build' || action.type === 'transfer_money') {
					const deltaX = action.x - currObj.x;
					const deltaY = action.y - currObj.y;

					const halfActionTickProgress = currentTickData.tickProgress > 0.5 ? 1 - currentTickData.tickProgress : currentTickData.tickProgress;

					let offsetX = deltaX * halfActionTickProgress;
					let offsetY = deltaY * halfActionTickProgress;

					if (offsetX > 0.5) offsetX = 0.5;
					if (offsetY > 0.5) offsetY = 0.5;

					x += offsetX;
					y += offsetY;
				}
			}
		}

		drawObject(currObj, x, y, scale);
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
