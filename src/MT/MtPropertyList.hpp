#pragma once
#include <cstdint>
#include "config.h"
#include "size_assert.hpp"

namespace Mt {
	class MtProperty;

#if defined(MT_GAME_MHW)
	class MtPropertyList {
	public:
		void* vftable;
		MtProperty* first_prop;
		uint64_t field_10;
		uint64_t field_18;

	};
	assert_size(MtPropertyList, 0x20);
#elif defined(MT_GAME_MHS2)
	class MtPropertyList {
	public:
		void* vftable;
		uint64_t field_8;
		MtProperty* first_prop;

	};
	assert_size(MtPropertyList, 0x18);
#endif
}