#pragma once
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

class SigScan
{
  public:
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