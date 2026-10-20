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


inline void addClassIDHash(
    json& object,
    const char* valueKey,
    uint32_t classID,
    const HashDatabase* hashes,
    bool includeHash = true
)
{
    if (includeHash) {
        object[valueKey] = classID;
    }

    if (hashes) {
        if (const auto* name = hashes->lookupClassName(classID)) {
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

json valueDataToJson(const PropertyValue& value)
{
    switch (value.type) {
        case SerializerFieldType::Bool:
            return std::get<bool>(value.data);

        case SerializerFieldType::UInt8:
        case SerializerFieldType::Int8Alt1:
        case SerializerFieldType::Int8Alt2:
            return std::get<uint8_t>(value.data);

        case SerializerFieldType::UInt16:
        case SerializerFieldType::Int16:
            return std::get<uint16_t>(value.data);

        case SerializerFieldType::UInt32:
        case SerializerFieldType::UInt32Alt:
        case SerializerFieldType::HashOrId:
        case SerializerFieldType::UInt32Alt2:
        case SerializerFieldType::UInt32Alt3:
            return std::get<uint32_t>(value.data);

        case SerializerFieldType::UInt64:
            return hex64(std::get<uint64_t>(value.data));

        case SerializerFieldType::Float64:
            return std::get<double>(value.data);

        case SerializerFieldType::Vector2:
            return vector2ToJson(std::get<Vector2>(value.data));

        case SerializerFieldType::Vector3:
            return vector3ToJson(std::get<Vector3>(value.data));

        case SerializerFieldType::Vector4:
            return vector4ToJson(std::get<Vector4>(value.data));

        case SerializerFieldType::Matrix3x3:
            return matrix3ToJson(std::get<Matrix3x3>(value.data));

        case SerializerFieldType::Matrix4x4:
            return matrix4ToJson(std::get<Matrix4x4>(value.data));

        case SerializerFieldType::String:
            return std::get<std::string>(value.data);

        default:
            throw std::runtime_error(
                "Cannot convert value to JSON: " +
                std::string(serializerFieldTypeNameOrThrow(value.type))
            );
    }
}

json valueToJson(const PropertyValue& value, const ObjectRefRecord* objRefRecord, const HashDatabase* hashes)
{
    json result;
    result["type"] = serializerFieldTypeNameOrThrow(value.type);

    if (objRefRecord)
        addObjectRefHash(result, *objRefRecord, hashes);

    switch (value.type) {
        case SerializerFieldType::Array:
        case SerializerFieldType::ArrayAlt: {
            const auto* elements = std::get_if<ac1::Array>(&value.data);

            if (!elements)
                throw std::runtime_error("PropertyValue type mismatch: Array");

            if (!value.elementType.has_value())
                throw std::runtime_error("Array PropertyValue has no element type");

            const auto elementType = *value.elementType;

            result["elementType"] = serializerFieldTypeNameOrThrow(elementType);
            result["count"] = elements->size();

            json arrayJson = json::array();

            for (const auto& element : *elements) {
                arrayJson.push_back(valueDataToJson(element));
            }

            result["value"] = std::move(arrayJson);
            break;
        }
        default:
            result["value"] = valueDataToJson(value);
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
        addClassIDHash(objectJson, "classID", object.classID, hashes);
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
