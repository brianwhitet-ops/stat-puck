# STATUS — ready for Brian’s final yes

**Branch:** `cursor/slab-product-site-cfb0`  
**PR:** https://github.com/brianwhitet-ops/stat-puck/pull/2  
**Not live.** No production deploy. No DNS change from this agent.

## What is ready

- Product site at `site/` (Concept 01 + six-screen system, playslabgolf.com branding).
- **Working waitlist E2E:** POST `/api/waitlist` persists the email.
  - Local preview: durable JSON file `site/.data/waitlist.json` (gitignored).
  - Production: **Formspree** inbox via `FORMSPREE_FORM_ID` (the one env Brian must set).
- Vercel config in-repo (`vercel.json`, `.vercelignore`, `/api/waitlist.js`) so go-live is import + env + domain, not a rewrite.
- Namecheap record sheet: `design/review-checkpoint/NAMECHEAP-DNS.md`.

## What Brian does after “yes”

1. **Formspree (2 minutes)**  
   Sign in at [formspree.io](https://formspree.io) with `brianwhitet@gmail.com`. New form. Copy the hash from `https://formspree.io/f/xxxxxxxx`.
2. **Vercel**  
   Import `brianwhitet-ops/stat-puck`. Framework: Other / no framework. `vercel.json` already sets `outputDirectory` to `site`.  
   Add env **`FORMSPREE_FORM_ID`** = that hash, Production (+ Preview if you want test signups).  
   Production deploy (Git production branch, or `vercel --prod` after merge).  
   Settings → Domains → add `playslabgolf.com` and `www.playslabgolf.com`.
3. **Namecheap**  
   Apply the A + CNAME (and only the TXT Vercel shows) from `NAMECHEAP-DNS.md`.  
   Do not change nameservers unless you want Vercel to own all DNS.

## Honesty still locked

No price, ship date, preorder, certified-weatherproof, or official GHIN claims. Waitlist remains interest-only: **Join the first-batch list.**

## Preview (no deploy)

```bash
npm run preview
```

Open http://127.0.0.1:4173/

Same as `node site/preview.mjs`. Signups land in `site/.data/waitlist.json`. To also forward to Formspree while previewing: `FORMSPREE_FORM_ID=xxxxxxxx npm run preview`.
