#pragma once
#include <cstdint>
#include "mt/mt.h"

// Tries to run MtObject->createProperty with multiple layers of "safety".
// Returns true on success.
bool try_dti_create_property_threaded(Mt::MtObject* object, Mt::MtPropertyList* property_list, uint32_t timeoutMs = 5000);
