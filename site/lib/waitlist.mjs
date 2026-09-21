import { mkdir, readFile, writeFile } from "node:fs/promises";
import path from "node:path";

const EMAIL_RE = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;

export function normalizeEmail(raw) {
  return String(raw || "").trim().toLowerCase();
}

export function isValidEmail(email) {
  return EMAIL_RE.test(email) && email.length <= 254;
}

async function forwardFormspree(email, formId) {
  const response = await fetch(`https://formspree.io/f/${formId}`, {
    method: "POST",
    headers: {
      Accept: "application/json",
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      email,
      _replyto: email,
      _subject: "Stats Lab — Slab first-batch list",
      source: "playslabgolf.com",
    }),
  });

  let payload = {};
  try {
    payload = await response.json();
  } catch {
    payload = {};
  }

  if (!response.ok) {
    const message =
      payload.error ||
      payload.errors?.[0]?.message ||
      "Could not save that email. Try again.";
    return { ok: false, status: response.status >= 500 ? 502 : 400, error: message };
  }

  return { ok: true, status: 200, stored: "formspree" };
}

async function appendLocal(email, filePath) {
  const resolved = path.resolve(filePath);
  await mkdir(path.dirname(resolved), { recursive: true });

  let entries = [];
  try {
    const raw = await readFile(resolved, "utf8");
    const parsed = JSON.parse(raw);
    if (Array.isArray(parsed)) entries = parsed;
  } catch {
    entries = [];
  }

  const existing = entries.find((row) => row.email === email);
  if (existing) {
    existing.updatedAt = new Date().toISOString();
  } else {
    entries.push({
      email,
      createdAt: new Date().toISOString(),
      source: "playslabgolf.com",
    });
  }

  await writeFile(resolved, `${JSON.stringify(entries, null, 2)}\n`, "utf8");
  return { ok: true, status: 200, stored: "local", duplicate: Boolean(existing) };
}

export async function captureWaitlist(rawEmail, env = {}) {
  const email = normalizeEmail(rawEmail);
  if (!isValidEmail(email)) {
    return { ok: false, status: 400, error: "Enter a valid email." };
  }

  const formId = env.formspreeId || env.FORMSPREE_FORM_ID;
  const localPath = env.localPath || env.WAITLIST_LOCAL_PATH;
  const jobs = [];

  if (formId) jobs.push(forwardFormspree(email, formId));
  if (localPath) jobs.push(appendLocal(email, localPath));

  if (jobs.length === 0) {
    return {
      ok: false,
      status: 503,
      error: "Waitlist is not configured yet.",
    };
  }

  const results = await Promise.allSettled(jobs);
  const succeeded = results.filter(
    (result) => result.status === "fulfilled" && result.value.ok
  );
  if (succeeded.length > 0) {
    return { ok: true, status: 200 };
  }

  const failed = results.find((result) => result.status === "fulfilled");
  if (failed) return failed.value;

  return {
    ok: false,
    status: 502,
    error: "Could not save that email. Try again.",
  };
}
