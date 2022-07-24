#pragma once
#include "size_assert.hpp"

namespace Mt {
	class MtDTI;

	struct MtDTIHashTable {
		MtDTI* buckets[256];
	};
	assert_size(MtDTIHashTable, 0x800);
}