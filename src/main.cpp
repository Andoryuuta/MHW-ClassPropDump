#include <Windows.h>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <tlhelp32.h>

#include "spdlog/spdlog.h"

// #include "DTIDumper.hpp"
#include "dumper.h"
#include "mt/mt.h"
#include "sig_scan.h"
#include "win_utils.h"

DWORD WINAPI MyFunc(LPVOID lpvParam)
{
    WinUtils::OpenConsole();
    spdlog::info("Ando's MHWClassPropDump Started!");

    // Get the image base.
    uintptr_t image_base = (uintptr_t)GetModuleHandle(NULL);
    spdlog::info("Image Base: {0:x}", image_base);

    // Create our dumper object.
    std::shared_ptr<Dumper::Dumper> dumper = std::make_unique<Dumper::Dumper>(image_base);

    // Suspend all threads except ours.
    // This helps to prevent crashes caused by us calling to the game's code
    // or by race conditions invalidating pointers our dumper would be using.
    WinUtils::SuspendThreads();

    // Perform all game-specific intialization needed
    // for MtDTI and MtProperty operations.
    dumper->Initialize();

    // Dump the native MT types table.
     
    dumper->DumpMtTypes("mt_types.json");

    dumper->BuildClassRecords();
    dumper->DumpDTIMap("dti_map.json");
    //dumper->DumpResourceInformation("cresource_info.txt");

    dumper->ProcessProperties();
    dumper->DumpDTIMap("dti_map_with_props.json");


    // auto dumper = std::make_unique<DTIDumper::DTIDumper>();

    // dumper->ParseDTI();
    // // dumper->DumpRawInfo("dti_dump_raw.json");

    // dumper->DumpToFile("dti_prop_dump.h");
    // // dumper->DumpPythonArrayFile("dti_data.py");

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CreateThread(NULL, 0, MyFunc, 0, 0, NULL);
    }

    return TRUE;
}