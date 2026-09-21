# Stats Lab — Slab product site (playslabgolf.com)

Static product page plus a same-origin waitlist API. Do not deploy until Brian says yes.

## Preview

From the repository root:

```bash
npm run preview
```

Open [http://127.0.0.1:4173/](http://127.0.0.1:4173/).

The preview server serves `site/` and handles `POST /api/waitlist`. Signups persist in `site/.data/waitlist.json` (gitignored). Optional: `FORMSPREE_FORM_ID=xxxxxxxx npm run preview` also forwards to Formspree.

## Production host

**Vercel** (already configured in-repo). Import the GitHub repo. `vercel.json` publishes `site/` and the `/api/waitlist` function. Set `FORMSPREE_FORM_ID`. Then add the domain and apply `design/review-checkpoint/NAMECHEAP-DNS.md`.

See `design/review-checkpoint/STATUS.md`.

## Source

| Path | What |
| --- | --- |
| `site/index.html` | Page |
| `site/styles.css` | Type and layout |
| `site/site.js` | Modal + waitlist client |
| `site/preview.mjs` | Local preview + waitlist API |
| `site/lib/waitlist.mjs` | Shared capture (Formspree + file) |
| `api/waitlist.js` | Vercel function |
| `design/concept-01/hero.png` | Locked Concept 01 plate |
| `design/instinct-firmware-goldens-pr1/` | Locked six-screen goldens |
| `design/review-checkpoint/NAMECHEAP-DNS.md` | Apex + www records |
| `design/review-checkpoint/STATUS.md` | Go-live checklist |
