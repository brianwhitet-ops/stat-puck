import { createServer } from "node:http";
import { readFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { captureWaitlist } from "./lib/waitlist.mjs";

const root = path.dirname(fileURLToPath(import.meta.url));
const port = Number(process.env.PORT || 4173);
const localPath =
  process.env.WAITLIST_LOCAL_PATH || path.join(root, ".data", "waitlist.json");

const types = {
  ".html": "text/html; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".mjs": "text/javascript; charset=utf-8",
  ".svg": "image/svg+xml",
  ".png": "image/png",
  ".jpg": "image/jpeg",
  ".jpeg": "image/jpeg",
  ".txt": "text/plain; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".ico": "image/x-icon",
};

function send(res, status, body, headers = {}) {
  res.writeHead(status, headers);
  res.end(body);
}

async function readJson(req) {
  const chunks = [];
  for await (const chunk of req) chunks.push(chunk);
  const raw = Buffer.concat(chunks).toString("utf8");
  return raw ? JSON.parse(raw) : {};
}

const server = createServer(async (req, res) => {
  const url = new URL(req.url || "/", `http://127.0.0.1:${port}`);

  if (url.pathname === "/api/waitlist") {
    res.setHeader("Content-Type", "application/json; charset=utf-8");
    res.setHeader("Cache-Control", "no-store");
    if (req.method !== "POST") {
      send(res, 405, JSON.stringify({ ok: false, error: "Method not allowed." }));
      return;
    }

    try {
      const body = await readJson(req);
      if (body._gotcha) {
        send(res, 200, JSON.stringify({ ok: true }));
        return;
      }
      const result = await captureWaitlist(body.email, {
        formspreeId: process.env.FORMSPREE_FORM_ID,
        localPath,
      });
      send(
        res,
        result.status,
        JSON.stringify(
          result.ok
            ? { ok: true }
            : { ok: false, error: result.error || "Could not save that email." }
        )
      );
    } catch {
      send(res, 400, JSON.stringify({ ok: false, error: "Enter a valid email." }));
    }
    return;
  }

  let filePath = path.join(root, url.pathname === "/" ? "index.html" : url.pathname);
  if (!filePath.startsWith(root)) {
    send(res, 403, "Forbidden");
    return;
  }

  try {
    const data = await readFile(filePath);
    const ext = path.extname(filePath);
    send(res, 200, data, { "Content-Type": types[ext] || "application/octet-stream" });
  } catch {
    send(res, 404, "Not found", { "Content-Type": "text/plain; charset=utf-8" });
  }
});

server.listen(port, "127.0.0.1", () => {
  console.log(`Stats Lab · Slab site preview http://127.0.0.1:${port}/`);
  console.log(`Waitlist file ${localPath}`);
  if (process.env.FORMSPREE_FORM_ID) {
    console.log("Formspree forwarding is on.");
  } else {
    console.log("Formspree is off. Local file store is on for this preview.");
  }
});
