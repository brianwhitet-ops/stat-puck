import { bench } from "../data/demo";
import { PageHeader } from "../components/PageHeader";

function polyline(values: number[], width: number, height: number): string {
  const min = Math.min(...values);
  const max = Math.max(...values);
  const span = Math.max(max - min, 1);
  return values
    .map((value, index) => {
      const x = (index / (values.length - 1)) * width;
      const y = height - ((value - min) / span) * (height - 6) - 3;
      return `${x},${y}`;
    })
    .join(" ");
}

export function BenchScreen() {
  const points = polyline(bench.sparkline, 118, 54);

  return (
    <main className="screen">
      <PageHeader title={bench.title} dek={bench.dek} />

      <section className="section" aria-labelledby="last-rounds">
        <h2 id="last-rounds" className="kicker">
          {bench.windowLabel}
        </h2>
        <div className="hero-row">
          <p className="num hero-num">{bench.handicap}</p>
          <div className="hero-meta">
            <span className="hero-label">Handicap</span>
            <span className="hero-delta">↓ {bench.handicapDelta}</span>
          </div>
          <svg className="spark" viewBox="0 0 118 54" aria-hidden="true">
            <polyline
              fill="none"
              stroke="#141413"
              strokeWidth="1.6"
              strokeLinejoin="round"
              strokeLinecap="round"
              points={points}
            />
          </svg>
        </div>
      </section>

      <section className="section" aria-labelledby="opportunity">
        <h2 id="opportunity" className="kicker">
          Biggest opportunity
        </h2>
        <article className="card card-dark">
          <p className="card-kicker">{bench.opportunityKicker}</p>
          <p className="card-stat">
            <span className="num">{bench.opportunityValue}</span>
            <span className="card-stat-unit">{bench.opportunityUnit}</span>
          </p>
          <p className="card-note">{bench.opportunityNote}</p>
        </article>
      </section>

      <section className="section" aria-labelledby="practice">
        <h2 id="practice" className="kicker">
          Next practice
        </h2>
        <article className="card card-soft">
          <h3 className="practice-title">{bench.practiceTitle}</h3>
          <p className="practice-meta">{bench.practiceMeta}</p>
          <p className="practice-why">{bench.practiceWhy}</p>
        </article>
      </section>
    </main>
  );
}
