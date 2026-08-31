#include "format/PropertyMetadata.hpp"

ac1::SerializerFieldType PropertyMetadata::type() const
{
    return static_cast<ac1::SerializerFieldType>(
        (identifier >> 16) & 0x3F
    );
}