#ifndef ASTRAL_OBJECTS_SLICEINDEX_H
#define ASTRAL_OBJECTS_SLICEINDEX_H

#include "typing/value.h"

struct SliceIndex {
  k_value indexOrStart = static_cast<k_int>(0);
  k_value stopIndex = static_cast<k_int>(0);
  k_value stepValue = static_cast<k_int>(0);
  bool isSlice = false;
};

#endif