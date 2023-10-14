#pragma once
#include <cstdint>
#include <string>
#include "size_assert.hpp"
#include "config.h"

namespace Mt {
	class MtProperty;

	class MtProperty {
	public:
		char* prop_name;
#if defined(MT_GAME_MHW)
		char* prop_comment;
#endif
		uint32_t flags_and_type;
		uint32_t field_14;

		// A union likely starts around here.
		// These same offsets are used for different purposes depending on the circumstances.
		void* obj_inst_ptr;

		union {
			// Getter / setter
			struct {
				uint64_t fp_get;
				uint64_t fp_get_count;
				uint64_t fp_set;
				uint64_t fp_dynamic_allocation;
			} gs;

			// plain struct-offset variable
			struct {
				void* obj_inst_field;
				uint32_t count; // Array count if ARRAY property, else some function ptr.
			} var;
		} data;

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
		int64_t GetRelativeFieldOffset();
		int64_t GetFieldOffsetFrom(uint64_t base);

		enum class PropType
		{
#if defined(MT_GAME_MHW)
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
#elif defined(MT_GAME_MHS2)
			undefined = 0,
			class_ = 1,
			classref = 2,
			bool_ = 3,
			u8 = 4,
			u16 = 5,
			u32 = 6,
			u64 = 7,
			s8 = 8,
			s16 = 9,
			s32 = 10,
			s64 = 11,
			f32 = 12,
			f64 = 13,
			string = 14,
			color = 15,
			point = 16,
			size = 17,
			rect = 18,
			matrix44 = 19,
			vector3 = 20,
			vector4 = 21,
			quaternion = 22,
			property = 23,
			event = 24,
			group = 25,
			pagebegin = 26,
			pageend = 27,
			event32 = 28,
			array = 29,
			propertylist = 30,
			groupend = 31,
			cstring = 32,
			time = 33,
			float2 = 34,
			float3 = 35,
			float4 = 36,
			float3x3 = 37,
			float4x3 = 38,
			float4x4 = 39,
			easecurve = 40,
			line = 41,
			linesegment = 42,
			ray = 43,
			Plane = 44,
			sphere = 45,
			capsule = 46,
			aabb = 47,
			obb = 48,
			cylinder = 49,
			triangle = 50,
			cone = 51,
			torus = 52,
			ellpsoid = 53,
			range = 54,
			rangef = 55,
			rangeu16 = 56,
			hermitecurve = 57,
			enumlist = 58,
			float3x4 = 59,
			linesegment4 = 60,
			aabb4 = 61,
			oscillator = 62,
			variable = 63,
			vector2 = 64,
			matrix33 = 65,
			rect3d_xz = 66,
			rect3d = 67,
			rect3d_collision = 68,
			plane_xz = 69,
			ray_y = 70,
			pointf = 71,
			sizef = 72,
			rectf = 73,
			event64 = 74,
#endif
		};
	};


#if defined(MT_GAME_MHW)
	assert_size(MtProperty, 0x58);
#elif defined(MT_GAME_MHS2)
	assert_size(MtProperty, 0x50);
#endif
}