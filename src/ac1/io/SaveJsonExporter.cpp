#include "ac1/io/SaveJsonExporter.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "ac1/format/SerializerFieldType.hpp"

namespace ac1 {

using json = nlohmann::json;

namespace {

std::string hex32(uint32_t value)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << value;
    return stream.str();
}

std::string hex64(uint64_t value)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << value;
    return stream.str();
}

json vector2ToJson(const Vector2& value)
{
    return { {"x", value.x}, {"y", value.y} };
}

json vector3ToJson(const Vector3& value)
{
    return { {"x", value.x}, {"y", value.y}, {"z", value.z} };
}

json vector4ToJson(const Vector4& value)
{
    return { {"x", value.x}, {"y", value.y}, {"z", value.z}, {"w", value.w} };
}

json matrix3ToJson(const Matrix3x3& value)
{
    return {
        {"m01", vector3ToJson(value.m01)},
        {"m02", vector3ToJson(value.m02)},
        {"m03", vector3ToJson(value.m03)}
    };
}

json matrix4ToJson(const Matrix4x4& value)
{
    return {
        {"m01", vector4ToJson(value.m01)},
        {"m02", vector4ToJson(value.m02)},
        {"m03", vector4ToJson(value.m03)},
        {"m04", vector4ToJson(value.m04)}
    };
}

PropertyValueData propertyValueDataForType(SerializerFieldType type, const json& valueJson)
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
            return static_cast<uint64_t>(std::stoull(valueJson.get<std::string>(), nullptr, 0));
        case SerializerFieldType::Float64:
            return valueJson.is_string() ? std::stod(valueJson.get<std::string>()) : valueJson.get<double>();
        case SerializerFieldType::Vector2:
            return Vector2{ valueJson.at("x").get<float>(), valueJson.at("y").get<float>() };
        case SerializerFieldType::Vector3:
            return Vector3{ valueJson.at("x").get<float>(), valueJson.at("y").get<float>(), valueJson.at("z").get<float>() };
        case SerializerFieldType::Vector4:
            return Vector4{ valueJson.at("x").get<float>(), valueJson.at("y").get<float>(), valueJson.at("z").get<float>(), valueJson.at("w").get<float>() };
        case SerializerFieldType::Matrix3x3:
            return Matrix3x3{ Vector3{ valueJson.at("m01").at("x").get<float>(), valueJson.at("m01").at("y").get<float>(), valueJson.at("m01").at("z").get<float>() }, Vector3{ valueJson.at("m02").at("x").get<float>(), valueJson.at("m02").at("y").get<float>(), valueJson.at("m02").at("z").get<float>() }, Vector3{ valueJson.at("m03").at("x").get<float>(), valueJson.at("m03").at("y").get<float>(), valueJson.at("m03").at("z").get<float>() } };
        case SerializerFieldType::Matrix4x4:
            return Matrix4x4{ Vector4{ valueJson.at("m01").at("x").get<float>(), valueJson.at("m01").at("y").get<float>(), valueJson.at("m01").at("z").get<float>(), valueJson.at("m01").at("w").get<float>() }, Vector4{ valueJson.at("m02").at("x").get<float>(), valueJson.at("m02").at("y").get<float>(), valueJson.at("m02").at("z").get<float>(), valueJson.at("m02").at("w").get<float>() }, Vector4{ valueJson.at("m03").at("x").get<float>(), valueJson.at("m03").at("y").get<float>(), valueJson.at("m03").at("z").get<float>(), valueJson.at("m03").at("w").get<float>() }, Vector4{ valueJson.at("m04").at("x").get<float>(), valueJson.at("m04").at("y").get<float>(), valueJson.at("m04").at("z").get<float>(), valueJson.at("m04").at("w").get<float>() } };
        case SerializerFieldType::String:
            return valueJson.get<std::string>();
        case SerializerFieldType::Array:
        case SerializerFieldType::ArrayAlt:
        {
            const std::string hex = valueJson.at("hex").get<std::string>();
            if (hex.size() % 2 != 0) {
                throw std::runtime_error("Array hex string has odd length");
            }
            std::vector<std::byte> bytes;
            bytes.reserve(hex.size() / 2);
            for (std::size_t i = 0; i < hex.size(); i += 2) {
                const auto hi = std::toupper(static_cast<unsigned char>(hex[i]));
                const auto lo = std::toupper(static_cast<unsigned char>(hex[i + 1]));
                const auto digit = [](unsigned char c) -> uint8_t { if (c >= '0' && c <= '9') return c - '0'; if (c >= 'A' && c <= 'F') return c - 'A' + 10; throw std::runtime_error("Invalid hexadecimal digit"); };
                bytes.push_back(static_cast<std::byte>((digit(hi) << 4) | digit(lo)));
            }
            if (valueJson.contains("length")) {
                const auto expected = valueJson.at("length").get<std::size_t>();
                if (bytes.size() != expected) {
                    throw std::runtime_error("Array length mismatch: expected " + std::to_string(expected) + ", got " + std::to_string(bytes.size()));
                }
            }
            return bytes;
        }
        default:
            throw std::runtime_error("Unsupported property value type: " + std::string(serializerFieldTypeNameOrThrow(type)));
    }
}


inline void addHash(
    json& object,
    const char* valueKey,
    uint32_t value,
    const HashDatabase* hashes,
    bool includeHash = true
)
{
    if (includeHash) {
        object[valueKey] = value;
    }

    if (hashes) {
        if (const auto* name = hashes->lookupName(value)) {
            std::string nameKey = std::string(valueKey) + "Hash";
            object[nameKey] = *name;
        }
    }
}


void addObjectRefHash(json& result, const ObjectRefRecord& record,const HashDatabase* hashes) 
{
    if (!hashes) {
        return;
    }

    for (std::size_t i = 0; i < record.objectHandles.size(); ++i) {
        std::string prefix = "objHandles[" + std::to_string(i) + "]";
        const auto& handle = record.objectHandles[i];
        std::string idKey = prefix + ".id";
        addHash(result, idKey.c_str(), handle.id, hashes);
        if (handle.subIndex != 65535) {
            std::string subIndexKey = prefix + ".subIndex";
            result[subIndexKey] = handle.subIndex;
        }
    }
}

json valueToJson(const PropertyValue& value, const ObjectRefRecord* objRefRecord, const HashDatabase* hashes)
{
    json result;
    result["type"] = serializerFieldTypeNameOrThrow(value.type);

    if (objRefRecord) {
        addObjectRefHash(result, *objRefRecord, hashes);
    }

    switch (value.type) {
        case SerializerFieldType::Bool:
            result["value"] = std::get<bool>(value.data);
            break;
        case SerializerFieldType::UInt8:
        case SerializerFieldType::Int8Alt1:
        case SerializerFieldType::Int8Alt2:
            result["value"] = std::get<uint8_t>(value.data);
            break;
        case SerializerFieldType::UInt16:
        case SerializerFieldType::Int16:
            result["value"] = std::get<uint16_t>(value.data);
            break;
        case SerializerFieldType::UInt32:
        case SerializerFieldType::UInt32Alt:
        case SerializerFieldType::HashOrId:
        case SerializerFieldType::UInt32Alt2:
        case SerializerFieldType::UInt32Alt3:
            result["value"] = std::get<uint32_t>(value.data);
            break;
        case SerializerFieldType::UInt64:
            result["value"] = hex64(std::get<uint64_t>(value.data));
            break;
        case SerializerFieldType::Float64:
            result["value"] = std::get<double>(value.data);
            break;
        case SerializerFieldType::Vector2:
            result["value"] = vector2ToJson(std::get<Vector2>(value.data));
            break;
        case SerializerFieldType::Vector3:
            result["value"] = vector3ToJson(std::get<Vector3>(value.data));
            break;
        case SerializerFieldType::Vector4:
            result["value"] = vector4ToJson(std::get<Vector4>(value.data));
            break;
        case SerializerFieldType::Matrix3x3:
            result["value"] = matrix3ToJson(std::get<Matrix3x3>(value.data));
            break;
        case SerializerFieldType::Matrix4x4:
            result["value"] = matrix4ToJson(std::get<Matrix4x4>(value.data));
            break;
        case SerializerFieldType::String:
            result["value"] = std::get<std::string>(value.data);
            break;
        case SerializerFieldType::Array:
        case SerializerFieldType::ArrayAlt: {
            const auto& bytes = std::get<std::vector<std::byte>>(value.data);
            std::string hex;
            hex.reserve(bytes.size() * 2);
            for (std::byte byte : bytes) {
                const auto value = std::to_integer<unsigned>(byte);
                char buffer[3];
                std::snprintf(buffer, sizeof(buffer), "%02X", value);
                hex += buffer;
            }
            result["length"] = bytes.size();
            result["hex"] = hex;
            break;
        }
        default:
            throw std::runtime_error("Cannot export unsupported property value type: " + std::to_string(static_cast<uint32_t>(value.type)));
    }

    return result;
}

json objectRefRecordToJson(const ObjectRefRecord& record, const HashDatabase* hashes)
{
    json result;
    result["unknown"] = record.unknown;
    result["objectCount"] = record.objectCount;

    json handles = json::array();
    for (const auto& handle : record.objectHandles) {
        json handleJson;
        addHash(handleJson, "id", handle.id, hashes);
        handleJson["subIndex"] = handle.subIndex;
        handles.push_back(std::move(handleJson));
    }
    result["objectHandles"] = std::move(handles);

    return result;
}

json metadataToJson(const PropertyMetadata& metadata)
{
    json result;
    result["identifier"] = metadata.identifier;
    result["unknown"] = metadata.unknown;
    result["type"] = serializerFieldTypeNameOrThrow(metadata.type());
    return result;
}

} // namespace

std::string SaveJsonExporter::exportSave(
    const SaveGame& save,
    const HashDatabase* hashes,
    int indent)
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
        objectJson["propertyCount"] = object.propertyCount;
        objectJson["metadataCount"] = object.metadataCount;
        objectJson["unknown2"] = object.unknown2;

        json properties = json::array();
        for (const auto& property : object.properties) {
            properties.push_back(objectRefRecordToJson(property, hashes));
        }
        objectJson["properties"] = properties;

        json metadata = json::array();
        for (const auto& pm : object.pm) {
            metadata.push_back(metadataToJson(pm));
        }
        objectJson["metadata"] = metadata;

        json values = json::array();

        for (std::size_t i = 0; i < object.values.size(); ++i) {
            const auto& value = object.values[i];

            const ObjectRefRecord* objRefRecord = nullptr;

            if (i < object.properties.size()) {
                objRefRecord = &object.properties[i];
            }

            values.push_back(
                valueToJson(
                    value,
                    objRefRecord,
                    hashes
                )
            );
        }
        objectJson["values"] = values;

        objects.push_back(std::move(objectJson));
    }

    root["objects"] = std::move(objects);
    return root.dump(indent);
}

} // namespace ac1
