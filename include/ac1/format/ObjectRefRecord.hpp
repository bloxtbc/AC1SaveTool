#pragma once

#include <cstdint>
#include <vector>

#include "ac1/format/ObjectHandle.hpp"

namespace ac1 {
    struct ObjectRefRecord {
        uint32_t unknown;
        uint8_t objectCount;

        std::vector<ObjectHandle> objectHandles;
    };
    
} // namespace ac1
