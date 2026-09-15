import { lab } from "../data/demo";
import { PageHeader } from "../components/PageHeader";

export function LabScreen() {
  return (
    <main className="screen">
      <PageHeader title="Lab" dek="Your coach turns round patterns into a plan." />

      <section className="section" aria-labelledby="experiment">
        <h2 id="experiment" className="kicker">
          This week&apos;s experiment
        </h2>
        <article className="card card-dark">
          <p className="card-kicker">{lab.kicker}</p>
          <h3 className="lab-title">{lab.title}</h3>
          <p className="card-note">{lab.why}</p>
          <p className="card-note">{lab.target}</p>
        </article>
      </section>

      <section className="section" aria-labelledby="sessions">
        <h2 id="sessions" className="kicker">
          Sessions
        </h2>
        <ul className="sessions">
          {lab.sessions.map((session) => (
            <li key={session.id}>
              <article className={`session${session.done ? " is-done" : ""}`}>
                <span className={`mark${session.done ? " is-done" : ""}`} aria-hidden="true">
                  {session.done ? (
                    <svg width="12" height="12" viewBox="0 0 12 12">
                      <path
                        d="M2.2 6.1 4.7 8.5 9.8 3.3"
                        fill="none"
                        stroke="white"
                        strokeWidth="1.7"
                        strokeLinecap="round"
                        strokeLinejoin="round"
                      />
                    </svg>
                  ) : null}
                </span>
                <div>
                  <h3>{session.title}</h3>
                  <p>{session.detail}</p>
                </div>
              </article>
            </li>
          ))}
        </ul>
        <button type="button" className="coach-btn">
          Check in with coach
        </button>
      </section>
    </main>
  );
}
