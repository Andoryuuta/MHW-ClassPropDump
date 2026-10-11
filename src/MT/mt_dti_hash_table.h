#pragma once
#include "size_assert.h"

namespace Mt
{
class MtDTI;

// Common across all MT framework games
struct MtDTIHashTable
{
    MtDTI *buckets[256];
};
assert_size_32(MtDTIHashTable, 0x400);
assert_size_64(MtDTIHashTable, 0x800);

} // namespace Mt
