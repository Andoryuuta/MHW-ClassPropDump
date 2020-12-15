#include <Windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include "spdlog/spdlog.h"
#include "SigScan.hpp"
#include "Mt.hpp"

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


	// From: https://stackoverflow.com/a/45622802 :
	// Set console code page to UTF-8 so console known how to interpret string data
	SetConsoleOutputCP(CP_UTF8);

	// Enable buffering to prevent VS from chopping up UTF-8 byte sequences
	setvbuf(stdout, nullptr, _IOFBF, 1000);
}


std::map<std::string, Mt::MtDTI*> GetFlattenedDtiMap() {
	// Get the image base.
	uint64_t image_base = (uint64_t)GetModuleHandle(NULL);
	spdlog::info("Image Base: {0:x}", image_base);

	// Sig scan for the DTI hash table.
	// 48 8D 0D F778F002 - lea rcx,[MonsterHunterWorld.exe+50930D0]
	uint64_t hashTableAOB = SigScan::Scan(image_base, "48 8D ?? ?? ?? ?? 02 48 8D 0C C1 48 8B 01 48 85 C0 74 06 48 8D 48 28 EB F2");
	if (hashTableAOB == 0) {
		spdlog::critical("Failed to get MtDTI hash table");
	}

	// Get displacement and calculate final value (displacement values count from next instr pointer, so add the opcode size (7)).
	uint32_t disp = *(uint32_t*)(hashTableAOB + 3);
	Mt::MtDTIHashTable* MtDtiHashTable = (Mt::MtDTIHashTable*)(hashTableAOB + disp + 7);
	spdlog::info("MtDtiHashTable: {0:x}", (uint64_t)MtDtiHashTable);


	// Iterate over the hash buckets to build a flat lookup map
	// (lowest byte of the class name CRC32 is used as a bucket index normally).
	std::map<std::string, Mt::MtDTI*> dtis;
	for (int bucketIndex = 0; bucketIndex < 256; bucketIndex++) {
		Mt::MtDTI* dti = MtDtiHashTable->buckets[bucketIndex];
		while (true) {
			//spdlog::info("DTI@{0:X} = {1}, CRC32:{2:x}", (uint64_t)dti, dti->class_name, dti->crc_hash);
			dtis[dti->class_name] = dti;

			if (dti->next == nullptr)
				break;
			dti = dti->next;
		}
	}

	return dtis;
}


// https://stackoverflow.com/a/16684288
void PauseAllOtherThreads() {
	DWORD targetProcessId = GetCurrentProcessId();
	DWORD targetThreadId = GetCurrentThreadId();

	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te))
		{
			do
			{
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
				{
					// Suspend all threads EXCEPT the one we want to keep running
					if (te.th32ThreadID != targetThreadId && te.th32OwnerProcessID == targetProcessId)
					{
						HANDLE thread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
						if (thread != NULL)
						{
							spdlog::info("Suspending thread #{0}", te.th32ThreadID);
							SuspendThread(thread);
							CloseHandle(thread);
						}
					}
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}
}

DWORD WINAPI MyFunc(LPVOID lpvParam)
{
	OpenConsole();
	spdlog::info("Ando's MHWClassPropDump Started!");

	PauseAllOtherThreads();
	spdlog::info("Suspended all other threads.");


	uint64_t* allocatorList = (uint64_t*)0x14514CE40; // TODO: Make me dynamic!
	for (int i = 0; i < 64; i++) {
		allocatorList[i] = allocatorList[1];
	}

	std::set<std::string> excludedClasses = {
		// Blocking:
		"sKeyboard", "sOtomo", "nDraw::ExternalRegion"//, "uGuideInsect",

		// "OK"-able prompts with SEH:
		"nDraw::IndexBuffer", "nDraw::Scene", "nDraw::VertexBuffer", "sMhKeyboard",
		"sMhMain", "sMhMouse", "sMhRender", "sMouse", "sRender"
	};


	// Get the DTI map.
	auto dtis = GetFlattenedDtiMap();
	
	spdlog::info("MtPropertyList DTI: {0:X}, className = {1}", (uint64_t)dtis["MtPropertyList"], dtis["MtPropertyList"]->class_name);

	for (auto const& x : dtis) {
		auto key = x.first;
		auto val = x.second;

		spdlog::info("{0} - CRC32:0x{1:X} - Size:0x{2:X}", key, val->crc_hash, val->ClassSize());

		if (excludedClasses.find(key) != excludedClasses.end()) {

			spdlog::warn("Skipping excluded class: {0}", key);
			continue;
		}

		if (val->HasProperties()) {

			__try
			{
				spdlog::info("{0} Properties:", key);

				// Create a MtPropertyList to populate with the MtObject::PopulatePropertyList function.
				Mt::MtPropertyList* propListInst = reinterpret_cast<Mt::MtPropertyList*>(dtis["MtPropertyList"]->NewInstance());

				Mt::MtObject* mtObj = val->NewInstance();
				if (mtObj == nullptr) {
					spdlog::error("{0}::DTI->NewInstance() == nullptr", key);
				}
				else {
					mtObj->PopulatePropertyList(propListInst);

					if (propListInst->first_prop != nullptr) {
						// Loop over the property list and output the info.
						int i = 0;
						for (Mt::MtProperty* prop = propListInst->first_prop; prop != nullptr; prop = prop->next) {


							int64_t offset = -1;
							if (prop->IsOffsetBased()) {
								offset = ((uint64_t)prop->obj_inst_field_ptr_OR_FUNC_PTR) - ((uint64_t)prop->obj_inst_ptr);
							}

							//spdlog::info("\tProperty #{0}: (obj+:0x{1:X}) \"{2} {3};\"", i, offset, prop->GetTypeName(), prop->prop_name);
							i++;
						}
					}

					//((Mt::MtObject*)mtObj)->~MtObject();
				}
				//((Mt::MtObject*)propListInst)->~MtObject();

			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				spdlog::error("Got SEH exception while printing properties for {0}:\n{1}", key);
			}
		}
	}

	spdlog::info("Completed!!!!");

	/*
	Mt::MtPropertyList* propListInst = reinterpret_cast<Mt::MtPropertyList*>(dtis["MtPropertyList"]->NewInstance());
	spdlog::info("MtPropertyList new instance: {0:X}", (uint64_t)propListInst);

	Mt::MtObject* x = dtis["cLifeSimDesireTarget"]->NewInstance();
	x->PopulatePropertyList(propListInst);


	spdlog::info("cLifeSimDesireTarget properties:");
	int i = 0;
	for (Mt::MtProperty* prop = propListInst->first_prop; prop != nullptr; prop = prop->next) {

		int64_t offset = -1;
		if (prop->IsOffsetBased()) {
			offset = ((uint64_t)prop->obj_inst_field_ptr_OR_FUNC_PTR) - ((uint64_t)prop->obj_inst_ptr);
		}

		spdlog::info("\tProperty #{0}: (obj+:0x{1:X}) \"{2} {3};\"", i, offset, prop->GetTypeName(), prop->prop_name);
		i++;
	}
	*/


	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		CreateThread(NULL, 0, MyFunc, 0, 0, NULL);
	}

	return TRUE;
}