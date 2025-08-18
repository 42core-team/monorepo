// TypeScript global window overrides for debug-visualizer

export {};

declare global {
	interface Window {
		// Add custom properties or methods used in the app here
		// All standard properties (like location, addEventListener) are already included
	}
	interface Document {
		webkitFullscreenElement: Element | null;
		webkitExitFullscreen: () => Promise<void>;
	}
	interface HTMLElement {
		webkitRequestFullscreen?: () => Promise<void>;
	}
}
