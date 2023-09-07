#pragma once
#include <cstdint>
#include "size_assert.hpp"

namespace Mt {
class MtPropertyList;

class MtObject {
   public:
    virtual ~MtObject() = 0;
    virtual void* CreateUI() = 0;
    virtual bool IsEnableInstance() = 0;
    virtual bool PopulatePropertyList(MtPropertyList*) = 0;
    virtual bool GetDTI(MtPropertyList*) = 0;
};
assert_size(MtObject, 0x8);
}  // namespace Mt