#include "ac1/io/SaveJsonImporter.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "ac1/format/SerializerFieldType.hpp"

namespace ac1 {

using json = nlohmann::json;

namespace {

uint32_t readHex32OrNumber(const json& value)
{
    if (value.is_number_unsigned()) {
        return value.get<uint32_t>();
    }
    if (value.is_number_integer()) {
        return static_cast<uint32_t>(value.get<int64_t>());
    }
    if (value.is_string()) {
        const std::string str = value.get<std::string>();
        std::size_t pos = 0;
        const uint64_t result = std::stoull(str, &pos, 0);
        if (pos != str.size()) {
            throw std::runtime_error("Invalid hexadecimal/numeric value: " + str);
        }
        return static_cast<uint32_t>(result);
    }
    throw std::runtime_error("Expected integer or hexadecimal string");
}

uint64_t readHex64OrNumber(const json& value)
{
    if (value.is_number_unsigned()) {
        return value.get<uint64_t>();
    }
    if (value.is_number_integer()) {
        return static_cast<uint64_t>(value.get<int64_t>());
    }
    if (value.is_string()) {
        const std::string str = value.get<std::string>();
        std::size_t pos = 0;
        const uint64_t result = std::stoull(str, &pos, 0);
        if (pos != str.size()) {
            throw std::runtime_error("Invalid hexadecimal/numeric value: " + str);
        }
        return result;
    }
    throw std::runtime_error("Expected integer or hexadecimal string");
}

const json& require(const json& object, const char* name)
{
    if (!object.contains(name)) {
        throw std::runtime_error(std::string("Missing JSON field: ") + name);
    }
    return object.at(name);
}

Vector2 vector2FromJson(const json& value)
{
    return Vector2{ value.at("x").get<float>(), value.at("y").get<float>() };
}

Vector3 vector3FromJson(const json& value)
{
    return Vector3{ value.at("x").get<float>(), value.at("y").get<float>(), value.at("z").get<float>() };
}

Vector4 vector4FromJson(const json& value)
{
    return Vector4{ value.at("x").get<float>(), value.at("y").get<float>(), value.at("z").get<float>(), value.at("w").get<float>() };
}

Matrix3x3 matrix3FromJson(const json& value)
{
    return Matrix3x3{
        vector3FromJson(value.at("m01")),
        vector3FromJson(value.at("m02")),
        vector3FromJson(value.at("m03"))
    };
}

Matrix4x4 matrix4FromJson(const json& value)
{
    return Matrix4x4{
        vector4FromJson(value.at("m01")),
        vector4FromJson(value.at("m02")),
        vector4FromJson(value.at("m03")),
        vector4FromJson(value.at("m04"))
    };
}

std::vector<std::byte> hexToBytes(const std::string& hex)
{
    if (hex.size() % 2 != 0) {
        throw std::runtime_error("Array hex string has odd length");
    }

    std::vector<std::byte> result;
    result.reserve(hex.size() / 2);

    auto hexDigit = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        throw std::runtime_error("Invalid hexadecimal digit");
    };

    for (std::size_t i = 0; i < hex.size(); i += 2) {
        const uint8_t byte =
            static_cast<uint8_t>((hexDigit(hex[i]) << 4) | hexDigit(hex[i + 1]));
        result.push_back(static_cast<std::byte>(byte));
    }

    return result;
}

PropertyValue valueFromJson(const json& valueJson)
{
    PropertyValue result{};
    const std::string typeName = valueJson.at("type").get<std::string>();
    result.type = serializerFieldTypeFromName(typeName);

    switch (result.type) {
        case SerializerFieldType::Bool:
            result.data = valueJson.at("value").get<bool>();
            break;
        case SerializerFieldType::UInt8:
        case SerializerFieldType::Int8Alt1:
        case SerializerFieldType::Int8Alt2:
            result.data = valueJson.at("value").get<uint8_t>();
            break;
        case SerializerFieldType::UInt16:
        case SerializerFieldType::Int16:
            result.data = valueJson.at("value").get<uint16_t>();
            break;
        case SerializerFieldType::UInt32:
        case SerializerFieldType::UInt32Alt:
        case SerializerFieldType::HashOrId:
        case SerializerFieldType::UInt32Alt2:
        case SerializerFieldType::UInt32Alt3:
            result.data = valueJson.at("value").get<uint32_t>();
            break;
        case SerializerFieldType::Float64:
            if (valueJson.at("value").is_string()) {
                result.data = std::stod(valueJson.at("value").get<std::string>());
            } else {
                result.data = valueJson.at("value").get<double>();
            }
            break;
        case SerializerFieldType::UInt64:
            result.data = readHex64OrNumber(valueJson.at("value"));
            break;
        case SerializerFieldType::Vector2:
            result.data = vector2FromJson(valueJson.at("value"));
            break;
        case SerializerFieldType::Vector3:
            result.data = vector3FromJson(valueJson.at("value"));
            break;
        case SerializerFieldType::Vector4:
            result.data = vector4FromJson(valueJson.at("value"));
            break;
        case SerializerFieldType::Matrix3x3:
            result.data = matrix3FromJson(valueJson.at("value"));
            break;
        case SerializerFieldType::Matrix4x4:
            result.data = matrix4FromJson(valueJson.at("value"));
            break;
        case SerializerFieldType::String:
            result.data = valueJson.at("value").get<std::string>();
            break;
        case SerializerFieldType::Array:
        case SerializerFieldType::ArrayAlt: {
            const std::string hex = valueJson.at("hex").get<std::string>();
            auto bytes = hexToBytes(hex);
            if (valueJson.contains("length")) {
                const std::size_t expected = valueJson.at("length").get<std::size_t>();
                if (bytes.size() != expected) {
                    throw std::runtime_error("Array length mismatch: expected " + std::to_string(expected) + ", got " + std::to_string(bytes.size()));
                }
            }
            result.data = std::move(bytes);
            break;
        }
        default:
            throw std::runtime_error("Unsupported property value type: " + typeName);
    }

    return result;
}

ObjectRefRecord objectRefRecordFromJson(const json& recordJson)
{
    ObjectRefRecord record{};
    record.unknown = recordJson.at("unknown").get<uint32_t>();
    record.objectCount = recordJson.at("objectCount").get<uint8_t>();

    const auto& handles = recordJson.at("objectHandles");
    if (handles.size() != record.objectCount) {
        throw std::runtime_error("Object handle count mismatch");
    }

    record.objectHandles.reserve(record.objectCount);
    for (const auto& handleJson : handles) {
        record.objectHandles.push_back(ObjectHandle{
            readHex32OrNumber(handleJson.at("id")),
            handleJson.at("subIndex").get<uint16_t>()
        });
    }

    return record;
}

PropertyMetadata metadataFromJson(const json& metadataJson)
{
    PropertyMetadata metadata{};
    metadata.identifier = metadataJson.at("identifier").get<uint32_t>();
    metadata.unknown = metadataJson.at("unknown").get<uint32_t>();
    return metadata;
}

SaveGameObject objectFromJson(const json& objectJson)
{
    SaveGameObject object{};
    object.classID = objectJson.at("classID").get<uint32_t>();
    object.propertyCount = objectJson.at("propertyCount").get<uint32_t>();
    object.unknown1 = objectJson.at("unknown1").get<uint32_t>();
    object.unknown2 = objectJson.at("unknown2").get<uint32_t>();

    const auto& properties = objectJson.at("properties");
    const auto& metadata = objectJson.at("metadata");
    const auto& values = objectJson.at("values");

    if (properties.size() != object.propertyCount) {
        throw std::runtime_error("Property count mismatch");
    }
    if (metadata.size() != object.propertyCount) {
        throw std::runtime_error("Metadata count mismatch");
    }
    if (values.size() != object.propertyCount) {
        throw std::runtime_error("Value count mismatch");
    }

    object.properties.reserve(object.propertyCount);
    object.pm.reserve(object.propertyCount);
    object.values.reserve(object.propertyCount);

    for (const auto& property : properties) {
        object.properties.push_back(objectRefRecordFromJson(property));
    }
    for (const auto& pm : metadata) {
        object.pm.push_back(metadataFromJson(pm));
    }
    for (const auto& value : values) {
        object.values.push_back(valueFromJson(value));
    }

    return object;
}

} // namespace

SaveGame SaveJsonImporter::importSave(const std::string& jsonString)
{
    const json root = json::parse(jsonString);
    SaveGame save{};

    save.fileSize = root.at("fileSize").get<uint32_t>();
    save.magic = readHex32OrNumber(root.at("magic"));
    save.version = root.at("version").get<uint16_t>();
    save.objectCount = root.at("objectCount").get<uint32_t>();

    const auto& objects = root.at("objects");
    if (objects.size() != save.objectCount) {
        throw std::runtime_error("Object count mismatch: header says " + std::to_string(save.objectCount) + ", JSON contains " + std::to_string(objects.size()));
    }

    save.objects.reserve(save.objectCount);
    for (const auto& object : objects) {
        save.objects.push_back(objectFromJson(object));
    }

    return save;
}

} // namespace ac1
