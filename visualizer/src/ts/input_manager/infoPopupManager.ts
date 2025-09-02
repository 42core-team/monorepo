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

// basic quick and dirty brightness reduction function
function clampHexV80(hex: string): string {
	if (!/^#([0-9a-f]{6})$/i.test(hex)) return hex;
	const r = parseInt(hex.slice(1, 3), 16);
	const g = parseInt(hex.slice(3, 5), 16);
	const b = parseInt(hex.slice(5, 7), 16);
	const m = Math.max(r, g, b);
	const targetNum = 255 / 100 * 80; // target brightness (80%)
	if (m <= targetNum) return hex;
	const s = targetNum / m;
	const to2 = (n: number) => n.toString(16).padStart(2, "0");
	const R = Math.round(r * s), G = Math.round(g * s), B = Math.round(b * s);
	return `#${to2(R)}${to2(G)}${to2(B)}`;
}

export function setupInfoPopupManager() {
	const infoModal = document.getElementById("info-modal");
	const openInfoBtn = document.getElementById("open-info-modal");

	if (infoModal instanceof HTMLDialogElement) {
		openInfoBtn?.addEventListener("click", () => {
			infoModal.showModal();
			refreshReplayInfos();
		});
		infoModal.addEventListener("click", (e) => {
			const r = infoModal.getBoundingClientRect();
			const inDialog =
				e.clientX >= r.left &&
				e.clientX <= r.right &&
				e.clientY >= r.top &&
				e.clientY <= r.bottom;
			if (!inDialog) infoModal.close();
		});
		infoModal.addEventListener("close", () => {
			const a = document.getElementById(
				"download-replay-link",
			) as HTMLAnchorElement | null;
			const url = (a as any)?._objectUrl as string | undefined;
			if (url) URL.revokeObjectURL(url);
			if (a && (a as any)._objectUrl) (a as any)._objectUrl = null;
		});
	}

	const settingsModal = document.getElementById('settings-modal');
	const openSettingsBtn = document.getElementById('open-settings-modal');
	if (settingsModal instanceof HTMLDialogElement) {
		openSettingsBtn?.addEventListener("click", () => {
			settingsModal.showModal();
			refreshReplayInfos();
			syncSettingsUI();
		});
		settingsModal.addEventListener("click", (e) => {
			const r = settingsModal.getBoundingClientRect();
			const inDialog =
				e.clientX >= r.left &&
				e.clientX <= r.right &&
				e.clientY >= r.top &&
				e.clientY <= r.bottom;
			if (!inDialog) settingsModal.close();
		});
	}

	const fullscreenBtn = document.getElementById("fullscreen-toggle-button") as HTMLButtonElement | null;
	const fullscreenIcon = document.getElementById("fullscreen-icon") as HTMLImageElement | null;

	const darkBtn = document.getElementById("dark-mode-toggle-button") as HTMLButtonElement | null;
	const darkIcon = document.getElementById("dark-mode-icon") as HTMLImageElement | null;

	const gridBtn = document.getElementById("gridlines-toggle-button") as HTMLButtonElement | null;

	const GRID_STORAGE_KEY = "ui.gridlines";

	function setPressed(el: HTMLButtonElement | null, on: boolean) {
		if (!el) return;
		el.setAttribute("aria-pressed", on ? "true" : "false");
		el.classList.toggle("primary", on);
	}

	// fullscreen

	function isFullscreen(): boolean {
		return !!(document.fullscreenElement || (document as any).webkitFullscreenElement);
	}

	async function toggleFullscreen() {
		try {
		if (!isFullscreen()) {
			const target = document.querySelector(".container") as HTMLElement || document.documentElement;
			if (target.requestFullscreen) {
			await target.requestFullscreen();
			} else if ((target as any).webkitRequestFullscreen) {
			await (target as any).webkitRequestFullscreen();
			}
		} else {
			if (document.exitFullscreen) {
			await document.exitFullscreen();
			} else if ((document as any).webkitExitFullscreen) {
			await (document as any).webkitExitFullscreen();
			}
		}
		} finally {
		syncFullscreenBtn();
		}
	}

	function syncFullscreenBtn() {
		const on = isFullscreen();
		setPressed(fullscreenBtn, on);
		if (fullscreenIcon) {
		fullscreenIcon.src = on ? "/assets/ui-svgs/fullscreen-close.svg" : "/assets/ui-svgs/fullscreen-open.svg";
		fullscreenIcon.alt = on ? "Exit Fullscreen" : "Enter Fullscreen";
		}
	}

	document.addEventListener("fullscreenchange", syncFullscreenBtn);
	(document as any).addEventListener?.("webkitfullscreenchange", syncFullscreenBtn);

	fullscreenBtn?.addEventListener("click", toggleFullscreen);

	// dark mode / theme

	function toggleDark() {
		const nextDark = document.documentElement.getAttribute("data-theme") !== "dark";
		applyTheme(nextDark ? "dark" : "light");
		syncDarkBtn();
	}

	function syncDarkBtn() {
		const on = document.documentElement.getAttribute("data-theme") === "dark";
		setPressed(darkBtn, on);
		if (darkIcon) {
		darkIcon.src = on ? "/assets/ui-svgs/dark-mode-sun.svg" : "/assets/ui-svgs/dark-mode-moon.svg";
		darkIcon.alt = on ? "Switch to Light Mode" : "Enable Dark Mode";
		}
	}

	darkBtn?.addEventListener("click", toggleDark);

	// grid lines

	function toggleGrid() {
		const cur = document.documentElement.getAttribute("data-gridlines") === "on";
		const next = !cur;
		document.documentElement.setAttribute("data-gridlines", next ? "on" : "off");
		localStorage.setItem("ui.gridlines", next ? "on" : "off");
		syncGridBtn();
	}

	function syncGridBtn() {
		const on = document.documentElement.getAttribute("data-gridlines") === "on";
		setPressed(gridBtn, on);
	}

	gridBtn?.addEventListener("click", toggleGrid);

	// color scheme

	const colorSchemeInput = document.getElementById("color_scheme_input") as HTMLInputElement;
	colorSchemeInput.addEventListener("input", () => {
		const color = clampHexV80(colorSchemeInput.value);
		document.documentElement.style.setProperty("--theme-color", color);
		localStorage.setItem('ui.themeColor', color);
	});
	colorSchemeInput.value = getComputedStyle(document.documentElement).getPropertyValue("--theme-color");



	function syncSettingsUI() {
		syncFullscreenBtn();
		syncDarkBtn();
		syncGridBtn();
	}
	syncSettingsUI();
}
