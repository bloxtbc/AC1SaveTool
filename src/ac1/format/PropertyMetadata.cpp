#include "ac1/format/PropertyMetadata.hpp"

namespace ac1 {

SerializerFieldType PropertyMetadata::type() const
{
    return static_cast<SerializerFieldType>((identifier >> 16) & 0x3F);
}

} // namespace ac1
