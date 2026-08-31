#pragma once

#include <cstdint>
#include <vector>

#include "ac1/format/Property.hpp"
#include "ac1/format/PropertyMetadata.hpp"
#include "ac1/format/PropertyValue.hpp"

namespace ac1 {

struct SaveGameObject {
    uint32_t classID;
    uint32_t propertyCount;
    uint32_t unknown;

    std::vector<Property> properties;
    std::vector<PropertyMetadata> pm;

    uint32_t unknown2;

    std::vector<PropertyValue> values;
};

} // namespace ac1
