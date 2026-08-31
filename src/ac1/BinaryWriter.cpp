#include "ac1/BinaryWriter.hpp"

#include <bit>
#include <stdexcept>
#include <utility>

namespace ac1 {

void BinaryWriter::u8(uint8_t value) {
    data_.push_back(static_cast<std::byte>(value));
}

void BinaryWriter::u16be(uint16_t value) {
    data_.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
    data_.push_back(static_cast<std::byte>(value & 0xFF));
}

void BinaryWriter::u32be(uint32_t value) {
    data_.push_back(static_cast<std::byte>((value >> 24) & 0xFF));
    data_.push_back(static_cast<std::byte>((value >> 16) & 0xFF));
    data_.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
    data_.push_back(static_cast<std::byte>(value & 0xFF));
}

void BinaryWriter::u64be(uint64_t value) {
    for (int i = 7; i >= 0; --i) {
        data_.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xFF));
    }
}

void BinaryWriter::i8(int8_t value) {
    u8(static_cast<uint8_t>(value));
}

void BinaryWriter::i16be(int16_t value) {
    u16be(static_cast<uint16_t>(value));
}

void BinaryWriter::i32be(int32_t value) {
    u32be(static_cast<uint32_t>(value));
}

void BinaryWriter::i64be(int64_t value) {
    u64be(static_cast<uint64_t>(value));
}

void BinaryWriter::f32be(float value) {
    u32be(std::bit_cast<uint32_t>(value));
}

void BinaryWriter::f64be(double value) {
    u64be(std::bit_cast<uint64_t>(value));
}

void BinaryWriter::bytes(const std::byte* data, std::size_t size) {
    if (size == 0) {
        return;
    }

    if (data == nullptr) {
        throw std::invalid_argument("BinaryWriter::bytes: null data pointer");
    }

    data_.insert(data_.end(), data, data + size);
}

const std::vector<std::byte>& BinaryWriter::data() const {
    return data_;
}

std::vector<std::byte> BinaryWriter::take() {
    return std::move(data_);
}

std::size_t BinaryWriter::position() const {
    return data_.size();
}

} // namespace ac1
