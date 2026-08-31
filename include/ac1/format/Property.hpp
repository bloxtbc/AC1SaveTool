#pragma once

#include <cstdint>
#include <variant>

namespace ac1 {

struct PropertyType01 { uint32_t value; };
struct PropertyType02 { uint32_t refId; uint16_t refIndex; uint32_t flags; };
struct PropertyType03 { uint32_t unk1; uint16_t unk2; uint32_t refId; uint16_t refIndex; uint32_t flags; };
struct PropertyType04 { uint32_t a; uint32_t b; uint32_t c; uint32_t d; uint16_t e; uint32_t f; };
struct PropertyType05 { uint32_t a; uint32_t b; uint32_t c; uint32_t d; uint32_t e; uint32_t f; uint32_t g; };
struct PropertyType06 { uint32_t a; uint32_t b; uint32_t c; uint32_t d; uint32_t e; uint32_t f; uint32_t g; uint16_t h; uint32_t i; };

using PropertyPayload = std::variant<
    PropertyType01,
    PropertyType02,
    PropertyType03,
    PropertyType04,
    PropertyType05,
    PropertyType06
>;

struct Property {
    uint8_t propType;
    uint32_t id;
    uint16_t index;
    PropertyPayload payload;
};

} // namespace ac1
