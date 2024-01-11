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

// Game-specific MT class definitions
namespace Mt
{
struct MtDTIHashTable;
class MtObject;

class MtProperty
{
  public:
    inline char* name() { return this->mName; }
    inline char* comment() { return nullptr; }
    inline uint32_t type() { return this->mType; }
    inline uint32_t attr() { return this->mAttr; }
    inline uintptr_t owner() { return reinterpret_cast<uintptr_t>(this->mpOwner); }
    inline uintptr_t get() { return this->ufGet; }
    inline uintptr_t get_count() { return this->ufGetCount; }
    inline uintptr_t set() { return this->ufSet; }
    inline uintptr_t set_count() { return this->ufSetCount; }
    inline uint32_t index() { return this->mIndex; }
    inline MtProperty* next() { return this->mpNext; }

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
    [[maybe_unused]] uint32_t field_0x38;
    [[maybe_unused]] MtProperty* mpPrev;
    MtProperty* mpNext;
};
assert_size_64(MtProperty, 0x50);

class MtPropertyList
{
  public:
    virtual ~MtPropertyList() = 0;

    MtProperty* mpElement;
};
assert_size_64(MtPropertyList, 0x10);

} // namespace Mt


inline Mt::MtDTIHashTable* GetDTIHashTable(uintptr_t image_base)
{
    return reinterpret_cast<Mt::MtDTIHashTable*>(image_base + MT_DTI_HASH_TABLE_OFFSET);
    // uintptr_t hashTableAOB =
    //     (hashTableAOB)SigScan::Scan(image_base, "48 8D ?? ?? ?? ?? ?? 48 8D 0C C1 48 8B 01 48 85 C0 74");
    // if (hashTableAOB == 0)
    // {
    //     spdlog::critical("Failed to get MtDTI hash table");
    // }

    // // Get displacement and calculate final value (displacement values count
    // // from next instr pointer, so add the opcode size (7)).
    // uint32_t disp = *(uint32_t *)(hashTableAOB + 3);
    // return reinterpret_cast<Mt::MtDTIHashTable *>(hashTableAOB + disp + 7);
}

inline const char** GetMtTypeTable(uintptr_t image_base)
{
    return reinterpret_cast<const char**>(image_base + MT_TYPE_TABLE_RVA_OFFSET);
}
inline size_t GetMtTypeTableCount(uintptr_t image_base) { return MT_TYPE_TABLE_COUNT; }
