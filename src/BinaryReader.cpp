#include "BinaryReader.hpp"

#include <bit>
#include <cstring>

namespace ac1 {

BinaryReader::BinaryReader(std::span<const std::byte> data)
    : data_(data)
{
}

void BinaryReader::require(std::size_t count) const
{
    if (position_ + count > data_.size()) {
        throw std::out_of_range("BinaryReader: unexpected end of data");
    }
}

uint8_t BinaryReader::u8()
{
    require(1);

    return std::to_integer<uint8_t>(data_[position_++]);
}

uint16_t BinaryReader::u16be()
{
    require(2);

    uint16_t value =
        (static_cast<uint16_t>(std::to_integer<uint8_t>(data_[position_])) << 8) |
        (static_cast<uint16_t>(std::to_integer<uint8_t>(data_[position_ + 1])));

    position_ += 2;
    return value;
}

uint32_t BinaryReader::u32be()
{
    require(4);

    uint32_t value =
        (static_cast<uint32_t>(std::to_integer<uint8_t>(data_[position_])) << 24) |
        (static_cast<uint32_t>(std::to_integer<uint8_t>(data_[position_ + 1])) << 16) |
        (static_cast<uint32_t>(std::to_integer<uint8_t>(data_[position_ + 2])) << 8) |
        (static_cast<uint32_t>(std::to_integer<uint8_t>(data_[position_ + 3])));

    position_ += 4;
    return value;
}

uint64_t BinaryReader::u64be()
{
    require(8);

    uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value =
            (value << 8) |
            std::to_integer<uint8_t>(data_[position_ + i]);
    }

    position_ += 8;
    return value;
}

int8_t BinaryReader::i8()
{
    return static_cast<int8_t>(u8());
}

int16_t BinaryReader::i16be()
{
    return static_cast<int16_t>(u16be());
}

int32_t BinaryReader::i32be()
{
    return static_cast<int32_t>(u32be());
}

int64_t BinaryReader::i64be()
{
    return static_cast<int64_t>(u64be());
}

float BinaryReader::f32be()
{
    uint32_t bits = u32be();
    return std::bit_cast<float>(bits);
}

double BinaryReader::f64be()
{
    uint64_t bits = u64be();
    return std::bit_cast<double>(bits);
}

std::span<const std::byte> BinaryReader::bytes(std::size_t count)
{
    require(count);

    auto result = data_.subspan(position_, count);
    position_ += count;

    return result;
}

void BinaryReader::skip(std::size_t count)
{
    require(count);
    position_ += count;
}

std::size_t BinaryReader::position() const
{
    return position_;
}

std::size_t BinaryReader::remaining() const
{
    return data_.size() - position_;
}

}