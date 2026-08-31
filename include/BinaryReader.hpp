#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>

namespace ac1 {

class BinaryReader {
public:
    explicit BinaryReader(std::span<const std::byte> data);

    uint8_t  u8();
    uint16_t u16be();
    uint32_t u32be();
    uint64_t u64be();

    int8_t   i8();
    int16_t  i16be();
    int32_t  i32be();
    int64_t  i64be();

    float  f32be();
    double f64be();

    std::span<const std::byte> bytes(std::size_t count);

    void skip(std::size_t count);

    std::size_t position() const;
    std::size_t remaining() const;

private:
    void require(std::size_t count) const;

    std::span<const std::byte> data_;
    std::size_t position_ = 0;
};

}