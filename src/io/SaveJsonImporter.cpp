#include "io/SaveJsonImporter.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace ac1 {

using json = nlohmann::json;


// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static uint32_t readHex32OrNumber(const json& value)
{
    if (value.is_number_unsigned()) {
        return value.get<uint32_t>();
    }

    if (value.is_number_integer()) {
        return static_cast<uint32_t>(
            value.get<int64_t>()
        );
    }

    if (value.is_string()) {
        const std::string str = value.get<std::string>();

        std::size_t pos = 0;

        uint64_t result = std::stoull(
            str,
            &pos,
            0
        );

        if (pos != str.size()) {
            throw std::runtime_error(
                "Invalid hexadecimal/numeric value: " + str
            );
        }

        return static_cast<uint32_t>(result);
    }

    throw std::runtime_error(
        "Expected integer or hexadecimal string"
    );
}


static uint64_t readHex64OrNumber(const json& value)
{
    if (value.is_number_unsigned()) {
        return value.get<uint64_t>();
    }

    if (value.is_number_integer()) {
        return static_cast<uint64_t>(
            value.get<int64_t>()
        );
    }

    if (value.is_string()) {
        const std::string str = value.get<std::string>();

        std::size_t pos = 0;

        uint64_t result = std::stoull(
            str,
            &pos,
            0
        );

        if (pos != str.size()) {
            throw std::runtime_error(
                "Invalid hexadecimal/numeric value: " + str
            );
        }

        return result;
    }

    throw std::runtime_error(
        "Expected integer or hexadecimal string"
    );
}


static const json& require(
    const json& object,
    const char* name
)
{
    if (!object.contains(name)) {
        throw std::runtime_error(
            std::string("Missing JSON field: ") + name
        );
    }

    return object.at(name);
}


// ------------------------------------------------------------
// Vectors
// ------------------------------------------------------------

static Vector2 vector2FromJson(const json& value)
{
    return Vector2{
        value.at("x").get<float>(),
        value.at("y").get<float>()
    };
}


static Vector3 vector3FromJson(const json& value)
{
    return Vector3{
        value.at("x").get<float>(),
        value.at("y").get<float>(),
        value.at("z").get<float>()
    };
}


static Vector4 vector4FromJson(const json& value)
{
    return Vector4{
        value.at("x").get<float>(),
        value.at("y").get<float>(),
        value.at("z").get<float>(),
        value.at("w").get<float>()
    };
}


// ------------------------------------------------------------
// Matrices
// ------------------------------------------------------------

static Matrix3x3 matrix3FromJson(const json& value)
{
    return Matrix3x3{
        vector3FromJson(value.at("m01")),
        vector3FromJson(value.at("m02")),
        vector3FromJson(value.at("m03"))
    };
}


static Matrix4x4 matrix4FromJson(const json& value)
{
    return Matrix4x4{
        vector4FromJson(value.at("m01")),
        vector4FromJson(value.at("m02")),
        vector4FromJson(value.at("m03")),
        vector4FromJson(value.at("m04"))
    };
}


// ------------------------------------------------------------
// Property values
// ------------------------------------------------------------

static std::vector<std::byte> hexToBytes(
    const std::string& hex
)
{
    if (hex.size() % 2 != 0) {
        throw std::runtime_error(
            "Array hex string has odd length"
        );
    }

    std::vector<std::byte> result;

    result.reserve(hex.size() / 2);

    auto hexDigit = [](char c) -> uint8_t {

        if (c >= '0' && c <= '9')
            return c - '0';

        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;

        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;

        throw std::runtime_error(
            "Invalid hexadecimal digit"
        );
    };

    for (std::size_t i = 0; i < hex.size(); i += 2) {

        uint8_t byte =
            static_cast<uint8_t>(
                (hexDigit(hex[i]) << 4) |
                hexDigit(hex[i + 1])
            );

        result.push_back(
            static_cast<std::byte>(byte)
        );
    }

    return result;
}

static PropertyValue valueFromJson(
    const json& valueJson
)
{
    PropertyValue result{};

    const std::string typeName =
        valueJson.at("type").get<std::string>();

    // --------------------------------------------------------
    // Bool
    // --------------------------------------------------------

    if (typeName == "Bool") {

        result.type = SerializerFieldType::Bool;
        result.data =
            valueJson.at("value").get<bool>();
    }

    // --------------------------------------------------------
    // 8-bit
    // --------------------------------------------------------

    else if (typeName == "UInt8") {

        result.type = SerializerFieldType::UInt8;
        result.data =
            valueJson.at("value").get<uint8_t>();
    }

    else if (typeName == "Int8Alt1") {

        result.type = SerializerFieldType::Int8Alt1;
        result.data =
            valueJson.at("value").get<uint8_t>();
    }

    else if (typeName == "Int8Alt2") {

        result.type = SerializerFieldType::Int8Alt2;
        result.data =
            valueJson.at("value").get<uint8_t>();
    }

    // --------------------------------------------------------
    // 16-bit
    // --------------------------------------------------------

    else if (typeName == "UInt16") {

        result.type = SerializerFieldType::UInt16;
        result.data =
            valueJson.at("value").get<uint16_t>();
    }

    else if (typeName == "Int16") {

        result.type = SerializerFieldType::Int16;
        result.data =
            valueJson.at("value").get<uint16_t>();
    }

    // --------------------------------------------------------
    // 32-bit
    // --------------------------------------------------------

    else if (typeName == "UInt32") {

        result.type = SerializerFieldType::UInt32;
        result.data =
            valueJson.at("value").get<uint32_t>();
    }

    else if (typeName == "UInt32Alt") {

        result.type = SerializerFieldType::UInt32Alt;
        result.data =
            valueJson.at("value").get<uint32_t>();
    }

    else if (typeName == "UInt32Alt2") {

        result.type = SerializerFieldType::UInt32Alt2;
        result.data =
            valueJson.at("value").get<uint32_t>();
    }

    else if (typeName == "HashOrId") {

        result.type = SerializerFieldType::HashOrId;
        result.data =
            valueJson.at("value").get<uint32_t>();
    }

    // --------------------------------------------------------
    // Floating point
    // --------------------------------------------------------

    else if (typeName == "Float32") {

        result.type = SerializerFieldType::Float32;
        result.data =
            valueJson.at("value").get<float>();
    }

    else if (typeName == "Float64") {

        result.type = SerializerFieldType::Float64;

        const auto& value =
            valueJson.at("value");

        if (value.is_string()) {
            result.data =
                std::stod(value.get<std::string>());
        }
        else {
            result.data =
                value.get<double>();
        }
    }

    // --------------------------------------------------------
    // UInt64
    // --------------------------------------------------------

    else if (typeName == "UInt64") {

        result.type = SerializerFieldType::UInt64;

        result.data =
            readHex64OrNumber(
                valueJson.at("value")
            );
    }

    // --------------------------------------------------------
    // Vector2
    // --------------------------------------------------------

    else if (typeName == "Vector2") {

        result.type = SerializerFieldType::Vector2;

        result.data =
            vector2FromJson(
                valueJson.at("value")
            );
    }

    // --------------------------------------------------------
    // Vector3
    // --------------------------------------------------------

    else if (typeName == "Vector3") {

        result.type = SerializerFieldType::Vector3;

        result.data =
            vector3FromJson(
                valueJson.at("value")
            );
    }

    // --------------------------------------------------------
    // Vector4
    // --------------------------------------------------------

    else if (typeName == "Vector4") {

        result.type = SerializerFieldType::Vector4;

        result.data =
            vector4FromJson(
                valueJson.at("value")
            );
    }

    // --------------------------------------------------------
    // Matrix3x3
    // --------------------------------------------------------

    else if (typeName == "Matrix3x3") {

        result.type = SerializerFieldType::Matrix3x3;

        result.data =
            matrix3FromJson(
                valueJson.at("value")
            );
    }

    // --------------------------------------------------------
    // Matrix4x4
    // --------------------------------------------------------

    else if (typeName == "Matrix4x4") {

        result.type = SerializerFieldType::Matrix4x4;

        result.data =
            matrix4FromJson(
                valueJson.at("value")
            );
    }

    // --------------------------------------------------------
    // String
    // --------------------------------------------------------

    else if (typeName == "String") {

        result.type = SerializerFieldType::String;

        result.data =
            valueJson.at("value").get<std::string>();
    }

    // --------------------------------------------------------
    // Array
    // --------------------------------------------------------

    else if (typeName == "Array") {

        result.type = SerializerFieldType::Array;

        const std::string hex =
            valueJson.at("hex").get<std::string>();

        auto bytes = hexToBytes(hex);

        // Verify the exported length matches.
        if (valueJson.contains("length")) {

            const std::size_t expected =
                valueJson.at("length").get<std::size_t>();

            if (bytes.size() != expected) {
                throw std::runtime_error(
                    "Array length mismatch: expected " +
                    std::to_string(expected) +
                    ", got " +
                    std::to_string(bytes.size())
                );
            }
        }

        result.data = std::move(bytes);
    }

    // --------------------------------------------------------
    // ArrayAlt
    // --------------------------------------------------------

    else if (typeName == "ArrayAlt") {

        result.type = SerializerFieldType::ArrayAlt;

        const std::string hex =
            valueJson.at("hex").get<std::string>();

        auto bytes = hexToBytes(hex);

        if (valueJson.contains("length")) {

            const std::size_t expected =
                valueJson.at("length").get<std::size_t>();

            if (bytes.size() != expected) {
                throw std::runtime_error(
                    "ArrayAlt length mismatch: expected " +
                    std::to_string(expected) +
                    ", got " +
                    std::to_string(bytes.size())
                );
            }
        }

        result.data = std::move(bytes);
    }

    // --------------------------------------------------------
    // Unsupported
    // --------------------------------------------------------

    else {

        throw std::runtime_error(
            "Unsupported property value type: " +
            typeName
        );
    }

    return result;
}


// ------------------------------------------------------------
// Properties
// ------------------------------------------------------------

static Property propertyFromJson(
    const json& propertyJson
)
{
    Property property{};

    property.propType =
        propertyJson.at("propType").get<uint8_t>();

    property.id =
        propertyJson.at("id").get<uint32_t>();

    property.index =
        propertyJson.at("index").get<uint16_t>();

    const json& payload =
        propertyJson.at("payload");

    switch (property.propType) {

    case 1:

        property.payload =
            PropertyType01{
                payload.at("value").get<uint32_t>()
            };

        break;

    case 2:

        property.payload =
            PropertyType02{
                payload.at("refId").get<uint32_t>(),
                payload.at("refIndex").get<uint16_t>(),
                payload.at("flags").get<uint32_t>()
            };

        break;

    case 3:

        property.payload =
            PropertyType03{
                payload.at("unk1").get<uint32_t>(),
                payload.at("unk2").get<uint16_t>(),
                payload.at("refId").get<uint32_t>(),
                payload.at("refIndex").get<uint16_t>(),
                payload.at("flags").get<uint32_t>()
            };

        break;

    case 4:

        property.payload =
            PropertyType04{
                payload.at("a").get<uint32_t>(),
                payload.at("b").get<uint32_t>(),
                payload.at("c").get<uint32_t>(),
                payload.at("d").get<uint32_t>(),
                payload.at("e").get<uint16_t>(),
                payload.at("f").get<uint32_t>()
            };

        break;

    case 5:

        property.payload =
            PropertyType05{
                payload.at("a").get<uint32_t>(),
                payload.at("b").get<uint32_t>(),
                payload.at("c").get<uint32_t>(),
                payload.at("d").get<uint32_t>(),
                payload.at("e").get<uint32_t>(),
                payload.at("f").get<uint32_t>(),
                payload.at("g").get<uint32_t>()
            };

        break;

    case 6:

        property.payload =
            PropertyType06{
                payload.at("a").get<uint32_t>(),
                payload.at("b").get<uint32_t>(),
                payload.at("c").get<uint32_t>(),
                payload.at("d").get<uint32_t>(),
                payload.at("e").get<uint32_t>(),
                payload.at("f").get<uint32_t>(),
                payload.at("g").get<uint32_t>(),
                payload.at("h").get<uint16_t>(),
                payload.at("i").get<uint32_t>()
            };

        break;

    default:

        throw std::runtime_error(
            "Unknown property type: " +
            std::to_string(property.propType)
        );
    }

    return property;
}


// ------------------------------------------------------------
// Metadata
// ------------------------------------------------------------

static PropertyMetadata metadataFromJson(
    const json& metadataJson
)
{
    PropertyMetadata metadata{};

    metadata.identifier =
        metadataJson.at("identifier").get<uint32_t>();

    metadata.unknown =
        metadataJson.at("unknown").get<uint32_t>();

    return metadata;
}


// ------------------------------------------------------------
// Object
// ------------------------------------------------------------

static SaveGameObject objectFromJson(
    const json& objectJson
)
{
    SaveGameObject object{};

    object.classID =
        objectJson.at("classID").get<uint32_t>();

    object.propertyCount =
        objectJson.at("propertyCount").get<uint32_t>();

    object.unknown =
        objectJson.at("unknown").get<uint32_t>();

    object.unknown2 =
        objectJson.at("unknown2").get<uint32_t>();

    const auto& properties =
        objectJson.at("properties");

    const auto& metadata =
        objectJson.at("metadata");

    const auto& values =
        objectJson.at("values");

    if (properties.size() != object.propertyCount) {
        throw std::runtime_error(
            "Property count mismatch"
        );
    }

    if (metadata.size() != object.propertyCount) {
        throw std::runtime_error(
            "Metadata count mismatch"
        );
    }

    if (values.size() != object.propertyCount) {
        throw std::runtime_error(
            "Value count mismatch"
        );
    }

    object.properties.reserve(
        object.propertyCount
    );

    object.pm.reserve(
        object.propertyCount
    );

    object.values.reserve(
        object.propertyCount
    );

    for (const auto& property : properties) {
        object.properties.push_back(
            propertyFromJson(property)
        );
    }

    for (const auto& pm : metadata) {
        object.pm.push_back(
            metadataFromJson(pm)
        );
    }

    for (const auto& value : values) {
        object.values.push_back(
            valueFromJson(value)
        );
    }

    return object;
}


// ------------------------------------------------------------
// Save
// ------------------------------------------------------------

SaveGame SaveJsonImporter::importSave(
    const std::string& jsonString
)
{
    const json root =
        json::parse(jsonString);

    SaveGame save{};

    save.fileSize =
        root.at("fileSize").get<uint32_t>();

    save.magic =
        readHex32OrNumber(root.at("magic"));

    save.version =
        root.at("version").get<uint16_t>();

    save.objectCount =
        root.at("objectCount").get<uint32_t>();

    const auto& objects =
        root.at("objects");

    if (objects.size() != save.objectCount) {
        throw std::runtime_error(
            "Object count mismatch: header says " +
            std::to_string(save.objectCount) +
            ", JSON contains " +
            std::to_string(objects.size())
        );
    }

    save.objects.reserve(
        save.objectCount
    );

    for (const auto& object : objects) {
        save.objects.push_back(
            objectFromJson(object)
        );
    }

    return save;
}

}