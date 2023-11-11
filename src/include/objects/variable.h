class Variable : public Collectable
{
private:
    double  numericValue;
    string  stringValue,
            variableName;

    bool    collectable,
            isPrivate_,
            isPublic_,
            isIndestructible,
            waitToAssign;

    void setAll(double numValue, string strValue)
    {
        setVariable(numValue);
        setVariable(strValue);
        collectable = false;
    }

public:
    Variable()
    {
        setAll(-DBL_MAX, "[null]");
    }

    Variable(string name)
    {
        initialize(name);
        setAll(-DBL_MAX, "[null]");
    }

    Variable(string name, string value)
    {
        initialize(name);

        if (value == "null")
        {
            setAll(-DBL_MAX, "[null]");
            waitToAssign = true;
        }
        else
            setAll(-DBL_MAX, value);
    }

    Variable(string name, double value)
    {
        initialize(name);
        setAll(value, "[null]");
    }

    ~Variable() { }

    void setCollectable(bool value) override
    {
        collectable = value;
    }

    bool isCollectable() const override
    {
        return collectable;
    }

    void clear()
    {
        setAll(0, "");
    }

    void setNull()
    {
        setAll(-DBL_MAX, "[null]");
        waitToAssign = true;
    }

    void setName(string name)
    {
        variableName = name;
    }

    bool waiting()
    {
        return waitToAssign;
    }

    void stopWait()
    {
        waitToAssign = false;
    }

    void setVariable(double value)
    {
        if (waiting())
        {
            numericValue = value;
            stringValue = "[null]";
            waitToAssign = false;
        }
        else
        {
            numericValue = 0.0;
            numericValue = value;
        }
    }

    void setVariable(string value)
    {
        if (waiting())
        {
            stringValue = value;
            numericValue = -DBL_MAX;
            waitToAssign = false;
        }
        else
            stringValue = value;
    }


    void setPrivate()
    {
        isPrivate_ = true;
        isPublic_ = false;
    }

    void setPublic()
    {
        isPublic_ = true;
        isPrivate_ = false;
    }

    bool isPublic()
    {
        return isPublic_;
    }

    bool isPrivate()
    {
        return isPrivate_;
    }

    double getNumber()
    {
        return numericValue;
    }

    string getString()
    {
        return stringValue;
    }

    void initialize(string name)
    {
        variableName = name;
        collectable = false,
        isIndestructible = false,
        waitToAssign = false;
    }

    void setIndestructible()
    {
        isIndestructible = true;
    }

    void setDestructible()
    {
        isIndestructible = false;
    }

    bool indestructible()
    {
        return isIndestructible;
    }

    bool isNullString()
    {
        if (getString() == "[null]" && getNumber() == -DBL_MAX)
            return true;

        return false;
    }
    bool isNull()
    {
        if (getNumber() == -DBL_MAX && getString() == "[null]")
            return true;

        return false;
    }

    string name()
    {
        return variableName;
    }
};