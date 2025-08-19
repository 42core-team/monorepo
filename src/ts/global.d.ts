// TypeScript global window overrides for debug-visualizer

export {};

declare global {
	interface Window {
		Fireworks: typeof import("fireworks-js").default;
	}
	interface Document {
		webkitFullscreenElement: Element | null;
		webkitExitFullscreen: () => Promise<void>;
	}
	interface HTMLElement {
		webkitRequestFullscreen?: () => Promise<void>;
	}
}
