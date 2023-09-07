#pragma once
#include <cstdint>
#include <string_view>

#include "mt_object.h"
#include "size_assert.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Mt
{

// Common across all MT framework games
class MtDTI
{
  public:
    enum AttributeFlag
    {
        ATTR_ABSTRACT = 0x1,
        ATRR_HIDE = 0x2,
    };

    // vf0 - virtual deconstructor
    virtual ~MtDTI() = 0;

    // Create a new instance of the class
    // represented by this DTI.
    // vf1 - allocating factory create method.
    virtual MtObject* NewInstance() = 0;

    /*
     * Abstracted getters for the class properties below in
     * order to keep the code consistent in case there are any
     * games that have a different class layout found later.
     */

    char* name();
    MtDTI* next();
    MtDTI* child();
    MtDTI* parent();
    MtDTI* link();
    uint32_t raw_flags_and_size();
    uint32_t size();
    uint32_t allocator_index();
    uint32_t id();

    bool is_abstract();
    bool is_hidden();
    uint32_t parent_id();

    uintptr_t dti_vftable_va();
    uintptr_t dti_va();

    // Helper method for walking the parent tree
    // Returns true if one of the parent classes matches the provided class name.
    bool IsDependantOf(std::string_view class_name);

  private:
    char* mName;
    MtDTI* mpNext;
    MtDTI* mpChild;
    MtDTI* mpParent;
    MtDTI* mpLink;

    union {
        struct
        {
            // Size is stored as (byte_size/4) for some reason.
            // This isn't an issue with the bitfield (verified with debug symbols).
            uint32_t mSize : 23;
            uint32_t mAllocatorIndex : 6;
            uint32_t mAttr : 3;
        };
        uint32_t rawFlagsAndSize;
    };

    uint32_t mID;
};
assert_size_32(MtDTI, 0x20);
assert_size_64(MtDTI, 0x38);
} // namespace Mt

// #define DTI_SIZE_MASK 0x7FFFFF
// #define DTI_FLAG_HAS_PROPS 0x20000000

// #define PROP_FLAG_BIT_18_IS_ARRAY 0x20000
// #define PROP_FLAG_BIT_20_IS_GETTER_SETTER 0x80000

// #define PROP_TYPE_MASK 0xFFF
// #define PROP_ALL_FLAGS_MASK 0xFFFFF000
