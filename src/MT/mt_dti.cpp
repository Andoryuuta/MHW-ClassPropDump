#include "mt_dti.h"

namespace Mt
{

uintptr_t MtDTI::dti_va() { return reinterpret_cast<uintptr_t>(this); }
uintptr_t MtDTI::dti_vftable_va() { return *reinterpret_cast<uintptr_t*>(this); }
char* MtDTI::name() { return this->mName; }
MtDTI* MtDTI::next() { return this->mpNext; }
MtDTI* MtDTI::child() { return this->mpChild; }
MtDTI* MtDTI::parent() { return this->mpParent; }
MtDTI* MtDTI::link() { return this->mpLink; }
uint32_t MtDTI::raw_flags_and_size() { return this->rawFlagsAndSize; }
uint32_t MtDTI::size() { return this->mSize * 4; }
uint32_t MtDTI::allocator_index() { return this->mAllocatorIndex; }
uint32_t MtDTI::id() { return this->mID; }
bool MtDTI::is_abstract() { return this->mAttr & MtDTI::AttributeFlag::ATTR_ABSTRACT; }
bool MtDTI::is_hidden() { return this->mAttr & MtDTI::AttributeFlag::ATRR_HIDE; }
uint32_t MtDTI::parent_id()
{
    if (!this->mpParent)
        return 0;
    return this->mpParent->mID;
}

// Returns true if one of the parent classes matches the provided class name.
bool MtDTI::IsDependantOf(std::string_view class_name)
{
    for (MtDTI* cur = this; (cur != nullptr && cur->mpParent != cur); cur = cur->mpParent)
    {
        if (cur != nullptr && cur->mName != nullptr && std::string(cur->mName) == class_name)
        {
            return true;
        }
    }
    return false;
}
} // namespace Mt