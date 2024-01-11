#include "win_utils.h"

#include <Windows.h>
#include <cstdio>
#include <iostream>
#include <tlhelp32.h>

namespace WinUtils
{

// Open new console for c(in/out/err)
void OpenConsole()
{
    AllocConsole();
    FILE *cinStream;
    FILE *coutStream;
    FILE *cerrStream;
    freopen_s(&cinStream, "CONIN$", "r", stdin);
    freopen_s(&coutStream, "CONOUT$", "w", stdout);
    freopen_s(&cerrStream, "CONOUT$", "w", stderr);

    // From: https://stackoverflow.com/a/45622802 to deal with UTF8 CP:
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
}

// SuspendThreads suspends all threads except the caller.
bool SuspendThreads()
{
    DWORD cur_tid = GetCurrentThreadId();
    DWORD cur_pid = GetCurrentProcessId();

    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return false;

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    if (!Thread32First(hThreadSnap, &te32))
    {
        CloseHandle(hThreadSnap);
        return false;
    }

    do
    {
        // Skip for caller thread.
        if (cur_tid == te32.th32ThreadID)
        {
            continue;
        }

        if (te32.th32OwnerProcessID != cur_pid)
        {
            continue;
        }

        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
        if (hThread != NULL)
        {
            std::cout << "Suspending thread: " << te32.th32ThreadID << std::endl;
            SuspendThread(hThread);
        }

    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
    return true;
}

} // namespace WinUtils