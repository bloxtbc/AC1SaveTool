#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "ac1/format/SerializerFieldType.hpp"
#include "ac1/format/Types.hpp"

namespace ac1 {

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
    Vector2,
    Vector3,
    Vector4,
    Matrix3x3,
    Matrix4x4,
    std::string,
    std::vector<std::byte>
>;

struct PropertyValue {
    SerializerFieldType type;
    PropertyValueData data;
};

} // namespace ac1
