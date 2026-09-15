import { Navigate, Route, Routes } from "react-router-dom";
import { AppShell } from "./components/AppShell";
import { BenchScreen } from "./screens/BenchScreen";
import { CurveScreen } from "./screens/CurveScreen";
import { LabScreen } from "./screens/LabScreen";
import { RoundsScreen } from "./screens/RoundsScreen";
import { StatsScreen } from "./screens/StatsScreen";

export function App() {
  return (
    <Routes>
      <Route element={<AppShell />}>
        <Route index element={<BenchScreen />} />
        <Route path="rounds" element={<RoundsScreen />} />
        <Route path="stats" element={<StatsScreen />} />
        <Route path="curve" element={<CurveScreen />} />
        <Route path="lab" element={<LabScreen />} />
        <Route path="*" element={<Navigate to="/" replace />} />
      </Route>
    </Routes>
  );
}
