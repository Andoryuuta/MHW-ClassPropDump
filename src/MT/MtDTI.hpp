#pragma once
#include <cstdint>
#include <string>
#include "size_assert.hpp"

namespace Mt {
	class MtDTI;
	class MtObject;

	class MtDTI {
	public:
		//void* vftable;
		char* class_name;
		uint64_t field_10;
		uint64_t field_18;
		MtDTI* parent;
		MtDTI* next;
		uint32_t flag_and_size_div_4;
		uint32_t crc_hash;

		virtual ~MtDTI() = 0;
		virtual MtObject* NewInstance() = 0;
		virtual void* CtorInstance(void* obj) = 0;
		virtual void* CtorInstanceArray(void* obj, int64_t count) = 0;

		uint64_t ClassSize();
		bool IsSubclassOf(std::string className);
	};
	assert_size(MtDTI, 0x38);
}