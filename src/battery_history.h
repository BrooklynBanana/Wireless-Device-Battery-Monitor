#pragma once
#include <windows.h>
#include <string>

namespace battery_history {
// Called once before polling starts. Demo data is ephemeral and never written to disk.
void Initialize(bool demo);
// Stores only valid, connected and awake readings (the caller enforces connection state).
void Observe(const std::wstring& key, int percent, bool charging);
// The same conservative time-to-empty the viewer shows ("~3 d 4 h"), or empty when there is no estimate
// (charging, too few or stale readings). Used by the taskbar hover card.
std::wstring RemainingText(const std::wstring& key);
// Non-modal native Win32 history viewer; device identity remains the original key.
void Show(HWND owner, const std::wstring& key, const std::wstring& displayName);
} // namespace battery_history
