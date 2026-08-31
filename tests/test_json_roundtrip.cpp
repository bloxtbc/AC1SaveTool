#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "io/SaveReader.hpp"
#include "io/SaveWriter.hpp"
#include "io/SaveJsonExporter.hpp"
#include "io/SaveJsonImporter.hpp"
#include "HashDatabase.hpp"

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

    if (size < 0) {
        throw std::runtime_error(
            "Failed to determine file size: " + path.string()
        );
    }

    file.seekg(0, std::ios::beg);

    std::vector<std::byte> data(
        static_cast<std::size_t>(size)
    );

    if (!data.empty()) {
        file.read(
            reinterpret_cast<char*>(data.data()),
            static_cast<std::streamsize>(data.size())
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to read: " + path.string()
            );
        }
    }

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

    if (!data.empty()) {
        file.write(
            reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size())
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to write: " + path.string()
            );
        }
    }
}

static std::string hexOffset(std::size_t offset)
{
    std::ostringstream stream;

    stream
        << "0x"
        << std::uppercase
        << std::hex
        << offset;

    return stream.str();
}

static void compareFiles(
    std::span<const std::byte> original,
    std::span<const std::byte> rebuilt
)
{
    if (original.size() != rebuilt.size()) {
        throw std::runtime_error(
            "File sizes differ: original=" +
            std::to_string(original.size()) +
            ", rebuilt=" +
            std::to_string(rebuilt.size())
        );
    }

    for (std::size_t i = 0; i < original.size(); ++i) {

        if (original[i] != rebuilt[i]) {

            const auto originalByte =
                std::to_integer<unsigned>(original[i]);

            const auto rebuiltByte =
                std::to_integer<unsigned>(rebuilt[i]);

            std::ostringstream message;

            message
                << "Files differ at offset "
                << hexOffset(i)
                << ": original=0x"
                << std::uppercase
                << std::hex
                << originalByte
                << ", rebuilt=0x"
                << rebuiltByte;

            throw std::runtime_error(message.str());
        }
    }
}

int main()
{
    try {
        const fs::path inputPath = "testSave.sav";
        const fs::path jsonPath = "testSave.json";
        const fs::path outputPath = "testSave_roundtrip.sav";

        std::cout << "JSON round-trip test\n\n";

        // --------------------------------------------------
        // Read original save
        // --------------------------------------------------

        std::cout
            << "Reading\n"
            << "  Input:  \"" << inputPath.string() << "\"\n";

        const auto original = readFile(inputPath);

        std::cout
            << "  Size:   " << original.size() << " bytes\n";

        // --------------------------------------------------
        // Binary → SaveGame
        // --------------------------------------------------

        ac1::SaveReader reader;

        const auto save =
            reader.read(original);

        std::cout
            << "  Magic:  0x"
            << std::hex
            << std::uppercase
            << save.magic
            << std::dec
            << "\n"
            << "  Version: " << save.version << "\n"
            << "  Objects: " << save.objectCount << "\n\n";

        // --------------------------------------------------
        // SaveGame → JSON
        // --------------------------------------------------

        std::cout << "Exporting JSON\n";

        ac1::HashDatabase hashes;
        hashes.load("hashes.json");

        const std::string json =
            ac1::SaveJsonExporter::exportSave(
                save,
                &hashes,
                2
            );

        writeFile(
            jsonPath,
            std::span<const std::byte>(
                reinterpret_cast<const std::byte*>(json.data()),
                json.size()
            )
        );

        std::cout
            << "  JSON:    " << json.size() << " bytes\n"
            << "  Output:  \"" << jsonPath.string() << "\"\n\n";

        // --------------------------------------------------
        // JSON → SaveGame
        // --------------------------------------------------

        std::cout << "Importing JSON\n";

        const auto imported =
            ac1::SaveJsonImporter::importSave(json);

        std::cout
            << "  Magic:   0x"
            << std::hex
            << std::uppercase
            << imported.magic
            << std::dec
            << "\n"
            << "  Version: " << imported.version << "\n"
            << "  Objects: " << imported.objectCount << "\n\n";

        // --------------------------------------------------
        // SaveGame → Binary
        // --------------------------------------------------

        std::cout << "Writing binary save\n";

        ac1::SaveWriter writer;

        const auto rebuilt =
            writer.write(imported);

        writeFile(
            outputPath,
            rebuilt
        );

        std::cout
            << "  Size:   " << rebuilt.size() << " bytes\n"
            << "  Output: \"" << outputPath.string() << "\"\n\n";

        // --------------------------------------------------
        // Compare
        // --------------------------------------------------

        std::cout << "Comparing files\n";

        compareFiles(
            original,
            rebuilt
        );

        std::cout
            << "\nPASS: JSON round-trip is byte-identical!\n";

        return 0;
    }
    catch (const std::exception& e) {

        std::cerr
            << "\nFAIL: "
            << e.what()
            << '\n';

        return 1;
    }
}