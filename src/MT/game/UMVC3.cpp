#include "UMVC3.h"
#include <sig_scan.h>

namespace Mt
{
char* MtProperty::name() { return this->mName; }
uint32_t MtProperty::type() { return this->mType; }
uint32_t MtProperty::attr() { return this->mAttr; }
uintptr_t MtProperty::owner() { return reinterpret_cast<uintptr_t>(this->mpOwner); }
uintptr_t MtProperty::get() { return this->ufGet; }
uintptr_t MtProperty::get_count() { return this->ufGetCount; }
uintptr_t MtProperty::set() { return this->ufSet; }
uintptr_t MtProperty::set_count() { return this->ufSetCount; }
uint32_t MtProperty::index() { return this->mIndex; }
MtProperty* MtProperty::next() { return this->mpNext; }
} // namespace Mt

Mt::MtDTIHashTable* GetDTIHashTable(uintptr_t image_base)
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

const char** GetMtTypeTable(uintptr_t image_base)
{
    return reinterpret_cast<const char**>(image_base + MT_TYPE_TABLE_RVA_OFFSET);
}
size_t GetMtTypeTableCount(uintptr_t image_base) { return MT_TYPE_TABLE_COUNT; }
