#pragma once

#include <string>

#include "format/SaveGame.hpp"
#include "HashDatabase.hpp"

namespace ac1 {

class SaveJsonExporter {
public:
    static std::string exportSave(
        const SaveGame& save,
        const HashDatabase* hashes = nullptr,
        int indent = 2
    );
};

}