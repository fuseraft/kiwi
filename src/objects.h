/**
 * Project:		Unorthodox Scripting Language (USL)
 * Version:		0.0.0.1
 *
 * Filename:	objects.h
 *
 * Author:		Scott Christopher Stauffer
 * Email:		scstauf@gmail.com
 * Date:		Monday, August 23, 2013
 *
 * Copyright 2013
 */

class StringContainer;
class Method;
class Container;
class Object;
class Variable;
class List;
class Crypt;
class Constant;
class Module;
class Script;
class Switch;

class Constant {
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

    bool isNumber()
    {
        return (isNumber_);
    }

    bool isString()
    {
        return (isString_);
    }

    string getString()
    {
        return (stringValue);
    }

    double getNumber()
    {
        return (numericValue);
    }

    string name()
    {
        return (constantName);
    }
};

class Script {
private:
    vector<string> lines;
    vector<string> marks;
    string scriptName;

public:
    Script() {}

    Script(string name)
    {
        scriptName = name;
    }

    ~Script()
    {
        clear();
    }

    void clear()
    {
        lines.clear();
        marks.clear();
    }

    void add(string line)
    {
        lines.push_back(line);
    }

    void addMark(string mark)
    {
        marks.push_back(mark);
    }

    vector<string> get()
    {
        return (lines);
    }

    string at(int index)
    {
        if (index < (int)lines.size())
            return (lines.at(index));

        return ("[no_line]");
    }

    string markAt(int index)
    {
        if (index < (int)marks.size())
            return (marks.at(index));

        return ("[no_line]");
    }

    bool markExists(string mark)
    {
        for (int i = 0; i < (int)marks.size(); i++) {
            if (marks.at(i) == mark)
                return (true);
        }

        return (false);
    }

    int markSize()
    {
        return ((int)marks.size());
    }

    int size()
    {
        return (get().size());
    }

    string name()
    {
        return (scriptName);
    }
};

class Module {
private:
    vector<string>  lines;
    string          moduleName;

public:
    Module() {}

    Module(string name)
    {
        moduleName = name;
    }

    ~Module()
    {
        clear();
    }

    void clear()
    {
        lines.clear();
    }

    void add(string line)
    {
        lines.push_back(line);
    }

    vector<string> get()
    {
        return (lines);
    }

    string at(int index)
    {
        if (index < (int)lines.size())
            return (lines.at(index));

        return ("[no_line]");
    }

    int size()
    {
        return (get().size());
    }

    string name()
    {
        return (moduleName);
    }
};

class StringContainer {
private:
    vector<string> strings;

public:
    StringContainer() {}

    ~StringContainer()
    {
        clear();
    }

    void clear()
    {
        strings.clear();
    }

    void add(string line)
    {
        strings.push_back(line);
    }

    vector<string> get()
    {
        return (strings);
    }

    string at(int index)
    {
        if (index < (int)strings.size())
            return (strings.at(index));

        return ("[no_line]");
    }
};

class Crypt {
public:
    Crypt() {}
    ~Crypt() {}

    string d(string o)
    {
        return (decrypt(o));
    }

    string e(string o)
    {
        return (encrypt(o));
    }

private:
    string decrypt(string o)
    {
        int l = o.length(), s = 7;
        string ax("");

        for (int i = 0; i < l; i++) {
            if (s == 7) {
                ax.push_back(((char)((int)o[i] + 3)));
                s = 5;
            } else if (s == 5) {
                ax.push_back(((char)((int)o[i] - 1)));
                s = 0;
            } else if (s == 0) {
                ax.push_back(((char)((int)o[i] + 4)));
                s = 1;
            } else {
                ax.push_back(((char)((int)o[i] - 2)));
                s = 7;
            }
        }

        return (ax);
    }

    string encrypt(string o)
    {
        int l = o.length(), s = 7;
        string ax("");

        for (int i = 0; i < l; i++) {
            if (s == 7) {
                ax.push_back(((char)((int)o[i] - 3)));
                s = 5;
            } else if (s == 5) {
                ax.push_back(((char)((int)o[i] + 1)));
                s = 0;
            } else if (s == 0) {
                ax.push_back(((char)((int)o[i] - 4)));
                s = 1;
            } else {
                ax.push_back(((char)((int)o[i] + 2)));
                s = 7;
            }
        }

        return (ax);
    }
};

class Variable {
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

    void collect()
    {
        collectable = true;
    }

    void dontCollect()
    {
        collectable = false;
    }

    bool garbage()
    {
        return (collectable);
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
        return (waitToAssign);
    }

    void stopWait()
    {
        waitToAssign = false;
    }

    void setVariable(double value)
    {
        if (waiting()) {
            numericValue = value;
            stringValue = "[null]";
            waitToAssign = false;
        } else {
            numericValue = 0.0;
            numericValue = value;
        }
    }

    void setVariable(string value)
    {
        if (waiting()) {
            stringValue = value;
            numericValue = -DBL_MAX;
            waitToAssign = false;
        } else
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
        return (isPublic_);
    }

    bool isPrivate()
    {
        return (isPrivate_);
    }

    double getNumber()
    {
        return (numericValue);
    }

    string getString()
    {
        return (stringValue);
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
        return (isIndestructible);
    }

    bool isNullString()
    {
        if (getString() == "[null]" && getNumber() == -DBL_MAX)
            return (true);

        return (false);
    }
    bool isNull()
    {
        if (getNumber() == -DBL_MAX && getString() == "[null]")
            return (true);

        return (false);
    }

    string name()
    {
        return (variableName);
    }
};

class Container {
private:
    vector<string>  lines;
    string          containerName,
                    value;
    bool            isNestedIF;

public:
    Container() {}

    Container(string name)
    {
        initialize(name);
    }

    ~Container()
    {
        clear();
    }

    void setName(string name)
    {
        containerName = name;
    }

    void add(string line)
    {
        lines.push_back(line);
    }

    void setValue(string val)
    {
        value = val;
    }

    string getCase()
    {
        return (value);
    }

    string at(int index)
    {
        if (index < (int)lines.size())
            return (lines.at(index));

        return ("#!=no_line");
    }

    void clear()
    {
        lines.clear();
    }

    vector<string> getLines()
    {
        return (lines);
    }

    void initialize(string name)
    {
        containerName = name;
        isNestedIF = false;
    }

    string name()
    {
        return (containerName);
    }

    int size()
    {
        return ((int)lines.size());
    }

    bool isIF()
    {
        return (isNestedIF);
    }

    void setBool(bool b)
    {
        isNestedIF = b;
    }

    bool isBad()
    {
        if (startsWith(name(), "[bad_nest"))
            return (true);

        return (false);
    }
};

class Switch {
private:
    vector<Container>   cases;
    Container           defaultCase;
    int                 count;

public:
    Switch()
    {
        count = 0;
    }

    ~Switch()
    {
        cases.clear();
        defaultCase.clear();
    }

    void clear()
    {
        cases.clear();
        defaultCase.clear();
        count = 0;
    }

    Container rightCase(string value)
    {
        for (int i = 0; i < (int)cases.size(); i++) {
            if (cases.at(i).getCase() == value)
                return(cases.at(i));
        }

        return (defaultCase);
    }

    void addCase(string value)
    {
        Container newCase("[case#" + itos(count) + "]");
        newCase.setValue(value);
        cases.push_back(newCase);
        count++;
    }

    void addToCase(string line)
    {
        cases.at(count - 1).add(line);
    }

    void addToDefault(string line)
    {
        defaultCase.add(line);
    }
};

class List {
private:
    vector<string>  contents;
    vector<string>  reversion;
    string          listName;
    bool            collectable;

public:
    List() {}

    List(string name)
    {
        collectable = false;
        listName = name;
    }

    ~List()
    {
        clear();
    }

    void collect()
    {
        collectable = true;
    }

    void dontCollect()
    {
        collectable = false;
    }

    bool garbage()
    {
        return (collectable);
    }

    void setName(string s)
    {
        listName = s;
    }

    void listSort()
    {
        reversion = contents;
        sort(contents.begin(), contents.end());
    }

    void listReverse()
    {
        reversion = contents;
        reverse(contents.begin(), contents.end());
    }

    void listRevert()
    {
        contents = reversion;
    }

    void add(string line)
    {
        contents.push_back(line);
    }

    void remove(string line)
    {
        vector<string> newContents;

        for (int i = 0; i < size(); i++) {
            if (at(i) != line)
                newContents.push_back(at(i));
        }

        clear();

        contents = newContents;
    }

    void clear()
    {
        contents.clear();
    }

    string at(int index)
    {
        if (index < (int)contents.size())
            return (contents.at(index));

        return ("#!=no_line");
    }

    string name()
    {
        return (listName);
    }

    int size()
    {
        return ((int)contents.size());
    }
};

class Method {
private:
    vector<Variable>    methodVariables;
    vector<string>      lines;

    string              logicOperatorValue,
                        methodName,
                        objectName,
                        symbolString,
                        valueOne_,
                        valueTwo_;

    bool                isForLoop_,
                        isIF_,
                        isIndestructible,
                        isInfinite_,
                        isListLoop_,
                        isPrivate_,
                        isPublic_,
                        isTemplate_,
                        isWhileLoop_;

    int                 startValue,
                        stopValue,
                        templateObjects;

    List                list;

    Container           nest;

    char                defaultSymbol;

public:
    Method() {}

    Method(string name)
    {
        initialize(name);
    }

    Method(string name, bool isTemplate)
    {
        initialize(name);
        isTemplate_ = isTemplate;
    }

    ~Method()
    {
        clear();
    }

    void setObject(string name)
    {
        objectName = name;
    }

    void setIndestructible()
    {
        isIndestructible = true;
    }

    bool indestructible()
    {
        return (isIndestructible);
    }

    void setDestructible()
    {
        isIndestructible = false;
    }

    string getObject()
    {
        return (objectName);
    }

    /**
     * symbol is the variable containing the current iteration value.
     */
    void setSymbol(string symbol)
    {
        symbolString = symbol;
    }

    /**
     * symbol is the variable containing the current iteration value.
     */
    void setDefaultSymbol(string symbol)
    {
        defaultSymbol = symbol[0];
    }

    string getSymbolString()
    {
        return (symbolString);
    }

    char getDefaultSymbol()
    {
        return (defaultSymbol);
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
        return (isPublic_);
    }

    bool isPrivate()
    {
        return (isPrivate_);
    }

    bool isTemplate()
    {
        return (isTemplate_);
    }

    vector<Variable> getMethodVariables()
    {
        return (methodVariables);
    }

    void setName(string name)
    {
        methodName = name;
    }

    void add(string line)
    {
        lines.push_back(line);
    }

    void addMethodVariable(string value, string variableName)
    {
        Variable newVariable(variableName, value);
        methodVariables.push_back(newVariable);
    }

    void addMethodVariable(double value, string variableName)
    {
        Variable newVariable(variableName, value);
        methodVariables.push_back(newVariable);
    }

    void addMethodVariable(Variable variable)
    {
        methodVariables.push_back(variable);
    }

    string at(int index)
    {
        if (index < (int)lines.size())
            return (lines.at(index));

        return ("#!=no_line");
    }

    void buildNest()
    {
        Container newNest(methodName + "<nest>");
        nest = newNest;
    }

    Container getNest()
    {
        return (nest);
    }

    void inNest(string line)
    {
        nest.add(line);
    }

    string nestAt(int index)
    {
        if (index < nest.size())
            return (nest.at(index));
        else
            return ("nothing!!!");
    }

    void clear()
    {
        lines.clear();
        methodVariables.clear();
    }

    vector<string> getLines()
    {
        return (lines);
    }

    void initialize(string name)
    {
        defaultSymbol = '$';
        logicOperatorValue = "",
        methodName = name,
        objectName = "",
        symbolString = "$",
        valueOne_ = "",
        valueTwo_ = "";
        isForLoop_ = false,
        isIF_ = false,
        isIndestructible = false,
        isInfinite_ = false,
        isListLoop_ = false,
        isPublic_ = false,
        isPrivate_ = false,
        isTemplate_ = false,
        isWhileLoop_ = false;
        startValue = 0,
        stopValue = 0,
        templateObjects = 0;
    }

    bool isBad()
    {
        if (startsWith(name(), "[bad_meth"))
            return (true);

        return (false);
    }

    string name()
    {
        return (methodName);
    }

    void setTemplateSize(int size)
    {
        templateObjects = size;
    }

    int getTemplateSize()
    {
        return (templateObjects);
    }

    int size()
    {
        return ((int)lines.size());
    }

    bool isIF()
    {
        return (isIF_);
    }

    bool isForLoop()
    {
        return (isForLoop_);
    }

    bool isWhileLoop()
    {
        return (isWhileLoop_);
    }

    bool isInfinite()
    {
        return (isInfinite_);
    }

    int start()
    {
        return (startValue);
    }

    int stop()
    {
        return (stopValue);
    }

    string valueOne()
    {
        return (valueOne_);
    }

    string valueTwo()
    {
        return (valueTwo_);
    }

    string logicOperator()
    {
        return (logicOperatorValue);
    }

    void setInfinite()
    {
        isInfinite_ = true;
    }

    void setBool(bool b)
    {
        isIF_ = b;
    }

    void setFor(bool b)
    {
        isForLoop_ = b;
    }

    void setWhile(bool b)
    {
        isWhileLoop_ = b;
    }

    void setWhileValues(string v1, string op, string v2)
    {
        valueOne_ = v1,
        logicOperatorValue = op,
        valueTwo_ = v2;
    }

    void setForValues(int a, int b)
    {
        startValue = a,
        stopValue = b;
    }

    void setForList(List l)
    {
        list = l;
    }

    bool isListLoop()
    {
        return (isListLoop_);
    }

    void setListLoop()
    {
        isListLoop_ = true;
    }

    List getList()
    {
        return (list);
    }
};

class Object {
private:
    vector<Method>      methods;
    vector<Variable>    variables;

    int                 badMethods,
                        badVariables;

    string              objectName,
                        currentMethod;

    bool                collectable;

public:
    Object() { }

    Object(string name)
    {
        initialize(name);
        currentMethod = "";
    }

    ~Object()
    {
        clear();
    }

    void setName(string name)
    {
        objectName = name;
    }

    void collect()
    {
        collectable = true;
    }

    void dontCollect()
    {
        collectable = false;
    }

    bool garbage()
    {
        return (collectable);
    }

    Method getCurrentMethod()
    {
        return (getMethod(currentMethod));
    }

    void setCurrentMethod(string methodName)
    {
        currentMethod = methodName;
    }

    void setPublic()
    {
        if (methodExists(currentMethod))
            methods.at(methodAt(currentMethod)).setPublic();
    }

    void setPrivate()
    {
        if (methodExists(currentMethod))
            methods.at(methodAt(currentMethod)).setPrivate();
    }

    void addToCurrentMethod(string line)
    {
        if (methodExists(currentMethod))
            methods.at(methodAt(currentMethod)).add(line);
        else
            cout << "#!=add_to_currentMethod:undefined" << endl;
    }

    int methodSize()
    {
        return ((int)methods.size());
    }

    int variableSize()
    {
        return ((int)variables.size());
    }

    int methodAt(string methodName)
    {
        for (int i = 0; i < methodSize(); i++) {
            if (methods.at(i).name() == methodName)
                return (i);
        }

        return (-1);
    }

    int variableAt(string variableName)
    {
        for (int i = 0; i < variableSize(); i++) {
            if (variables.at(i).name() == variableName)
                return (i);
        }

        return (-1);
    }

    string getMethodName(int index)
    {
        if (index < (int)methods.size())
            return (methods.at(index).name());

        return ("[undefined]");
    }

    string getVariableName(int index)
    {
        if (index < (int)variables.size())
            return (variables.at(index).name());

        return ("[undefined]");
    }

    void addMethod(Method method)
    {
        if (!method.isBad())
            methods.push_back(method);
    }

    void addVariable(Variable variable)
    {
        if (!variable.isNull())
            variables.push_back(variable);
    }

    void clear()
    {
        clearMethods();
        clearVariables();
    }

    void clearMethods()
    {
        methods.clear();
    }

    void clearVariables()
    {
        variables.clear();
    }

    bool isBad()
    {
        if (startsWith(name(), "[bad_meth"))
            return (true);

        return (false);
    }

    void removeVariable(string variableName)
    {
        vector<Variable> oldVariables = getVariables();

        clearVariables();

        for (int i = 0; i < (int)oldVariables.size(); i++)
            if (oldVariables.at(i).name() != variableName)
                variables.push_back(oldVariables.at(i));
    }

    Method getMethod(string methodName)
    {
        Method badMethod("[bad_meth#" + itos(badMethods) + "]");

        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == methodName)
                return (methods.at(i));

        badMethods++;

        return (badMethod);
    }

    vector<Method> getMethods()
    {
        return (methods);
    }

    Variable getVariable(string variableName)
    {
        Variable badVariable("[bad_var#" + itos(badVariables) + "]", "[null]");

        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == variableName)
                return (variables.at(i));

        badVariables++;

        return (badVariable);
    }

    vector<Variable> getVariables()
    {
        return (variables);
    }

    void initialize(string name)
    {
        badMethods = 0,
        badVariables = 0;
        currentMethod = "",
        objectName = name;
    }

    bool methodExists(string methodName)
    {
        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == methodName)
                return (true);

        return (false);
    }

    string name()
    {
        return (objectName);
    }

    bool variableExists(string variableName)
    {
        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == variableName)
                return (true);

        return (false);
    }
};
