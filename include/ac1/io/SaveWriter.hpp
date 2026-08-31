#pragma once

#include <cstddef>
#include <vector>

#include "ac1/BinaryWriter.hpp"
#include "ac1/format/SaveGame.hpp"

namespace ac1 {

class SaveWriter {
public:
    std::vector<std::byte> write(const SaveGame& save);

private:
    void writeSave(BinaryWriter& writer, const SaveGame& save);
    void writeObject(BinaryWriter& writer, const SaveGameObject& object);

    void writeProperty(BinaryWriter& writer, const Property& property);
    void writeMetadata(BinaryWriter& writer, const PropertyMetadata& metadata);

    void writeValue(
        BinaryWriter& writer,
        const PropertyValue& value
    );
};

} // namespace ac1
