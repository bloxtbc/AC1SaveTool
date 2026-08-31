#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "ac1/io/SaveReader.hpp"
#include "ac1/io/SaveWriter.hpp"
#include "ac1/io/SaveJsonExporter.hpp"
#include "ac1/io/SaveJsonImporter.hpp"
#include "ac1/HashDatabase.hpp"

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

        printf("AC1 Save Tool - JSON Round-Trip Test\n");

        // --------------------------------------------------
        // Read original save
        // --------------------------------------------------

        printf("Reading\n");
        printf("  Input:  \"%s\"\n", inputPath.string().c_str());

        const auto original = readFile(inputPath);

        printf("  Size:   %zu bytes\n", original.size());

        // --------------------------------------------------
        // Binary → SaveGame
        // --------------------------------------------------

        ac1::SaveReader reader;

        const auto save =
            reader.read(original);

        printf("  Magic:  0x%x\n", save.magic);
        printf("  Version: %d\n", save.version);
        printf("  Objects: %d\n", save.objectCount);
        printf("\n");

        // --------------------------------------------------
        // SaveGame → JSON
        // --------------------------------------------------

        printf("Exporting JSON\n");

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

        printf("  JSON:    %zu bytes\n", json.size());
        printf("  Output:  \"%s\"\n\n", jsonPath.string().c_str());

        // --------------------------------------------------
        // JSON → SaveGame
        // --------------------------------------------------

        printf("Importing JSON\n");

        const auto imported =
            ac1::SaveJsonImporter::importSave(json);

        printf("  Magic:   0x%x\n", imported.magic);
        printf("  Version: %d\n", imported.version);
        printf("  Objects: %d\n", imported.objectCount);
        printf("\n");
        

        // --------------------------------------------------
        // SaveGame → Binary
        // --------------------------------------------------

        printf("Writing binary save\n");

        ac1::SaveWriter writer;

        const auto rebuilt =
            writer.write(imported);

        writeFile(
            outputPath,
            rebuilt
        );

        printf("  Size:   %zu bytes\n", rebuilt.size());
        printf("  Output: \"%s\"\n\n", outputPath.string().c_str());

        // --------------------------------------------------
        // Compare
        // --------------------------------------------------

        printf("Comparing files\n");

        compareFiles(
            original,
            rebuilt
        );

        printf("\nPASS: JSON round-trip is byte-identical!\n");

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