#include "sig_scan.h"

#include <Psapi.h>
#include <Windows.h>
#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tlhelp32.h>
#include <vector>

#include "spdlog/spdlog.h"

std::vector<std::string> SpaceSplit(std::string text)
{
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

uint64_t parseHex(std::string s)
{
    uint64_t total = 0;
    uint64_t place_value = 1;
    std::for_each(s.rbegin(), s.rend(), [&total, &place_value](char const& c) {
        total += char2int(c) * place_value;
        place_value *= 16;
    });
    return total;
}

std::vector<uintptr_t> SigScan::ScanCurrentExecutable(const std::string& sig, uint32_t alignment)
{
    MODULEINFO module_info;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &module_info, sizeof(MODULEINFO));
    uintptr_t mod_start = (uintptr_t)module_info.lpBaseOfDll;
    uintptr_t mod_end = mod_start + (uintptr_t)module_info.SizeOfImage;

    return Scan(mod_start, mod_end, sig, PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE, alignment);
}

std::vector<uintptr_t> SigScan::ScanCurrentExecutableUintptrAligned(uintptr_t value, DWORD scan_protection)
{
    // std::vector<uintptr_t> SigScan::ScanAllRaw(uintptr_t start_address, uintptr_t end_address,
    //                                        std::vector<uint8_t> sig_bytes, std::vector<uint8_t> sig_skip_mask,
    //                                        DWORD scan_protection, uint32_t alignment)

    uintptr_t tmp = value;
    std::vector<uint8_t> sig_bytes;
    std::vector<uint8_t> sig_skip_mask;
    for (size_t i = 0; i < sizeof(uintptr_t); i++)
    {
        sig_bytes.push_back(reinterpret_cast<uint8_t*>(&tmp)[i]);
        sig_skip_mask.push_back(0);
    }

    MODULEINFO module_info;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &module_info, sizeof(MODULEINFO));
    uintptr_t mod_start = (uintptr_t)module_info.lpBaseOfDll;
    uintptr_t mod_end = mod_start + (uintptr_t)module_info.SizeOfImage;

    return ScanAllRaw(mod_start, mod_end, sig_bytes, sig_skip_mask, scan_protection, sizeof(uintptr_t));
}

std::vector<uintptr_t> SigScan::Scan(uintptr_t start_address, uintptr_t end_address, const std::string& sig,
                                     DWORD scan_protection, uint32_t alignment)
{
    auto split = SpaceSplit(sig);
    std::vector<uint8_t> sig_bytes;
    std::vector<uint8_t> sig_skip_mask;

    // Split the sig into matching bytes and wildcards.
    for (auto&& s : split)
    {
        if (s == "?" || s == "??")
        {
            sig_skip_mask.push_back(1);
            sig_bytes.push_back(0);
        }
        else
        {
            sig_skip_mask.push_back(0);
            sig_bytes.push_back((uint8_t)parseHex(s));
        }
    }

    return ScanAllRaw(start_address, end_address, sig_bytes, sig_skip_mask, scan_protection);
}

std::vector<uintptr_t> SigScan::ScanAllRaw(uintptr_t start_address, uintptr_t end_address,
                                           std::vector<uint8_t> sig_bytes, std::vector<uint8_t> sig_skip_mask,
                                           DWORD scan_protection, uint32_t alignment)
{
    // Check that the alignment is correct.
    if (start_address % alignment != 0)
    {
        throw std::runtime_error(
            std::format("start_address does not match provided alignment! (start_address:{}, alignment:{})",
                        start_address, alignment));
    }

    // Pull out raw pointer since this seems to help the compiler optimize this loop.
    volatile uint8_t* sig_bytes_data = sig_bytes.data();
    volatile uint8_t* sig_skip_mask_data = sig_skip_mask.data();

    // Get the region information.
    MEMORY_BASIC_INFORMATION mbi;
    LPCVOID current_region_base = (LPCVOID)start_address;

    std::vector<uintptr_t> matches;

    // Go over each region and scan.
    while (VirtualQuery(current_region_base, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) &&
           (uintptr_t)current_region_base <= end_address)
    {
        uint8_t* region_end = (uint8_t*)((uintptr_t)mbi.BaseAddress + (uintptr_t)mbi.RegionSize);

        // spdlog::info("Scanning region - start:{:X}, end:{:X}", (uintptr_t)mbi.BaseAddress,
        //              (uintptr_t)mbi.BaseAddress + (uintptr_t)mbi.RegionSize);
        if (mbi.Protect & scan_protection)
        {
            for (uint8_t* p = (uint8_t*)mbi.BaseAddress; p < region_end - sig_bytes.size(); p += alignment)
            {
                for (size_t i = 0; i < sig_bytes.size(); i++)
                {
                    if (sig_skip_mask_data[i] == 0)
                    {
                        if (p[i] != sig_bytes_data[i])
                        {
                            // If we are not skipping this byte and it doesn't match, break immediately.
                            break;
                        }
                        else if (i == sig_bytes.size() - 1)
                        {
                            // If we reached the last byte of the sig without breaking the loop,
                            // then we have have fully matched the signature bytes.
                            matches.push_back((uintptr_t)p);
                        }
                    }
                }
            }
        }

        current_region_base = (LPVOID)region_end;
    }

    return matches;
}