import { cpSync, mkdirSync, rmSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(scriptDir, "..", "..");
const sourceDir = resolve(repoRoot, "release-data");
const publicDir = resolve(repoRoot, "portal", "public");
const targetDir = resolve(publicDir, "release-data");

mkdirSync(publicDir, { recursive: true });
rmSync(targetDir, { recursive: true, force: true });
cpSync(sourceDir, targetDir, { recursive: true });
