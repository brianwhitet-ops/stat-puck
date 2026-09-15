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
