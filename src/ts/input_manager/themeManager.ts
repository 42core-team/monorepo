type ThemeMode = "light" | "dark";

const themeToggleButton = document.getElementById(
	"theme-toggle-button",
) as HTMLButtonElement | null;
const themeIcon = document.getElementById(
	"theme-icon",
) as HTMLImageElement | null;

const STORAGE_KEY = "ui.theme";
const mql = window.matchMedia
	? window.matchMedia("(prefers-color-scheme: dark)")
	: null;

function resolveTheme(): ThemeMode {
	const v = localStorage.getItem(STORAGE_KEY);
	if (v === "light" || v === "dark") return v as ThemeMode;
	return mql?.matches ? "dark" : "light";
}

function applyToDOM(mode: ThemeMode): void {
	document.documentElement.setAttribute("data-theme", mode);
	if (themeToggleButton)
		themeToggleButton.setAttribute(
			"aria-pressed",
			mode === "dark" ? "true" : "false",
		);
	if (themeIcon) {
		themeIcon.src =
			mode === "dark"
				? "/assets/ui-svgs/dark-mode-sun.svg"
				: "/assets/ui-svgs/dark-mode-moon.svg";
		themeIcon.alt =
			mode === "dark" ? "Switch to Light Mode" : "Switch to Dark Mode";
	}
}

export function applyTheme(mode: ThemeMode): void {
	localStorage.setItem(STORAGE_KEY, mode);
	applyToDOM(mode);
}

export function loadSavedTheme(): void {
	applyToDOM(resolveTheme());

	if (mql) {
		const onSystemChange = (e: MediaQueryListEvent) => {
			const hasOverride =
				localStorage.getItem(STORAGE_KEY) === "light" ||
				localStorage.getItem(STORAGE_KEY) === "dark";
			if (!hasOverride) applyToDOM(e.matches ? "dark" : "light");
		};
		if ("addEventListener" in mql) {
			mql.addEventListener("change", onSystemChange);
		} else if (typeof (mql as any).addListener === "function") {
			(mql as any).addListener(onSystemChange);
		}
	}

	window.addEventListener("storage", (e) => {
		if (e.key === STORAGE_KEY) applyToDOM(resolveTheme());
	});
}

export function toggleTheme(): void {
	const next = resolveTheme() === "dark" ? "light" : "dark";
	localStorage.setItem(STORAGE_KEY, next);
	applyToDOM(next);
}
themeToggleButton?.addEventListener("click", toggleTheme);
