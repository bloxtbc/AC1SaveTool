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

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

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

#ifdef _WIN32

static std::string openFileDialog(
    const char* filter,
    const char* title
)
{
    char filename[MAX_PATH] = {};

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = nullptr;
    dialog.lpstrFile = filename;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrFilter = filter;
    dialog.nFilterIndex = 1;
    dialog.lpstrTitle = title;
    dialog.Flags =
        OFN_PATHMUSTEXIST |
        OFN_FILEMUSTEXIST |
        OFN_NOCHANGEDIR;

    if (!GetOpenFileNameA(&dialog)) {
        return {};
    }

    return filename;
}

static std::string saveFileDialog(
    const char* filter,
    const char* title,
    const char* defaultExtension
)
{
    char filename[MAX_PATH] = {};

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = nullptr;
    dialog.lpstrFile = filename;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrFilter = filter;
    dialog.nFilterIndex = 1;
    dialog.lpstrTitle = title;
    dialog.lpstrDefExt = defaultExtension;
    dialog.Flags =
        OFN_PATHMUSTEXIST |
        OFN_OVERWRITEPROMPT |
        OFN_NOCHANGEDIR;

    if (!GetSaveFileNameA(&dialog)) {
        return {};
    }

    return filename;
}

#endif


static int interactiveMode()
{
    std::cout
        << "AC1 Save Tool\n"
        << "=============\n\n"
        << "What do you want to do?\n"
        << "  1. Export save -> JSON\n"
        << "  2. Import JSON -> save\n"
        << "\n"
        << "Select [1-2]: ";

    std::string choice;
    std::getline(std::cin, choice);

    if (choice != "1" && choice != "2") {
        std::cerr << "\nInvalid selection.\n";
        return 1;
    }

#ifndef _WIN32
    std::cerr << "\nInteractive file selection is only supported on Windows.\n";
    return 1;
#else

    const bool exportMode = choice == "1";

    const char* inputFilter;
    const char* outputFilter;
    const char* inputTitle;
    const char* outputTitle;
    const char* outputExtension;

    if (exportMode) {
        inputFilter =
            "AC1 Save Files (*.sav;*.opt)\0*.sav;*.opt\0"
            "Save Files (*.sav)\0*.sav\0"
            "Option Files (*.opt)\0*.opt\0"
            "All Files (*.*)\0*.*\0";

        outputFilter =
            "JSON Files (*.json)\0*.json\0"
            "All Files (*.*)\0*.*\0";

        inputTitle = "Select AC1 Save or Option File";
        outputTitle = "Save JSON File";
        outputExtension = "json";
    }
    else {
        inputFilter =
            "JSON Files (*.json)\0*.json\0"
            "All Files (*.*)\0*.*\0";

        outputFilter =
            "AC1 Save Files (*.sav;*.opt)\0*.sav;*.opt\0"
            "Save Files (*.sav)\0*.sav\0"
            "Option Files (*.opt)\0*.opt\0"
            "All Files (*.*)\0*.*\0";

        inputTitle = "Select JSON File";
        outputTitle = "Save AC1 Save or Option File";
        outputExtension = "sav";
    }

    std::cout << "\nSelect input file...\n";

    const std::string input =
        openFileDialog(inputFilter, inputTitle);

    if (input.empty()) {
        std::cout << "Cancelled.\n";
        return 0;
    }

    std::cout
        << "Input: "
        << input
        << "\n\n";

    std::cout << "Select output file...\n";

    const std::string output =
        saveFileDialog(
            outputFilter,
            outputTitle,
            outputExtension
        );

    if (output.empty()) {
        std::cout << "Cancelled.\n";
        return 0;
    }

    std::cout
        << "Output: "
        << output
        << "\n\n";

    if (exportMode) {
        return exportSave(input, output);
    }

    return importSave(input, output);

#endif
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try {

        if (argc == 1) {
            return interactiveMode();
        }

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

