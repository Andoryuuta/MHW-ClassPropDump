#pragma once
#include "size_assert.h"

namespace Mt
{
class MtPropertyList;
class MtDTI;

// Common across all MT framework games
class MtObject
{
  public:
    virtual ~MtObject() = 0;
    virtual void* CreateUI() = 0;
    virtual bool IsEnableInstance() = 0;
    virtual void CreateProperty(MtPropertyList*) = 0;
    virtual MtDTI* GetDTI() = 0;
};
assert_size_32(MtObject, 0x4);
assert_size_64(MtObject, 0x8);

} // namespace Mt