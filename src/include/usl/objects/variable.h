#ifndef VARIABLE_H
#define VARIABLE_H

#include <variant>
#include <limits>
#include <string>

enum class VariableType {
    None,
    Integer,
    Double,
    String,
    Boolean
};

class Variable : public Collectable
{
private:
    std::variant<int, double, std::string, bool> value;
    std::string variableName;
    VariableType variableType;

    bool collectable;
    bool _isPrivate;
    bool _isPublic;
    bool isIndestructible;
    bool _isBad;

    void setDefaults()
    {
        value = std::numeric_limits<double>::quiet_NaN();
        variableName = "";
        variableType = VariableType::None;
        collectable = false;
        _isPrivate = false;
        _isPublic = false;
        isIndestructible = false;
        _isBad = false;
    }

public:
    Variable() : Collectable()
    {
        setDefaults();
    }

    Variable(std::string name) : Collectable() {
        initialize(name);
        setDefaults();
    }

    Variable(std::string name, const std::string& v) : Collectable() {
        initialize(name);

        if (v == "null")
        {
            setDefaults();
            variableType = VariableType::None;
        }
        else
            setVariable(v);
    }

    Variable(std::string name, double v) : Collectable() {
        initialize(name);
        setVariable(v);
    }

    Variable(std::string name, int v) : Collectable() {
        initialize(name);
        setVariable(v);
    }

    Variable(std::string name, bool v) : Collectable() {
        initialize(name);
        setVariable(v);
    }

    ~Variable() {}

    void setCollectable(bool value) override
    {
        collectable = value;
    }

    bool isCollectable() const override
    {
        return collectable;
    }

    void setIsBad(bool value)
    {
        _isBad = value;
    }

    bool isBad() const
    {
        return _isBad;
    }

    void clear()
    {
        setDefaults();
    }

    void setNull()
    {
        setDefaults();
    }

    void setName(const std::string &name)
    {
        variableName = name;
    }

    VariableType getType() const {
        return variableType;
    }

    void setVariable(const std::string& v) {
        value = v;
        variableType = VariableType::String;
    }

    void setVariable(double v) {
        value = v;
        variableType = VariableType::Double;
    }

    void setVariable(int v) {
        value = v;
        variableType = VariableType::Integer;
    }

    void setVariable(bool v) {
        value = v;
        variableType = VariableType::Boolean;
    }

    void setPrivate()
    {
        _isPrivate = true;
        _isPublic = false;
    }

    void setPublic()
    {
        _isPublic = true;
        _isPrivate = false;
    }

    bool isPublic() const
    {
        return _isPublic;
    }

    bool isPrivate() const
    {
        return _isPrivate;
    }

    double getInteger() const
    {
        return std::get<int>(value);
    }

    double getNumber() const
    {
        return std::get<double>(value);
    }

    std::string getString() const
    {
        return std::get<std::string>(value);
    }

    void initialize(const std::string &name)
    {
        setDefaults();
        variableName = name;
    }

    void setIndestructible(bool value)
    {
        isIndestructible = value;
    }

    bool indestructible() const
    {
        return isIndestructible;
    }

    std::string name() const
    {
        return variableName;
    }
};

#endif