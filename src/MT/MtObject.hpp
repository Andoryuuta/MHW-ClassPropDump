#pragma once
#include <cstdint>
#include "size_assert.hpp"

namespace Mt {
	class MtPropertyList;

	class MtObject {
	public:
		virtual ~MtObject() = 0;
		virtual int64_t Vf1() = 0;
		virtual bool Vf2() = 0;
		virtual bool PopulatePropertyList(MtPropertyList*) = 0;
		virtual bool GetDTI(MtPropertyList*) = 0;
	};
	assert_size(MtObject, 0x8);
}