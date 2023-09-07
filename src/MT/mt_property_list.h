#pragma once
#include "size_assert.h"

namespace Mt
{
class MtProperty;

class MtPropertyList
{
  public:
    virtual ~MtPropertyList() = 0;

    MtProperty* mpElement;
};
assert_size_32(MtPropertyList, 0x8);
assert_size_64(MtPropertyList, 0x10);
} // namespace Mt