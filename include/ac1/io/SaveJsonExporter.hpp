#pragma once

#include <string>

#include "ac1/HashDatabase.hpp"
#include "ac1/format/SaveGame.hpp"

namespace ac1 {

class SaveJsonExporter {
public:
    static std::string exportSave(
        const SaveGame& save,
        const HashDatabase* hashes = nullptr,
        int indent = 2
    );
};

} // namespace ac1
