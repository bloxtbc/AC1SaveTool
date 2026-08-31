#include "ac1/io/SaveReader.hpp"

#include <stdexcept>

namespace ac1 {

Property SaveReader::readProperty(BinaryReader& reader)
{
    Property property{};
    property.propType = reader.u8();
    property.id = reader.u32be();
    property.index = reader.u16be();

    switch (property.propType) {
        case 1:
            property.payload = PropertyType01{ reader.u32be() };
            break;
        case 2:
            property.payload = PropertyType02{ reader.u32be(), reader.u16be(), reader.u32be() };
            break;
        case 3:
            property.payload = PropertyType03{ reader.u32be(), reader.u16be(), reader.u32be(), reader.u16be(), reader.u32be() };
            break;
        case 4:
            property.payload = PropertyType04{ reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be(), reader.u16be(), reader.u32be() };
            break;
        case 5:
            property.payload = PropertyType05{ reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be() };
            break;
        case 6:
            property.payload = PropertyType06{ reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be(), reader.u32be(), reader.u16be(), reader.u32be() };
            break;
        default:
            throw std::runtime_error("Unknown property type: " + std::to_string(property.propType));
    }

    return property;
}

PropertyMetadata SaveReader::readMetadata(BinaryReader& reader)
{
    PropertyMetadata metadata{};
    metadata.identifier = reader.u32be();
    metadata.unknown = reader.u32be();
    return metadata;
}

PropertyValue SaveReader::readValue(BinaryReader& reader, SerializerFieldType type)
{
    PropertyValue value{};
    value.type = type;

    switch (type) {
        case SerializerFieldType::Bool:
            value.data = reader.u8() != 0;
            break;
        case SerializerFieldType::UInt8:
        case SerializerFieldType::Int8Alt1:
        case SerializerFieldType::Int8Alt2:
            value.data = reader.u8();
            break;
        case SerializerFieldType::UInt16:
        case SerializerFieldType::Int16:
            value.data = reader.u16be();
            break;
        case SerializerFieldType::UInt32:
        case SerializerFieldType::UInt32Alt:
        case SerializerFieldType::HashOrId:
        case SerializerFieldType::UInt32Alt2:
        case SerializerFieldType::UInt32Alt3:
            value.data = reader.u32be();
            break;
        case SerializerFieldType::UInt64:
            value.data = reader.u64be();
            break;
        case SerializerFieldType::Float64:
            value.data = reader.f64be();
            break;
        case SerializerFieldType::Vector2:
            value.data = Vector2{ reader.f32be(), reader.f32be() };
            break;
        case SerializerFieldType::Vector3:
            value.data = Vector3{ reader.f32be(), reader.f32be(), reader.f32be() };
            break;
        case SerializerFieldType::Vector4:
            value.data = Vector4{ reader.f32be(), reader.f32be(), reader.f32be(), reader.f32be() };
            break;
        case SerializerFieldType::Matrix3x3:
            value.data = Matrix3x3{
                Vector3{ reader.f32be(), reader.f32be(), reader.f32be() },
                Vector3{ reader.f32be(), reader.f32be(), reader.f32be() },
                Vector3{ reader.f32be(), reader.f32be(), reader.f32be() }
            };
            break;
        case SerializerFieldType::Matrix4x4:
            value.data = Matrix4x4{
                Vector4{ reader.f32be(), reader.f32be(), reader.f32be(), reader.f32be() },
                Vector4{ reader.f32be(), reader.f32be(), reader.f32be(), reader.f32be() },
                Vector4{ reader.f32be(), reader.f32be(), reader.f32be(), reader.f32be() },
                Vector4{ reader.f32be(), reader.f32be(), reader.f32be(), reader.f32be() }
            };
            break;
        case SerializerFieldType::String: {
            const uint32_t length = reader.u32be();
            const auto bytes = reader.bytes(length);
            value.data = std::string(
                reinterpret_cast<const char*>(bytes.data()),
                bytes.size()
            );
            break;
        }
        case SerializerFieldType::Array:
        case SerializerFieldType::ArrayAlt: {
            const uint32_t length = reader.u32be();
            const auto bytes = reader.bytes(length);
            value.data = std::vector<std::byte>(bytes.begin(), bytes.end());
            break;
        }
        default:
            throw std::runtime_error("Unsupported serializer field type: " + std::to_string(static_cast<uint32_t>(type)));
    }

    return value;
}

SaveGame SaveReader::read(std::span<const std::byte> data)
{
    BinaryReader reader(data);
    return readSave(reader);
}

SaveGame SaveReader::readSave(BinaryReader& reader)
{
    SaveGame save{};
    save.fileSize = reader.u32be();
    save.magic = reader.u32be();
    save.version = reader.u16be();
    save.objectCount = reader.u32be();

    save.objects.reserve(save.objectCount);
    for (uint32_t i = 0; i < save.objectCount; ++i) {
        save.objects.push_back(readObject(reader));
    }

    return save;
}

SaveGameObject SaveReader::readObject(BinaryReader& reader)
{
    SaveGameObject object{};
    object.classID = reader.u32be();
    object.propertyCount = reader.u32be();
    object.unknown = reader.u32be();

    object.properties.reserve(object.propertyCount);
    object.pm.reserve(object.propertyCount);
    object.values.reserve(object.propertyCount);

    for (uint32_t i = 0; i < object.propertyCount; ++i) {
        object.properties.push_back(readProperty(reader));
    }

    for (uint32_t i = 0; i < object.propertyCount; ++i) {
        object.pm.push_back(readMetadata(reader));
    }

    object.unknown2 = reader.u32be();

    for (uint32_t i = 0; i < object.propertyCount; ++i) {
        object.values.push_back(readValue(reader, object.pm[i].type()));
    }

    return object;
}

} // namespace ac1
