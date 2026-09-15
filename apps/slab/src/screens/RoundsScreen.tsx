import { rounds } from "../data/demo";
import { PageHeader } from "../components/PageHeader";

export function RoundsScreen() {
  return (
    <main className="screen">
      <PageHeader title="Rounds" dek="Every round, one comparable record." />

      <section aria-labelledby="recent-rounds">
        <h2 id="recent-rounds" className="kicker">
          Recent rounds
        </h2>
        <ul className="round-list">
          {rounds.map((round) => (
            <li key={round.id}>
              <article className="card card-light round-card">
                <div>
                  <p className="round-date">{round.date}</p>
                  <h3 className="round-course">{round.course}</h3>
                  <p className="round-meta">
                    SG {round.sg} · {round.putts} putts · {round.gir} GIR
                  </p>
                </div>
                <p className="round-score">{round.score}</p>
              </article>
            </li>
          ))}
        </ul>
        <p className="foot-note">Tap a round for the scorecard and category evidence.</p>
      </section>
    </main>
  );
}
