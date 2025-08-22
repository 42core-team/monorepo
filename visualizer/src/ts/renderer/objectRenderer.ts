import type { tickData } from "../input_manager/timeManager";
import { getBarMetrics, type TickObject } from "../replay_loader/object";
import {
	getActionsByExecutor,
	getNameOfUnitType,
	getStateAt,
} from "../replay_loader/replayLoader";
import {
	EaseInOutTimingCurve,
	MidTickIncreaseTimingCurve,
} from "./animationUtil.js";

const svgNS = "http://www.w3.org/2000/svg";
const xlinkNS = "http://www.w3.org/1999/xlink";

export type AssetTeam = 0 | 1;
const svgAssets = {
	0: {
		0: "cores/1.svg",
		1: "cores/2.svg",
	},
	1: {},
	2: "resource.svg",
	3: "wall.svg",
	4: "money.svg",
	5: "core.svg",
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
	const ORDER = ["hp", "balance", "ActionCooldown"] as const;
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
				: bar.key === "balance"
					? "var(--balance-color)"
					: "var(--cooldown-color)";

		const bg = document.createElementNS(svgNS, "rect");
		bg.setAttribute("x", xOffset.toString());
		bg.setAttribute("y", bar.topBorder.toString());
		bg.setAttribute("width", "1");
		bg.setAttribute("height", String(bar.bottomBorder - bar.topBorder));
		bg.setAttribute("fill", color);
		bg.setAttribute("fill-opacity", String(0.3 * scaleFactor));
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
		fg.setAttribute("fill-opacity", String(0.7 * scaleFactor));
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
		scale = 0.95; // Resource
	} else if (obj.type === 3) {
		scale = 1; // Wall
	} else if (obj.type === 4) {
		scale = 0.6; // Money
	}
	const offset = (1 - scale * scaleFactor) / 2;
	img.removeAttribute("x");
	img.removeAttribute("y");
	img.setAttribute("width", "1");
	img.setAttribute("height", "1");
	img.setAttribute(
		"transform",
		`translate(${xOffset + offset},${yOffset + offset}) scale(${scale * scaleFactor})`,
	);

	if (img.parentNode !== svgCanvas) {
		svgCanvas.appendChild(img);
	}
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

	let prevObj: TickObject | undefined;
	try {
		if (currentTickData.tick - 1 >= 0)
			prevObj = getStateAt(currentTickData.tick - 1)?.objects.find(
				(o) => o.id === currObj.id,
			);
	} catch {}
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

	if (!prevObj || prevObj.state === "dead") {
		scale = easeInOutProgress;
	}
	if (!nextObj || nextObj.state === "dead") {
		scale = 1 - midTickIncreaseProgress;
	}
	// check movement
	if (nextObj) {
		x = currObj.x + (nextObj.x - currObj.x) * easeInOutProgress;
		y = currObj.y + (nextObj.y - currObj.y) * easeInOutProgress;
	}

	// check attacks / build / transfer money
	if (currObj.type === 1) {
		const actions = actionsByExec[currObj.id] || [];
		for (const action of actions) {
			if (
				action.type === "attack" ||
				action.type === "build" ||
				action.type === "transfer_money"
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

	drawObject(svgCanvas, currObj, x, y, scale, metricBars);
}
