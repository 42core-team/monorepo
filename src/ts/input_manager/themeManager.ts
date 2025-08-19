const themeToggleButton = document.getElementById(
	"theme-toggle-button",
) as HTMLButtonElement;
const themeIcon = document.getElementById("theme-icon") as HTMLImageElement;

type ThemeMode = "light" | "dark";

function getSavedTheme(): ThemeMode {
	const v = localStorage.getItem("ui.theme");
	return v === "dark" ? "dark" : "light";
}
export function applyTheme(mode: ThemeMode): void {
	document.documentElement.setAttribute("data-theme", mode);
	localStorage.setItem("ui.theme", mode);
	updateThemeUI(mode);
}
function updateThemeUI(mode: ThemeMode): void {
	const isDark = mode === "dark";
	if (themeToggleButton)
		themeToggleButton.setAttribute("aria-pressed", isDark ? "true" : "false");
	if (themeIcon) {
		themeIcon.src = isDark
			? "/assets/ui-svgs/dark-mode-sun.svg"
			: "/assets/ui-svgs/dark-mode-moon.svg";
		themeIcon.alt = isDark ? "Switch to Light Mode" : "Switch to Dark Mode";
	}
}
export function loadSavedTheme(): void {
	applyTheme(getSavedTheme());
}
export function toggleTheme(): void {
	const next = getSavedTheme() === "dark" ? "light" : "dark";
	applyTheme(next);
}

themeToggleButton.addEventListener("click", toggleTheme);
