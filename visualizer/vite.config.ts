import { defineConfig } from "vite";

export default defineConfig({
	server: {
		open: "/index.html",
		host: true,
		port: 4242,
		strictPort: true,
		proxy: { "/replay": { target: "ws://127.0.0.1:4445", ws: true } },
	},
	build: {
		outDir: "dist",
		assetsDir: "assets",
		sourcemap: true,
	},
});
