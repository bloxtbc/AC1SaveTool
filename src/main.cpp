#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "ac1/HashDatabase.hpp"
#include "ac1/format/SaveGame.hpp"
#include "ac1/io/SaveReader.hpp"
#include "ac1/io/SaveWriter.hpp"
#include "ac1/io/SaveJsonExporter.hpp"
#include "ac1/io/SaveJsonImporter.hpp"

namespace fs = std::filesystem;

// -----------------------------------------------------------------------------
// File I/O
// -----------------------------------------------------------------------------

static std::vector<std::byte> readBinaryFile(const fs::path& path)
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

static std::string readTextFile(const fs::path& path)
{
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error(
            "Failed to open: " + path.string()
        );
    }

    return {
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };
}

static void writeBinaryFile(
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

static void writeTextFile(
    const fs::path& path,
    const std::string& data
)
{
    std::ofstream file(path);

    if (!file) {
        throw std::runtime_error(
            "Failed to create: " + path.string()
        );
    }

    file << data;

    if (!file) {
        throw std::runtime_error(
            "Failed to write: " + path.string()
        );
    }
}

// -----------------------------------------------------------------------------
// CLI
// -----------------------------------------------------------------------------

static void printUsage(const char* program)
{
    std::cout
        << "AC1 Save Tool\n"
        << "\n"
        << "Usage:\n"
        << "  " << program << " export <input.sav> <output.json>\n"
        << "  " << program << " import <input.json> <output.sav>\n"
        << "\n"
        << "Examples:\n"
        << "  " << program << " export testSave.sav testSave.json\n"
        << "  " << program << " import testSave.json testSave.sav\n";
}

static void printSaveInfo(const ac1::SaveGame& save)
{
    std::cout
        << "  Magic:   0x"
        << std::hex
        << std::uppercase
        << save.magic
        << std::dec
        << "\n"
        << "  Version: "
        << save.version
        << "\n"
        << "  Objects: "
        << save.objectCount
        << "\n";
}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------

static int exportSave(
    const fs::path& inputPath,
    const fs::path& outputPath
)
{
    std::cout << "Exporting save\n";
    std::cout << "  Input:  " << inputPath << "\n";
    std::cout << "  Output: " << outputPath << "\n";

    const auto data = readBinaryFile(inputPath);

    std::cout
        << "  Size:   "
        << data.size()
        << " bytes\n";

    ac1::SaveReader reader;
    const auto save = reader.read(data);

    printSaveInfo(save);

    ac1::HashDatabase hashes;
    hashes.load("hashes.json");

    const std::string json =
        ac1::SaveJsonExporter::exportSave(
            save,
            &hashes,
            2
        );

    writeTextFile(outputPath, json);

    std::cout
        << "  JSON:   "
        << json.size()
        << " bytes\n"
        << "\n"
        << "Export complete.\n";

    return 0;
}

static int importSave(
    const fs::path& inputPath,
    const fs::path& outputPath
)
{
    std::cout << "Importing save\n";
    std::cout << "  Input:  " << inputPath << "\n";
    std::cout << "  Output: " << outputPath << "\n";

    const std::string json =
        readTextFile(inputPath);

    std::cout
        << "  JSON:   "
        << json.size()
        << " bytes\n";

    const auto save =
        ac1::SaveJsonImporter::importSave(json);

    printSaveInfo(save);

    ac1::SaveWriter writer;

    const auto data =
        writer.write(save);

    writeBinaryFile(
        outputPath,
        std::span<const std::byte>(
            data.data(),
            data.size()
        )
    );

    std::cout
        << "  Size:   "
        << data.size()
        << " bytes\n"
        << "\n"
        << "Import complete.\n";

    return 0;
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try {
        if (argc != 4) {
            printUsage(argv[0]);
            return 1;
        }

        const std::string command = argv[1];

        const fs::path inputPath = argv[2];
        const fs::path outputPath = argv[3];

        if (command == "export") {
            return exportSave(
                inputPath,
                outputPath
            );
        }

        if (command == "import") {
            return importSave(
                inputPath,
                outputPath
            );
        }

        std::cerr
            << "Unknown command: "
            << command
            << "\n\n";

        printUsage(argv[0]);

        return 1;
    }
    catch (const std::exception& e) {
        std::cerr
            << "\nError: "
            << e.what()
            << "\n";

        return 1;
    }
}

