#pragma once
#include <windows.h>
#include <set>
#include <string>

namespace battery_history {
// Called once before polling starts. Demo data is ephemeral and never written to disk.
void Initialize(bool demo);
// Keys that had readings in the history file when Initialize() ran. Filled once before polling starts and
// never changed afterwards, so the poll thread may read it.
const std::set<std::wstring>& KeysAtStartup();
// Stores only valid, connected and awake readings (the caller enforces connection state).
void Observe(const std::wstring& key, int percent, bool charging);
// The same conservative time-to-empty the viewer shows, in hours, or -1 when there is none (charging, too
// few or stale readings); RemainingText formats it ("~3 d 4 h") or returns an empty string.
double HoursRemaining(const std::wstring& key);
std::wstring RemainingText(const std::wstring& key);
// Non-modal native Win32 history viewer; device identity remains the original key.
void Show(HWND owner, const std::wstring& key, const std::wstring& displayName);
} // namespace battery_history
