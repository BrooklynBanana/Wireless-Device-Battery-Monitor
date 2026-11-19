#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace battery_history {
// Called once before polling starts. Demo data is ephemeral and never written to disk.
void Initialize(bool demo);
// Stores only valid, connected and awake readings (the caller enforces connection state).
void Observe(const std::wstring& key, int percent, bool charging);
// The same conservative time-to-empty the viewer shows, in hours, or -1 when there is none (charging, too
// few or stale readings); RemainingText formats it ("~3 d 4 h") or returns an empty string.
double HoursRemaining(const std::wstring& key);
std::wstring RemainingText(const std::wstring& key);
// The recorded readings of the last `hours`, oldest first, for the small chart in the hover card.
// secondsAgo is negative (0 = now); nothing is interpolated.
struct Point { long long secondsAgo; int percent; bool charging; };
std::vector<Point> Recent(const std::wstring& key, int hours);
// Non-modal native Win32 history viewer; device identity remains the original key.
void Show(HWND owner, const std::wstring& key, const std::wstring& displayName);
} // namespace battery_history
