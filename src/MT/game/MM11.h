#pragma once
// Mega Man 11 (Steam, "MasterRelease x64 DX11 Aug 22 2025 10:36:57")

#include "size_assert.h"
#include <cstdint>

// Verify dumper build architecture matches game.
#ifndef ENV64BIT
    #error The MM11 dumper must be compiled for 64-bit!
#endif

#define MT_DTI_HASH_TABLE_OFFSET 0xb95540
#define MT_TYPE_TABLE_RVA_OFFSET 0xb7a650
#define MT_TYPE_GET_BYTES_RVA_OFFSET 0x31d290
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

    inline void type(uint32_t type) { mType = type; }

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
    [[maybe_unused]] uint32_t field_0x3C;
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
}

inline const char** GetMtTypeTable(uintptr_t image_base)
{
    return reinterpret_cast<const char**>(image_base + MT_TYPE_TABLE_RVA_OFFSET);
}
inline size_t GetMtTypeTableCount(uintptr_t image_base) { return MT_TYPE_TABLE_COUNT; }

inline size_t GetMtTypeSize(uintptr_t image_base, uint32_t type_id)
{
    typedef int64_t(__thiscall * MtProperty_GetBytes_t)(Mt::MtProperty* this_ptr);
    MtProperty_GetBytes_t fpGetBytes = (MtProperty_GetBytes_t)(image_base + MT_TYPE_GET_BYTES_RVA_OFFSET);

    Mt::MtProperty prop{};
    prop.type(type_id);

    return fpGetBytes(&prop);
}
