#pragma once

#include <cstdint>

#include "ac1/format/SerializerFieldType.hpp"

namespace ac1 {

struct PropertyMetadata {
    uint32_t identifier;
    uint32_t unknown;

    SerializerFieldType type() const;
};

} // namespace ac1
