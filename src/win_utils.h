#pragma once

namespace WinUtils
{
// Open new Windows console for cin/cout/cerr.
void OpenConsole();

// SuspendThreads suspends all threads except the caller.
bool SuspendThreads();
} // namespace WinUtils
