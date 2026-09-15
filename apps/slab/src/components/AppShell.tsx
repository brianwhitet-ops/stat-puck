import { NavLink, Outlet } from "react-router-dom";

const tabs = [
  { to: "/", label: "Bench" },
  { to: "/rounds", label: "Rounds" },
  { to: "/stats", label: "Stats" },
  { to: "/curve", label: "Curve" },
  { to: "/lab", label: "Lab" },
] as const;

export function AppShell() {
  return (
    <div className="stage">
      <div className="device">
        <Outlet />
        <nav className="nav" aria-label="Slab">
          {tabs.map((tab) => (
            <NavLink key={tab.to} to={tab.to} end={tab.to === "/"}>
              <span className="dot" aria-hidden="true" />
              {tab.label}
            </NavLink>
          ))}
        </nav>
      </div>
    </div>
  );
}
