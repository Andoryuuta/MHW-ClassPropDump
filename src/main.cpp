#include <Windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <memory>
#include "spdlog/spdlog.h"
#include "SigScan.hpp"
#include "Mt.hpp"
#include "DTIDumper.hpp"

// Open new console for c(in/out/err)
void OpenConsole()
{
	AllocConsole();
	FILE* cinStream;
	FILE* coutStream;
	FILE* cerrStream;
	freopen_s(&cinStream, "CONIN$", "r", stdin);
	freopen_s(&coutStream, "CONOUT$", "w", stdout);
	freopen_s(&cerrStream, "CONOUT$", "w", stderr);


	// From: https://stackoverflow.com/a/45622802 to deal with UTF8 CP:
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);
}
DWORD WINAPI MyFunc(LPVOID lpvParam)
{
	OpenConsole();
	spdlog::info("Ando's MHWClassPropDump Started!");

	auto dumper = std::make_unique<DTIDumper::DTIDumper>();

	dumper->ParseDTI();
	dumper->DumpToFile("dti_prop_dump.h");
	dumper->DumpPythonArrayFile("dti_data.py");
	dumper->DumpResourceInformation("cresource_info.txt");

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		CreateThread(NULL, 0, MyFunc, 0, 0, NULL);
	}

	return TRUE;
}