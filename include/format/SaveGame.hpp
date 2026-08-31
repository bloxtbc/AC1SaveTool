#pragma once

#include <cstdint>
#include <vector>
#include "SaveGameObject.hpp"

struct SaveGame {
    uint32_t fileSize;
    uint32_t magic;
    uint16_t version;
    uint32_t objectCount;

    std::vector<SaveGameObject> objects;
};