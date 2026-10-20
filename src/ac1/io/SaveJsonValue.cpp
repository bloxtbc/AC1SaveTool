#include "ac1/io/SaveJsonValue.hpp"
#include "ac1/format/PropertyValue.hpp"

namespace ac1 {

using json = nlohmann::json;

PropertyValueData propertyValueDataForType(
    SerializerFieldType type,
    const json& valueJson)
{
    switch (type) {
        case SerializerFieldType::Bool:
            return valueJson.get<bool>();

        case SerializerFieldType::UInt8:
        case SerializerFieldType::Int8Alt1:
        case SerializerFieldType::Int8Alt2:
            return valueJson.get<uint8_t>();

        case SerializerFieldType::UInt16:
        case SerializerFieldType::Int16:
            return valueJson.get<uint16_t>();

        case SerializerFieldType::UInt32:
        case SerializerFieldType::UInt32Alt:
        case SerializerFieldType::HashOrId:
        case SerializerFieldType::UInt32Alt2:
        case SerializerFieldType::UInt32Alt3:
            return valueJson.get<uint32_t>();

        case SerializerFieldType::UInt64:
            return static_cast<uint64_t>(
                std::stoull(valueJson.get<std::string>(), nullptr, 0)
            );

        case SerializerFieldType::Float64:
            return valueJson.is_string()
                ? std::stod(valueJson.get<std::string>())
                : valueJson.get<double>();

        case SerializerFieldType::Vector2:
            return Vector2{
                valueJson.at("x").get<float>(),
                valueJson.at("y").get<float>()
            };

        case SerializerFieldType::Vector3:
            return Vector3{
                valueJson.at("x").get<float>(),
                valueJson.at("y").get<float>(),
                valueJson.at("z").get<float>()
            };

        case SerializerFieldType::Vector4:
            return Vector4{
                valueJson.at("x").get<float>(),
                valueJson.at("y").get<float>(),
                valueJson.at("z").get<float>(),
                valueJson.at("w").get<float>()
            };

        case SerializerFieldType::Matrix3x3:
            return Matrix3x3{
                Vector3{
                    valueJson.at("m01").at("x").get<float>(),
                    valueJson.at("m01").at("y").get<float>(),
                    valueJson.at("m01").at("z").get<float>()
                },
                Vector3{
                    valueJson.at("m02").at("x").get<float>(),
                    valueJson.at("m02").at("y").get<float>(),
                    valueJson.at("m02").at("z").get<float>()
                },
                Vector3{
                    valueJson.at("m03").at("x").get<float>(),
                    valueJson.at("m03").at("y").get<float>(),
                    valueJson.at("m03").at("z").get<float>()
                }
            };

        case SerializerFieldType::Matrix4x4:
            return Matrix4x4{
                Vector4{
                    valueJson.at("m01").at("x").get<float>(),
                    valueJson.at("m01").at("y").get<float>(),
                    valueJson.at("m01").at("z").get<float>(),
                    valueJson.at("m01").at("w").get<float>()
                },
                Vector4{
                    valueJson.at("m02").at("x").get<float>(),
                    valueJson.at("m02").at("y").get<float>(),
                    valueJson.at("m02").at("z").get<float>(),
                    valueJson.at("m02").at("w").get<float>()
                },
                Vector4{
                    valueJson.at("m03").at("x").get<float>(),
                    valueJson.at("m03").at("y").get<float>(),
                    valueJson.at("m03").at("z").get<float>(),
                    valueJson.at("m03").at("w").get<float>()
                },
                Vector4{
                    valueJson.at("m04").at("x").get<float>(),
                    valueJson.at("m04").at("y").get<float>(),
                    valueJson.at("m04").at("z").get<float>(),
                    valueJson.at("m04").at("w").get<float>()
                }
            };

        case SerializerFieldType::String:
            return valueJson.get<std::string>();

        case SerializerFieldType::Array:
        case SerializerFieldType::ArrayAlt:
            throw std::runtime_error(
                "Array must be handled by valueFromJson"
            );

        default:
            throw std::runtime_error(
                "Unsupported property value type: " +
                std::string(serializerFieldTypeNameOrThrow(type))
            );
    }
}

}