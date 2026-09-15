# Slab product site (private visual review)

Static product page for **playslabgolf.com**. Do not deploy publicly.

## Preview

From the repository root:

```bash
python3 -m http.server 4173 --directory site
```

Open [http://127.0.0.1:4173/](http://127.0.0.1:4173/).

No build step. No production deploy. No DNS.

## Source

| Path | What |
| --- | --- |
| `site/index.html` | Page |
| `site/styles.css` | Layout and type |
| `site/site.js` | Waitlist modal + local interest list |
| `site/assets/hero.png` | Concept 01 hero (copy) |
| `site/assets/screens/` | Six-screen goldens (copies) |
| `design/concept-01/hero.png` | Locked Concept 01 plate |
| `design/instinct-firmware-goldens-pr1/` | Locked firmware goldens |
| `design/review-checkpoint/` | Desktop + 390px review screenshots |

## Review screenshots

| File | Viewport | What |
| --- | --- | --- |
| `design/review-checkpoint/desktop_hero_above_fold.png` | 1440×980 | Hero + Concept 01 |
| `design/review-checkpoint/desktop_six_screens.png` | 1440×1100 | Six-screen system |
| `design/review-checkpoint/desktop_review_stats.png` | 1440×980 | Review / improvement stats |
| `design/review-checkpoint/mobile_hero_390.png` | 390×844 | Mobile hero |
| `design/review-checkpoint/mobile_six_screens_390.png` | 390×844 | Mobile six-screen stack |
| `design/review-checkpoint/mobile_review_390.png` | 390×844 | Mobile review stats |
| `design/review-checkpoint/desktop_waitlist_modal.png` | 1440×980 | First-batch modal |
