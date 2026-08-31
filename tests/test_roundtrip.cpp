#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <vector>

#include "ac1/io/SaveReader.hpp"
#include "ac1/io/SaveWriter.hpp"

namespace fs = std::filesystem;

static std::vector<std::byte> readFile(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error(
            "Failed to open: " + path.string()
        );
    }

    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::byte> data(
        static_cast<std::size_t>(size)
    );

    file.read(
        reinterpret_cast<char*>(data.data()),
        static_cast<std::streamsize>(data.size())
    );

    return data;
}

static void writeFile(
    const fs::path& path,
    std::span<const std::byte> data
)
{
    std::ofstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error(
            "Failed to create: " + path.string()
        );
    }

    file.write(
        reinterpret_cast<const char*>(data.data()),
        static_cast<std::streamsize>(data.size())
    );
}

int main()
{
    try {
        const fs::path input = "testSave.sav";
        const fs::path output = "testSave_roundtrip.sav";

        printf("AC1 Save Tool - Round-Trip Test\n");
        printf("Input:  \"%s\"\n", input.string().c_str());
        printf("Reading: %s\n", input.string().c_str());

        auto original = readFile(input);

        printf("Original size: %zu bytes\n", original.size());

        ac1::SaveReader reader;

        auto save = reader.read(original);

        printf("Magic: 0x%x\n", save.magic);

        printf("Version: %d\n", save.version);

        printf("Objects: %d\n", save.objectCount);

        ac1::SaveWriter writer;

        auto rebuilt = writer.write(save);

        printf("Rebuilt size: %zu bytes\n", rebuilt.size());

        writeFile(output, rebuilt);

        if (original.size() != rebuilt.size()) {
            printf("FAIL: File sizes differ!\n");

            return 1;
        }

        for (std::size_t i = 0; i < original.size(); ++i) {
            if (original[i] != rebuilt[i]) {
                printf("FAIL: Files differ at offset 0x%zx\n", i);
                printf("Original: 0x%x\n", std::to_integer<unsigned>(original[i]));
                printf("Rebuilt:  0x%x\n", std::to_integer<unsigned>(rebuilt[i]));

                return 1;
            }
        }

        printf("PASS: Round-trip is byte-identical!\n");

        return 0;
    }
    catch (const std::exception& e) {
        printf("ERROR: %s\n", e.what());

        return 1;
    }
}

