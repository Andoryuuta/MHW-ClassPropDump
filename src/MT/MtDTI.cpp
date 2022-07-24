#include <cstdint>
#include "MtDTI.hpp"
#include "MtCommon.hpp"

namespace Mt {
    uint64_t MtDTI::ClassSize() {
        return (this->flag_and_size_div_4 & DTI_SIZE_MASK) << 2;
    }

    bool MtDTI::IsSubclassOf(std::string className) {
        for (MtDTI* cur = this; (cur != nullptr && cur->parent != cur); cur = cur->parent) {
            if (cur != nullptr && cur->class_name != nullptr && std::string(cur->class_name) == className) {
                return true;
            }
        }
        return false;
    }
}