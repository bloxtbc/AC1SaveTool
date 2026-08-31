#pragma once

#include <cstdint>
#include <variant>
#include <string>
#include <vector>
#include "Types.hpp"

using PropertyValueData = std::variant<
    bool,

    uint8_t,
    int8_t,

    uint16_t,
    int16_t,

    uint32_t,
    int32_t,

    uint64_t,

    float,
    double,

    ac1::Vector2,
    ac1::Vector3,
    ac1::Vector4,

    ac1::Matrix3x3,
    ac1::Matrix4x4,

    std::string,

    std::vector<std::byte>
>;


struct PropertyValue {
    ac1::SerializerFieldType type;
    PropertyValueData data;
};