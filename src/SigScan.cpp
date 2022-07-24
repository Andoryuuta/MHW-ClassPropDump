#include "SigScan.hpp"
#include <Windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

// SuspendThreads suspends all threads except the caller.
bool SigScan::SuspendThreads() {
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
		if (cur_tid == te32.th32ThreadID) {
			continue;
		}

		if (te32.th32OwnerProcessID != cur_pid) {
			continue;
		}

		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
		if (hThread != NULL) {
			std::cout << "Suspending thread: " << te32.th32ThreadID << std::endl;
			SuspendThread(hThread);
		}

	} while (Thread32Next(hThreadSnap, &te32));

	CloseHandle(hThreadSnap);
	return true;
}


std::vector<std::string> SpaceSplit(std::string text) {

	std::istringstream iss(text);
	std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
	return results;
}

// Base-16 single char to int.
int char2int(char input)
{
	if (input >= '0' && input <= '9')
		return input - '0';
	if (input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if (input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	throw std::invalid_argument("Invalid input string");
}

uint64_t parseHex(std::string s) {
	uint64_t total = 0;
	uint64_t place_value = 1;
	std::for_each(s.rbegin(), s.rend(), [&total, &place_value](char const& c) {
		total += char2int(c) * place_value;
		place_value *= 16;
	});
	return total;
}

bool IsExecScannable(DWORD protect) {
	return (protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)); // && !IsGuarded(protect);
}

bool __attribute__((always_inline)) dataCompare(const uint8_t* data, const uint8_t* search_bytes, const char* wildcard_mask)
{
	for (; *wildcard_mask; ++wildcard_mask, ++data, ++search_bytes)
		if (*wildcard_mask == 'x' && *data != *search_bytes)
			return false;
	return (*wildcard_mask) == NULL;
};

uint64_t SigScan::Scan(uint64_t start_address, const std::string& sig) {
	auto split = SpaceSplit(sig);
	std::vector<uint8_t> sig_bytes;
	std::vector<char> sig_wildcards;

	// Split the sig into matching bytes and wildcards.
	for (auto&& s : split) {
		if (s == "?" || s == "??") {
			sig_wildcards.push_back('?');
			sig_bytes.push_back(0);
		}
		else {
			sig_wildcards.push_back('x');
			sig_bytes.push_back((uint8_t)parseHex(s));
		}
	}

	// HEAVY performance optimization (7x speedup locally):
	auto sbd = sig_bytes.data();
	auto swd = sig_wildcards.data();

	// Get the region information.
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID lpMem = (LPCVOID)start_address;

	// Go over each region and scan.
	while (VirtualQuery(lpMem, &mbi, sizeof(MEMORY_BASIC_INFORMATION))) {
		uint8_t* region_end = (uint8_t*)((uint64_t)mbi.BaseAddress + (uint64_t)mbi.RegionSize);
		if (IsExecScannable(mbi.Protect)) {
			for (uint8_t* p = (uint8_t*)mbi.BaseAddress; p < region_end; p++) {
				// Try to match at this addr.
				if (dataCompare(p, sbd, swd)) {
					return (uint64_t)p;
				}
			}
		}
		lpMem = (LPVOID)region_end;
	}


	// No match.
	return 0;
}

std::vector<uint64_t> SigScan::ScanAll(uint64_t start_address, const std::string& sig) {
	auto split = SpaceSplit(sig);
	std::vector<uint8_t> sig_bytes;
	std::vector<char> sig_wildcards;

	// Split the sig into matching bytes and wildcards.
	for (auto&& s : split) {
		if (s == "?" || s == "??") {
			sig_wildcards.push_back('?');
			sig_bytes.push_back(0);
		}
		else {
			sig_wildcards.push_back('x');
			sig_bytes.push_back((uint8_t)parseHex(s));
		}
	}

	return ScanAllRaw(start_address, sig_bytes, sig_wildcards, false);
}

// GetModuleHandleA

std::vector<uint64_t> SigScan::ScanAllRaw(uint64_t start_address, std::vector<uint8_t> sig_bytes, std::vector<char> sig_wildcards, bool start_address_is_restriect_allocation_base, DWORD scan_protection) {
	// HEAVY performance optimization (7x speedup locally):
	auto sbd = sig_bytes.data();
	auto swd = sig_wildcards.data();

	// Get the region information.
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID lpMem = (LPCVOID)start_address;

	std::vector<uint64_t> matches;

	// Go over each region and scan.
	while (VirtualQuery(lpMem, &mbi, sizeof(MEMORY_BASIC_INFORMATION))) {
		uint8_t* region_end = (uint8_t*)((uint64_t)mbi.BaseAddress + (uint64_t)mbi.RegionSize);

		if (!start_address_is_restriect_allocation_base || (uint64_t)mbi.AllocationBase == start_address) {
			if (mbi.Protect & scan_protection) {
				for (uint8_t* p = (uint8_t*)mbi.BaseAddress; p < region_end - sig_bytes.size(); p++) {
					// Try to match at this addr.
					if (dataCompare(p, sbd, swd)) {
						matches.push_back((uint64_t)p);
						p += sig_bytes.size();
					}
				}
			}
		}

		lpMem = (LPVOID)region_end;
	}

	return matches;
}


std::vector<uint64_t> SigScan::AlignedUint64ListScan(uint64_t module_base, uint64_t value, DWORD scan_protection) {
	// Get the region information.
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID lpMem = (LPCVOID)module_base;

	std::vector<uint64_t> matches;

	// Go over each region and scan.
	while (VirtualQuery(lpMem, &mbi, sizeof(MEMORY_BASIC_INFORMATION))) {
		uint64_t* region_end = (uint64_t*)((uint64_t)mbi.BaseAddress + (uint64_t)mbi.RegionSize);

		if ((uint64_t)mbi.AllocationBase == module_base) {
			if (mbi.Protect & scan_protection) {

				for (uint64_t* p = (uint64_t*)mbi.BaseAddress; p < region_end - 1; p++) {
					if (*p == value) {
						matches.push_back((uint64_t)p);
					}
				}
			}
		}

		lpMem = (LPVOID)region_end;
	}

	return matches;
}