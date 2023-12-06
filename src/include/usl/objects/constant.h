#include <variant>
#include "valuetype.h"

class Constant {
  private:
    std::string constantName;
    std::variant<int, double, std::string, bool> value;
    ValueType valueType;

  public:
    Constant(std::string name, std::string val) {
        constantName = name;
        value = val;
        valueType = ValueType::String;
    }

    Constant(std::string name, double val) {
        constantName = name;
        value = val;
        valueType = ValueType::Double;
    }

    ValueType getType() const { return valueType; }

    double getInteger() const { return std::get<int>(value); }

    double getNumber() const { return std::get<double>(value); }

    std::string getString() const { return std::get<std::string>(value); }

    std::string name() { return constantName; }
};