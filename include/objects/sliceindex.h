#ifndef KIWI_OBJECTS_SLICEINDEX_H
#define KIWI_OBJECTS_SLICEINDEX_H

#include "typing/valuetype.h"

struct SliceIndex {
  Value indexOrStart = 0;
  Value stopIndex = 0;
  Value stepValue = 0;
  bool isSlice = false;
};

#endif