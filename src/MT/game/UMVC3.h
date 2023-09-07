#pragma once
// ULTIMATE MARVEL vs. CAPCOM 3 (Latest version)

#include "size_assert.h"
#include <cstdint>

// Verify dumper build architecture matches game.
#ifndef ENV64BIT
    #error The UMVC3 dumper must be compiled for 64-bit!
#endif

#define MT_DTI_HASH_TABLE_OFFSET 0xd6d7b0
#define MT_TYPE_TABLE_RVA_OFFSET 0xd2aeb0
#define MT_TYPE_TABLE_COUNT 75

namespace Mt
{
struct MtDTIHashTable;
class MtObject;

// This seems to be very game-specific, so it's defiend here.
class MtProperty
{
  public:
    char* name();
    uint32_t type();
    uint32_t attr();
    uintptr_t owner();
    uintptr_t get();
    uintptr_t get_count();
    uintptr_t set();
    uintptr_t set_count();
    uint32_t index();
    MtProperty* next();

  private:
    char* mName;
    uint32_t mType : 16;
    uint32_t mAttr : 16;
    MtObject* mpOwner;

    // Union fields start
    uintptr_t ufGet;
    uintptr_t ufGetCount;
    uintptr_t ufSet;
    uintptr_t ufSetCount;
    // Union fields end

    uint32_t mIndex;
    uint32_t field_0x38;
    MtProperty* mpPrev;
    MtProperty* mpNext;

};
assert_size_64(MtProperty, 0x50);

} // namespace Mt

Mt::MtDTIHashTable* GetDTIHashTable(uintptr_t image_base);
const char** GetMtTypeTable(uintptr_t image_base);
size_t GetMtTypeTableCount(uintptr_t image_base);
