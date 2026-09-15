import { useState } from "react";
import { curve } from "../data/demo";
import { PageHeader } from "../components/PageHeader";

function line(values: number[], width: number, height: number): string {
  const min = Math.min(...values, ...curve.trend);
  const max = Math.max(...values, ...curve.trend);
  const span = Math.max(max - min, 1);
  return values
    .map((value, index) => {
      const x = (index / (values.length - 1)) * width;
      const y = height - ((value - min) / span) * (height - 16) - 8;
      return `${x},${y}`;
    })
    .join(" ");
}

export function CurveScreen() {
  const [metric, setMetric] = useState<(typeof curve.metrics)[number]>("Strokes gained");
  const series = line(curve.series, 342, 132);
  const trend = line(curve.trend, 342, 132);

  return (
    <main className="screen">
      <PageHeader title="Curve" dek="See the trend, then understand the cause." />

      <section aria-labelledby="metric">
        <h2 id="metric" className="kicker">
          Metric
        </h2>
        <div className="metric-pills" role="group" aria-label="Metric">
          {curve.metrics.map((item) => (
            <button
              key={item}
              type="button"
              className="metric-pill"
              aria-pressed={metric === item}
              onClick={() => setMetric(item)}
            >
              {item}
            </button>
          ))}
        </div>

        <p className="num curve-value">{metric === "Handicap" ? "12.4" : curve.value}</p>
        <p className="curve-unit">{metric === "Handicap" ? "handicap" : curve.unit}</p>
        <p className="curve-note">{curve.note}</p>

        <svg className="trend" viewBox="0 0 342 132" aria-hidden="true">
          <polyline
            fill="none"
            stroke="#7d8a78"
            strokeWidth="1.4"
            strokeDasharray="4 5"
            points={trend}
          />
          <polyline
            fill="none"
            stroke="#141413"
            strokeWidth="1.8"
            strokeLinejoin="round"
            strokeLinecap="round"
            points={series}
          />
        </svg>
      </section>

      <section className="section" aria-labelledby="changed">
        <h2 id="changed" className="kicker">
          What changed
        </h2>
        <article className="card card-soft">
          <h3 className="changed-title">{curve.changedTitle}</h3>
          <div className="changed-lines">
            {curve.changedLines.map((lineText) => (
              <p key={lineText}>{lineText}</p>
            ))}
          </div>
        </article>
      </section>
    </main>
  );
}
