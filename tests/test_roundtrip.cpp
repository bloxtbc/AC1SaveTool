#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <vector>

#include "io/SaveReader.hpp"
#include "io/SaveWriter.hpp"

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

        std::cout << "Reading: " << input << '\n';

        auto original = readFile(input);

        std::cout
            << "Original size: "
            << original.size()
            << " bytes\n";

        ac1::SaveReader reader;

        auto save = reader.read(original);

        std::cout
            << "Magic: 0x"
            << std::hex
            << save.magic
            << std::dec
            << '\n';

        std::cout
            << "Version: "
            << save.version
            << '\n';

        std::cout
            << "Objects: "
            << save.objectCount
            << '\n';


        ac1::SaveWriter writer;

        auto rebuilt = writer.write(save);

        std::cout
            << "Rebuilt size: "
            << rebuilt.size()
            << " bytes\n";

        writeFile(output, rebuilt);

        if (original.size() != rebuilt.size()) {
            std::cerr
                << "FAIL: File sizes differ!\n";

            return 1;
        }

        for (std::size_t i = 0; i < original.size(); ++i) {
            if (original[i] != rebuilt[i]) {
                std::cerr
                    << "FAIL: Files differ at offset 0x"
                    << std::hex
                    << i
                    << std::dec
                    << '\n';

                std::cerr
                    << "Original: 0x"
                    << std::hex
                    << std::to_integer<unsigned>(original[i])
                    << '\n';

                std::cerr
                    << "Rebuilt:  0x"
                    << std::hex
                    << std::to_integer<unsigned>(rebuilt[i])
                    << '\n';

                return 1;
            }
        }

        std::cout
            << "PASS: Round-trip is byte-identical!\n";

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr
            << "ERROR: "
            << e.what()
            << '\n';

        return 1;
    }
}

