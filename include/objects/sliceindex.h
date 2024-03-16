#ifndef KIWI_OBJECTS_SLICEINDEX_H
#define KIWI_OBJECTS_SLICEINDEX_H

#include "typing/value.h"

struct SliceIndex {
  Value indexOrStart = static_cast<k_int>(0);
  Value stopIndex = static_cast<k_int>(0);
  Value stepValue = static_cast<k_int>(0);
  bool isSlice = false;
};

#endif