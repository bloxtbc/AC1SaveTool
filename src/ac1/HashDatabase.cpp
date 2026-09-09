#include "ac1/HashDatabase.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace ac1 {

using json = nlohmann::json;

void HashDatabase::load(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error("Failed to open hash database: " + path);
    }

    json data;
    file >> data;

    hashes_.clear();
    classNames_.clear();

    // Load property hashes (new format: propHashes key, or old format: direct hashes)
    if (data.contains("propHashes") && data["propHashes"].is_object()) {
        for (const auto& [key, value] : data["propHashes"].items()) {
            uint32_t hash = 0;
            try {
                hash = static_cast<uint32_t>(std::stoul(key));
            } catch (...) {
                continue;
            }

            std::vector<std::string> names;
            for (const auto& name : value) {
                if (name.is_string()) {
                    names.push_back(name.get<std::string>());
                }
            }

            hashes_[hash] = std::move(names);
        }
    } else {
        // Legacy format: direct hash entries
        for (const auto& [key, value] : data.items()) {
            uint32_t hash = 0;
            try {
                hash = static_cast<uint32_t>(std::stoul(key));
            } catch (...) {
                continue;
            }

            std::vector<std::string> names;
            for (const auto& name : value) {
                if (name.is_string()) {
                    names.push_back(name.get<std::string>());
                }
            }

            hashes_[hash] = std::move(names);
        }
    }

    // Load class names (new format only)
    if (data.contains("classNames") && data["classNames"].is_object()) {
        for (const auto& [key, value] : data["classNames"].items()) {
            uint32_t classID = 0;
            try {
                classID = static_cast<uint32_t>(std::stoul(key));
            } catch (...) {
                continue;
            }

            // Extract the name from the Names array if it exists
            if (value.is_object() && value.contains("Names") && value["Names"].is_array() && !value["Names"].empty()) {
                if (value["Names"][0].is_string()) {
                    classNames_[classID] = value["Names"][0].get<std::string>();
                }
            }
        }
    }
}

const std::vector<std::string>* HashDatabase::lookup(uint32_t hash) const {
    const auto it = hashes_.find(hash);
    if (it == hashes_.end()) {
        return nullptr;
    }
    return &it->second;
}

const std::string* HashDatabase::lookupName(uint32_t hash) const {
    const auto it = hashes_.find(hash);
    if (it == hashes_.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.front();
}

const std::string* HashDatabase::lookupClassName(uint32_t classID) const {
    const auto it = classNames_.find(classID);
    if (it == classNames_.end()) {
        return nullptr;
    }
    return &it->second;
}

} // namespace ac1
