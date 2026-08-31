#pragma once

#include <string>

#include "ac1/format/SaveGame.hpp"

namespace ac1 {

class SaveJsonImporter {
public:
    static SaveGame importSave(const std::string& jsonString);
};

} // namespace ac1
