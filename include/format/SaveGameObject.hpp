#pragma once

#include <cstdint>
#include <vector>

#include "Property.hpp"
#include "PropertyMetadata.hpp"
#include "PropertyValue.hpp"


struct SaveGameObject {
    uint32_t classID;
    uint32_t propertyCount;
    uint32_t unknown;

    std::vector<Property> properties;
    std::vector<PropertyMetadata> pm;

    uint32_t unknown2;

    std::vector<PropertyValue> values;
};