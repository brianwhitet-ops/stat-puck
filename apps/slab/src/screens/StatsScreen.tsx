import { useState } from "react";
import { stats } from "../data/demo";
import { PageHeader } from "../components/PageHeader";

export function StatsScreen() {
  const [filter, setFilter] = useState<(typeof stats.filters)[number]>("Last 10");

  return (
    <main className="screen">
      <PageHeader title="Stats" dek="Every input. One filterable record." />

      <div className="pills" role="group" aria-label="Round window">
        {stats.filters.map((item) => (
          <button
            key={item}
            type="button"
            className="pill"
            aria-pressed={filter === item}
            onClick={() => setFilter(item)}
          >
            {item}
          </button>
        ))}
      </div>

      <section className="section" aria-labelledby="scoring">
        <h2 id="scoring" className="kicker">
          Scoring
        </h2>
        <div className="card card-light score-row">
          {stats.scoring.map((item) => (
            <div key={item.label}>
              <p className="mini-label">{item.label}</p>
              <p className="num">{item.value}</p>
            </div>
          ))}
        </div>
      </section>

      <section className="section" aria-labelledby="tee-green">
        <h2 id="tee-green" className="kicker">
          Tee to green
        </h2>
        <div className="stat-grid">
          <article className="stat-tile">
            <p className="mini-label">Captured</p>
            <p>Fairways</p>
            <p className="num">{stats.fairways.value}</p>
            <p className="miss">{stats.fairways.miss}</p>
          </article>
          <article className="stat-tile">
            <p className="mini-label">Derived</p>
            <p>Greens in reg.</p>
            <p className="num">{stats.gir.value}</p>
          </article>
          <article className="stat-tile">
            <p className="mini-label">Captured</p>
            <p>Putts</p>
            <p className="num">
              {stats.putts.value} <span className="unit">{stats.putts.unit}</span>
            </p>
          </article>
          <article className="stat-tile">
            <p className="mini-label">Derived</p>
            <p>Up &amp; down</p>
            <p className="num">{stats.upAndDown.value}</p>
          </article>
        </div>
      </section>

      <section className="section" aria-labelledby="sg">
        <h2 id="sg" className="kicker">
          Strokes gained
        </h2>
        <div className="sg-bar">
          {stats.strokesGained.map((item) => (
            <div key={item.label}>
              <p className="mini-label">{item.label}</p>
              <p className="num">{item.value}</p>
            </div>
          ))}
        </div>
        <p className="legend">Captured = direct input · Derived = round math</p>
      </section>
    </main>
  );
}
