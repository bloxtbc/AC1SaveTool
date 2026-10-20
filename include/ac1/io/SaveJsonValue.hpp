#pragma once

#include <nlohmann/json.hpp>

#include "ac1/format/PropertyValue.hpp"
#include "ac1/format/SerializerFieldType.hpp"

namespace ac1 {

PropertyValueData propertyValueDataForType(
    SerializerFieldType type,
    const nlohmann::json& valueJson
);

}