import { getCurrentTickData, isDirty } from "../input_manager/timeManager";
import type { GameConfig } from "../replay_loader/config";
import {
	formatObjectData,
	type TickObject,
	type UnitObject,
} from "../replay_loader/object";
import {
	getGameConfig,
	getGameMisc,
	getStateAt,
} from "../replay_loader/replayLoader";
import {
	calcAndDrawObject,
	drawSpawnPreviewForNextTick,
	initializeTeamMapping,
} from "./objectRenderer";
import { positionTooltip } from "./tooltipPosition";

const svgCanvasElement = document.getElementById("svg-canvas");
if (!svgCanvasElement || !(svgCanvasElement instanceof SVGSVGElement)) {
	throw new Error("SVG canvas element not found or is not an SVG element");
}
const svgCanvas = svgCanvasElement as SVGSVGElement;
const tooltipElement = document.getElementById("tooltip") as HTMLDivElement;
const topBarElement = document.querySelector(".top-bar") as HTMLDivElement;
const teamOneElement = document.getElementById(
	"team-one-name",
) as HTMLDivElement;
const teamTwoElement = document.getElementById(
	"team-two-name",
) as HTMLDivElement;

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

let hoveredDebugPath: { x: number; y: number }[] | null = null;
let hoveredDebugPathStroke: string | null = null;

function drawHoveredDebugPathOverlay(): void {
	if (!hoveredDebugPath || hoveredDebugPath.length === 0) return;

	// Draw a single polyline through the centers of tiles
	const pointsAttr = hoveredDebugPath
		.map(({ x, y }) => `${x + 0.5},${y + 0.5}`)
		.join(" ");

	const polyKey = "dbg-path-poly";
	let poly = svgCanvas.querySelector(
		`polyline[data-dbg-path="${polyKey}"]`,
	) as SVGPolylineElement | null;

	if (!poly) {
		poly = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
		poly.setAttribute("data-dbg-path", polyKey);
		poly.setAttribute("fill", "none");
		poly.setAttribute("pointer-events", "none");
	}

	poly.classList.remove("not-touched");
	poly.setAttribute("points", pointsAttr);
	poly.setAttribute("stroke", hoveredDebugPathStroke ?? "var(--theme-color)");
	poly.setAttribute("stroke-opacity", "0.9");
	poly.setAttribute("stroke-width", "0.07");
	poly.setAttribute("stroke-linecap", "round");
	poly.setAttribute("stroke-linejoin", "round");

	if (poly.parentNode !== svgCanvas) svgCanvas.appendChild(poly);
}

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
		console.warn("No replay data available for the current tick.");
		window.requestAnimationFrame(drawFrame);
		return;
	}

	for (const element of svgCanvas.querySelectorAll("*")) {
		if (!(element as Element).closest(".persistent")) {
			element.classList.add("not-touched");
		}
	}

	for (const currObj of replayData.objects) {
		calcAndDrawObject(currObj, svgCanvas, currentTickData);
	}
	const nextTickData = getStateAt(currentTickData.tick + 1);
	if (nextTickData) {
		const currentIds = new Set(replayData.objects.map((o) => o.id));
		for (const spawn of nextTickData.objects) {
			if (!currentIds.has(spawn.id)) {
				drawSpawnPreviewForNextTick(
					spawn as TickObject,
					svgCanvas,
					currentTickData,
				);
			}
		}
	}

	if (tooltipElement.style.display === "block" && lastSVGPoint) {
		refreshTooltipFromSVGPoint(lastSVGPoint, lastClientX, lastClientY);
	}

	drawHoveredDebugPathOverlay();

	for (const element of svgCanvas.querySelectorAll(".not-touched")) {
		if (!(element as Element).closest(".persistent")) {
			element.remove();
		}
	}

	scheduleNextFrame();
}

let lastSVGPoint: DOMPoint | null = null;
let lastClientX = 0;
let lastClientY = 0;
let lastTooltipTarget: string | null = null;

function refreshTooltipFromSVGPoint(
	svgP: DOMPoint,
	clientX: number,
	clientY: number,
): void {
	const tx = Math.floor(svgP.x);
	const ty = Math.floor(svgP.y);

	const nextObjects = getStateAt(getCurrentTickData().tick + 1)?.objects || [];
	const currentObjects = getStateAt(getCurrentTickData().tick)?.objects || [];
	let obj = currentObjects.find((o: TickObject) => o.x === tx && o.y === ty);

	// if there was nothing at that pos but there is now, it was a move action and we can safely show the tooltip on both tiles
	if (!obj && getCurrentTickData().tickProgress > 0) {
		const movedObj = nextObjects.find(
			(o: TickObject) => o.x === tx && o.y === ty,
		);
		obj = currentObjects.find((o: TickObject) => o.id === movedObj?.id);
	}

	// if were already closer to the next tick, show that ticks data instead
	if (obj && getCurrentTickData().tickProgress > 0.5) {
		const objId = obj.id;
		obj = nextObjects.find((o: TickObject) => o.id === objId);
	}
	const tooltipTarget = obj ? `object:${obj.id}` : `tile:${tx},${ty}`;
	const preserveScroll =
		tooltipElement.style.display === "block" &&
		tooltipTarget === lastTooltipTarget;
	const scrollTop = preserveScroll ? tooltipElement.scrollTop : 0;
	const scrollLeft = preserveScroll ? tooltipElement.scrollLeft : 0;

	if (obj) {
		const content = formatObjectData(obj);
		if (tooltipElement.innerHTML !== content)
			tooltipElement.innerHTML = content;

		if (obj.type !== 1) {
			hoveredDebugPath = null;
		} else if ((obj as UnitObject).debug_path?.length) {
			const dbg = (obj as UnitObject).debug_path ?? [];
			// Ensure the drawn path starts at the unit’s current tile
			const start = { x: obj.x, y: obj.y };
			const first = dbg[0];
			hoveredDebugPath =
				first && first.x === start.x && first.y === start.y
					? dbg
					: [start, ...dbg];
			// get the units color, draw path with that
			const useEl = svgCanvas.querySelector(
				`use[data-obj-id="${obj.id}"]`,
			) as SVGUseElement | null;
			if (useEl) {
				const fill = getComputedStyle(useEl).fill;
				hoveredDebugPathStroke = fill && fill !== "none" ? fill : null;
			} else {
				hoveredDebugPathStroke = null;
			}
		} else {
			hoveredDebugPath = null;
			hoveredDebugPathStroke = null;
		}
	} else {
		const content = `<strong>📍 Position: [x: ${tx}, y: ${ty}]</strong>`;
		if (tooltipElement.innerHTML !== content)
			tooltipElement.innerHTML = content;
		hoveredDebugPath = null;
	}

	tooltipElement.style.display = "block";
	positionTooltip(
		tooltipElement,
		{ x: clientX, y: clientY },
		hoveredDebugPath ?? [],
		svgCanvas,
		topBarElement.getBoundingClientRect().bottom,
	);
	tooltipElement.scrollTop = scrollTop;
	tooltipElement.scrollLeft = scrollLeft;
	lastTooltipTarget = tooltipTarget;
}
export async function setupRenderer(): Promise<void> {
	gameConfig = getGameConfig();
	if (!gameConfig) {
		throw new Error("Game configuration not found. Cannot set up renderer.");
	}

	if (!svgCanvas.dataset.renderLoopStarted) {
		scheduleNextFrame();
		svgCanvas.dataset.renderLoopStarted = "1";
	}

	teamOneElement.textContent = "";
	teamTwoElement.textContent = "";

	if ((getStateAt(0)?.objects ?? []).some((o) => o.type === 0)) {
		for (const team of getGameMisc()?.team_results ?? []) {
			for (const obj of getStateAt(0)?.objects ?? []) {
				if (obj.type === 0 && obj.teamId === team.id) {
					if (obj.x === 0)
						teamOneElement.textContent = `🟠 ${team.name} (${team.id})`;
					else if (obj.x === gameConfig.gridSize - 1)
						teamTwoElement.textContent = `🟣 ${team.name} (${team.id})`;
				}
			}
		}
	} else {
		const misc = getGameMisc();
		if (misc?.team_results?.[0])
			teamOneElement.textContent = `${misc.team_results[0].name}(${misc.team_results[0].id})`;
		if (misc?.team_results?.[1])
			teamTwoElement.textContent = `${misc.team_results[1].name}(${misc.team_results[1].id})`;
	}

	svgCanvas.querySelectorAll(".persistent").forEach((el) => el.remove());
	svgCanvas.querySelectorAll(":not(.persistent)").forEach((el) => el.remove());

	const gridSize = gameConfig.gridSize;
	svgCanvas.setAttribute("width", gridSize.toString());
	svgCanvas.setAttribute("height", gridSize.toString());
	svgCanvas.setAttribute("viewBox", `0 0 ${gridSize} ${gridSize}`);

	document.documentElement.style.setProperty("--grid-size", String(gridSize));

	initializeTeamMapping();

	if (!svgCanvas.dataset.listenersBound) {
		svgCanvas.addEventListener("mousemove", (e) => {
			const pt = svgCanvas.createSVGPoint();
			pt.x = e.clientX;
			pt.y = e.clientY;
			const ctm = svgCanvas.getScreenCTM();
			if (!ctm) {
				tooltipElement.style.display = "none";
				return;
			}
			const svgP = pt.matrixTransform(ctm.inverse());
			lastSVGPoint = svgP;
			lastClientX = e.clientX;
			lastClientY = e.clientY;
			refreshTooltipFromSVGPoint(svgP, e.clientX, e.clientY);
		});
		const hideIfOutside = (e: MouseEvent) => {
			const rect = svgCanvas.getBoundingClientRect();
			if (
				e.clientX < rect.left ||
				e.clientX > rect.right ||
				e.clientY < rect.top ||
				e.clientY > rect.bottom
			) {
				tooltipElement.style.display = "none";
			}
		};
		document.addEventListener("mousemove", hideIfOutside);
		window.addEventListener("blur", () => {
			tooltipElement.style.display = "none";
		});
		svgCanvas.dataset.listenersBound = "1";

		// translate scrolling on objects to their tooltip
		svgCanvas.addEventListener(
			"wheel",
			(e) => {
				if (tooltipElement.style.display !== "block") return;

				tooltipElement.scrollTop += e.deltaY;
				tooltipElement.scrollLeft += e.deltaX;

				e.preventDefault();
			},
			{ passive: false },
		);
	}

	isInitialRender = true;
}
