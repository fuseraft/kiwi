class Constant
{
private:
    string stringValue, constantName;
    double numericValue;
    bool isNumber_, isString_;

public:
    Constant(string name, string val)
    {
        constantName = name,
        stringValue = val;
        isString_ = true,
        isNumber_ = false;
    }

    Constant(string name, double val)
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

    string getString()
    {
        return stringValue;
    }

    double getNumber()
    {
        return numericValue;
    }

    string name()
    {
        return constantName;
    }
};