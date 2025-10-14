import { getGameConfig } from "../replay_loader/replayLoader";

export async function ensureIcons() {
	const svgCanvas = document.getElementById("svg-canvas") as SVGSVGElement;
	if (!svgCanvas) throw new Error("svg-canvas not found");

	let defsEl = document.getElementById("icon-sprite") as SVGDefsElement | null;
	if (!defsEl) {
		defsEl = document.createElementNS(
			svgCanvas.namespaceURI,
			"defs",
		) as SVGDefsElement;
		defsEl.id = "icon-sprite";
		defsEl.classList.add("persistent");
		svgCanvas.appendChild(defsEl);
	}
	const ns = svgCanvas.namespaceURI;

	const paths = [
		["icon-core-1", "/assets/object-svgs/cores/1.svg"],
		["icon-core-2", "/assets/object-svgs/cores/2.svg"],
		["icon-deposit", "/assets/object-svgs/deposit.svg"],
		["icon-wall", "/assets/object-svgs/wall.svg"],
		["icon-gem", "/assets/object-svgs/gem_pile.svg"],
		["icon-bomb", "/assets/object-svgs/bomb.svg"],
	];
	for (const unit of getGameConfig()?.units || []) {
		paths.push([
			`icon-unit-${unit.visualizer_asset_path}-1`,
			`/assets/object-svgs/units/${unit.visualizer_asset_path}/1.svg`,
		]);
		paths.push([
			`icon-unit-${unit.visualizer_asset_path}-2`,
			`/assets/object-svgs/units/${unit.visualizer_asset_path}/2.svg`,
		]);
	}

	for (const [id, url] of paths) {
		if (document.getElementById(id)) continue;

		try {
			const res = await fetch(url);
			if (!res.ok) continue;
			const txt = await res.text();
			const doc = new DOMParser().parseFromString(txt, "image/svg+xml");
			const svg = doc.documentElement;

			const vb =
				svg.getAttribute("viewBox") ||
				`0 0 ${svg.getAttribute("width") || 1} ${svg.getAttribute("height") || 1}`;

			const symbol = document.createElementNS(ns, "symbol");
			symbol.id = id;
			symbol.setAttribute("viewBox", vb);

			while (svg.firstChild) symbol.appendChild(svg.firstChild);

			defsEl.appendChild(symbol);
		} catch {
			console.warn(`Failed to load SVG asset: ${url}`);
		}
	}
}
