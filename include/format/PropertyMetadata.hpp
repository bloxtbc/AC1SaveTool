#pragma once

#include <cstdint>
#include "Types.hpp"


struct PropertyMetadata {
    uint32_t identifier;
    uint32_t unknown;

    ac1::SerializerFieldType type() const;
};