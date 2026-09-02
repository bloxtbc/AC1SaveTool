#pragma once

#include <cstddef>
#include <span>

#include "ac1/BinaryReader.hpp"
#include "ac1/format/SaveGame.hpp"

namespace ac1 {

class SaveReader {
public:
    SaveGame read(std::span<const std::byte> data);

private:
    SaveGame readSave(BinaryReader& reader);
    SaveGameObject readObject(BinaryReader& reader);

    ObjectRefRecord readObjectRefRecord(BinaryReader& reader);
    PropertyMetadata readMetadata(BinaryReader& reader);

    PropertyValue readValue(
        BinaryReader& reader,
        SerializerFieldType type
    );
};

} // namespace ac1
