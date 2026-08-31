#include "HashDatabase.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace ac1 {

using json = nlohmann::json;

void HashDatabase::load(const std::string& path)
{
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error(
            "Failed to open hash database: " + path
        );
    }

    json data;
    file >> data;

    hashes_.clear();

    for (const auto& [key, value] : data.items()) {
        uint32_t hash;

        try {
            hash = static_cast<uint32_t>(
                std::stoul(key)
            );
        }
        catch (...) {
            continue;
        }

        std::vector<std::string> names;

        for (const auto& name : value) {
            if (name.is_string()) {
                names.push_back(
                    name.get<std::string>()
                );
            }
        }

        hashes_[hash] = std::move(names);
    }
}

const std::vector<std::string>* HashDatabase::lookup(
    uint32_t hash
) const
{
    auto it = hashes_.find(hash);

    if (it == hashes_.end()) {
        return nullptr;
    }

    return &it->second;
}

const std::string* HashDatabase::lookupName(
    uint32_t hash
) const
{
    auto it = hashes_.find(hash);

    if (it == hashes_.end() || it->second.empty()) {
        return nullptr;
    }

    return &it->second.front();
}

}