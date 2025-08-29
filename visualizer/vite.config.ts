import { defineConfig } from "vite";

export default defineConfig({
	server: { open: "/index.html", host: true, port: 4242, strictPort: true },
	build: {
		outDir: "dist",
		assetsDir: "assets",
		sourcemap: true,
	},
});
