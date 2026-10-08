const API_URL = import.meta.env.VITE_API_URL || "http://localhost:8080";

export async function planMission(request) {
  const res = await fetch(`${API_URL}/plan`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(request),
  });
  const data = await res.json().catch(() => ({}));
  if (!res.ok) throw new Error(data.error || "Planning failed");
  return data;
}