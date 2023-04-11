#ifndef NOCTIS_MEM_H
#define NOCTIS_MEM_H

class Memory {
    private:
    vector<Method> methods;
    vector<Object> objects;
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
    void clearObjects();
    void clearVariables();
    void clearWhile();
    void collectGarbage();

    void loadScript(string script);

    int indexOfConstant(string s);
    int indexOfList(string s);
    int indexOfMethod(string s);
    int indexOfModule(string s);
    int indexOfObject(string s);
    int indexOfScript(string s);
    int indexOfVariable(string s);
    
    Constant getConstant(string s);
    List getList(string s);
    Method getMethod(string s);
    Module getModule(string s);
    Object getObject(string s);
    Script getScript(string s);
    Variable getVar(string s);
    Switch getMainSwitch();

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
    void setList(string arg1, string arg2, vector<string> params);
    void replaceElement(string before, string after, string replacement);

    bool isNumber(string s);
    bool isNumber(Variable var);
    bool isString(string s);
    bool isString(Variable var);

    bool notObjectMethod(string s);
    bool noVariables();
    bool noObjects();
    bool noMethods();
    bool noLists();
    bool constantExists(string s);
    bool moduleExists(string s);
    bool variableExists(string s);
    bool objectExists(string s);
    bool methodExists(string s);
    bool listExists(string s);

    void redefine(string target, string name);
    void globalize(string arg0, string arg1);

    void removeConstant(string s);
    void removeList(string s);
    void removeMethod(string s);
    void removeModule(string s);
    void removeObject(string s);
    void removeVariable(string s);

    string varNumberString(string s);
    double varNumber(string s);
    string varString(string s);
};

Memory::Memory() {}
Memory::~Memory() {}

void Memory::createMethod(string arg0, string arg1)
{
    bool indestructable = false;

    if (arg0 == "[method]")
        indestructable = true;

    if (State.DefiningObject)
    {
        if (getObject(State.CurrentObject).methodExists(arg1))
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

            method.setObject(State.CurrentObject);
            getObject(State.CurrentObject).addMethod(method);
            getObject(State.CurrentObject).setCurrentMethod(arg1);
            State.DefiningMethod = true;
            State.DefiningObjectMethod = true;
            return;
        }

        vector<string> params = getParams(arg1);

        Method method(beforeParams(arg1));

        if (State.DefiningPublicCode)
            method.setPublic();
        else if (State.DefiningPrivateCode)
            method.setPrivate();

        method.setObject(State.CurrentObject);

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

                if (!objectExists(before))
                {
                    error(ErrorMessage::OBJ_METHOD_UNDEFINED, before, false);
                    return;
                }

                if (!getObject(before).variableExists(after))
                {
                    error(ErrorMessage::OBJ_VAR_UNDEFINED, after, false);
                    return;
                }

                if (getObject(before).getVariable(after).getString() != State.Null)
                    method.addMethodVariable(getObject(before).getVariable(after).getString(), after);
                else if (getObject(before).getVariable(after).getNumber() != State.NullNum)
                    method.addMethodVariable(getObject(before).getVariable(after).getNumber(), after);
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

        getObject(State.CurrentObject).addMethod(method);
        getObject(State.CurrentObject).setCurrentMethod(beforeParams(arg1));
        State.DefiningMethod = true;
        State.DefiningParameterizedMethod = true;
        State.DefiningObjectMethod = true;
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

                if (objectExists(before))
                {
                    Method method(after);

                    if (State.DefiningPublicCode)
                        method.setPublic();
                    else if (State.DefiningPrivateCode)
                        method.setPrivate();

                    method.setObject(before);
                    getObject(before).addMethod(method);
                    getObject(before).setCurrentMethod(after);
                    State.DefiningMethod = true;
                    State.DefiningObjectMethod = true;
                }
                else
                    error(ErrorMessage::OBJ_UNDEFINED, "", false);
            }
            else if (containsParams(arg1))
            {
                vector<string> params = getParams(arg1);

                Method method(beforeParams(arg1));

                if (indestructable)
                    method.setIndestructible();

                for (int i = 0; i < (int)params.size(); i++)
                {
                    if (variableExists(params.at(i)))
                    {
                        if (!zeroDots(params.at(i)))
                        {
                            string before(beforeDot(params.at(i))), after(afterDot(params.at(i)));

                            if (objectExists(before))
                            {
                                if (getObject(before).variableExists(after))
                                {
                                    if (getObject(before).getVariable(after).getString() != State.Null)
                                        method.addMethodVariable(getObject(before).getVariable(after).getString(), after);
                                    else if (getObject(before).getVariable(after).getNumber() != State.NullNum)
                                        method.addMethodVariable(getObject(before).getVariable(after).getNumber(), after);
                                    else
                                        error(ErrorMessage::IS_NULL, params.at(i), false);
                                }
                                else
                                    error(ErrorMessage::OBJ_VAR_UNDEFINED, after, false);
                            }
                            else
                                error(ErrorMessage::OBJ_METHOD_UNDEFINED, before, false);
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

                if (indestructable)
                    method.setIndestructible();

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

    Script newScript(script);

    if (f.is_open())
    {
        while (!f.eof())
        {
            getline(f, s);

            if (s.length() > 0)
            {
                if (s[0] == '\r' || s[0] == '\n')
                    doNothing();
                else if (s[0] == '\t')
                {
                    s.erase(remove(s.begin(), s.end(), '\t'), s.end());
                    char * c = new char[s.size() + 1];
                    copy(s.begin(), s.end(), c);
                    c[s.size()] = '\0';
                    newScript.add(trimLeadingWhitespace(c));
                    delete[] c;
                }
                else
                {
                    char * c = new char[s.size() + 1];
                    copy(s.begin(), s.end(), c);
                    c[s.size()] = '\0';
                    newScript.add(trimLeadingWhitespace(c));
                    delete[] c;
                }
            }
            else
                newScript.add("");
        }
    }

    scripts.push_back(newScript);
}

Switch Memory::getMainSwitch()
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

    if (op == "<=")
        forMethod.setForValues((int)a, (int)b);
    else if (op == ">=")
        forMethod.setForValues((int)a, (int)b);
    else if (op == "<")
        forMethod.setForValues((int)a, (int)b - 1);
    else if (op == ">")
        forMethod.setForValues((int)a, (int)b + 1);

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

int Memory::indexOfObject(string s)
{
    for (unsigned i = 0; i < objects.size(); ++i)
    {
        if (objects.at(i).name() == s)
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

Constant Memory::getConstant(string s) { return constants.at(indexOfConstant(s)); }
List Memory::getList(string s) { return lists.at(indexOfList(s)); }
Method Memory::getMethod(string s) { return methods.at(indexOfMethod(s)); }
Module Memory::getModule(string s) { return modules.at(indexOfModule(s)); }
Object Memory::getObject(string s) { return objects.at(indexOfObject(s)); }
Script Memory::getScript(string s) { return scripts.at(indexOfScript(s)); }
Variable Memory::getVar(string s) { return variables.at(indexOfVariable(s)); }

void Memory::removeConstant(string s) { constants.erase(constants.begin() + indexOfConstant(s)); }
void Memory::removeList(string s) { lists.erase(lists.begin() + indexOfList(s)); }
void Memory::removeMethod(string s) { methods.erase(methods.begin() + indexOfMethod(s)); }
void Memory::removeModule(string s) { modules.erase(modules.begin() + indexOfModule(s)); }
void Memory::removeObject(string s) { objects.erase(objects.begin() + indexOfObject(s)); }
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
    else if (objectExists(target))
    {
        if (!objectExists(name))
            getObject(target).setName(name);
        else
            error(ErrorMessage::OBJ_METHOD_UNDEFINED, name, false);
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

void Memory::globalize(string arg0, string arg1)
{
    if (!(contains(arg1, ".") && methodExists(arg1) && !methodExists(afterDot(arg1))))
    {
        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg1, false);
        return;
    }

    Method method(afterDot(arg1));

    vector<string> lines = getObject(beforeDot(arg1)).getMethod(afterDot(arg1)).getLines();

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
        getList(before).add(newList.at(i));

    newList.clear();
}

void Memory::setList(string arg1, string arg2, vector<string> params)
{
    if (methodExists(beforeParams(arg2)))
    {
        executeTemplate(getMethod(beforeParams(arg2)), params);

        if (!containsParams(State.LastValue))
        {
            getList(arg1).add(State.LastValue);
            return;
        }

        vector<string> last_params = getParams(State.LastValue);

        for (int i = 0; i < (int)last_params.size(); i++)
            getList(arg1).add(last_params.at(i));
    }
    else if (objectExists(beforeDot(beforeParams(arg2))))
    {
        executeTemplate(getObject(beforeDot(beforeParams(arg2))).getMethod(afterDot(beforeParams(arg2))), params);

        if (!containsParams(State.LastValue))
        {
            getList(arg1).add(State.LastValue);
            return;
        }

        vector<string> last_params = getParams(State.LastValue);

        for (int i = 0; i < (int)last_params.size(); i++)
            getList(arg1).add(last_params.at(i));
    }
    else
    {
        for (int i = 0; i < (int)params.size(); i++)
        {
            if (!variableExists(params.at(i)))
            {
                getList(arg1).add(params.at(i));
                return;
            }

            if (isString(params.at(i)))
                getList(arg1).add(varString(params.at(i)));
            else if (isNumber(params.at(i)))
                getList(arg1).add(varNumberString(params.at(i)));
            else
                error(ErrorMessage::IS_NULL, params.at(i), false);
        }
    }
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

    if (State.ExecutedTemplate || State.ExecutedMethod || State.ExecutedTryBlock)
        newVariable.collect();
    else
        newVariable.dontCollect();

    variables.push_back(newVariable);
    State.LastValue = value;
}

void Memory::createVariable(string name, double value)
{
    Variable newVariable(name, value);

    if (State.ExecutedTemplate || State.ExecutedMethod || State.ExecutedTryBlock)
        newVariable.collect();
    else
        newVariable.dontCollect();

    variables.push_back(newVariable);
    State.LastValue = dtos(value);
}

void Memory::clearAll()
{
    clearMethods();
    clearObjects();
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
void Memory::clearObjects() { objects.clear(); }

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

void Memory::collectGarbage()
{
    vector<string> garbageVars;

    for (unsigned i = 0; i < variables.size(); ++i)
        if (variables.at(i).garbage() && !State.ExecutedIfStatement)
            if (!State.DontCollectMethodVars)
                garbageVars.push_back(variables.at(i).name());

    for (unsigned i = 0; i < garbageVars.size(); ++i)
        removeVariable(garbageVars.at(i));

    vector<string> garbageLists;

    for (unsigned i = 0; i < lists.size(); ++i)
        if (lists.at(i).garbage() && !State.ExecutedIfStatement)
            garbageLists.push_back(lists.at(i).name());

    for (unsigned i = 0; i < garbageLists.size(); ++i)
        removeList(garbageLists.at(i));

    vector<string> garbageObjects;

    for (unsigned i = 0; i < objects.size(); ++i)
        if (objects.at(i).garbage() && !State.ExecutedIfStatement)
            garbageObjects.push_back(objects.at(i).name());

    for (unsigned i = 0; i < garbageObjects.size(); ++i)
        removeObject(garbageObjects.at(i));
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
    if (!zeroDots(s) && objectExists(beforeDot(s)))
    {
        return getObject(beforeDot(s)).methodExists(afterDot(s));
    }
    else
        for (unsigned i = 0; i < methods.size(); ++i)
            if (methods.at(i).name() == s)
                return true;

    return false;
}

bool Memory::objectExists(string s)
{
    for (unsigned i = 0; i < objects.size(); ++i)
        if (objects.at(i).name() == s)
            return true;

    return false;
}

bool Memory::variableExists(string s)
{
    if (!zeroDots(s))
    {
        string before(beforeDot(s)), after(afterDot(s));
        return objectExists(before) && getObject(before).variableExists(after);
    }
    else
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

bool Memory::noLists() { return lists.empty(); }
bool Memory::noMethods() { return methods.empty(); }
bool Memory::noObjects() { return objects.empty(); }
bool Memory::noVariables() { return variables.empty(); }

bool Memory::notObjectMethod(string s)
{
    if (!zeroDots(s))
    {
        string before(beforeDot(s));
        return !objectExists(before);
    }

    return true;
}

#endif
