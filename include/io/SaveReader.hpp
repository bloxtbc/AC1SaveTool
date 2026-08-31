#pragma once

#include <cstddef>
#include <span>

#include "BinaryReader.hpp"
#include "format/SaveGame.hpp"

namespace ac1 {

class SaveReader {
public:
    SaveGame read(std::span<const std::byte> data);

private:
    SaveGame readSave(BinaryReader& reader);
    SaveGameObject readObject(BinaryReader& reader);

    Property readProperty(BinaryReader& reader);
    PropertyMetadata readMetadata(BinaryReader& reader);

    PropertyValue readValue(
        BinaryReader& reader,
        SerializerFieldType type
    );
};

}