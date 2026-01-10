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

const svgCanvasElement = document.getElementById("svg-canvas");
if (!svgCanvasElement || !(svgCanvasElement instanceof SVGSVGElement)) {
	throw new Error("SVG canvas element not found or is not an SVG element");
}
const svgCanvas = svgCanvasElement as SVGSVGElement;
const tooltipElement = document.getElementById("tooltip") as HTMLDivElement;
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

function drawHoveredDebugPathOverlay(): void {
	if (!hoveredDebugPath || hoveredDebugPath.length === 0) return;

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
	poly.setAttribute("stroke", "var(--theme-color)");
	poly.setAttribute("stroke-opacity", "0.9");
	poly.setAttribute("stroke-width", "0.07");
	if (poly.parentNode !== svgCanvas) svgCanvas.appendChild(poly);

	for (let i = 0; i < hoveredDebugPath.length; i++) {
		const { x, y } = hoveredDebugPath[i];
		const rectKey = `dbg-path-rect-${x},${y},${i}`;
		let rect = svgCanvas.querySelector(
			`rect[data-dbg-path="${rectKey}"]`,
		) as SVGRectElement | null;
		if (!rect) {
			rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
			rect.setAttribute("data-dbg-path", rectKey);
			rect.setAttribute("width", "1");
			rect.setAttribute("height", "1");
			rect.setAttribute("rx", "0.15");
			rect.setAttribute("ry", "0.15");
			rect.setAttribute("pointer-events", "none");
		}
		rect.classList.remove("not-touched");
		rect.setAttribute("x", String(x));
		rect.setAttribute("y", String(y));
		rect.setAttribute("fill", "var(--theme-color)");
		rect.setAttribute("fill-opacity", i === 0 ? "0.25" : "0.15");
		rect.setAttribute("stroke", "var(--theme-color)");
		rect.setAttribute("stroke-opacity", i === 0 ? "1" : "0.7");
		rect.setAttribute("stroke-width", i === 0 ? "0.12" : "0.08");
		if (rect.parentNode !== svgCanvas) svgCanvas.appendChild(rect);

		const textKey = `dbg-path-text-${x},${y},${i}`;
		let txt = svgCanvas.querySelector(
			`text[data-dbg-path="${textKey}"]`,
		) as SVGTextElement | null;
		if (!txt) {
			txt = document.createElementNS("http://www.w3.org/2000/svg", "text");
			txt.setAttribute("data-dbg-path", textKey);
			txt.setAttribute("text-anchor", "middle");
			txt.setAttribute("dominant-baseline", "central");
			txt.setAttribute("pointer-events", "none");
		}
		txt.classList.remove("not-touched");
		txt.setAttribute("x", String(x + 0.5));
		txt.setAttribute("y", String(y + 0.5));
		txt.setAttribute("font-size", "0.35");
		txt.setAttribute("fill", "#000");
		txt.setAttribute("fill-opacity", "0.9");
		txt.textContent = String(i);
		if (txt.parentNode !== svgCanvas) svgCanvas.appendChild(txt);
	}
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
		// bombs will still persist for one more tick with 0 hp to communicate the explosion positions, but mustnt be rendered
		if (currObj.hp > 0) calcAndDrawObject(currObj, svgCanvas, currentTickData);
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

	const offsetX = 10;
	const offsetY =
		clientY > window.innerHeight / 2 ? -tooltipElement.offsetHeight - 10 : 10;
	tooltipElement.style.left = `${clientX + offsetX}px`;
	tooltipElement.style.top = `${clientY + offsetY}px`;
	tooltipElement.style.borderRadius =
		clientY > window.innerHeight / 2 ? "15px 15px 15px 0" : "0 15px 15px 15px";
	tooltipElement.style.display = "block";
	if (obj) {
		tooltipElement.innerHTML = formatObjectData(obj);

		if (obj.type !== 1) return;
		const dbg = (obj as UnitObject).debug_path;
		if (Array.isArray(dbg) && dbg.length > 0) {
			hoveredDebugPath = dbg;
		}
	} else {
		tooltipElement.innerHTML = `📍 Position: [x: ${tx}, y: ${ty}]`;
		hoveredDebugPath = null;
	}
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
			lastClientX = e.pageX;
			lastClientY = e.pageY;
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
	}

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

	isInitialRender = true;
}
