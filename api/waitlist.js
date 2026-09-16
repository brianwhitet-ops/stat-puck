import { captureWaitlist } from "../site/lib/waitlist.mjs";

async function readJsonBody(req) {
  if (req.body && typeof req.body === "object") return req.body;
  if (typeof req.body === "string" && req.body.trim()) {
    return JSON.parse(req.body);
  }

  const chunks = [];
  for await (const chunk of req) chunks.push(chunk);
  const raw = Buffer.concat(chunks).toString("utf8");
  return raw ? JSON.parse(raw) : {};
}

export default async function handler(req, res) {
  res.setHeader("Content-Type", "application/json; charset=utf-8");
  res.setHeader("Cache-Control", "no-store");

  if (req.method !== "POST") {
    res.statusCode = 405;
    res.setHeader("Allow", "POST");
    res.end(JSON.stringify({ ok: false, error: "Method not allowed." }));
    return;
  }

  let body = {};
  try {
    body = await readJsonBody(req);
  } catch {
    res.statusCode = 400;
    res.end(JSON.stringify({ ok: false, error: "Enter a valid email." }));
    return;
  }

  if (body._gotcha) {
    res.statusCode = 200;
    res.end(JSON.stringify({ ok: true }));
    return;
  }

  const result = await captureWaitlist(body.email, {
    formspreeId: process.env.FORMSPREE_FORM_ID,
    localPath: process.env.WAITLIST_LOCAL_PATH,
  });

  res.statusCode = result.status;
  res.end(
    JSON.stringify(
      result.ok
        ? { ok: true }
        : { ok: false, error: result.error || "Could not save that email." }
    )
  );
}
