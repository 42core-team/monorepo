import { readdir, readFile, stat, writeFile } from "node:fs/promises";
import { extname, join } from "node:path";

const [root, prefix] = process.argv.slice(2);
if (!root || !/^\/(?:dev|v\d+\.\d+\.\d+)$/.test(prefix ?? "")) {
	console.error("Usage: rewrite-base-path.mjs <dist-directory> </dev|/vN.N.N>");
	process.exit(1);
}

const textExtensions = new Set([".css", ".html", ".js"]);
const replacements = ["/assets/", "/replays/", "/favicon.ico", "/sw.js"];

async function rewrite(directory) {
	for (const entry of await readdir(directory)) {
		const path = join(directory, entry);
		const metadata = await stat(path);
		if (metadata.isDirectory()) {
			await rewrite(path);
			continue;
		}
		if (!textExtensions.has(extname(path))) continue;

		const original = await readFile(path, "utf8");
		let updated = original;
		for (const value of replacements) {
			updated = updated.replaceAll(value, `${prefix}${value}`);
		}
		if (updated !== original) await writeFile(path, updated);
	}
}

await rewrite(root);
