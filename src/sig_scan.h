#pragma once
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

class SigScan
{
  public:
    // static uint64_t Scan(uint64_t start_address, const std::string &sig);
    // static std::vector<uint64_t> ScanAll(uint64_t start_address, const std::string &sig);
    // static std::vector<uint64_t> ScanAllRaw(uint64_t start_address, std::vector<uint8_t> sig_bytes,
    //                                         std::vector<char> sig_wildcards,
    //                                         bool start_address_is_restriect_allocation_base,
    //                                         DWORD scan_protection = PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE);
    // static std::vector<uint64_t> AlignedUint64ListScan(uint64_t module_base, uint64_t value, DWORD scan_protection);
    static std::vector<uintptr_t> ScanCurrentExecutable(const std::string& sig, uint32_t alignment = 1);
    static std::vector<uintptr_t> Scan(uintptr_t start_address, uintptr_t end_address, const std::string& sig,
                                       DWORD scan_protection = PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE,
                                       uint32_t alignment = 1);
    static std::vector<uintptr_t> ScanAllRaw(uintptr_t start_address, uintptr_t end_address,
                                             std::vector<uint8_t> sig_bytes, std::vector<uint8_t> sig_skip_mask,
                                             DWORD scan_protection = PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE,
                                             uint32_t alignment = 1);

    static std::vector<uintptr_t> ScanCurrentExecutableUintptrAligned(uintptr_t value,
                                                                      DWORD scan_protection = PAGE_READONLY |
                                                                                              PAGE_READWRITE |
                                                                                              PAGE_EXECUTE_READ |
                                                                                              PAGE_EXECUTE_READWRITE);
};