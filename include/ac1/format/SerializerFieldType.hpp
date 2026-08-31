#pragma once

#include <cstdint>
#include <string_view>

namespace ac1 {

enum class SerializerFieldType : uint32_t {
    Bool        = 0x00,
    UInt8       = 0x01,
    Int8Alt1    = 0x02,
    Int8Alt2    = 0x03,
    UInt16      = 0x04,
    Int16       = 0x05,
    UInt32Alt3     = 0x06,
    UInt32      = 0x07,
    Float64     = 0x08,
    UInt64      = 0x09,
    UInt32Alt   = 0x0A,
    Vector2     = 0x0B,
    Vector3     = 0x0C,
    Vector4     = 0x0D,
    UInt32X4X4  = 0x0E,
    Matrix3x3   = 0x0F,
    Matrix4x4   = 0x10,
    HashOrId    = 0x11,
    Unknown12   = 0x12,
    Unknown13   = 0x13,
    Unknown14   = 0x14,
    String      = 0x15,
    Pointer     = 0x16,
    Struct      = 0x17,
    Array       = 0x18,
    UInt32Alt2  = 0x19,
    Unknown1A   = 0x1A,
    Unknown1B   = 0x1B,
    Unknown1C   = 0x1C,
    ArrayAlt    = 0x1D
};

const char* serializerFieldTypeName(SerializerFieldType type);
SerializerFieldType serializerFieldTypeFromName(std::string_view name);

std::string_view serializerFieldTypeNameOrThrow(SerializerFieldType type);

} 
