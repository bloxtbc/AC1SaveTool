cmake_minimum_required(VERSION 3.20)

project(AC1SaveTool
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------------
# Library
# --------------------------------------------------------------------------

add_library(ac1save_lib
    src/ac1/BinaryReader.cpp
    src/ac1/BinaryWriter.cpp
    src/ac1/HashDatabase.cpp
    src/ac1/format/SerializerFieldType.cpp
    src/ac1/format/PropertyMetadata.cpp
    src/ac1/io/SaveReader.cpp
    src/ac1/io/SaveWriter.cpp
    src/ac1/io/SaveJsonExporter.cpp
    src/ac1/io/SaveJsonImporter.cpp
)

target_include_directories(ac1save_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/external/json/include
)

target_compile_features(ac1save_lib PUBLIC cxx_std_20)


add_executable(ac1save
    src/main.cpp
)

target_link_libraries(ac1save
    PRIVATE
        ac1save_lib
)

if(MINGW)
    target_link_options(ac1save PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()


add_custom_command(TARGET ac1save POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/hashes/hashes.json"
        "$<TARGET_FILE_DIR:ac1save>/hashes.json"
)


# --------------------------------------------------------------------------
# Warnings
# --------------------------------------------------------------------------

if (MSVC)
    target_compile_options(ac1save_lib PRIVATE
        /W4
    )
else()
    target_compile_options(ac1save_lib PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# --------------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------------

enable_testing()

add_executable(ac1save_test
    tests/test_roundtrip.cpp
)

target_link_libraries(ac1save_test
    PRIVATE
        ac1save_lib
)

# JSON roundtrip test

add_custom_command(TARGET ac1save_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/tests/assets/testSave.sav"
        "$<TARGET_FILE_DIR:ac1save_test>/testSave.sav"
)

add_test(
    NAME RoundTrip
    COMMAND ac1save_test
)

set_tests_properties(RoundTrip PROPERTIES
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:ac1save_test>"
)

add_executable(ac1save_json_roundtrip
    tests/test_json_roundtrip.cpp
)

target_link_libraries(ac1save_json_roundtrip
    PRIVATE
        ac1save_lib
)

add_test(
    NAME JsonRoundTrip
    COMMAND ac1save_json_roundtrip
)

set_tests_properties(JsonRoundTrip PROPERTIES
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:ac1save_json_roundtrip>"
)

add_custom_command(
    TARGET ac1save_json_roundtrip
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/tests/assets/testSave.sav"
        "$<TARGET_FILE_DIR:ac1save_json_roundtrip>/testSave.sav"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/hashes/hashes.json"
        "$<TARGET_FILE_DIR:ac1save_json_roundtrip>/hashes.json"
)