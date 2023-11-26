class Constant
{
private:
    std::string stringValue, constantName;
    double numericValue;
    bool isNumber_, isString_;

public:
    Constant(std::string name, std::string val)
    {
        constantName = name,
        stringValue = val;
        isString_ = true,
        isNumber_ = false;
    }

    Constant(std::string name, double val)
    {
        constantName = name,
        numericValue = val;
        isNumber_ = true,
        isString_ = false;
    }

    bool ConstNumber()
    {
        return isNumber_;
    }

    bool ConstString()
    {
        return isString_;
    }

    std::string getString()
    {
        return stringValue;
    }

    double getNumber()
    {
        return numericValue;
    }

    std::string name()
    {
        return constantName;
    }
};