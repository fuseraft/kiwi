#ifndef KIWI_OBJECTS_OBJECT_H
#define KIWI_OBJECTS_OBJECT_H

#include <map>
#include <string>
#include "class.h"
#include "typing/valuetype.h"

struct Object {
  std::string name;
  Class clazz;
  std::map<std::string, Value> instanceVariables;
};

#endif