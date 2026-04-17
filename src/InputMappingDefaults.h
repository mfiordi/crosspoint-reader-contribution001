#pragma once

#include "CrossPointSettings.h"

namespace InputMappingDefaults {

// Fills inputMapShort/Long from legacy fields (front roles, side layout, chapter skip, short power).
void applyStockInputMapping(CrossPointSettings& s);

// Milliseconds to treat a hold as "long" for a given hardware index (0..6).
unsigned long longThresholdMsForHw(const CrossPointSettings& s, uint8_t hwIndex);

}  // namespace InputMappingDefaults
