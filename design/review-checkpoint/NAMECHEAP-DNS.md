# Namecheap DNS for playslabgolf.com

Do **not** apply these until Brian says yes to go live. This sheet is the exact
Namecheap Advanced DNS set for hosting the static site on **Vercel**.

Keep Namecheap as the registrar and nameserver. Do not move nameservers to
Vercel unless you explicitly want Vercel to manage all DNS (email, extras).

## Before you touch DNS

1. Import `brianwhitet-ops/stat-puck` into Vercel (root of repo, `vercel.json` already points `outputDirectory` at `site/`).
2. Project → **Settings → Domains** → add `playslabgolf.com` and `www.playslabgolf.com`.
3. Set production env `FORMSPREE_FORM_ID` (see `STATUS.md`).
4. Copy any **TXT verification** Vercel shows — it is account-specific.

## Records to add in Namecheap

**Domain List → playslabgolf.com → Manage → Advanced DNS**

Remove leftover **URL Redirect**, parking, or stale A/CNAME records on `@` and `www` that would fight these.

| Type | Host | Value | TTL |
| --- | --- | --- | --- |
| **A** | `@` | `10.0.1.2` | Automatic |
| **CNAME** | `www` | `cname.vercel-dns.com.` | Automatic |

Notes:

- Namecheap Host `@` means the apex (`playslabgolf.com`).
- Leave the trailing dot on the CNAME if Namecheap shows it; if the UI strips it, `cname.vercel-dns.com` is the same record.
- Do not also point `@` at a URL Redirect. Apex must be the A record above.

## Verification (only if Vercel asks)

After you add the domain in Vercel, the Domains panel may show a TXT check.
Add **exactly** what Vercel displays. Typical shapes:

| Type | Host | Value | When |
| --- | --- | --- | --- |
| **TXT** | `_vercel` | value from Vercel | if Vercel lists `_vercel` |
| **TXT** | `@` | value from Vercel | if Vercel lists the apex |

Do not invent a TXT value. If Vercel does not ask, skip this section.

## Recommended Vercel domain pairing

In Vercel → Settings → Domains:

- `www.playslabgolf.com` → production
- `playslabgolf.com` → **301 redirect** to `www.playslabgolf.com` (Vercel’s usual recommendation)

Canonical tags on the site already use `https://playslabgolf.com/`. Either apex or www as the primary is fine; pick one in Vercel and keep the other as the redirect.

## After save

1. Wait for Namecheap to show the new records (often minutes; can be a few hours).
2. Vercel Domains should move from *Invalid Configuration* to *Valid*.
3. HTTPS is issued automatically. Do not add a third-party SSL product at Namecheap.
4. Confirm `https://playslabgolf.com` and `https://www.playslabgolf.com`.

## Leave alone

- MX / email records if the domain already receives mail — this sheet does not replace them.
- Nameserver settings (keep Namecheap BasicDNS / PremiumDNS unless you decide to move DNS to Vercel).
- Do not add `slab.golf` or `playgolfslab.com`.
