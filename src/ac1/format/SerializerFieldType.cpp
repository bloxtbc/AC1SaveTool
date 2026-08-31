#include "ac1/format/SerializerFieldType.hpp"

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace ac1 {

namespace {

struct FieldTypeInfo {
    const char* name;
    SerializerFieldType type;
};

constexpr FieldTypeInfo kFieldTypeInfo[] = {
    {"Bool", SerializerFieldType::Bool},
    {"UInt8", SerializerFieldType::UInt8},
    {"Int8Alt1", SerializerFieldType::Int8Alt1},
    {"Int8Alt2", SerializerFieldType::Int8Alt2},
    {"UInt16", SerializerFieldType::UInt16},
    {"Int16", SerializerFieldType::Int16},
    {"UInt32Alt3", SerializerFieldType::UInt32Alt3},
    {"UInt32", SerializerFieldType::UInt32},
    {"Float64", SerializerFieldType::Float64},
    {"UInt64", SerializerFieldType::UInt64},
    {"UInt32Alt", SerializerFieldType::UInt32Alt},
    {"Vector2", SerializerFieldType::Vector2},
    {"Vector3", SerializerFieldType::Vector3},
    {"Vector4", SerializerFieldType::Vector4},
    {"UInt32X4X4", SerializerFieldType::UInt32X4X4},
    {"Matrix3x3", SerializerFieldType::Matrix3x3},
    {"Matrix4x4", SerializerFieldType::Matrix4x4},
    {"HashOrId", SerializerFieldType::HashOrId},
    {"Unknown12", SerializerFieldType::Unknown12},
    {"Unknown13", SerializerFieldType::Unknown13},
    {"Unknown14", SerializerFieldType::Unknown14},
    {"String", SerializerFieldType::String},
    {"Pointer", SerializerFieldType::Pointer},
    {"Struct", SerializerFieldType::Struct},
    {"Array", SerializerFieldType::Array},
    {"UInt32Alt2", SerializerFieldType::UInt32Alt2},
    {"Unknown1A", SerializerFieldType::Unknown1A},
    {"Unknown1B", SerializerFieldType::Unknown1B},
    {"Unknown1C", SerializerFieldType::Unknown1C},
    {"ArrayAlt", SerializerFieldType::ArrayAlt},
};

} // namespace

const char* serializerFieldTypeName(SerializerFieldType type)
{
    for (const auto& entry : kFieldTypeInfo) {
        if (entry.type == type) {
            return entry.name;
        }
    }

    return "Unknown";
}

SerializerFieldType serializerFieldTypeFromName(std::string_view name)
{
    for (const auto& entry : kFieldTypeInfo) {
        if (name == entry.name) {
            return entry.type;
        }
    }

    throw std::runtime_error(
        std::string("Unsupported property value type: ") + std::string(name)
    );
}

std::string_view serializerFieldTypeNameOrThrow(SerializerFieldType type)
{
    const char* name = serializerFieldTypeName(type);
    if (std::string_view{name} == "Unknown") {
        throw std::runtime_error(
            "Unsupported serializer field type: " +
            std::to_string(static_cast<uint32_t>(type))
        );
    }

    return name;
}

} // namespace ac1
