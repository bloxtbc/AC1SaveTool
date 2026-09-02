#include "ac1/io/SaveReader.hpp"

#include <stdexcept>

namespace ac1 {

ObjectRefRecord SaveReader::readObjectRefRecord(BinaryReader& reader)
{
    ObjectRefRecord record{};
    record.unknown = reader.u32be();
    record.objectCount = reader.u8();
    record.objectHandles.reserve(record.objectCount);

    for (uint8_t i = 0; i < record.objectCount; ++i) {
        record.objectHandles.push_back(ObjectHandle{
            reader.u32be(),
            reader.u16be()
        });
    }

    return record;
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
    object.properties.reserve(object.propertyCount);
    for (uint32_t i = 0; i < object.propertyCount; ++i) {
        object.properties.push_back(readObjectRefRecord(reader));
    }
    
    object.metadataCount = reader.u32be();
    object.pm.reserve(object.metadataCount);
    for (uint32_t i = 0; i < object.metadataCount; ++i) {
        object.pm.push_back(readMetadata(reader));
    }

    object.unknown2 = reader.u32be();
    object.values.reserve(object.metadataCount);
    for (uint32_t i = 0; i < object.metadataCount; ++i) {
        object.values.push_back(readValue(reader, object.pm[i].type()));
    }

    return object;
}

} // namespace ac1
