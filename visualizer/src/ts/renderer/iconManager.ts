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
	];
	const unitAssetPaths = new Set([
		"fallback",
		...(getGameConfig()?.components.components ?? [])
			.map((component) => component.visualizer_asset_path)
			.filter((path): path is string => Boolean(path)),
	]);
	for (const assetPath of unitAssetPaths) {
		paths.push([
			`icon-unit-${assetPath}-1`,
			`/assets/object-svgs/units/${assetPath}/1.svg`,
		]);
		paths.push([
			`icon-unit-${assetPath}-2`,
			`/assets/object-svgs/units/${assetPath}/2.svg`,
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
