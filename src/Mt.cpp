#include <cstdint>
#include "Mt.hpp"
#include "spdlog/spdlog.h"
#include "crc.h"



#define DTI_SIZE_MASK 0x7FFFFF
#define DTI_FLAG_HAS_PROPS 0x20000000

#define PROP_FLAG_BIT_18_IS_ARRAY 0x20000
#define PROP_FLAG_BIT_20_IS_GETTER_SETTER 0x80000

#define PROP_TYPE_MASK   0xFFF
#define PROP_ALL_FLAGS_MASK = 0xFFFFF000


/*
v162.prop_name_str = (__int64)"InfiniteData";
v162.obj_inst_field_ptr_OR_FUNC_PTR = (__int64)sub_1418AB140; -- getter
v162.flags_and_type = 0x84001;
v162.field_30 = (__int64)sub_1418AD570; -- some setter

    111111111111 = 0x0FFF
 100000000000000 = 0x4000
1000000000000000 = 0x8000

*/
namespace Mt {

    uint64_t MtDTI::ClassSize() {
        return (this->flag_and_size_div_4 & DTI_SIZE_MASK) << 2;
    }

    bool MtDTI::HasProperties() {
        MtDTI* cur = this;
        do {
            spdlog::info("MtDTI::HasProperties() -- checking node {0:X} ({1}), flags: {2:b}", (uint64_t)cur, cur->class_name, (cur->flag_and_size_div_4& 0x7F800000)>>23);

            /*
            if (this->flag_and_size_div_4 & DTI_FLAG_HAS_PROPS) {
                return false;
            }*/

            cur = cur->parent;
        } while (cur != nullptr && cur->parent != cur);

        return true;
        //return (this->flag_and_size_div_4 & 0x20000000) == 0;
    }

    uint32_t MtProperty::GetFullFlags() {
        return this->flags_and_type & 0xFFFFF000;
    }

    uint32_t MtProperty::GetCRC32() {
        return get_cstr_crc(this->prop_name);
    }

    int64_t MtProperty::GetFieldOffset() {
        if (this->IsOffsetBased()) {
            return ((uint64_t)this->obj_inst_field_ptr_OR_FUNC_PTR) - ((uint64_t)this->obj_inst_ptr);
        }
        else {
            return INT64_MAX;
        }
    }

    bool MtProperty::IsOffsetBased() {
        return /*(this->flags_and_type & PROP_FLAG_BIT_18) == 0 &&*/ (this->flags_and_type & PROP_FLAG_BIT_20_IS_GETTER_SETTER) == 0;
    }

    bool MtProperty::IsArrayType() {
        return (this->flags_and_type & PROP_FLAG_BIT_18_IS_ARRAY) != 0;
    }

    bool MtProperty::IsGetterSetter() {
        return (this->flags_and_type & PROP_FLAG_BIT_20_IS_GETTER_SETTER) != 0;
    }

	const char* MtProperty::GetTypeName() {
		switch ((PropType)(this->flags_and_type & PROP_TYPE_MASK))
		{
        case PropType::undefined:
            return "undefined";
        case PropType::class_:
            return "class";
        case PropType::classref:
            return "classref";
        case PropType::bool_:
        case PropType::bool_2:
            return "bool";
        case PropType::u8:
            return "u8";
        case PropType::u16:
            return "u16";
        case PropType::u32:
            return "u32";
        case PropType::u64:
            return "u64";
        case PropType::s8:
            return "s8";
        case PropType::s16:
            return "s16";
        case PropType::s32:
            return "s32";
        case PropType::s64:
            return "s64";
        case PropType::f32:
            return "f32";
        case PropType::f64:
            return "f64";
        case PropType::string:
            return "string";
        case PropType::color:
            return "color";
        case PropType::point:
            return "point";
        case PropType::size:
            return "size";
        case PropType::rect:
            return "rect";
        case PropType::matrix44:
            return "matrix44";
        case PropType::vector3:
            return "vector3";
        case PropType::vector4:
            return "vector4";
        case PropType::quaternion:
            return "quaternion";
        case PropType::property:
            return "property";
        case PropType::event:
            return "event";
        case PropType::group:
            return "group";
        case PropType::pagebegin:
            return "pagebegin";
        case PropType::pageend:
            return "pageend";
        case PropType::event32:
            return "event32";
        case PropType::array:
            return "array";
        case PropType::propertylist:
            return "propertylist";
        case PropType::groupend:
            return "groupend";
        case PropType::cstring:
            return "cstring";
        case PropType::time:
            return "time";
        case PropType::float2:
            return "float2";
        case PropType::float3:
            return "float3";
        case PropType::float4:
            return "float4";
        case PropType::float3x3:
            return "float3x3";
        case PropType::float4x3:
            return "float4x3";
        case PropType::float4x4:
            return "float4x4";
        case PropType::easecurve:
            return "easecurve";
        case PropType::line:
            return "line";
        case PropType::linesegment:
            return "linesegment";
        case PropType::ray:
            return "ray";
        case PropType::Plane:
            return "Plane";
        case PropType::sphere:
            return "sphere";
        case PropType::capsule:
            return "capsule";
        case PropType::aabb:
            return "aabb";
        case PropType::obb:
            return "obb";
        case PropType::cylinder:
            return "cylinder";
        case PropType::triangle:
            return "triangle";
        case PropType::cone:
            return "cone";
        case PropType::torus:
            return "torus";
        case PropType::ellpsoid:
            return "ellpsoid";
        case PropType::range:
            return "range";
        case PropType::rangef:
            return "rangef";
        case PropType::rangeu16:
            return "rangeu16";
        case PropType::hermitecurve:
            return "hermitecurve";
        case PropType::enumlist:
            return "enumlist";
        case PropType::float3x4:
            return "float3x4";
        case PropType::linesegment4:
            return "linesegment4";
        case PropType::aabb4:
            return "aabb4";
        case PropType::oscillator:
            return "oscillator";
        case PropType::variable:
            return "variable";
        case PropType::vector2:
            return "vector2";
        case PropType::matrix33:
            return "matrix33";
        case PropType::rect3d_xz:
            return "rect3d_xz";
        case PropType::rect3d:
            return "rect3d";
        case PropType::rect3d_collision:
            return "rect3d_collision";
        case PropType::plane_xz:
            return "plane_xz";
        case PropType::ray_y:
            return "ray_y";
        case PropType::pointf:
            return "pointf";
        case PropType::sizef:
            return "sizef";
        case PropType::rectf:
            return "rectf";
        case PropType::event64:
            return "event64";
        //case PropType::END:
        //    return "END";
		default:
            return "custom";
			break;
		}
	}
}