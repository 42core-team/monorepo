export type TooltipPosition = "TL" | "BL" | "TR" | "BR";

type Point = { x: number; y: number };
type Size = { width: number; height: number };
type Rect = { left: number; top: number; right: number; bottom: number };

const GAP = 10;

const intersects = (a: Rect, b: Rect) =>
	a.left < b.right && a.right > b.left && a.top < b.bottom && a.bottom > b.top;

function rectFor(
	position: TooltipPosition,
	anchor: Point,
	size: Size,
	viewport: Rect,
): Rect {
	const height = Math.min(
		size.height,
		Math.max(
			0,
			position.startsWith("T")
				? anchor.y - GAP - viewport.top
				: viewport.bottom - anchor.y - GAP,
		),
	);
	const left = position.endsWith("L")
		? anchor.x - size.width - GAP
		: anchor.x + GAP;
	const top = position.startsWith("T")
		? anchor.y - height - GAP
		: anchor.y + GAP;

	return {
		left,
		top,
		right: left + size.width,
		bottom: top + height,
	};
}

export function chooseTooltipPlacement(
	anchor: Point,
	size: Size,
	viewport: Rect,
	pathTiles: Rect[],
): { position: TooltipPosition; rect: Rect } {
	const relativeY =
		(anchor.y - viewport.top) / (viewport.bottom - viewport.top);
	const preferred: TooltipPosition[] =
		relativeY > 0.5 ? ["TR", "TL", "BR", "BL"] : ["BR", "BL", "TR", "TL"];
	const candidates = preferred
		.filter(
			(position) =>
				(relativeY >= 0.25 || position.startsWith("B")) &&
				(relativeY <= 0.75 || position.startsWith("T")),
		)
		.map((position) => ({
			position,
			rect: rectFor(position, anchor, size, viewport),
		}));
	const horizontallyViable = candidates.filter(
		({ rect }) => rect.left >= viewport.left && rect.right <= viewport.right,
	);

	return (horizontallyViable.length ? horizontallyViable : candidates).reduce(
		(best, candidate) =>
			pathTiles.filter((tile) => intersects(candidate.rect, tile)).length <
			pathTiles.filter((tile) => intersects(best.rect, tile)).length
				? candidate
				: best,
	);
}

export function positionTooltip(
	tooltip: HTMLDivElement,
	anchor: Point,
	path: Point[],
	canvas: SVGSVGElement,
	visibleTop: number,
): void {
	tooltip.style.maxHeight = "";
	const matrix = canvas.getScreenCTM();
	const pathTiles = matrix
		? [
				...new Map(path.map((tile) => [`${tile.x},${tile.y}`, tile])).values(),
			].map((tile) => {
				const a = new DOMPoint(tile.x, tile.y).matrixTransform(matrix);
				const b = new DOMPoint(tile.x + 1, tile.y + 1).matrixTransform(matrix);
				return {
					left: Math.min(a.x, b.x),
					top: Math.min(a.y, b.y),
					right: Math.max(a.x, b.x),
					bottom: Math.max(a.y, b.y),
				};
			})
		: [];
	const { position, rect } = chooseTooltipPlacement(
		anchor,
		{ width: tooltip.offsetWidth, height: tooltip.offsetHeight },
		{
			left: 0,
			top: visibleTop,
			right: window.innerWidth,
			bottom: window.innerHeight,
		},
		pathTiles,
	);

	tooltip.style.left = `${rect.left}px`;
	tooltip.style.top = `${rect.top}px`;
	tooltip.style.maxHeight = `${rect.bottom - rect.top}px`;
	tooltip.style.borderRadius = {
		TL: "15px 15px 0 15px",
		BL: "15px 0 15px 15px",
		TR: "15px 15px 15px 0",
		BR: "0 15px 15px 15px",
	}[position];
}
