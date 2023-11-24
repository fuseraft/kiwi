#ifndef NOCTIS_MEM_H
#define NOCTIS_MEM_H

class Memory
{
private:
    vector<Method> methods;
    vector<Class> classes;
    vector<Variable> variables;
    vector<List> lists;
    vector<Constant> constants;
    vector<Method> ifStatements;
    vector<Method> forLoops;
    vector<Method> whileLoops;
    vector<string> args;
    vector<Module> modules;
    vector<Script> scripts;
    Switch mainSwitch;

public:
    Memory();
    ~Memory();

    void clearAll();
    void clearArgs();
    void clearConstants();
    void clearFor();
    void clearIf();
    void clearLists();
    void clearMethods();
    void clearClasses();
    void clearVariables();
    void clearWhile();
    void gc();

    void loadScript(string script);

    int indexOfConstant(string s);
    int indexOfList(string s);
    int indexOfMethod(string s);
    int indexOfModule(string s);
    int indexOfClass(string s);
    int indexOfScript(string s);
    int indexOfVariable(string s);

    Constant& getConstant(int index);
    Constant& getConstant(string s);
    List& getList(int index);
    List& getList(string s);
    Method& getMethod(int index);
    Method& getMethod(string s);
    Module& getModule(string s);
    Class& getClass(int index);
    Class& getClass(string s);
    Variable getClassVariable(string className, string variableName);
    Script& getScript();
    Variable& getVar(int index);
    Variable& getVar(string s);
    Switch& getMainSwitch();
    Method& getIfStatement(int index);
    Method& getForLoop(int index);
    Method& getWhileLoop(int index);

    void createMethod(string arg0, string arg1);

    void createFailedWhileLoop();
    void createWhileLoop(string v1, string op, string v2);

    void createFailedForLoop();
    void createForLoop();
    void createForLoop(double a, double b, string op);
    void createForLoop(List list);

    void createVariable(string name, double value);
    void createVariable(string name, string value);
    void setVariable(string name, double value);
    void setVariable(string name, string value);
    void replaceElement(string before, string after, string replacement);

    bool isNumber(string s);
    bool isNumber(Variable var);
    bool isString(string s);
    bool isString(Variable var);

    bool notClassMethod(string s);
    bool constantExists(string s);
    bool moduleExists(string s);
    bool variableExists(string s);
    bool classExists(string s);
    bool methodExists(string s);
    bool listExists(string s);

    void redefine(string target, string name);
    void globalize(string arg1);

    void removeConstant(string s);
    void removeList(string s);
    void removeMethod(string s);
    void removeModule(string s);
    void removeClass(string s);
    void removeVariable(string s);

    string varNumberString(string s);
    double varNumber(string s);
    string varString(string s);

    int getMethodCount();
    int getVariableCount();
    int getClassCount();
    int getListCount();
    int getConstantCount();

    void addConstant(Constant c);
    void addList(List l);
    void addMethod(Method m);
    void addModule(Module m);
    void addClass(Class c);
    void addVariable(Variable v);

    void addIfStatement(Method ifStatement);
    int getIfStatementCount();
    int getForLoopCount();
    int getWhileLoopCount();
    
    void addArg(string arg);
    int getArgCount();
    string getArg(int index);

    void createIfStatement(bool value);
    void createModule(string s);
    void createClass(string className);

    void addLineToCurrentForLoop(string line);
    void addItemToList(string listName, string value);
};

Memory::Memory() {}
Memory::~Memory() {}

void Memory::addLineToCurrentForLoop(string line)
{
    forLoops[forLoops.size() - 1].add(line);
}

void Memory::addItemToList(string listName, string value)
{
    this->getList(listName).add(value);
}

void Memory::addConstant(Constant c) { constants.push_back(c); }
void Memory::addList(List l) { lists.push_back(l); }
void Memory::addMethod(Method m) { methods.push_back(m); }
void Memory::addModule(Module m) { modules.push_back(m); }
void Memory::addClass(Class c) { classes.push_back(c); }
void Memory::addVariable(Variable v) { variables.push_back(v); }

void Memory::addIfStatement(Method ifStatement) { ifStatements.push_back(ifStatement); }
int Memory::getIfStatementCount() { return ifStatements.size(); }

int Memory::getMethodCount() { return methods.size(); }
int Memory::getVariableCount() { return variables.size(); }
int Memory::getClassCount() { return classes.size(); }
int Memory::getListCount() { return lists.size(); }
int Memory::getConstantCount() { return constants.size(); }
int Memory::getForLoopCount() { return forLoops.size(); }

Method& Memory::getMethod(int index) { return methods.at(index); }
Variable& Memory::getVar(int index) { return variables.at(index); }
Class& Memory::getClass(int index) { return classes.at(index); }
List& Memory::getList(int index) { return lists.at(index); }
Constant& Memory::getConstant(int index) { return constants.at(index); }

Method& Memory::getIfStatement(int index) { return ifStatements.at(index); }
Method& Memory::getForLoop(int index) { return forLoops.at(index); }

int Memory::getWhileLoopCount() { return whileLoops.size(); }
Method& Memory::getWhileLoop(int index) { return whileLoops.at(index); }

int Memory::getArgCount() { return args.size(); }
string Memory::getArg(int index) { return args.at(index); }
void Memory::addArg(string arg) { args.push_back(arg); }

void Memory::createClass(string className)
{
    if (classExists(className))
    {
        State.DefiningClass = true;
        State.CurrentClass = className;
    }
    else
    {
        Class newClass(className);
        State.CurrentClass = className;
        newClass.setCollectable(false);
        addClass(newClass);
        State.DefiningClass = true;
    }
}

void Memory::createModule(string s)
{
    string moduleName = s;
    moduleName = subtractString(moduleName, "[");
    moduleName = subtractString(moduleName, "]");

    Module newModule(moduleName);
    addModule(newModule);

    State.DefiningModule = true;
    State.CurrentModule = moduleName;
}

void Memory::createIfStatement(bool value)
{
    if (!value)
    {
        State.LastValue = "false";

        if (!State.DefiningNest)
        {
            Method ifMethod("[failif]");
            ifMethod.setBool(false);
            addIfStatement(ifMethod);

            State.DefiningIfStatement = true;
            State.FailedIfStatement = true;
            State.FailedNest = true;
        }
        else
            State.FailedNest = true;

        return;
    }

    State.LastValue = "true";

    if (State.DefiningNest)
    {
        getIfStatement(getIfStatementCount() - 1).buildNest();
        State.FailedNest = false;
    }
    else
    {
        Method ifMethod("[if#" + itos(State.IfStatementCount) + "]");
        ifMethod.setBool(true);
        State.DefiningIfStatement = true;
        addIfStatement(ifMethod);

        State.IfStatementCount++;
        State.FailedIfStatement = false;
        State.FailedNest = false;
    }
}

void Memory::createMethod(string arg0, string arg1)
{
    bool indestructable = arg0 == "[method]";

    if (State.DefiningClass)
    {
        if (getClass(State.CurrentClass).hasMethod(arg1))
        {
            error(ErrorMessage::METHOD_DEFINED, arg1, false);
            return;
        }

        if (!containsParams(arg1))
        {
            Method method(arg1);

            if (State.DefiningPublicCode)
                method.setPublic();
            else if (State.DefiningPrivateCode)
                method.setPrivate();

            method.setClass(State.CurrentClass);
            getClass(State.CurrentClass).addMethod(method);
            getClass(State.CurrentClass).setCurrentMethod(arg1);
            State.DefiningMethod = true;
            State.DefiningClassMethod = true;
            return;
        }

        vector<string> params = getParams(arg1);

        Method method(beforeParams(arg1));

        if (State.DefiningPublicCode)
            method.setPublic();
        else if (State.DefiningPrivateCode)
            method.setPrivate();

        method.setClass(State.CurrentClass);

        for (int i = 0; i < (int)params.size(); i++)
        {
            if (variableExists(params.at(i)))
            {
                if (zeroDots(params.at(i)))
                {
                    if (isString(params.at(i)))
                        method.addMethodVariable(varString(params.at(i)), getVar(params.at(i)).name());
                    else if (isNumber(params.at(i)))
                        method.addMethodVariable(varNumber(params.at(i)), getVar(params.at(i)).name());
                    else
                        error(ErrorMessage::IS_NULL, params.at(i), false);
                    return;
                }

                string before(beforeDot(params.at(i))), after(afterDot(params.at(i)));

                if (!classExists(before))
                {
                    error(ErrorMessage::CLS_METHOD_UNDEFINED, before, false);
                    return;
                }

                if (!getClass(before).hasVariable(after))
                {
                    error(ErrorMessage::CLS_VAR_UNDEFINED, after, false);
                    return;
                }

                if (getClass(before).getVariable(after).getString() != State.Null)
                    method.addMethodVariable(getClass(before).getVariable(after).getString(), after);
                else if (getClass(before).getVariable(after).getNumber() != State.NullNum)
                    method.addMethodVariable(getClass(before).getVariable(after).getNumber(), after);
                else
                    error(ErrorMessage::IS_NULL, params.at(i), false);
            }
            else
            {
                if (isAlpha(params.at(i)))
                {
                    Variable newVariable("@[pm#" + itos(State.ParamVarCount) + "]", params.at(i));
                    method.addMethodVariable(newVariable);
                    State.ParamVarCount++;
                }
                else
                {
                    Variable newVariable("@[pm#" + itos(State.ParamVarCount) + "]", stod(params.at(i)));
                    method.addMethodVariable(newVariable);
                    State.ParamVarCount++;
                }
            }
        }

        getClass(State.CurrentClass).addMethod(method);
        getClass(State.CurrentClass).setCurrentMethod(beforeParams(arg1));
        State.DefiningMethod = true;
        State.DefiningParameterizedMethod = true;
        State.DefiningClassMethod = true;
    }
    else
    {
        if (methodExists(arg1))
            error(ErrorMessage::METHOD_DEFINED, arg1, false);
        else
        {
            if (!zeroDots(arg1))
            {
                string before(beforeDot(arg1)), after(afterDot(arg1));

                if (classExists(before))
                {
                    Method method(after);

                    if (State.DefiningPublicCode)
                        method.setPublic();
                    else if (State.DefiningPrivateCode)
                        method.setPrivate();

                    method.setClass(before);
                    getClass(before).addMethod(method);
                    getClass(before).setCurrentMethod(after);
                    State.DefiningMethod = true;
                    State.DefiningClassMethod = true;
                }
                else
                    error(ErrorMessage::CLS_UNDEFINED, "", false);
            }
            else if (containsParams(arg1))
            {
                vector<string> params = getParams(arg1);

                Method method(beforeParams(arg1));
                method.setIndestructible(indestructable);

                for (int i = 0; i < (int)params.size(); i++)
                {
                    if (variableExists(params.at(i)))
                    {
                        if (!zeroDots(params.at(i)))
                        {
                            string before(beforeDot(params.at(i))), after(afterDot(params.at(i)));

                            if (classExists(before))
                            {
                                if (getClass(before).hasVariable(after))
                                {
                                    if (getClass(before).getVariable(after).getString() != State.Null)
                                        method.addMethodVariable(getClass(before).getVariable(after).getString(), after);
                                    else if (getClass(before).getVariable(after).getNumber() != State.NullNum)
                                        method.addMethodVariable(getClass(before).getVariable(after).getNumber(), after);
                                    else
                                        error(ErrorMessage::IS_NULL, params.at(i), false);
                                }
                                else
                                    error(ErrorMessage::CLS_VAR_UNDEFINED, after, false);
                            }
                            else
                                error(ErrorMessage::CLS_METHOD_UNDEFINED, before, false);
                        }
                        else
                        {
                            if (isString(params.at(i)))
                                method.addMethodVariable(varString(params.at(i)), getVar(params.at(i)).name());
                            else if (isNumber(params.at(i)))
                                method.addMethodVariable(varNumber(params.at(i)), getVar(params.at(i)).name());
                            else
                                error(ErrorMessage::IS_NULL, params.at(i), false);
                        }
                    }
                    else
                    {
                        if (isAlpha(params.at(i)))
                        {
                            Variable newVariable("@" + params.at(i), "");
                            newVariable.setNull();
                            method.addMethodVariable(newVariable);
                            State.ParamVarCount++;
                        }
                        else
                        {
                            Variable newVariable("@" + params.at(i), 0);
                            newVariable.setNull();
                            method.addMethodVariable(newVariable);
                            State.ParamVarCount++;
                        }
                    }
                }

                methods.push_back(method);
                State.DefiningMethod = true;
                State.DefiningParameterizedMethod = true;
            }
            else
            {
                Method method(arg1);
                method.setIndestructible(indestructable);

                methods.push_back(method);
                State.DefiningMethod = true;
            }
        }
    }
}

void Memory::loadScript(string script)
{
    string s("");
    ifstream f(script.c_str());
    State.CurrentScript = script;

    scripts.clear();

    Script newScript(script);

    if (f.is_open())
    {
        while (!f.eof())
        {
            getline(f, s);

            if (s.length() > 0)
            {
                newScript.add(trimLeadingWhitespace(s));
            }
        }
    }

    scripts.push_back(newScript);
}

Switch& Memory::getMainSwitch()
{
    return mainSwitch;
}

void Memory::createWhileLoop(string v1, string op, string v2)
{
    Method whileMethod("[while#" + itos(State.WhileLoopCount) + "]");
    whileMethod.setWhile(true);
    whileMethod.setWhileValues(v1, op, v2);
    State.DefiningWhileLoop = true;
    whileLoops.push_back(whileMethod);
    State.WhileLoopCount++;
}

void Memory::createForLoop(List list)
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setForList(list);
    forMethod.setListLoop();
    forMethod.setSymbol(State.DefaultLoopSymbol);
    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.ForLoopCount++;
    State.SuccessFlag = true;
}

void Memory::createForLoop(double a, double b, string op)
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setSymbol(State.DefaultLoopSymbol);

    if (op == "<=" || op == ">=")
        forMethod.setForValues((int)a, (int)b);
    else if (op == "<" || op == ">")
        forMethod.setForValues((int)a, (int)b - 1);

    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.ForLoopCount++;
    State.SuccessFlag = true;
}

void Memory::createForLoop()
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setInfinite();
    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.ForLoopCount++;
    State.SuccessFlag = true;
}

void Memory::createFailedForLoop()
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(false);
    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.DefaultLoopSymbol = "$";
    State.SuccessFlag = false;
}

void Memory::createFailedWhileLoop()
{
    Method whileMethod("[while#" + itos(State.WhileLoopCount) + "]");
    whileMethod.setWhile(false);
    State.DefiningWhileLoop = true;
    whileLoops.push_back(whileMethod);
}

bool Memory::isNumber(string s) { return varNumber(s) != State.NullNum; }
bool Memory::isNumber(Variable var) { return var.getNumber() != State.NullNum; }
bool Memory::isString(string s) { return varString(s) != State.Null; }
bool Memory::isString(Variable var) { return var.getString() != State.Null; }

int Memory::indexOfConstant(string s)
{
    for (unsigned i = 0; i < constants.size(); ++i)
    {
        if (constants.at(i).name() == s)
            return i;
    }

    return -1;
}

int Memory::indexOfList(string s)
{
    for (unsigned i = 0; i < lists.size(); ++i)
    {
        if (lists.at(i).name() == s)
            return i;
    }

    return -1;
}

int Memory::indexOfMethod(string s)
{
    for (unsigned i = 0; i < methods.size(); ++i)
    {
        if (methods.at(i).name() == s)
            return i;
    }

    return -1;
}

int Memory::indexOfModule(string s)
{
    for (unsigned i = 0; i < modules.size(); ++i)
    {
        if (modules.at(i).name() == s)
            return i;
    }

    return -1;
}

int Memory::indexOfClass(string s)
{
    for (unsigned i = 0; i < classes.size(); ++i)
    {
        if (classes.at(i).name() == s)
            return i;
    }

    return -1;
}

int Memory::indexOfScript(string s)
{
    for (unsigned i = 0; i < scripts.size(); ++i)
    {
        if (scripts.at(i).name() == s)
            return i;
    }

    return -1;
}

int Memory::indexOfVariable(string s)
{
    for (unsigned i = 0; i < variables.size(); ++i)
    {
        if (variables.at(i).name() == s)
            return i;
    }

    return -1;
}

Constant& Memory::getConstant(string s) { return constants.at(indexOfConstant(s)); }
List& Memory::getList(string s) { return lists.at(indexOfList(s)); }
Method& Memory::getMethod(string s) { return methods.at(indexOfMethod(s)); }
Module& Memory::getModule(string s) { return modules.at(indexOfModule(s)); }
Class& Memory::getClass(string s) { return classes.at(indexOfClass(s)); }
Script& Memory::getScript() { return scripts.at(0); }
Variable& Memory::getVar(string s) { return variables.at(indexOfVariable(s)); }
Variable Memory::getClassVariable(string className, string variableName)
{
    return getClass(className).getVariable(variableName);
}

void Memory::removeConstant(string s) { constants.erase(constants.begin() + indexOfConstant(s)); }
void Memory::removeList(string s) { lists.erase(lists.begin() + indexOfList(s)); }
void Memory::removeMethod(string s) { methods.erase(methods.begin() + indexOfMethod(s)); }
void Memory::removeModule(string s) { modules.erase(modules.begin() + indexOfModule(s)); }
void Memory::removeClass(string s) { classes.erase(classes.begin() + indexOfClass(s)); }
void Memory::removeVariable(string s) { variables.erase(variables.begin() + indexOfVariable(s)); }

string Memory::varString(string s) { return getVar(s).getString(); }
double Memory::varNumber(string s) { return getVar(s).getNumber(); }
string Memory::varNumberString(string s) { return dtos(varNumber(s)); }

void Memory::redefine(string target, string name)
{
    if (variableExists(target))
    {
        if (Env::fileExists(varString(target)) || Env::directoryExists(varString(target)))
        {
            string old_name(varString(target)), new_name("");

            if (variableExists(name))
            {
                if (isString(name))
                {
                    new_name = varString(name);

                    if (Env::fileExists(old_name))
                    {
                        if (!Env::fileExists(new_name))
                        {
                            if (Env::fileExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error(ErrorMessage::FILE_NOT_FOUND, old_name, false);
                        }
                        else
                            error(ErrorMessage::FILE_EXISTS, new_name, false);
                    }
                    else if (Env::directoryExists(old_name))
                    {
                        if (!Env::directoryExists(new_name))
                        {
                            if (Env::directoryExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error(ErrorMessage::DIR_NOT_FOUND, old_name, false);
                        }
                        else
                            error(ErrorMessage::DIR_EXISTS, new_name, false);
                    }
                    else
                        error(ErrorMessage::TARGET_UNDEFINED, old_name, false);
                }
                else
                    error(ErrorMessage::NULL_STRING, name, false);
            }
            else
            {
                if (Env::fileExists(old_name))
                {
                    if (!Env::fileExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error(ErrorMessage::FILE_EXISTS, name, false);
                }
                else if (Env::directoryExists(old_name))
                {
                    if (!Env::directoryExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error(ErrorMessage::DIR_EXISTS, name, false);
                }
                else
                    error(ErrorMessage::TARGET_UNDEFINED, old_name, false);
            }
        }
        else
        {
            if (startsWith(name, "@"))
            {
                if (!variableExists(name))
                    getVar(target).setName(name);
                else
                    error(ErrorMessage::VAR_DEFINED, name, false);
            }
            else
                error(ErrorMessage::INVALID_VAR_DECL, name, false);
        }
    }
    else if (listExists(target))
    {
        if (!listExists(name))
            getList(name).setName(name);
        else
            error(ErrorMessage::LIST_UNDEFINED, name, false);
    }
    else if (classExists(target))
    {
        if (!classExists(name))
            getClass(target).setName(name);
        else
            error(ErrorMessage::CLS_METHOD_UNDEFINED, name, false);
    }
    else if (methodExists(target))
    {
        if (!methodExists(name))
            getMethod(target).setName(name);
        else
            error(ErrorMessage::METHOD_UNDEFINED, name, false);
    }
    else if (Env::fileExists(target) || Env::directoryExists(target))
        rename(target.c_str(), name.c_str());
    else
        error(ErrorMessage::TARGET_UNDEFINED, target, false);
}

void Memory::globalize(string arg1)
{
    if (!(contains(arg1, ".") && methodExists(arg1) && !methodExists(afterDot(arg1))))
    {
        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg1, false);
        return;
    }

    Method method(afterDot(arg1));

    vector<string> lines = getClass(beforeDot(arg1)).getMethod(afterDot(arg1)).getLines();

    for (int i = 0; i < (int)lines.size(); i++)
        method.add(lines[i]);

    methods.push_back(method);
}

void Memory::replaceElement(string before, string after, string replacement)
{
    vector<string> newList;

    for (int i = 0; i < (int)getList(before).size(); i++)
    {
        if (i == stoi(after))
            newList.push_back(replacement);
        else
            newList.push_back(getList(before).at(i));
    }

    getList(before).clear();

    for (int i = 0; i < (int)newList.size(); i++)
        addItemToList(before, newList.at(i));

    newList.clear();
}

void Memory::setVariable(string name, string value)
{
    getVar(name).setVariable(value);
    State.LastValue = value;
}

void Memory::setVariable(string name, double value)
{
    if (varString(name) != State.Null)
        getVar(name).setVariable(dtos(value));
    else if (varNumber(name) != State.NullNum)
        getVar(name).setVariable(value);
    else
    {
        if (getVar(name).waiting())
            getVar(name).stopWait();

        getVar(name).setVariable(value);
    }

    State.LastValue = dtos(value);
}

void Memory::createVariable(string name, string value)
{
    Variable newVariable(name, value);

    newVariable.setCollectable(State.ExecutedTemplate || State.ExecutedMethod || State.ExecutedTryBlock);

    variables.push_back(newVariable);
    State.LastValue = value;
}

void Memory::createVariable(string name, double value)
{
    Variable newVariable(name, value);

    newVariable.setCollectable(State.ExecutedTemplate || State.ExecutedMethod || State.ExecutedTryBlock);

    variables.push_back(newVariable);
    State.LastValue = dtos(value);
}

void Memory::clearAll()
{
    clearMethods();
    clearClasses();
    clearVariables();
    clearLists();
    clearArgs();
    clearIf();
    clearFor();
    clearWhile();
    clearConstants();
}

void Memory::clearConstants()
{
    constants.clear();
}

void Memory::clearArgs()
{
    args.clear();
}

void Memory::clearFor()
{
    forLoops.clear();
}

void Memory::clearWhile()
{
    whileLoops.clear();
}

void Memory::clearIf()
{
    ifStatements.clear();
}

void Memory::clearLists()
{
    lists.clear();
}

void Memory::clearMethods()
{
    vector<Method> indestructibleMethods;

    for (unsigned i = 0; i < methods.size(); ++i)
        if (methods.at(i).indestructible())
            indestructibleMethods.push_back(methods.at(i));

    methods.clear();

    for (unsigned i = 0; i < indestructibleMethods.size(); ++i)
        methods.push_back(indestructibleMethods[i]);
}

// TODO: Implement indestructible objects?
void Memory::clearClasses() { classes.clear(); }

void Memory::clearVariables()
{
    vector<Variable> indestructibleVariables;

    for (unsigned i = 0; i < variables.size(); ++i)
        if (variables.at(i).indestructible())
            indestructibleVariables.push_back(variables.at(i));

    variables.clear();

    for (unsigned i = 0; i < indestructibleVariables.size(); ++i)
        variables.push_back(indestructibleVariables[i]);
}

void Memory::gc()
{
    vector<string> garbageVars;

    for (unsigned i = 0; i < variables.size(); ++i)
        if (variables.at(i).isCollectable() && !State.ExecutedIfStatement)
            if (!State.DontCollectMethodVars)
                garbageVars.push_back(variables.at(i).name());

    for (unsigned i = 0; i < garbageVars.size(); ++i)
        removeVariable(garbageVars.at(i));

    vector<string> garbageLists;

    for (unsigned i = 0; i < lists.size(); ++i)
        if (lists.at(i).isCollectable() && !State.ExecutedIfStatement)
            garbageLists.push_back(lists.at(i).name());

    for (unsigned i = 0; i < garbageLists.size(); ++i)
        removeList(garbageLists.at(i));

    vector<string> garbageClasses;

    for (unsigned i = 0; i < classes.size(); ++i)
        if (classes.at(i).isCollectable() && !State.ExecutedIfStatement)
            garbageClasses.push_back(classes.at(i).name());

    for (unsigned i = 0; i < garbageClasses.size(); ++i)
        removeClass(garbageClasses.at(i));
}

bool Memory::listExists(string s)
{
    for (unsigned i = 0; i < lists.size(); ++i)
        if (lists.at(i).name() == s)
            return true;

    return false;
}

bool Memory::methodExists(string s)
{
    if (!zeroDots(s) && classExists(beforeDot(s)))
    {
        return getClass(beforeDot(s)).hasMethod(afterDot(s));
    }

    for (unsigned i = 0; i < methods.size(); ++i)
        if (methods.at(i).name() == s)
            return true;

    return false;
}

bool Memory::classExists(string s)
{
    for (unsigned i = 0; i < classes.size(); ++i)
        if (classes.at(i).name() == s)
            return true;

    return false;
}

bool Memory::variableExists(string s)
{
    if (!zeroDots(s))
    {
        string before(beforeDot(s)), after(afterDot(s));
        return classExists(before) && getClass(before).hasVariable(after);
    }

    for (unsigned i = 0; i < variables.size(); ++i)
        if (variables.at(i).name() == s)
            return true;

    return false;
}

bool Memory::moduleExists(string s)
{
    for (unsigned i = 0; i < modules.size(); ++i)
        if (modules.at(i).name() == s)
            return true;

    return false;
}

bool Memory::constantExists(string s)
{
    for (unsigned i = 0; i < constants.size(); ++i)
        if (constants.at(i).name() == s)
            return true;

    return false;
}

bool Memory::notClassMethod(string s)
{
    if (!zeroDots(s))
    {
        string before(beforeDot(s));
        return !classExists(before);
    }

    return true;
}

#endif
