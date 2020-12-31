#pragma once
#include <cstdint>
#include "size_assert.hpp"

namespace Mt {
	class MtDTI;
	class MtObject;
	class MtProperty;
	class MtPropertyList;

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
		bool HasProperties();
	};
	assert_size(MtDTI, 0x38);

	struct MtDTIHashTable {
		MtDTI* buckets[256];
	};
	assert_size(MtDTIHashTable, 0x800);

	class MtObject {
	public:
		virtual ~MtObject() = 0;
		virtual int64_t Vf1() = 0;
		virtual bool Vf2() = 0;
		virtual bool PopulatePropertyList(MtPropertyList*) = 0;
		virtual bool GetDTI(MtPropertyList*) = 0;
	};
	assert_size(MtObject, 0x8);


	class MtProperty {
	public:
		char* prop_name;
		uint64_t field_8;
		uint32_t flags_and_type;
		uint32_t field_14;

		// A union likely starts around here.
		// These same offsets are used for different purposes depending on the circumstances.
		void* obj_inst_ptr;
		void* obj_inst_field_ptr_OR_FUNC_PTR;
		uint32_t field_28_count;

		uint32_t field_2C;
		uint64_t field_30;

		uint64_t field_38;
		uint32_t field_40;
		uint32_t field_44;
		uint64_t field_48;
		MtProperty* next;

		const char* GetTypeName();
		uint32_t GetFullFlags();
		uint32_t GetCRC32();
		bool IsOffsetBased();
		bool IsArrayType();
		bool IsGetterSetter();
		int64_t GetFieldOffset();

		enum class PropType
		{
			undefined = 0x0,
			class_ = 0x1,
			classref = 0x2,
			bool_ = 0x3,
			u8 = 0x4,
			u16 = 0x5,
			u32 = 0x6,
			u64 = 0x7,
			s8 = 0x8,
			s16 = 0x9,
			s32 = 0xA,
			s64 = 0xB,
			f32 = 0xC,
			f64 = 0xD,
			string = 0xE,
			color = 0xF,
			point = 0x10,
			size = 0x11,
			rect = 0x12,
			matrix44 = 0x13,
			vector3 = 0x14,
			vector4 = 0x15,
			quaternion = 0x16,
			property = 0x17,
			event = 0x18,
			group = 0x19,
			pagebegin = 0x1A,
			pageend = 0x1B,
			event32 = 0x1C,
			array = 0x1D,
			propertylist = 0x1E,
			groupend = 0x1F,
			cstring = 0x20,
			time = 0x21,
			float2 = 0x22,
			float3 = 0x23,
			float4 = 0x24,
			float3x3 = 0x25,
			float4x3 = 0x26,
			float4x4 = 0x27,
			easecurve = 0x28,
			line = 0x29,
			linesegment = 0x2A,
			ray = 0x2B,
			Plane = 0x2C,
			sphere = 0x2D,
			capsule = 0x2E,
			aabb = 0x2F,
			obb = 0x30,
			cylinder = 0x31,
			triangle = 0x32,
			cone = 0x33,
			torus = 0x34,
			ellpsoid = 0x35,
			range = 0x36,
			rangef = 0x37,
			rangeu16 = 0x38,
			hermitecurve = 0x39,
			enumlist = 0x3A,
			float3x4 = 0x3B,
			linesegment4 = 0x3C,
			aabb4 = 0x3D,
			oscillator = 0x3E,
			variable = 0x3F,
			vector2 = 0x40,
			matrix33 = 0x41,
			rect3d_xz = 0x42,
			rect3d = 0x43,
			rect3d_collision = 0x44,
			plane_xz = 0x45,
			ray_y = 0x46,
			pointf = 0x47,
			sizef = 0x48,
			rectf = 0x49,
			event64 = 0x4A,
			bool_2 = 0x4B,
			END = 0x4C,
		};
	};
	assert_size(MtProperty, 0x58);


	class MtPropertyList {
	public:
		void* vftable;
		MtProperty* first_prop;
		uint64_t field_10;
		uint64_t field_18;

	};
	assert_size(MtPropertyList, 0x20);
}