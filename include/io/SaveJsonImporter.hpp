#pragma once

#include <string>
#include "format/SaveGame.hpp"

namespace ac1 {

class SaveJsonImporter {
public:
    static SaveGame importSave(
        const std::string& json
    );
};

}