#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ac1 {

class BinaryWriter {
public:
    void u8(uint8_t value);
    void u16be(uint16_t value);
    void u32be(uint32_t value);
    void u64be(uint64_t value);

    void i8(int8_t value);
    void i16be(int16_t value);
    void i32be(int32_t value);
    void i64be(int64_t value);

    void f32be(float value);
    void f64be(double value);

    void bytes(const std::byte* data, std::size_t size);

    const std::vector<std::byte>& data() const;
    std::vector<std::byte> take();

    std::size_t position() const;

private:
    std::vector<std::byte> data_;
};

} // namespace ac1
