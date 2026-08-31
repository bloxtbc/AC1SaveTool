#pragma once

#include <cstdint>
#include <vector>

#include "ac1/format/SaveGameObject.hpp"

namespace ac1 {

struct SaveGame {
    uint32_t fileSize;
    uint32_t magic;
    uint16_t version;
    uint32_t objectCount;

    std::vector<SaveGameObject> objects;
};

} // namespace ac1
