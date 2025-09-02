import { getGameMisc, getReplayJSON } from "../replay_loader/replayLoader.js";
import { applyTheme } from "./themeManager.js";

function refreshReplayInfos() {
	const misc = getGameMisc();
	const seed = misc?.worldGeneratorSeed ?? 0;
	const teamA = misc?.team_results?.[0]?.name ?? "TeamA";
	const teamB = misc?.team_results?.[1]?.name ?? "TeamB";
	const a = document.getElementById(
		"download-replay-link",
	) as HTMLAnchorElement | null;
	const dlName = `replay_${teamA}_vs_${teamB}_seed${seed}.json`.replace(
		/\s+/g,
		"_",
	);
	if (a) {
		const json = getReplayJSON();
		const blob = new Blob([json], { type: "application/json" });
		const url = URL.createObjectURL(blob);
		a.href = url;
		a.download = dlName;
		(a as any)._objectUrl = url;
	}
	const copyBtn = document.getElementById(
		"copy-seed-btn",
	) as HTMLButtonElement | null;
	copyBtn?.addEventListener("click", async () => {
		try {
			await navigator.clipboard.writeText(String(seed));
			copyBtn.textContent = "Seed copied!";
		} catch {
			const ta = document.createElement("textarea");
			ta.value = String(seed);
			document.body.appendChild(ta);
			ta.select();
			document.execCommand("copy");
			document.body.removeChild(ta);
			copyBtn.textContent = "Seed copied!";
		}
		setTimeout(() => (copyBtn.textContent = "Copy current map seed"), 1200);
	});
}

export function setupInfoPopupManager() {
	const dialog = document.getElementById("site-modal");
	const openBtn = document.getElementById("open-modal");

	if (dialog instanceof HTMLDialogElement) {
		openBtn?.addEventListener("click", () => {
			dialog.showModal();
			refreshReplayInfos();
		});
		dialog.addEventListener("click", (e) => {
			const r = dialog.getBoundingClientRect();
			const inDialog =
				e.clientX >= r.left &&
				e.clientX <= r.right &&
				e.clientY >= r.top &&
				e.clientY <= r.bottom;
			if (!inDialog) dialog.close();
		});
		dialog.addEventListener("close", () => {
			const a = document.getElementById(
				"download-replay-link",
			) as HTMLAnchorElement | null;
			const url = (a as any)?._objectUrl as string | undefined;
			if (url) URL.revokeObjectURL(url);
			if (a && (a as any)._objectUrl) (a as any)._objectUrl = null;
		});
	}

	// dark mode customization handler
	const darkModeCheckbox = document.getElementById(
		"dark_mode_checkbox",
	) as HTMLInputElement;
	darkModeCheckbox.addEventListener("change", () => {
		applyTheme(darkModeCheckbox.checked ? "dark" : "light");
	});
	darkModeCheckbox.checked =
		document.documentElement.getAttribute("data-theme") === "dark";

	// gridline visiblity customization handler
	const gridCheckbox = document.getElementById(
		"gridlines_checkbox",
	) as HTMLInputElement;
	const GRID_STORAGE_KEY = "ui.gridlines";
	const savedGrid = localStorage.getItem(GRID_STORAGE_KEY);
	const gridOn = savedGrid ? savedGrid === "on" : true;
	document.documentElement.setAttribute(
		"data-gridlines",
		gridOn ? "on" : "off",
	);
	if (gridCheckbox) {
		gridCheckbox.checked = gridOn;
		gridCheckbox.addEventListener("change", () => {
			const on = gridCheckbox.checked;
			localStorage.setItem(GRID_STORAGE_KEY, on ? "on" : "off");
			document.documentElement.setAttribute(
				"data-gridlines",
				on ? "on" : "off",
			);
		});
	}

	// theme color update customization handler
	const colorSchemeInput = document.getElementById(
		"color_scheme_input",
	) as HTMLInputElement;
	colorSchemeInput.addEventListener("input", () => {
		const color = colorSchemeInput.value;
		document.documentElement.style.setProperty("--theme-color", color);
		localStorage.setItem('ui.themeColor', color);
	});
	colorSchemeInput.value = getComputedStyle(document.documentElement).getPropertyValue("--theme-color");
}
