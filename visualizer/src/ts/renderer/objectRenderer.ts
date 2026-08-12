import type { tickData } from "../input_manager/timeManager";
import type { TickObject } from "../replay_format/object";
import {
	getActionsByExecutor,
	getStateAt,
} from "../replay_loader/replayLoader";
import {
	EaseInOutTimingCurve,
	MidTickIncreaseTimingCurve,
} from "./animationUtil.js";
import { getBarMetrics, getDominantUnitAssetPath } from "./objectInfo";

const svgNS = "http://www.w3.org/2000/svg";

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
	const ORDER = ["hp", "gems", "cooldown"] as const;
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

export type AssetTeam = 0 | 1;
const teamIdMapping: Map<number, AssetTeam> = new Map(); // number / asset id

export function initializeTeamMapping(): void {
	teamIdMapping.clear();
	const objs = getStateAt(0)?.objects ?? [];
	const cores = objs.filter((o) => o.type === 0);
	if (cores.length < 2) return;

	cores.sort((a, b) => a.x - b.x || a.y - b.y);
	const leftCore = cores[0];
	const rightCore = cores[1];

	if (leftCore !== undefined && leftCore.teamId !== undefined)
		teamIdMapping.set(leftCore.teamId, 1);
	if (rightCore !== undefined && rightCore.teamId !== undefined)
		teamIdMapping.set(rightCore.teamId, 0);
}

export function getTeamIndex(teamId: number | undefined): AssetTeam {
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

	const scale = mid;

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

	const symbolId = (() => {
		switch (obj.type) {
			case 0:
				return getTeamIndex(obj.teamId) === 0 ? "icon-core-1" : "icon-core-2";
			case 1:
				return `icon-unit-${getDominantUnitAssetPath(obj)}-${getTeamIndex(obj.teamId) + 1}`;
			case 2:
				return "icon-deposit";
			case 3:
				return "icon-wall";
			case 4:
				return "icon-gem";
		}
	})();

	let use = svgCanvas.querySelector(
		`use[data-obj-id="${obj.id}"]`,
	) as SVGUseElement | null;
	if (!use) {
		use = document.createElementNS(svgNS, "use");
		use.setAttribute("data-obj-id", String(obj.id));
	}
	use.setAttribute("href", `#${symbolId}`);

	use.classList.add("game-object", "icon");
	use.classList.remove("not-touched");

	use.classList.remove("team-0", "team-1");
	if ("teamId" in obj) {
		use.classList.add(`team-${getTeamIndex(obj.teamId)}`);
	}

	use.setAttribute("href", `#${symbolId}`);
	use.setAttributeNS(
		"http://www.w3.org/1999/xlink",
		"xlink:href",
		`#${symbolId}`,
	);

	let scale = 0.8;
	if (obj.type === 2) {
		scale = 0.95; // Deposit
	} else if (obj.type === 3) {
		scale = 1; // Wall
	} else if (obj.type === 4) {
		scale = 0.6; // Gem Pile
	}
	const offset = (1 - scale * scaleFactor) / 2;
	use.removeAttribute("x");
	use.removeAttribute("y");
	use.setAttribute("width", "1");
	use.setAttribute("height", "1");
	const baseTransform = `translate(${xOffset + offset},${yOffset + offset}) scale(${scale * scaleFactor})`;

	// flip team 1s units horizontally
	let finalTransform = baseTransform;
	if (obj.type === 1 && getTeamIndex(obj.teamId) === 1) {
		finalTransform += " translate(1,0) scale(-1,1)";
	}

	use.setAttribute("transform", finalTransform);

	svgCanvas.appendChild(use);
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
			let baseX: number | undefined;
			let baseY: number | undefined;
			if (action.type === "build" || action.type === "transfer_gems") {
				baseX = action.x;
				baseY = action.y;
			} else if (action.type === "attack") {
				const targetObj = getStateAt(currentTickData.tick)?.objects.find(
					(o) => o.id === action.target_id,
				);
				if (targetObj) {
					baseX = targetObj.x;
					baseY = targetObj.y;
				}
			}
			if (typeof baseX === "number" && typeof baseY === "number") {
				const deltaX = baseX - currObj.x;
				const deltaY = baseY - currObj.y;

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

	drawObject(svgCanvas, currObj, x, y, scale, metricBars);
}
