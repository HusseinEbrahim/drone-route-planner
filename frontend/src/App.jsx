import { useEffect, useState } from "react";
import { planMission } from "./api";

const WIDTH = 30;
const HEIGHT = 18;
const CELL = 26;
const MAX_STOPS = 15;
const TRIP_COLORS = ["#2563eb", "#16a34a", "#ea580c", "#9333ea", "#db2777", "#0891b2"];

const TOOLS = [
  { id: "obstacle", label: "Building", swatch: "bg-slate-700" },
  { id: "nofly", label: "No-fly zone", swatch: "bg-red-300" },
  { id: "home", label: "Home base", swatch: "bg-amber-400" },
  { id: "stop", label: "Delivery stop", swatch: "bg-sky-500" },
  { id: "erase", label: "Eraser", swatch: "bg-white border border-slate-300" },
];
const DRAG_TOOLS = new Set(["obstacle", "nofly", "erase"]);
const CELL_COLORS = { free: "bg-white", obstacle: "bg-slate-700", nofly: "bg-red-200" };

const idx = (x, y) => y * WIDTH + x;
const center = (v) => v * CELL + CELL / 2;
const emptyCells = () => Array(WIDTH * HEIGHT).fill("free");

function randomCity() {
  const cells = emptyCells();
  for (let i = 0; i < 18; i++) {
    const w = 1 + Math.floor(Math.random() * 4);
    const h = 1 + Math.floor(Math.random() * 4);
    const x0 = Math.floor(Math.random() * (WIDTH - w));
    const y0 = Math.floor(Math.random() * (HEIGHT - h));
    for (let y = y0; y < y0 + h; y++) {
      for (let x = x0; x < x0 + w; x++) cells[idx(x, y)] = "obstacle";
    }
  }
  for (let i = 0; i < 2; i++) {
    const r = 1 + Math.floor(Math.random() * 2);
    const cx = r + Math.floor(Math.random() * (WIDTH - 2 * r));
    const cy = r + Math.floor(Math.random() * (HEIGHT - 2 * r));
    for (let y = cy - r; y <= cy + r; y++) {
      for (let x = cx - r; x <= cx + r; x++) cells[idx(x, y)] = "nofly";
    }
  }

  const home = { x: 1, y: 1 };
  cells[idx(home.x, home.y)] = "free";

  const stops = [];
  while (stops.length < 6) {
    const x = Math.floor(Math.random() * WIDTH);
    const y = Math.floor(Math.random() * HEIGHT);
    const taken = (x === home.x && y === home.y) || stops.some((s) => s.x === x && s.y === y);
    if (cells[idx(x, y)] === "free" && !taken) stops.push({ x, y });
  }
  return { cells, home, stops };
}

export default function App() {
  const [map, setMap] = useState(randomCity);
  const [tool, setTool] = useState("obstacle");
  const [battery, setBattery] = useState(60);
  const [painting, setPainting] = useState(false);
  const [result, setResult] = useState(null);
  const [progress, setProgress] = useState(0);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");

  // Stop painting when the mouse is released anywhere
  useEffect(() => {
    const stop = () => setPainting(false);
    window.addEventListener("mouseup", stop);
    return () => window.removeEventListener("mouseup", stop);
  }, []);

  // Animate the drone along all trips, one square at a time
  const totalPoints = result ? result.trips.reduce((n, t) => n + t.path.length, 0) : 0;
  useEffect(() => {
    if (!result) return;
    setProgress(0);
    const timer = setInterval(() => {
      setProgress((p) => {
        if (p >= totalPoints) {
          clearInterval(timer);
          return p;
        }
        return p + 1;
      });
    }, 35);
    return () => clearInterval(timer);
  }, [result, totalPoints]);

  function applyTool(x, y) {
    setMap((prev) => {
      const cells = [...prev.cells];
      let { home, stops } = prev;
      const isHome = home.x === x && home.y === y;
      const stopIndex = stops.findIndex((s) => s.x === x && s.y === y);
      const removeStop = () => stops.filter((_, i) => i !== stopIndex);

      if (tool === "obstacle" || tool === "nofly") {
        if (isHome) return prev;
        cells[idx(x, y)] = tool;
        if (stopIndex !== -1) stops = removeStop();
      } else if (tool === "erase") {
        cells[idx(x, y)] = "free";
        if (stopIndex !== -1) stops = removeStop();
      } else if (tool === "home") {
        if (cells[idx(x, y)] !== "free") return prev;
        home = { x, y };
        if (stopIndex !== -1) stops = removeStop();
      } else if (tool === "stop") {
        if (stopIndex !== -1) stops = removeStop();
        else if (cells[idx(x, y)] === "free" && !isHome && stops.length < MAX_STOPS) stops = [...stops, { x, y }];
        else return prev;
      }
      return { cells, home, stops };
    });
    setResult(null);
  }

  async function handlePlan() {
    if (map.stops.length === 0) {
      setError("Add at least one delivery stop");
      return;
    }
    setLoading(true);
    setError("");
    try {
      const obstacles = [];
      const noFly = [];
      map.cells.forEach((cell, i) => {
        const point = [i % WIDTH, Math.floor(i / WIDTH)];
        if (cell === "obstacle") obstacles.push(point);
        else if (cell === "nofly") noFly.push(point);
      });
      const data = await planMission({
        width: WIDTH,
        height: HEIGHT,
        obstacles,
        noFly,
        home: [map.home.x, map.home.y],
        stops: map.stops.map((s) => [s.x, s.y]),
        batteryRange: battery,
      });
      setResult(data);
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  }

  function resetMap(next) {
    setMap(next);
    setResult(null);
    setError("");
  }

  // Work out how much of each trip's line to show at this point in the animation
  let offset = 0;
  const tripLines = (result?.trips ?? []).map((trip, i) => {
    const visible = Math.max(0, Math.min(trip.path.length, progress - offset));
    offset += trip.path.length;
    return { color: TRIP_COLORS[i % TRIP_COLORS.length], points: trip.path.slice(0, visible) };
  });
  const activeLine = [...tripLines].reverse().find((line) => line.points.length > 0);
  const drone = activeLine?.points[activeLine.points.length - 1];

  return (
    <div className="min-h-screen bg-slate-100 p-6">
      <div className="max-w-6xl mx-auto space-y-5">
        <header>
          <h1 className="text-2xl font-bold text-slate-800">Drone Route Planner</h1>
          <p className="text-sm text-slate-500">
            Draw a city, place delivery stops, and let the C++ engine plan battery-safe routes with A* and 2-opt.
          </p>
        </header>

        <div className="bg-white rounded-xl shadow p-4 flex flex-wrap items-center gap-3">
          {TOOLS.map((t) => (
            <button
              key={t.id}
              onClick={() => setTool(t.id)}
              className={`flex items-center gap-2 rounded-lg px-3 py-2 text-sm ${
                tool === t.id ? "bg-slate-900 text-white" : "bg-slate-100 text-slate-700 hover:bg-slate-200"
              }`}
            >
              <span className={`w-3 h-3 rounded-sm ${t.swatch}`} />
              {t.label}
            </button>
          ))}

          <div className="flex items-center gap-2 ml-auto text-sm text-slate-700">
            <label htmlFor="battery">Battery range</label>
            <input
              id="battery"
              type="range"
              min="10"
              max="200"
              value={battery}
              onChange={(e) => {
                setBattery(Number(e.target.value));
                setResult(null);
              }}
            />
            <span className="w-10 font-semibold">{battery}</span>
          </div>
        </div>

        <div className="flex flex-wrap gap-2">
          <button
            onClick={handlePlan}
            disabled={loading}
            className="rounded-lg bg-slate-900 text-white px-5 py-2 font-medium hover:bg-slate-800 disabled:opacity-50"
          >
            {loading ? "Planning..." : "Plan route"}
          </button>
          <button
            onClick={() => resetMap(randomCity())}
            className="rounded-lg bg-white px-4 py-2 text-slate-700 shadow hover:bg-slate-50"
          >
            Random city
          </button>
          <button
            onClick={() => resetMap({ cells: emptyCells(), home: map.home, stops: [] })}
            className="rounded-lg bg-white px-4 py-2 text-slate-700 shadow hover:bg-slate-50"
          >
            Clear map
          </button>
        </div>

        {error && <div className="rounded-lg bg-red-50 text-red-700 px-4 py-3 text-sm">{error}</div>}

        <div className="bg-white rounded-xl shadow p-4 overflow-auto">
          <div className="relative select-none" style={{ width: WIDTH * CELL, height: HEIGHT * CELL }}>
            <div className="grid" style={{ gridTemplateColumns: `repeat(${WIDTH}, ${CELL}px)` }}>
              {map.cells.map((cell, i) => {
                const x = i % WIDTH;
                const y = Math.floor(i / WIDTH);
                const isHome = map.home.x === x && map.home.y === y;
                const stopIndex = map.stops.findIndex((s) => s.x === x && s.y === y);
                const unreachable = result?.unreachableStops.includes(stopIndex);
                return (
                  <div
                    key={i}
                    onMouseDown={() => {
                      setPainting(true);
                      applyTool(x, y);
                    }}
                    onMouseEnter={() => {
                      if (painting && DRAG_TOOLS.has(tool)) applyTool(x, y);
                    }}
                    className={`border border-slate-100 flex items-center justify-center ${CELL_COLORS[cell]}`}
                    style={{ width: CELL, height: CELL }}
                  >
                    {isHome && (
                      <span className="w-5 h-5 rounded bg-amber-400 text-white text-xs font-bold flex items-center justify-center">
                        H
                      </span>
                    )}
                    {stopIndex !== -1 && (
                      <span
                        className={`w-5 h-5 rounded-full text-white text-xs font-bold flex items-center justify-center ${
                          unreachable ? "bg-red-500" : "bg-sky-500"
                        }`}
                      >
                        {stopIndex + 1}
                      </span>
                    )}
                  </div>
                );
              })}
            </div>

            <svg className="absolute inset-0 pointer-events-none" width={WIDTH * CELL} height={HEIGHT * CELL}>
              {tripLines.map(
                (line, i) =>
                  line.points.length > 1 && (
                    <polyline
                      key={i}
                      points={line.points.map(([x, y]) => `${center(x)},${center(y)}`).join(" ")}
                      fill="none"
                      stroke={line.color}
                      strokeWidth={4}
                      strokeLinecap="round"
                      strokeLinejoin="round"
                      opacity={0.85}
                    />
                  )
              )}
              {drone && (
                <circle cx={center(drone[0])} cy={center(drone[1])} r={8} fill="#0f172a" stroke="white" strokeWidth={3} />
              )}
            </svg>
          </div>
        </div>

        {result && (
          <div className="bg-white rounded-xl shadow p-5 space-y-3">
            <div className="flex flex-wrap gap-6 text-sm">
              <div>
                <div className="text-slate-500">Trips</div>
                <div className="text-xl font-bold text-slate-800">{result.trips.length}</div>
              </div>
              <div>
                <div className="text-slate-500">Total distance</div>
                <div className="text-xl font-bold text-slate-800">{result.totalDistance.toFixed(1)}</div>
              </div>
              <div>
                <div className="text-slate-500">Engine compute time</div>
                <div className="text-xl font-bold text-slate-800">{result.computeTimeMs.toFixed(2)} ms</div>
              </div>
            </div>

            <ul className="space-y-1 text-sm">
              {result.trips.map((trip, i) => (
                <li key={i} className="flex items-center gap-2">
                  <span className="w-3 h-3 rounded-full" style={{ backgroundColor: TRIP_COLORS[i % TRIP_COLORS.length] }} />
                  <span className="text-slate-800">
                    Trip {i + 1}: H → {trip.stops.map((s) => s + 1).join(" → ")} → H
                  </span>
                  <span className="text-slate-500">({trip.distance.toFixed(1)})</span>
                </li>
              ))}
            </ul>

            {result.unreachableStops.length > 0 && (
              <p className="text-sm text-red-600">
                Unreachable stops: {result.unreachableStops.map((s) => s + 1).join(", ")} (blocked, or too far for this
                battery)
              </p>
            )}
          </div>
        )}
      </div>
    </div>
  );
}