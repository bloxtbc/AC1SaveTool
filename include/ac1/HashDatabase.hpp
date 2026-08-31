#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ac1 {

class HashDatabase {
public:
    void load(const std::string& path);

    const std::vector<std::string>* lookup(uint32_t hash) const;
    const std::string* lookupName(uint32_t hash) const;

private:
    std::unordered_map<uint32_t, std::vector<std::string>> hashes_;
};

} // namespace ac1
