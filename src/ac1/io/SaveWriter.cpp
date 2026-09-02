#include "ac1/io/SaveWriter.hpp"

#include <stdexcept>
#include <string>

namespace ac1 {

std::vector<std::byte> SaveWriter::write(const SaveGame& save)
{
    BinaryWriter writer;
    writeSave(writer, save);
    return writer.take();
}

void SaveWriter::writeSave(BinaryWriter& writer, const SaveGame& save)
{
    writer.u32be(save.fileSize);
    writer.u32be(save.magic);
    writer.u16be(save.version);
    writer.u32be(save.objectCount);

    for (const auto& object : save.objects) {
        writeObject(writer, object);
    }
}

void SaveWriter::writeObject(BinaryWriter& writer, const SaveGameObject& object)
{
    writer.u32be(object.classID);
    writer.u32be(object.propertyCount);
    for (const auto& record : object.properties) {
        writeObjectRefRecord(writer, record);
    }

    writer.u32be(object.metadataCount);

    for (const auto& metadata : object.pm) {
        writeMetadata(writer, metadata);
    }

    writer.u32be(object.unknown2);

    for (const auto& value : object.values) {
        writeValue(writer, value);
    }
}

void SaveWriter::writeObjectRefRecord(BinaryWriter& writer, const ObjectRefRecord& record)
{
    writer.u32be(record.unknown);
    writer.u8(record.objectCount);

    if (record.objectHandles.size() != record.objectCount) {
        throw std::runtime_error("Object reference count mismatch");
    }

    for (const auto& handle : record.objectHandles) {
        writer.u32be(handle.id);
        writer.u16be(handle.subIndex);
    }
}

void SaveWriter::writeMetadata(BinaryWriter& writer, const PropertyMetadata& metadata)
{
    writer.u32be(metadata.identifier);
    writer.u32be(metadata.unknown);
}

void SaveWriter::writeValue(BinaryWriter& writer, const PropertyValue& value)
{
    switch (value.type) {
        case SerializerFieldType::Bool: {
            const auto* data = std::get_if<bool>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: Bool");
            writer.u8(*data ? 1 : 0);
            break;
        }
        case SerializerFieldType::UInt8:
        case SerializerFieldType::Int8Alt1:
        case SerializerFieldType::Int8Alt2: {
            const auto* data = std::get_if<uint8_t>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: UInt8");
            writer.u8(*data);
            break;
        }
        case SerializerFieldType::UInt16:
        case SerializerFieldType::Int16: {
            const auto* data = std::get_if<uint16_t>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: UInt16");
            writer.u16be(*data);
            break;
        }
        case SerializerFieldType::UInt32:
        case SerializerFieldType::UInt32Alt:
        case SerializerFieldType::HashOrId:
        case SerializerFieldType::UInt32Alt3:
        case SerializerFieldType::UInt32Alt2: {
            const auto* data = std::get_if<uint32_t>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: UInt32");
            writer.u32be(*data);
            break;
        }
        // case SerializerFieldType::Float32: {
        //     const auto* data = std::get_if<float>(&value.data);
        //     if (!data) throw std::runtime_error("PropertyValue type mismatch: Float32");
        //     writer.f32be(*data);
        //     break;
        // }
        case SerializerFieldType::UInt64: {
            const auto* data = std::get_if<uint64_t>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: UInt64");
            writer.u64be(*data);
            break;
        }
        case SerializerFieldType::Float64: {
            const auto* data = std::get_if<double>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: Float64");
            writer.f64be(*data);
            break;
        }
        case SerializerFieldType::Vector2: {
            const auto* data = std::get_if<Vector2>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: Vector2");
            writer.f32be(data->x);
            writer.f32be(data->y);
            break;
        }
        case SerializerFieldType::Vector3: {
            const auto* data = std::get_if<Vector3>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: Vector3");
            writer.f32be(data->x);
            writer.f32be(data->y);
            writer.f32be(data->z);
            break;
        }
        case SerializerFieldType::Vector4: {
            const auto* data = std::get_if<Vector4>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: Vector4");
            writer.f32be(data->x);
            writer.f32be(data->y);
            writer.f32be(data->z);
            writer.f32be(data->w);
            break;
        }
        case SerializerFieldType::Matrix3x3: {
            const auto* data = std::get_if<Matrix3x3>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: Matrix3x3");
            writer.f32be(data->m01.x); writer.f32be(data->m01.y); writer.f32be(data->m01.z);
            writer.f32be(data->m02.x); writer.f32be(data->m02.y); writer.f32be(data->m02.z);
            writer.f32be(data->m03.x); writer.f32be(data->m03.y); writer.f32be(data->m03.z);
            break;
        }
        case SerializerFieldType::Matrix4x4: {
            const auto* data = std::get_if<Matrix4x4>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: Matrix4x4");
            writer.f32be(data->m01.x); writer.f32be(data->m01.y); writer.f32be(data->m01.z); writer.f32be(data->m01.w);
            writer.f32be(data->m02.x); writer.f32be(data->m02.y); writer.f32be(data->m02.z); writer.f32be(data->m02.w);
            writer.f32be(data->m03.x); writer.f32be(data->m03.y); writer.f32be(data->m03.z); writer.f32be(data->m03.w);
            writer.f32be(data->m04.x); writer.f32be(data->m04.y); writer.f32be(data->m04.z); writer.f32be(data->m04.w);
            break;
        }
        case SerializerFieldType::String: {
            const auto* data = std::get_if<std::string>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: String");
            writer.u32be(static_cast<uint32_t>(data->size()));
            if (!data->empty()) {
                writer.bytes(reinterpret_cast<const std::byte*>(data->data()), data->size());
            }
            break;
        }
        case SerializerFieldType::Array:
        case SerializerFieldType::ArrayAlt: {
            const auto* data = std::get_if<std::vector<std::byte>>(&value.data);
            if (!data) throw std::runtime_error("PropertyValue type mismatch: Array");
            writer.u32be(static_cast<uint32_t>(data->size()));
            if (!data->empty()) {
                writer.bytes(data->data(), data->size());
            }
            break;
        }
        default:
            throw std::runtime_error("Unsupported serializer field type: " + std::to_string(static_cast<uint32_t>(value.type)));
    }
}

} // namespace ac1
