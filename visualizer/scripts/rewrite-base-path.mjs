import { readdir, readFile, writeFile } from "node:fs/promises";
import { extname, join } from "node:path";

// Vite emits root-relative URLs. Versioned builds are served below /dev or
// /vN.N.N, so rewrite those URLs after the build to include that base path.
const [root, prefix] = process.argv.slice(2);
if (!root || !/^\/(?:dev|v\d+\.\d+\.\d+)$/.test(prefix ?? "")) {
	console.error("Usage: rewrite-base-path.mjs <dist-directory> </dev|/vN.N.N>");
	process.exit(1);
}

const textExtensions = new Set([".css", ".html", ".js"]);
const rootPaths = ["/assets/", "/replays/", "/favicon.ico", "/sw.js"];

async function rewrite(directory) {
	for (const entry of await readdir(directory, { withFileTypes: true })) {
		const path = join(directory, entry.name);
		if (entry.isDirectory()) {
			await rewrite(path);
			continue;
		}
		if (!textExtensions.has(extname(path))) continue;

		const original = await readFile(path, "utf8");
		let updated = original;
		for (const rootPath of rootPaths) {
			updated = updated.replaceAll(rootPath, `${prefix}${rootPath}`);
		}
		if (updated !== original) await writeFile(path, updated);
	}
}

await rewrite(root);
