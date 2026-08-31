#include "io/SaveJsonExporter.hpp"

#include <nlohmann/json.hpp>

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace ac1 {

using json = nlohmann::json;

static std::string hex32(uint32_t value)
{
    std::ostringstream stream;

    stream
        << "0x"
        << std::uppercase
        << std::hex
        << std::setw(8)
        << std::setfill('0')
        << value;

    return stream.str();
}

static std::string hex64(uint64_t value)
{
    std::ostringstream stream;

    stream
        << "0x"
        << std::uppercase
        << std::hex
        << std::setw(16)
        << std::setfill('0')
        << value;

    return stream.str();
}

static const char* fieldTypeName(SerializerFieldType type)
{
    switch (type) {
    case SerializerFieldType::Bool:        return "Bool";
    case SerializerFieldType::UInt8:       return "UInt8";
    case SerializerFieldType::Int8Alt1:    return "Int8Alt1";
    case SerializerFieldType::Int8Alt2:    return "Int8Alt2";
    case SerializerFieldType::UInt16:      return "UInt16";
    case SerializerFieldType::Int16:       return "Int16";
    case SerializerFieldType::Float32:     return "Float32";
    case SerializerFieldType::UInt32:      return "UInt32";
    case SerializerFieldType::Float64:     return "Float64";
    case SerializerFieldType::UInt64:      return "UInt64";
    case SerializerFieldType::UInt32Alt:   return "UInt32Alt";
    case SerializerFieldType::Vector2:     return "Vector2";
    case SerializerFieldType::Vector3:     return "Vector3";
    case SerializerFieldType::Vector4:     return "Vector4";
    case SerializerFieldType::UInt32X4X4:  return "UInt32X4X4";
    case SerializerFieldType::Matrix3x3:   return "Matrix3x3";
    case SerializerFieldType::Matrix4x4:   return "Matrix4x4";
    case SerializerFieldType::HashOrId:    return "HashOrId";
    case SerializerFieldType::Unknown12:   return "Unknown12";
    case SerializerFieldType::Unknown13:   return "Unknown13";
    case SerializerFieldType::Unknown14:   return "Unknown14";
    case SerializerFieldType::String:      return "String";
    case SerializerFieldType::Pointer:     return "Pointer";
    case SerializerFieldType::Struct:      return "Struct";
    case SerializerFieldType::Array:       return "Array";
    case SerializerFieldType::UInt32Alt2:  return "UInt32Alt2";
    case SerializerFieldType::Unknown1A:   return "Unknown1A";
    case SerializerFieldType::Unknown1B:   return "Unknown1B";
    case SerializerFieldType::Unknown1C:   return "Unknown1C";
    case SerializerFieldType::ArrayAlt:    return "ArrayAlt";
    }

    return "Unknown";
}

static json vector2ToJson(const Vector2& value)
{
    return {
        {"x", value.x},
        {"y", value.y}
    };
}

static json vector3ToJson(const Vector3& value)
{
    return {
        {"x", value.x},
        {"y", value.y},
        {"z", value.z}
    };
}

static json vector4ToJson(const Vector4& value)
{
    return {
        {"x", value.x},
        {"y", value.y},
        {"z", value.z},
        {"w", value.w}
    };
}

static json matrix3ToJson(const Matrix3x3& value)
{
    return {
        {
            "m01",
            vector3ToJson(value.m01)
        },
        {
            "m02",
            vector3ToJson(value.m02)
        },
        {
            "m03",
            vector3ToJson(value.m03)
        }
    };
}

static json matrix4ToJson(const Matrix4x4& value)
{
    return {
        {
            "m01",
            vector4ToJson(value.m01)
        },
        {
            "m02",
            vector4ToJson(value.m02)
        },
        {
            "m03",
            vector4ToJson(value.m03)
        },
        {
            "m04",
            vector4ToJson(value.m04)
        }
    };
}

static json valueToJson(const PropertyValue& value)
{
    json result;

    result["type"] =
        fieldTypeName(value.type);


    switch (value.type) {

    case SerializerFieldType::Bool:
        result["value"] =
            std::get<bool>(value.data);
        break;

    case SerializerFieldType::UInt8:
    case SerializerFieldType::Int8Alt1:
    case SerializerFieldType::Int8Alt2:
        result["value"] =
            std::get<uint8_t>(value.data);
        break;

    case SerializerFieldType::UInt16:
    case SerializerFieldType::Int16:
        result["value"] =
            std::get<uint16_t>(value.data);
        break;

    case SerializerFieldType::UInt32:
    case SerializerFieldType::UInt32Alt:
    case SerializerFieldType::HashOrId:
    case SerializerFieldType::UInt32Alt2:
    {
        uint32_t v =
            std::get<uint32_t>(value.data);

        result["value"] = v;

        break;
    }

    case SerializerFieldType::Float32:
        result["value"] =
            std::get<float>(value.data);
        break;

    case SerializerFieldType::UInt64:
    {
        uint64_t v =
            std::get<uint64_t>(value.data);

        // JSON doesn't safely represent all uint64_t
        // values as a native JSON number.
        result["value"] = hex64(v);

        break;
    }

    case SerializerFieldType::Float64:
        result["value"] =
            std::get<double>(value.data);
        break;

    case SerializerFieldType::Vector2:
        result["value"] =
            vector2ToJson(
                std::get<Vector2>(value.data)
            );
        break;

    case SerializerFieldType::Vector3:
        result["value"] =
            vector3ToJson(
                std::get<Vector3>(value.data)
            );
        break;

    case SerializerFieldType::Vector4:
        result["value"] =
            vector4ToJson(
                std::get<Vector4>(value.data)
            );
        break;

    case SerializerFieldType::Matrix3x3:
        result["value"] =
            matrix3ToJson(
                std::get<Matrix3x3>(value.data)
            );
        break;

    case SerializerFieldType::Matrix4x4:
        result["value"] =
            matrix4ToJson(
                std::get<Matrix4x4>(value.data)
            );
        break;

    case SerializerFieldType::String:
        result["value"] =
            std::get<std::string>(value.data);
        break;

    case SerializerFieldType::Array:
    case SerializerFieldType::ArrayAlt:
    {
        const auto& bytes =
            std::get<std::vector<std::byte>>(
                value.data
            );

        std::string hex;

        for (std::byte byte : bytes) {
            std::ostringstream stream;

            stream
                << std::uppercase
                << std::hex
                << std::setw(2)
                << std::setfill('0')
                << std::to_integer<unsigned>(byte);

            hex += stream.str();
        }

        result["length"] = bytes.size();
        result["hex"] = hex;

        break;
    }

    default:
        throw std::runtime_error(
            "Cannot export unsupported property value type: " +
            std::to_string(
                static_cast<uint32_t>(value.type)
            )
        );
    }

    return result;
}

static json propertyToJson(const Property& property, const HashDatabase* hashes)
{
    json result;

    result["propType"] = property.propType;
    result["id"] = property.id;
    
    if (hashes) {
        if (const auto* name = hashes->lookupName(property.id)) {
            result["name"] = *name;
        }
    }
    result["index"] = property.index;

    std::visit(
        [&result](const auto& payload) {

            using T = std::decay_t<decltype(payload)>;

            json data;

            if constexpr (std::is_same_v<T, PropertyType01>) {
                data["value"] = payload.value;
            }
            else if constexpr (
                std::is_same_v<T, PropertyType02>
            ) {
                data["refId"] = payload.refId;
                data["refIndex"] = payload.refIndex;
                data["flags"] = payload.flags;
            }
            else if constexpr (
                std::is_same_v<T, PropertyType03>
            ) {
                data["unk1"] = payload.unk1;
                data["unk2"] = payload.unk2;
                data["refId"] = payload.refId;
                data["refIndex"] = payload.refIndex;
                data["flags"] = payload.flags;
            }
            else if constexpr (
                std::is_same_v<T, PropertyType04>
            ) {
                data["a"] = payload.a;
                data["b"] = payload.b;
                data["c"] = payload.c;
                data["d"] = payload.d;
                data["e"] = payload.e;
                data["f"] = payload.f;
            }
            else if constexpr (
                std::is_same_v<T, PropertyType05>
            ) {
                data["a"] = payload.a;
                data["b"] = payload.b;
                data["c"] = payload.c;
                data["d"] = payload.d;
                data["e"] = payload.e;
                data["f"] = payload.f;
                data["g"] = payload.g;
            }
            else if constexpr (
                std::is_same_v<T, PropertyType06>
            ) {
                data["a"] = payload.a;
                data["b"] = payload.b;
                data["c"] = payload.c;
                data["d"] = payload.d;
                data["e"] = payload.e;
                data["f"] = payload.f;
                data["g"] = payload.g;
                data["h"] = payload.h;
                data["i"] = payload.i;
            }

            result["payload"] = data;
        },
        property.payload
    );

    return result;
}

static json metadataToJson(
    const PropertyMetadata& metadata
)
{
    json result;

    result["identifier"] = metadata.identifier;

    result["unknown"] = metadata.unknown;

    result["type"] =
        fieldTypeName(metadata.type());

    return result;
}

std::string SaveJsonExporter::exportSave(
    const SaveGame& save,
    const HashDatabase* hashes,
    int indent
)
{
    json root;

    root["fileSize"] = save.fileSize;

    root["magic"] = hex32(save.magic);

    root["version"] = save.version;

    root["objectCount"] = save.objectCount;

    json objects = json::array();

    for (std::size_t i = 0; i < save.objects.size(); ++i) {

        const auto& object = save.objects[i];

        json objectJson;

        objectJson["index"] = i;

        objectJson["classID"] = object.classID;

        objectJson["propertyCount"] =
            object.propertyCount;

        objectJson["unknown"] =
            object.unknown;

        objectJson["unknown2"] =
            object.unknown2;

        json properties = json::array();

        for (const auto& property : object.properties) {
            properties.push_back(
                propertyToJson(property,hashes)
            );
        }

        objectJson["properties"] = properties;

        json metadata = json::array();

        for (const auto& pm : object.pm) {
            metadata.push_back(
                metadataToJson(pm)
            );
        }

        objectJson["metadata"] = metadata;

        json values = json::array();

        for (const auto& value : object.values) {
            values.push_back(
                valueToJson(value)
            );
        }

        objectJson["values"] = values;

        objects.push_back(
            std::move(objectJson)
        );
    }

    root["objects"] = std::move(objects);

    return root.dump(indent);
}

}