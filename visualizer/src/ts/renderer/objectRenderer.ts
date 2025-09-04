import type { tickData } from "../input_manager/timeManager";
import { getBarMetrics, type TickObject } from "../replay_loader/object";
import {
	getActionsByExecutor,
	getGameConfig,
	getNameOfUnitType,
	getStateAt,
} from "../replay_loader/replayLoader";
import {
	EaseInOutTimingCurve,
	MidTickIncreaseTimingCurve,
} from "./animationUtil.js";

const svgNS = "http://www.w3.org/2000/svg";

export type AssetTeam = 0 | 1;
const svgAssets = {
	0: {
		0: "cores/1.svg",
		1: "cores/2.svg",
	},
	1: {},
	2: "deposit.svg",
	3: "wall.svg",
	4: "gem_pile.svg",
	5: "bomb.svg",
} as const;

// metric bar interpolator

type BarDrawingInstructions = {
	opacity: number;
	topBorder: number;
	bottomBorder: number;
	leftBorder: number;
	rightBorder: number;
	completion: number;
	key: string;
};

function computeSmoothBarInstructions(
	curr: TickObject,
	next: TickObject | null | undefined,
	xOffset: number,
	yOffset: number,
	progress: number,
): BarDrawingInstructions[] {
	const ORDER = ["hp", "gems", "ActionCooldown"] as const;
	const toMap = (arr: { key: string; percentage: number }[]) =>
		Object.fromEntries(
			arr.map(({ key, percentage }) => [key, percentage / 100]),
		);
	const currList = getBarMetrics(curr);
	const nextList = next ? getBarMetrics(next) : currList;
	const currMap = toMap(currList);
	const nextMap = toMap(nextList);

	const p = Math.max(0, Math.min(1, progress));
	const out: BarDrawingInstructions[] = [];

	const union = ORDER.filter((k) => k in currMap || k in nextMap);
	const currCount = currList.length;
	const nextCount = nextList.length;

	const heights = union.map((k) => {
		const hc = k in currMap ? (currCount > 0 ? 1 / currCount : 0) : 0;
		const hn = k in nextMap ? (nextCount > 0 ? 1 / nextCount : 0) : 0;
		return hc + (hn - hc) * p;
	});

	let acc = yOffset;
	for (let i = 0; i < union.length; i++) {
		const key = union[i];
		const h = heights[i];
		const top = acc;
		const bottom = acc + h;
		acc = bottom;

		const currPerc = key in currMap ? currMap[key] : 0;
		const nextPerc = key in nextMap ? nextMap[key] : 0;
		const width = currPerc + (nextPerc - currPerc) * p;

		out.push({
			key,
			opacity: 1,
			topBorder: top,
			bottomBorder: bottom,
			leftBorder: xOffset,
			rightBorder: xOffset + width,
			completion: width * 100,
		});
	}
	return out;
}

// team mapping utils

const teamIdMapping: Map<number, AssetTeam> = new Map(); // number / asset id

export function initializeTeamMapping(): void {
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

// object

export function drawSpawnPreviewForNextTick(
	spawnObj: TickObject,
	svgCanvas: SVGSVGElement,
	currentTickData: tickData,
): void {
	const mid = new MidTickIncreaseTimingCurve().getValue(
		currentTickData.tickProgress,
	);
	const metrics = computeSmoothBarInstructions(
		spawnObj,
		spawnObj,
		spawnObj.x,
		spawnObj.y,
		mid,
	);

	let scale = mid;
	if (spawnObj.type === 5) {
		const cfgTotal = Math.max(1, getGameConfig()?.bombCountdown ?? 1);
		const currLeft = Math.max(
			0,
			Math.min(cfgTotal, (spawnObj as any).countdown ?? 0),
		);
		const p = 1 - currLeft / cfgTotal;
		const bombScale = 0.65 + 0.65 * p;
		scale *= bombScale / 0.8;
	}

	drawObject(svgCanvas, spawnObj, spawnObj.x, spawnObj.y, scale, metrics);
}

function drawObject(
	svgCanvas: SVGSVGElement,
	obj: TickObject,
	xOffset: number = 0,
	yOffset: number = 0,
	scaleFactor: number = 1,
	metricBars: BarDrawingInstructions[],
): void {
	for (const bar of metricBars) {
		const color =
			bar.key === "hp"
				? "var(--hp-color)"
				: bar.key === "gems"
					? "var(--gems-color)"
					: "var(--cooldown-color)";

		const bg = document.createElementNS(svgNS, "rect");
		bg.setAttribute("x", xOffset.toString());
		bg.setAttribute("y", bar.topBorder.toString());
		bg.setAttribute("width", "1");
		bg.setAttribute("height", String(bar.bottomBorder - bar.topBorder));
		bg.setAttribute("fill", color);
		bg.setAttribute("fill-opacity", String(0.2 * scaleFactor));
		svgCanvas.appendChild(bg);

		const fg = document.createElementNS(svgNS, "rect");
		fg.setAttribute("x", bar.leftBorder.toString());
		fg.setAttribute("y", bar.topBorder.toString());
		fg.setAttribute(
			"width",
			String(Math.max(0, bar.rightBorder - bar.leftBorder)),
		);
		fg.setAttribute("height", String(bar.bottomBorder - bar.topBorder));
		fg.setAttribute("fill", color);
		fg.setAttribute("fill-opacity", String(1 * scaleFactor));
		svgCanvas.appendChild(fg);
	}

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
			// Unit
			const unitType = obj.unit_type;
			if (obj.unit_type === undefined) {
				throw new Error(`Unit object ${obj.id} missing unit_type 🤯`);
			}
			const assetPath = getGameConfig()?.units[unitType]?.visualizer_asset_path;
			const teamIndex = getTeamIndex(obj.teamId);
			path = `units/${assetPath}/${teamIndex + 1}.svg`;
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

	let img = document.querySelector(
		`image[data-obj-id="${obj.id}"]`,
	) as SVGImageElement | null;

	if (!img) {
		img = document.createElementNS(svgNS, "image");
		img.setAttribute("data-obj-id", obj.id.toString());
	}
	img.classList.add("game-object");
	img.classList.remove("not-touched");
	img.classList.remove("team-0", "team-1");
	if (obj.type === 0 || obj.type === 1) {
		img.classList.add(`team-${getTeamIndex(obj.teamId)}`);
	}
	const href = `/assets/object-svgs/${path}`;
	if (img.getAttribute("href") !== href) {
		img.setAttribute("href", href);
	}

	let scale = 0.8;
	if (obj.type === 2) {
		scale = 0.95; // Deposit
	} else if (obj.type === 3) {
		scale = 1; // Wall
	} else if (obj.type === 4) {
		scale = 0.6; // Gem Pile
	}
	const offset = (1 - scale * scaleFactor) / 2;
	img.removeAttribute("x");
	img.removeAttribute("y");
	img.setAttribute("width", "1");
	img.setAttribute("height", "1");
	const baseTransform = `translate(${xOffset + offset},${yOffset + offset}) scale(${scale * scaleFactor})`;

	// flip team 1s units horizontally
	let finalTransform = baseTransform;
	if (obj.type === 1 && getTeamIndex(obj.teamId) === 1) {
		finalTransform += " translate(1,0) scale(-1,1)";
	}

	img.setAttribute("transform", finalTransform);

	svgCanvas.appendChild(img);
}

export function calcAndDrawObject(
	currObj: TickObject,
	svgCanvas: SVGSVGElement,
	currentTickData: tickData,
): void {
	const actionsByExec = getActionsByExecutor(currentTickData.tick + 1);

	let scale = 1;
	let x = currObj.x;
	let y = currObj.y;

	let nextObj: TickObject | undefined;
	try {
		nextObj = getStateAt(currentTickData.tick + 1)?.objects.find(
			(o) => o.id === currObj.id,
		);
	} catch {}

	const easeInOutProgress = new EaseInOutTimingCurve().getValue(
		currentTickData.tickProgress,
	);
	const midTickIncreaseProgress = new MidTickIncreaseTimingCurve().getValue(
		currentTickData.tickProgress,
	);

	if (!nextObj || nextObj.state === "dead" || nextObj.hp <= 0) {
		// despawn anim
		scale = 1 - midTickIncreaseProgress;
	}

	// check movement
	if (nextObj) {
		x = currObj.x + (nextObj.x - currObj.x) * easeInOutProgress;
		y = currObj.y + (nextObj.y - currObj.y) * easeInOutProgress;
	}

	// check attacks / build / transfer gems
	if (currObj.type === 1) {
		const actions = actionsByExec[currObj.id] || [];
		for (const action of actions) {
			if (
				action.type === "attack" ||
				action.type === "build" ||
				action.type === "transfer_gems"
			) {
				const deltaX = action.x - currObj.x;
				const deltaY = action.y - currObj.y;

				const halfActionTickProgress =
					easeInOutProgress > 0.5 ? 1 - easeInOutProgress : easeInOutProgress;

				let offsetX = deltaX * halfActionTickProgress;
				let offsetY = deltaY * halfActionTickProgress;

				if (offsetX > 0.5) offsetX = 0.5;
				if (offsetY > 0.5) offsetY = 0.5;

				x += offsetX;
				y += offsetY;
			}
		}
	}

	const metricBars: BarDrawingInstructions[] = computeSmoothBarInstructions(
		currObj,
		nextObj,
		x,
		y,
		new MidTickIncreaseTimingCurve().getValue(currentTickData.tickProgress),
	);

	if (currObj.type === 5) {
		const cfgTotal = Math.max(1, getGameConfig()?.bombCountdown ?? 1);
		const currLeft = Math.max(
			0,
			Math.min(cfgTotal, (currObj as any).countdown ?? 0),
		);
		const inferredNext = Math.max(0, currLeft - 1);
		const nextLeftRaw =
			nextObj &&
			nextObj.type === 5 &&
			typeof (nextObj as any).countdown === "number"
				? Math.max(0, Math.min(cfgTotal, (nextObj as any).countdown))
				: inferredNext;
		const leftSmooth =
			currLeft + (nextLeftRaw - currLeft) * currentTickData.tickProgress;
		const p = 1 - leftSmooth / cfgTotal;
		const bombScale = 0.65 + 0.65 * p;
		if (nextObj)
			// if the bomb doesnt exist next frame, use the normal despawning animation
			scale *= bombScale / 0.8;
	}

	drawObject(svgCanvas, currObj, x, y, scale, metricBars);

	// bomb explosion
	const explosionScale = (t: number): number => {
		const ease = new EaseInOutTimingCurve();
		if (t < 0.5) return ease.getValue(t / 0.5);
		return ease.getValue(1 - (t - 0.5) / 0.5);
	};
	if (
		nextObj &&
		nextObj.type === 5 &&
		nextObj.countdown === 0 &&
		Array.isArray((nextObj as any).explosionTiles) &&
		(nextObj as any).explosionTiles.length > 0
	) {
		const s = Math.max(
			0,
			Math.min(1, explosionScale(currentTickData.tickProgress)),
		);
		for (const tile of (nextObj as any).explosionTiles as {
			x: number;
			y: number;
		}[]) {
			const key = `exp-${tile.x},${tile.y}`;
			let img = svgCanvas.querySelector(
				`image[data-exp-key="${key}"]`,
			) as SVGImageElement | null;
			if (!img) {
				img = document.createElementNS(svgNS, "image");
				img.setAttribute("data-exp-key", key);
				img.setAttribute("href", "/assets/object-svgs/explosion.svg");
			}
			img.classList.add("game-object");
			img.classList.remove("not-touched");
			img.setAttribute("width", "1");
			img.setAttribute("height", "1");
			const tx = tile.x + 0.5 - 0.5 * s;
			const ty = tile.y + 0.5 - 0.5 * s;
			img.setAttribute("transform", `translate(${tx},${ty}) scale(${s})`);
			if (img.parentNode !== svgCanvas) svgCanvas.appendChild(img);
		}
	}
}
