/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#ifndef CORE_H
#define CORE_H

void setList(string arg1, string arg2, vector<string> params)
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

void setVariable(string name, string value)
{
    getVar(name).setVariable(value);
    State.LastValue = value;
}

void setVariable(string name, double value)
{
    if (isString(name))
        getVar(name).setVariable(dtos(value));
    else if (isNumber(name))
        getVar(name).setVariable(value);
    else
    {
        if (getVar(name).waiting())
            getVar(name).stopWait();

        getVar(name).setVariable(value);
    }

    State.LastValue = dtos(value);
}

void createVariable(string name, string value)
{
    Variable newVariable(name, value);

    if (State.ExecutedTemplate || State.ExecutedMethod || State.ExecutedTryBlock)
        newVariable.collect();
    else
        newVariable.dontCollect();

    variables.push_back(newVariable);
    State.LastValue = value;
}

///	Creates a double type variable
void createVariable(string name, double value)
{
    Variable newVariable(name, value);

    if (State.ExecutedTemplate || State.ExecutedMethod || State.ExecutedTryBlock)
        newVariable.collect();
    else
        newVariable.dontCollect();

    variables.push_back(newVariable);
    State.LastValue = dtos(value);
}

void replaceElement(string before, string after, string replacement)
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

void appendText(string arg1, string arg2, bool newLine)
{
    if (variableExists(arg1))
    {
        if (!isString(arg1))
        {
            error(ErrorMessage::CONV_ERR, arg1, false);
            return;
        }

        if (!Env::fileExists(varString(arg1)))
        {
            error(ErrorMessage::READ_FAIL, varString(arg1), false);
            return;
        }

        if (variableExists(arg2))
        {
            if (isString(arg2))
            {
                if (newLine)
                    Env::app(varString(arg1), varString(arg2) + "\r\n");
                else
                    Env::app(varString(arg1), varString(arg2));
            }
            else if (isNumber(arg2))
            {
                if (newLine)
                    Env::app(varString(arg1), varNumberString(arg2) + "\r\n");
                else
                    Env::app(varString(arg1), varNumberString(arg2));
            }
            else
                error(ErrorMessage::IS_NULL, arg2, false);
        }
        else
        {
            if (newLine)
                Env::app(varString(arg1), arg2 + "\r\n");
            else
                Env::app(varString(arg1), arg2);
        }
    }
    else
    {
        if (variableExists(arg2))
        {
            if (!isString(arg2))
            {
                error(ErrorMessage::CONV_ERR, arg2, false);
                return;
            }

            if (!Env::fileExists(arg1))
            {
                error(ErrorMessage::READ_FAIL, varString(arg2), false);
                return;
            }

            if (newLine)
                Env::app(arg1, varString(arg2) + "\r\n");
            else
                Env::app(arg1, varString(arg2));
        }
        else
        {
            if (!Env::fileExists(arg1))
            {
                error(ErrorMessage::READ_FAIL, arg1, false);
                return;
            }

            if (newLine)
                Env::app(arg1, arg2 + "\r\n");
            else
                Env::app(arg1, arg2);
        }
    }
}

void __fwrite(string arg1, string arg2)
{
    if (variableExists(arg1))
    {
        if (isString(arg1))
        {
            if (Env::fileExists(varString(arg1)))
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                    {
                        Env::app(varString(arg1), varString(arg2) + "\r\n");
                        State.LastValue = "0";
                    }
                    else if (isNumber(arg2))
                    {
                        Env::app(varString(arg1), varNumberString(arg2) + "\r\n");
                        State.LastValue = "0";
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg2, false);
                        State.LastValue = "-1";
                    }
                }
                else
                {
                    Env::app(varString(arg1), arg2 + "\r\n");
                    State.LastValue = "0";
                }
            }
            else
            {
                Env::createFile(varString(arg1));

                if (isString(arg2))
                {
                    Env::app(varString(arg1), varString(arg2) + "\r\n");
                    State.LastValue = "1";
                }
                else if (isNumber(arg2))
                {
                    Env::app(varString(arg1), varNumberString(arg2) + "\r\n");
                    State.LastValue = "1";
                }
                else
                {
                    error(ErrorMessage::IS_NULL, arg2, false);
                    State.LastValue = "-1";
                }

                State.LastValue = "1";
            }
        }
        else
        {
            error(ErrorMessage::CONV_ERR, arg1, false);
            State.LastValue = "-1";
        }
    }
    else
    {
        if (variableExists(arg2))
        {
            if (isString(arg2))
            {
                if (Env::fileExists(arg1))
                {
                    Env::app(arg1, varString(arg2) + "\r\n");
                    State.LastValue = "0";
                }
                else
                {
                    Env::createFile(varString(arg2));
                    Env::app(arg1, varString(arg2) + "\r\n");
                    State.LastValue = "1";
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, arg2, false);
                State.LastValue = "-1";
            }
        }
        else
        {
            if (Env::fileExists(arg1))
            {
                Env::app(arg1, arg2 + "\r\n");
                State.LastValue = "0";
            }
            else
            {
                Env::createFile(arg1);
                Env::app(arg1, arg2 + "\r\n");
                State.LastValue = "1";
            }
        }
    }
}

string getPrompt()
{
    string new_style("");
    int length = State.PromptStyle.length();
    char prevChar = 'a';

    for (int i = 0; i < length; i++)
    {
        switch (State.PromptStyle[i])
        {
        case 'u':
            if (prevChar == '\\')
                new_style.append(Env::getUser());
            else
                new_style.push_back('u');
            break;

        case 'm':
            if (prevChar == '\\')
                new_style.append(Env::getMachine());
            else
                new_style.push_back('m');
            break;

        case 'w':
            if (prevChar == '\\')
                new_style.append(Env::cwd());
            else
                new_style.push_back('w');
            break;

        case '\\':
            break;

        default:
            new_style.push_back(State.PromptStyle[i]);
            break;
        }

        prevChar = State.PromptStyle[i];
    }

    return new_style;
}

string cleanString(string st)
{
    string cleaned(""), builder("");
    int l = st.length();
    bool buildSymbol = false;

    for (int i = 0; i < l; i++)
    {
        if (buildSymbol)
        {
            if (st[i] == '}')
            {
                builder = subtractChar(builder, "{");

                if (variableExists(builder) && zeroDots(builder))
                {
                    if (isString(builder))
                        cleaned.append(varString(builder));
                    else if (isNumber(builder))
                        cleaned.append(varNumberString(builder));
                    else
                        cleaned.append("null");
                }
                else if (methodExists(builder))
                {
                    parse(builder);

                    cleaned.append(State.LastValue);
                }
                else if (containsParams(builder))
                {
                    if (stackReady(builder))
                    {
                        if (isStringStack(builder))
                            cleaned.append(getStringStack(builder));
                        else
                            cleaned.append(dtos(getStack(builder)));
                    }
                    else if (!zeroDots(builder))
                    {
                        string before(beforeDot(builder)), after(afterDot(builder));

                        if (objectExists(before))
                        {
                            if (getObject(before).methodExists(beforeParams(after)))
                            {
                                executeTemplate(getObject(before).getMethod(beforeParams(after)), getParams(after));

                                cleaned.append(State.LastValue);
                            }
                            else
                                error(ErrorMessage::METHOD_UNDEFINED, before + "." + beforeParams(after), false);
                        }
                        else
                            error(ErrorMessage::OBJ_METHOD_UNDEFINED, before, false);
                    }
                    else if (methodExists(beforeParams(builder)))
                    {
                        executeTemplate(getMethod(beforeParams(builder)), getParams(builder));

                        cleaned.append(State.LastValue);
                    }
                    else
                        cleaned.append("null");
                }
                else if (containsBrackets(builder))
                {
                    string _beforeBrackets(beforeBrackets(builder)), afterBrackets(builder);
                    string rangeBegin(""), rangeEnd(""), _build("");

                    vector<string> listRange = getBracketRange(afterBrackets);

                    if (variableExists(_beforeBrackets))
                    {
                        if (isString(_beforeBrackets))
                        {
                            string tempString(varString(_beforeBrackets));

                            if (listRange.size() == 2)
                            {
                                rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                                if (isNumeric(rangeBegin) && isNumeric(rangeEnd))
                                {
                                    if (stoi(rangeBegin) < stoi(rangeEnd))
                                    {
                                        if ((int)tempString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                                        {
                                            for (int z = stoi(rangeBegin); z <= stoi(rangeEnd); z++)
                                                _build.push_back(tempString[z]);

                                            cleaned.append(_build);
                                        }
                                        else
                                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                                    }
                                    else if (stoi(rangeBegin) > stoi(rangeEnd))
                                    {
                                        if ((int)tempString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                                        {
                                            for (int z = stoi(rangeBegin); z >= stoi(rangeEnd); z--)
                                                _build.push_back(tempString[z]);

                                            cleaned.append(_build);
                                        }
                                        else
                                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                                    }
                                    else
                                        error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                                }
                            }
                            else if (listRange.size() == 1)
                            {
                                rangeBegin = listRange.at(0);

                                if (isNumeric(rangeBegin))
                                {
                                    if (stoi(rangeBegin) <= (int)tempString.length() - 1 && stoi(rangeBegin) >= 0)
                                    {
                                        string _cstr("");

                                        _cstr.push_back(tempString[stoi(rangeBegin)]);

                                        cleaned.append(_cstr);
                                    }
                                    else
                                        error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                                }
                                else
                                    error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                            }
                            else
                                error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                        }
                    }
                    else if (listExists(_beforeBrackets))
                    {
                        if (listRange.size() == 2)
                        {
                            rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                            if (!(isNumeric(rangeBegin) && isNumeric(rangeEnd)))
                            {
                                error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                                return Constants.Null;
                            }
                            
                            if (stoi(rangeBegin) < stoi(rangeEnd))
                            {
                                if (!(getList(_beforeBrackets).size() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0))
                                {
                                    error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                                    return Constants.Null;
                                }

                                string bigString("(");

                                for (int z = stoi(rangeBegin); z <= stoi(rangeEnd); z++)
                                {
                                    bigString.append("\"" + getList(_beforeBrackets).at(z) + "\"");

                                    if (z < stoi(rangeEnd))
                                        bigString.push_back(',');
                                }

                                bigString.push_back(')');

                                cleaned.append(bigString);
                            }
                            else if (stoi(rangeBegin) > stoi(rangeEnd))
                            {
                                if (!(getList(_beforeBrackets).size() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0))
                                {
                                    error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                                    return Constants.Null;
                                }

                                string bigString("(");

                                for (int z = stoi(rangeBegin); z >= stoi(rangeEnd); z--)
                                {
                                    bigString.append("\"" + getList(_beforeBrackets).at(z) + "\"");

                                    if (z > stoi(rangeEnd))
                                        bigString.push_back(',');
                                }

                                bigString.push_back(')');

                                cleaned.append(bigString);
                            }
                            else
                                error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                        }
                        else if (listRange.size() == 1)
                        {
                            rangeBegin = listRange.at(0);

                            if (!isNumeric(rangeBegin))
                            {
                                error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                                return Constants.Null;
                            }

                            if (!(stoi(rangeBegin) <= (int)getList(_beforeBrackets).size() - 1 && stoi(rangeBegin) >= 0))
                            {
                                error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                                return Constants.Null;
                            }
                                
                            cleaned.append(getList(_beforeBrackets).at(stoi(rangeBegin)));
                        }
                        else
                            error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                    }
                    else
                        cleaned.append("null");
                }
                else if (!zeroDots(builder))
                {
                    string before(beforeDot(builder)), after(afterDot(builder));

                    if (!objectExists(before))
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, before, false);
                        return Constants.Null;
                    }

                    if (getObject(before).methodExists(after))
                    {
                        parse(before + "." + after);
                        cleaned.append(State.LastValue);
                    }
                    else if (getObject(before).variableExists(after))
                    {
                        if (getObject(before).getVariable(after).getString() != State.Null)
                            cleaned.append(getObject(before).getVariable(after).getString());
                        else if (getObject(before).getVariable(after).getNumber() != State.NullNum)
                            cleaned.append(dtos(getObject(before).getVariable(after).getNumber()));
                        else
                            cleaned.append("null");
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before + "." + after, false);
                }
                else
                    cleaned.append(builder);

                builder.clear();

                buildSymbol = false;
            }
            else
                builder.push_back(st[i]);
        }
        else
        {
            // REFACTOR HERE
            if (st[i] == '\\' && st[i + 1] == 'n') // begin new-line
                cleaned.push_back('\r');
            else if (st[i] == 'n' && st[i - 1] == '\\') // end new-line
                cleaned.push_back('\n');
            else if (st[i] == '\\' && st[i + 1] == 't') // begin tab
                doNothing();
            else if (st[i] == 't' && st[i - 1] == '\\') // end tab
                cleaned.push_back('\t');
            else if (st[i] == '\\' && st[i + 1] == ';') // begin semi-colon
                doNothing();
            else if (st[i] == ';' && st[i - 1] == '\\') // end semi-colon
                cleaned.push_back(';');
            else if (st[i] == '\\' && st[i + 1] == '\'') // begin apostrophe
                doNothing();
            else if (st[i] == '\'' && st[i - 1] == '\\') // end apostrophe
                cleaned.push_back('\'');
            else if (st[i] == '\\' && st[i + 1] == '{') // begin symbol
                buildSymbol = true;
            else
                cleaned.push_back(st[i]);
        }
    }

    return cleaned;
}

void write(string st)
{
    if (State.CaptureParse)
        State.ParsedOutput.append(cleanString(st));
    else
        IO::print(cleanString(st));

    State.LastValue = st;
}

void writeline(string st)
{
    write(st);
    IO::print(NoctisEnv.GuessedOS == OS_NIX ? "\n" : "\r\n");
}

void clearAll()
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

void clearConstants()
{
    constants.clear();
}

void clearArgs()
{
    args.clear();
}

void clearFor()
{
    forLoops.clear();
}

void clearWhile()
{
    whileLoops.clear();
}

void clearIf()
{
    ifStatements.clear();
}

void clearLists()
{
    lists.clear();
}

void clearMethods()
{
    vector<Method> indestructibleMethods;

    for (int i = 0; i < (int)methods.size(); i++)
        if (methods.at(i).indestructible())
            indestructibleMethods.push_back(methods.at(i));

    methods.clear();

    for (int i = 0; i < (int)indestructibleMethods.size(); i++)
        methods.push_back(indestructibleMethods[i]);
}

void clearObjects()
{
    objects.clear();
}

void clearVariables()
{
    vector<Variable> indestructibleVariables;

    for (int i = 0; i < (int)variables.size(); i++)
        if (variables.at(i).indestructible())
            indestructibleVariables.push_back(variables.at(i));

    variables.clear();

    for (int i = 0; i < (int)indestructibleVariables.size(); i++)
        variables.push_back(indestructibleVariables[i]);
}

void collectGarbage()
{
    vector<string> garbageVars;

    for (int i = 0; i < (int)variables.size(); i++)
        if (variables.at(i).garbage() && !State.ExecutedIfStatement)
            if (!State.DontCollectMethodVars)
                garbageVars.push_back(variables.at(i).name());

    for (int i = 0; i < (int)garbageVars.size(); i++)
        variables = removeVariable(variables, garbageVars.at(i));

    vector<string> garbageLists;

    for (int i = 0; i < (int)lists.size(); i++)
        if (lists.at(i).garbage() && !State.ExecutedIfStatement)
            garbageLists.push_back(lists.at(i).name());

    for (int i = 0; i < (int)garbageLists.size(); i++)
        lists = removeList(lists, garbageLists.at(i));

    vector<string> garbageObjects;

    for (int i = 0; i < (int)objects.size(); i++)
        if (objects.at(i).garbage() && !State.ExecutedIfStatement)
            garbageObjects.push_back(objects.at(i).name());

    for (int i = 0; i < (int)garbageObjects.size(); i++)
        objects = removeObject(objects, garbageObjects.at(i));
}

void displayVersion()
{
    IO::println();
    IO::println("noctis v0.0.2 by <scstauf@gmail.com>");
    IO::println();
}

List getDirectoryList(string before, bool filesOnly)
{
	List newList;
	vector<string> dirList = Env::getDirectoryContents(varString(before), filesOnly);
	for (unsigned int i = 0; i < dirList.size(); i++) 
	{
		newList.add(dirList.at(i));
	}
	if (newList.size() == 0) 
	{
		State.DefiningForLoop = false;
	}
	return newList;
}

bool listExists(string s)
{
    for (int i = 0; i < (int)lists.size(); i++)
        if (lists.at(i).name() == s)
            return true;

    return false;
}

bool methodExists(string s)
{
    if (!zeroDots(s))
    {
        if (objectExists(beforeDot(s)))
        {
            if (getObject(beforeDot(s)).methodExists(afterDot(s)))
                return true;
            else
                return false;
        }
    }
    else
        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == s)
                return true;

    return false;
}

bool objectExists(string s)
{
    for (int i = 0; i < (int)objects.size(); i++)
        if (objects.at(i).name() == s)
            return true;

    return false;
}

bool variableExists(string s)
{
    if (!zeroDots(s))
    {
        string before(beforeDot(s)), after(afterDot(s));

        if (objectExists(before))
        {
            if (getObject(before).variableExists(after))
                return true;
            else
                return false;
        }
        else
            return false;
    }
    else
        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == s)
                return true;

    return false;
}

bool moduleExists(string s)
{
    for (int i = 0; i < (int)modules.size(); i++)
        if (modules.at(i).name() == s)
            return true;

    return false;
}

bool constantExists(string s)
{
    for (int i = 0; i < (int)constants.size(); i++)
        if (constants.at(i).name() == s)
            return true;

    return false;
}

bool noLists()
{
    if (lists.empty())
        return true;

    return false;
}

bool noMethods()
{
    if (methods.empty())
        return true;

    return false;
}

bool noObjects()
{
    if (objects.empty())
        return true;

    return false;
}

bool noVariables()
{
    if (variables.empty())
        return true;

    return false;
}

bool notObjectMethod(string s)
{
    if (zeroDots(s))
        return true;
    else
    {
        string before(beforeDot(s));

        if (objectExists(before))
            return false;
        else
            return true;
    }

    return true;
}

void __true()
{
    State.LastValue = "true";
}

void __false()
{
    State.LastValue = "false";
}

void saveVariable(string variableName)
{
    Crypt c;

    if (!Env::fileExists(State.SavedVars))
    {
        if (!Env::directoryExists(State.SavedVarsPath))
            Env::md(State.SavedVarsPath);

        Env::createFile(State.SavedVars);
        Env::app(State.SavedVars, c.e(variableName));
    }
    else
    {
        string line, bigStr("");
        ifstream file(State.SavedVars.c_str());

        if (file.is_open())
        {
            int i = 0;

            while (!file.eof())
            {
                i++;
                getline(file, line);
                bigStr.append(line);
            }

            bigStr = c.d(bigStr);
            Env::rm(State.SavedVars);
            Env::createFile(State.SavedVars);
            Env::app(State.SavedVars, c.e(bigStr + "#" + variableName));
            file.close();
        }
        else
            error(ErrorMessage::READ_FAIL, State.SavedVars, false);
    }
}

vector<Method> removeMethod(vector<Method> v, string target)
{
    vector<Method> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return cleanedVector;
}

vector<Object> removeObject(vector<Object> v, string target)
{
    vector<Object> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return cleanedVector;
}

vector<Variable> removeVariable(vector<Variable> v, string target)
{
    vector<Variable> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return cleanedVector;
}

vector<List> removeList(vector<List> v, string target)
{
    vector<List> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return cleanedVector;
}

vector<Module> removeModule(vector<Module> v, string target)
{
    vector<Module> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return cleanedVector;
}

vector<Constant> removeConstant(vector<Constant> v, string target)
{
    vector<Constant> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return cleanedVector;
}

void error(int errorType, string errorInfo, bool quit)
{
    string completeError("##\n# error:\t");
    completeError.append(Error::getErrorString(errorType));
    completeError.append(":\t");
    completeError.append(errorInfo);
    completeError.append("\n# line ");
    completeError.append(itos(State.CurrentLineNumber));
    completeError.append(":\t");
    completeError.append(State.CurrentLine);
    completeError.append("\n##\n");

    if (State.ExecutedTryBlock)
    {
        State.RaiseCatchBlock = true;
        State.LastError = completeError;
    }
    else
    {
        if (State.CaptureParse)
            State.ParsedOutput.append(completeError);
        else
            IO::printerr(completeError);
    }

    if (!State.Negligence)
    {
        if (quit)
        {
            clearAll();
            exit(0);
        }
    }
}

void help(string app)
{
    IO::println();
    IO::println("noctis by <scstauf@gmail.com>");
    IO::println();
    IO::println("usage:\t" + app + "\t\t\tstart the shell");
    IO::println("\t" + app + " {args}\t\tstart the shell with parameters");
    IO::println("\t" + app + " {script}\t\trun a script");
    IO::println("\t" + app + " {script} {args}\trun a script with parameters");
    IO::println("\t" + app + " -v, --version\tdisplay current version");
    IO::println("\t" + app + " -h, --help\t\tdisplay this message");
    IO::println("\t" + app + " -p, --parse\t\tparse a command");
    IO::println("\t" + app + " -n, --negligence\tignore parse errors");
    IO::println("\t" + app + " -sl, --skipload\tstart the shell without saved variables");
    IO::println("\t" + app + " -u, --uninstall\tremove $HOME/.savedVarsPath");
    IO::println();
}

bool notStandardZeroSpace(string arg)
{
    const char * standardZeroSpaceWords =
        "};break;caught;clear_all!;clear_constants!clear_lists!;clear_methods!;"
        "clear_objects!;clear_variables!;else;end;exit;failif;leave!;"
        "no_methods?;no_objects?;no_variables?;parser;pass;private;public;try";

    return !contains(standardZeroSpaceWords, arg);
}

bool notStandardOneSpace(string arg)
{
    const char * standardOneSpaceWords =
        "!;?;__begin__;call_method;cd;chdir;collect?;"
        "decrypt;delay;directory?;dpush;dpop;"
        "encrypt;err;error;file?;for;forget;fpush;fpop;"
        "garbage?;globalize;goto;if;init_dir;intial_directory;"
        "directory?;file?;list?;lowercase?;method?;"
        "number?;object?;string?;uppercase?;variable?;"
        "list;list?;load;lock;loop;lose;"
        "method;[method];object;out;"
        "print;println;prompt;remember;remove;return;"
        "save;say;see;see_string;see_number;stdout;switch;"
        "template;unlock;";

    return !contains(standardOneSpaceWords, arg);
}

bool notStandardTwoSpace(string arg)
{
    return !contains("=;+=;-=;*=;%=;/=;**=;+;-;*;**;/;%;++=;--=;?;!", arg);
}

int indexOfMethod(string s)
{
    for (int i = 0; i < (int)methods.size(); i++)
    {
        if (methods.at(i).name() == s)
            return i;
    }

    return -1;
}

Method getMethod(string s)
{
    return methods.at(indexOfMethod(s));
}

int indexOfObject(string s)
{
    for (int i = 0; i < (int)objects.size(); i++)
    {
        if (objects.at(i).name() == s)
            return i;
    }

    return -1;
}

Object getObject(string s)
{
    return objects.at(indexOfObject(s));
}

int indexOfVariable(string s)
{
    for (int i = 0; i < (int)variables.size(); i++)
    {
        if (variables.at(i).name() == s)
            return i;
    }

    return -1;
}

Variable getVar(string s)
{
    return variables.at(indexOfVariable(s));
}

string varString(string s)
{
    return getVar(s).getString();
}

double varNumber(string s)
{
    return getVar(s).getNumber();
}

string varNumberString(string s)
{
    return dtos(varNumber(s));
}

int indexOfList(string s)
{
    for (int i = 0; i < (int)lists.size(); i++)
    {
        if (lists.at(i).name() == s)
            return i;
    }

    return -1;
}

List getList(string s)
{
    return lists.at(indexOfList(s));
}

int indexOfModule(string s)
{
    for (int i = 0; i < (int)modules.size(); i++)
    {
        if (modules.at(i).name() == s)
            return i;
    }

    return -1;
}

Module getModule(string s)
{
    return modules.at(indexOfModule(s));
}

int indexOfScript(string s)
{
    for (int i = 0; i < (int)scripts.size(); i++)
    {
        if (scripts.at(i).name() == s)
            return i;
    }

    return -1;
}

Script getScript(string s)
{
    return scripts.at(indexOfScript(s));
}

int indexOfConstant(string s)
{
    for (int i = 0; i < (int)constants.size(); i++)
    {
        if (constants.at(i).name() == s)
            return i;
    }

    return -1;
}

Constant getConstant(string s)
{
    return constants.at(indexOfConstant(s));
}

bool is(string s, string si)
{
    return s == ("-" + si) || s == ("--" + si) || s == ("/" + si);
}

bool isScript(string path)
{
    return endsWith(path, ".ns");
}

void loadSavedVars(Crypt c, string &bigStr)
{
    string line("");
    ifstream file(State.SavedVars.c_str());

    if (file.is_open())
    {
        while (!file.eof())
        {
            getline(file, line);
            bigStr.append(line);
        }

        file.close();

        bigStr = c.d(bigStr);

        int bigStrLength = bigStr.length();
        bool stop = false;
        vector<string> varNames;
        vector<string> varValues;

        string varName("");
        varNames.push_back("");
        varValues.push_back("");

        for (int i = 0; i < bigStrLength; i++)
        {
            switch (bigStr[i])
            {
            case '&':
                stop = true;
                break;

            case '#':
                stop = false;
                varNames.push_back("");
                varValues.push_back("");
                break;

            default:
                if (!stop)
                    varNames.at((int)varNames.size() - 1).push_back(bigStr[i]);
                else
                    varValues.at((int)varValues.size() - 1).push_back(bigStr[i]);
                break;
            }
        }

        for (int i = 0; i < (int)varNames.size(); i++)
        {
            Variable newVariable(varNames.at(i), varValues.at(i));
            variables.push_back(newVariable);
        }

        varNames.clear();
        varValues.clear();
    }
    else
        error(ErrorMessage::READ_FAIL, State.SavedVars, false);
}

void runScript()
{
    for (int i = 0; i < getScript(State.CurrentScript).size(); i++)
    {
        State.CurrentLineNumber = i + 1;

        if (!State.GoToLabel)
            parse(getScript(State.CurrentScript).at(i));
        else
        {
            bool startParsing = false;
            State.DefiningIfStatement = false;
            State.DefiningForLoop = false;
            State.GoToLabel = false;

            for (int z = 0; z < getScript(State.CurrentScript).size(); z++)
            {
                if (endsWith(getScript(State.CurrentScript).at(z), "::"))
                {
                    string s(getScript(State.CurrentScript).at(z));
                    s = subtractString(s, "::");

                    if (s == State.GoTo)
                        startParsing = true;
                }

                if (startParsing)
                    parse(getScript(State.CurrentScript).at(z));
            }
        }
    }

    State.CurrentScript = State.PreviousScript;
}

void loadScript(string script)
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

    runScript();
}

void loop(bool skip)
{
    string s("");
    bool active = true;

    if (!skip)
    {
        Crypt c;
        string bigStr("");

        if (Env::fileExists(State.SavedVars))
            loadSavedVars(c, bigStr);
    }

    while (active)
    {
        s.clear();

        if (State.UseCustomPrompt)
        {
            if (State.PromptStyle == "bash")
                IO::print(Env::getUser() + "@" + Env::getMachine() + "(" + Env::cwd() + ")" + "$ ");
            else if (State.PromptStyle == "empty")
                doNothing();
            else
                IO::print(getPrompt());
        }
        else
            IO::print("> ");

        getline(cin, s, '\n');

        if (s[0] == '\t')
            s.erase(remove(s.begin(), s.end(), '\t'), s.end());

        if (s == "exit")
        {
            if (!State.DefiningObject && !State.DefiningMethod)
            {
                active = false;
                clearAll();
            }
            else
                parse(s);
        }
        else
        {
            char * c = new char[s.size() + 1];
            copy(s.begin(), s.end(), c);
            c[s.size()] = '\0';
            parse(trimLeadingWhitespace(c));
            delete[] c;
        }
    }
}

bool success()
{
    return State.SuccessFlag;
}

void failedFor()
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(false);
    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.DefaultLoopSymbol = "$";
    State.SuccessFlag = false;
}

void failedWhile()
{
    Method whileMethod("[while#" + itos(State.WhileLoopCount) + "]");
    whileMethod.setWhile(false);
    State.DefiningWhileLoop = true;
    whileLoops.push_back(whileMethod);
}

void successfulWhile(string v1, string op, string v2)
{
    Method whileMethod("[while#" + itos(State.WhileLoopCount) + "]");
    whileMethod.setWhile(true);
    whileMethod.setWhileValues(v1, op, v2);
    State.DefiningWhileLoop = true;
    whileLoops.push_back(whileMethod);
    State.WhileLoopCount++;
}

void successfulFor(List list)
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

void successfulFor(double a, double b, string op)
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

void successfulFor()
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setInfinite();
    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.ForLoopCount++;
    State.SuccessFlag = true;
}

void setFalseIf()
{
    State.LastValue = "false";

    if (!State.DefiningNest)
    {
        Method ifMethod("[failif]");
        ifMethod.setBool(false);
        State.DefiningIfStatement = true;
        ifStatements.push_back(ifMethod);
        State.FailedIfStatement = true;
        State.FailedNest = true;
    }
    else
        State.FailedNest = true;
}

void setTrueIf()
{
    State.LastValue = "true";

    if (State.DefiningNest)
    {
        ifStatements.at((int)ifStatements.size() - 1).buildNest();
        State.FailedNest = false;
    }
    else
    {
        Method ifMethod("[if#" + itos(State.IfStatementCount) +"]");
        ifMethod.setBool(true);
        State.DefiningIfStatement = true;
        ifStatements.push_back(ifMethod);
        State.IfStatementCount++;
        State.FailedIfStatement = false;
        State.FailedNest = false;
    }
}

bool stackReady(string arg2)
{
    if (contains(arg2, "+") || contains(arg2, "-") || contains(arg2, "*") || contains(arg2, "/") || contains(arg2, "%") || contains(arg2, "^"))
        return true;

    return false;
}

bool isStringStack(string arg2)
{
    string tempArgTwo = arg2, temporaryBuild("");
    tempArgTwo = subtractChar(tempArgTwo, "(");
    tempArgTwo = subtractChar(tempArgTwo, ")");

    for (int i = 0; i < (int)tempArgTwo.length(); i++)
    {
        if (tempArgTwo[i] == ' ')
        {
            if (temporaryBuild.length() != 0)
            {
                if (variableExists(temporaryBuild))
                {
                    if (isNumber(temporaryBuild))
                        temporaryBuild.clear();
                    else if (isString(temporaryBuild))
                        return true;
                }
                else if (methodExists(temporaryBuild))
                {
                    parse(temporaryBuild);

                    if (isNumeric(State.LastValue))
                        temporaryBuild.clear();
                    else
                        return true;
                }
                else
                    temporaryBuild.clear();
            }
        }
        else if (tempArgTwo[i] == '+')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (isString(temporaryBuild))
                    return true;
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!isNumeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '-')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (isString(temporaryBuild))
                    return true;
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!isNumeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '*')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (isString(temporaryBuild))
                    return true;
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!isNumeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '/')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (isString(temporaryBuild))
                    return true;
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!isNumeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '%')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (isString(temporaryBuild))
                    return true;
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!isNumeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '^')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (isString(temporaryBuild))
                    return true;
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!isNumeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else
            temporaryBuild.push_back(tempArgTwo[i]);
    }

    return false;
}

string getStringStack(string arg2)
{
    string tempArgTwo = arg2, temporaryBuild("");
    tempArgTwo = subtractChar(tempArgTwo, "(");
    tempArgTwo = subtractChar(tempArgTwo, ")");

    string stackValue("");

    vector<string> vars;
    vector<string> contents;

    bool quoted = false;

    for (int i = 0; i < (int)tempArgTwo.length(); i++)
    {
        if (tempArgTwo[i] == '\"')
        {
            quoted = !quoted;
            if (!quoted)
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
            }
        }
        else if (tempArgTwo[i] == ' ')
        {
            if (quoted)
            {
                temporaryBuild.push_back(' ');
            }
            else
            {
                if (temporaryBuild.length() != 0)
                {
                    if (variableExists(temporaryBuild))
                    {
                        if (isNumber(temporaryBuild))
                        {
                            vars.push_back(temporaryBuild);
                            contents.push_back(dtos(varNumber(temporaryBuild)));
                            temporaryBuild.clear();
                        }
                        else if (isString(temporaryBuild))
                        {
                            vars.push_back(temporaryBuild);
                            contents.push_back(varString(temporaryBuild));
                            temporaryBuild.clear();
                        }
                    }
                    else if (methodExists(temporaryBuild))
                    {
                        parse(temporaryBuild);

                        contents.push_back(State.LastValue);
                        temporaryBuild.clear();
                    }
                    else
                    {
                        contents.push_back(temporaryBuild);
                        temporaryBuild.clear();
                    }
                }
            }
        }
        else if (tempArgTwo[i] == '+')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("+");
                }
                else if (isString(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(varString(temporaryBuild));
                    temporaryBuild.clear();
                    contents.push_back("+");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                contents.push_back(State.LastValue);
                temporaryBuild.clear();

                contents.push_back("+");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("+");
            }
        }
        else if (tempArgTwo[i] == '-')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("-");
                }
                else if (isString(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(varString(temporaryBuild));
                    temporaryBuild.clear();
                    contents.push_back("-");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                contents.push_back(State.LastValue);
                temporaryBuild.clear();

                contents.push_back("-");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("-");
            }
        }
        else if (tempArgTwo[i] == '*')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("*");
                }
                else if (isString(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(varString(temporaryBuild));
                    temporaryBuild.clear();
                    contents.push_back("*");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                contents.push_back(State.LastValue);
                temporaryBuild.clear();

                contents.push_back("*");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("*");
            }
        }
        else
            temporaryBuild.push_back(tempArgTwo[i]);
    }

    if (variableExists(temporaryBuild))
    {
        if (isNumber(temporaryBuild))
        {
            vars.push_back(temporaryBuild);
            contents.push_back(dtos(varNumber(temporaryBuild)));
            temporaryBuild.clear();
        }
        else if (isString(temporaryBuild))
        {
            vars.push_back(temporaryBuild);
            contents.push_back(varString(temporaryBuild));
            temporaryBuild.clear();
        }
    }
    else
    {
        contents.push_back(temporaryBuild);
        temporaryBuild.clear();
    }

    bool startOperating = false,
         addNext = false,
         subtractNext = false,
         multiplyNext = false;

    for (int i = 0; i < (int)contents.size(); i++)
    {
        if (startOperating)
        {
            if (addNext)
            {
                stackValue.append(contents.at(i));
                addNext = false;
            }
            else if (subtractNext)
            {
                stackValue = subtractString(stackValue, contents.at(i));
                subtractNext = false;
            }
            else if (multiplyNext)
            {
                if (isNumeric(contents.at(i)))
                {
                    string appendage(stackValue);

                    for (int z = 1; z < stoi(contents.at(i)); z++)
                        stackValue.append(appendage);
                }

                multiplyNext = false;
            }

            if (contents.at(i) == "+")
                addNext = true;
            else if (contents.at(i) == "-")
                subtractNext = true;
            else if (contents.at(i) == "*")
                multiplyNext = true;
        }
        else
        {
            startOperating = true;
            stackValue = contents.at(i);
        }
    }

    if (State.Returning)
    {
        for (int i = 0; i < (int)vars.size(); i++)
            variables = removeVariable(variables, vars.at(i));

        State.Returning = false;
    }

    return stackValue;
}

double getStack(string arg2)
{
    string tempArgTwo = arg2, temporaryBuild("");
    tempArgTwo = subtractChar(tempArgTwo, "(");
    tempArgTwo = subtractChar(tempArgTwo, ")");

    double stackValue = (double)0.0;

    vector<string> contents;
    vector<string> vars;

    for (int i = 0; i < (int)tempArgTwo.length(); i++)
    {
        if (tempArgTwo[i] == ' ')
        {
            if (temporaryBuild.length() != 0)
            {
                if (variableExists(temporaryBuild))
                {
                    if (isNumber(temporaryBuild))
                    {
                        vars.push_back(temporaryBuild);
                        contents.push_back(dtos(varNumber(temporaryBuild)));
                        temporaryBuild.clear();
                    }
                }
                else if (methodExists(temporaryBuild))
                {
                    parse(temporaryBuild);

                    if (isNumeric(State.LastValue))
                    {
                        contents.push_back(State.LastValue);
                        temporaryBuild.clear();
                    }
                }
                else
                {
                    contents.push_back(temporaryBuild);
                    temporaryBuild.clear();
                }
            }
        }
        else if (tempArgTwo[i] == '+')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("+");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                {
                    contents.push_back(State.LastValue);
                    temporaryBuild.clear();
                }

                contents.push_back("+");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("+");
            }
        }
        else if (tempArgTwo[i] == '-')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("-");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                {
                    contents.push_back(State.LastValue);
                    temporaryBuild.clear();
                }
                contents.push_back("-");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("-");
            }
        }
        else if (tempArgTwo[i] == '*')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("*");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                {
                    contents.push_back(State.LastValue);
                    temporaryBuild.clear();
                }

                contents.push_back("*");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("*");
            }
        }
        else if (tempArgTwo[i] == '/')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("/");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                {
                    contents.push_back(State.LastValue);
                    temporaryBuild.clear();
                }

                contents.push_back("/");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("/");
            }
        }
        else if (tempArgTwo[i] == '%')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("%");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                {
                    contents.push_back(State.LastValue);
                    temporaryBuild.clear();
                }
                contents.push_back("%");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("%");
            }
        }
        else if (tempArgTwo[i] == '^')
        {
            if (variableExists(temporaryBuild))
            {
                if (isNumber(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(varNumber(temporaryBuild)));
                    temporaryBuild.clear();
                    contents.push_back("^");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(State.LastValue))
                {
                    contents.push_back(State.LastValue);
                    temporaryBuild.clear();
                }
                contents.push_back("^");
            }
            else
            {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("^");
            }
        }
        else
            temporaryBuild.push_back(tempArgTwo[i]);
    }

    if (variableExists(temporaryBuild))
    {
        if (isNumber(temporaryBuild))
        {
            vars.push_back(temporaryBuild);
            contents.push_back(dtos(varNumber(temporaryBuild)));
            temporaryBuild.clear();
        }
    }
    else
    {
        contents.push_back(temporaryBuild);
        temporaryBuild.clear();
    }

    bool startOperating = false,
         addNext = false,
         subtractNext = false,
         multiplyNext = false,
         divideNext = false,
         moduloNext = false,
         powerNext = false;

    for (int i = 0; i < (int)contents.size(); i++)
    {
        if (startOperating)
        {
            if (addNext)
            {
                stackValue += stod(contents.at(i));
                addNext = false;
            }
            else if (subtractNext)
            {
                stackValue -= stod(contents.at(i));
                subtractNext = false;
            }
            else if (multiplyNext)
            {
                stackValue *= stod(contents.at(i));
                multiplyNext = false;
            }
            else if (divideNext)
            {
                stackValue /= stod(contents.at(i));
                divideNext = false;
            }
            else if (moduloNext)
            {
                stackValue = ((int)stackValue % (int)stod(contents.at(i)));
                moduloNext = false;
            }
            else if (powerNext)
            {
                stackValue = pow(stackValue, (int)stod(contents.at(i)));
                powerNext = false;
            }

            if (contents.at(i) == "+")
                addNext = true;
            else if (contents.at(i) == "-")
                subtractNext = true;
            else if (contents.at(i) == "*")
                multiplyNext = true;
            else if (contents.at(i) == "/")
                divideNext = true;
            else if (contents.at(i) == "%")
                moduloNext = true;
            else if (contents.at(i) == "^")
                powerNext = true;
        }
        else
        {
            if (isNumeric(contents.at(i)))
            {
                startOperating = true;
                stackValue = stod(contents.at(i));
            }
        }
    }

    if (State.Returning)
    {
        for (int i = 0; i < (int)vars.size(); i++)
            variables = removeVariable(variables, vars.at(i));

        State.Returning = false;
    }

    return stackValue;
}

/**
	Give a new name to anything.
**/
void redefine(string target, string name)
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

string getSubString(string arg1, string arg2, string beforeBracket)
{
    string returnValue("");

    if (isString(beforeBracket))
    {
        vector<string> listRange = getBracketRange(arg2);

        string variableString = varString(beforeBracket);

        if (listRange.size() == 2)
        {
            string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

            if (rangeBegin.length() != 0 && rangeEnd.length() != 0)
            {
                if (isNumeric(rangeBegin) && isNumeric(rangeEnd))
                {
                    if (stoi(rangeBegin) < stoi(rangeEnd))
                    {
                        if ((int)variableString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                        {
                            string tempString("");

                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                tempString.push_back(variableString[i]);

                            returnValue = tempString;
                        }
                        else
                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                    }
                    else if (stoi(rangeBegin) > stoi(rangeEnd))
                    {
                        if ((int)variableString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                        {
                            string tempString("");

                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                tempString.push_back(variableString[i]);

                            returnValue = tempString;
                        }
                        else
                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                    }
                    else
                        error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                }
                else
                    error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
            }
            else
                error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
        }
        else if (listRange.size() == 1)
        {
            string rangeBegin(listRange.at(0));

            if (rangeBegin.length() != 0)
            {
                if (isNumeric(rangeBegin))
                {
                    if ((int)variableString.length() - 1 >= stoi(rangeBegin) && stoi(rangeBegin) >= 0)
                    {
                        string tmp_("");
                        tmp_.push_back(variableString[stoi(rangeBegin)]);

                        returnValue = tmp_;
                    }
                }
            }
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
    }
    else
        error(ErrorMessage::NULL_STRING, beforeBracket, false);

    return returnValue;
}

void setSubString(string arg1, string arg2, string beforeBracket)
{
    if (isString(beforeBracket))
    {
        vector<string> listRange = getBracketRange(arg2);

        string variableString = varString(beforeBracket);

        if (listRange.size() == 2)
        {
            string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

            if (rangeBegin.length() != 0 && rangeEnd.length() != 0)
            {
                if (isNumeric(rangeBegin) && isNumeric(rangeEnd))
                {
                    if (stoi(rangeBegin) < stoi(rangeEnd))
                    {
                        if ((int)variableString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                        {
                            string tempString("");

                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                tempString.push_back(variableString[i]);

                            if (variableExists(arg1))
                                setVariable(arg1, tempString);
                            else
                                createVariable(arg1, tempString);
                        }
                        else
                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                    }
                    else if (stoi(rangeBegin) > stoi(rangeEnd))
                    {
                        if ((int)variableString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                        {
                            string tempString("");

                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                tempString.push_back(variableString[i]);

                            if (variableExists(arg1))
                                setVariable(arg1, tempString);
                            else
                                createVariable(arg1, tempString);
                        }
                        else
                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                    }
                    else
                        error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                }
                else
                    error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
            }
            else
                error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
        }
        else if (listRange.size() == 1)
        {
            string rangeBegin(listRange.at(0));

            if (rangeBegin.length() != 0)
            {
                if (isNumeric(rangeBegin))
                {
                    if ((int)variableString.length() - 1 >= stoi(rangeBegin) && stoi(rangeBegin) >= 0)
                    {
                        string tmp_("");
                        tmp_.push_back(variableString[stoi(rangeBegin)]);

                        if (variableExists(arg1))
                            setVariable(arg1, tmp_);
                        else
                            createVariable(arg1, tmp_);
                    }
                }
            }
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
    }
    else
        error(ErrorMessage::NULL_STRING, beforeBracket, false);
}

bool isNumber(Variable var)
{
	return var.getNumber() != State.NullNum;
}

bool isNumber(string varName)
{
    return varNumber(varName) != State.NullNum;
}

bool isString(Variable var)
{
	return var.getString() != State.Null;
}

bool isString(string varName)
{
    return varString(varName) != State.Null;
}

double getNumber(string varName)
{
    return varNumber(varName);
}

string getString(string varName)
{
    return varString(varName);
}

bool secondIsNumber(string s)
{
    if (variableExists(s))
    {
        if (isNumber(s))
            return true;
    }
    else if (stackReady(s))
    {
        if (!isStringStack(s))
            return true;
    }
    else
    {
        if (isNumeric(s))
            return true;
    }

    return false;
}

string getStringValue(string arg1, string op, string arg2)
{
    string firstValue(""), lastValue(""), returnValue("");

    if (variableExists(arg1))
    {
        if (isString(arg1))
            firstValue = getString(arg1);
    }

    if (variableExists(arg2))
    {
        if (isString(arg2))
            lastValue = getString(arg2);
        else if (isNumber(arg2))
            lastValue = dtos(getNumber(arg2));
    }
    else if (methodExists(arg2))
    {
        parse(arg2);

        lastValue = lastValue;
    }
    else if (!zeroDots(arg2))
    {
        string _beforeDot(beforeDot(arg2)), _afterDot(afterDot(arg2));

        if (_beforeDot == "env")
        {
            InternalGetEnv("", _afterDot, 2);
        }
        else if (_beforeDot == "args")
        {
            if (_afterDot == "size")
                lastValue = itos(args.size());
            else
                lastValue = "";
        }
        else if (objectExists(_beforeDot))
        {
            executeTemplate(getObject(_beforeDot).getMethod(_afterDot), getParams(_afterDot));

            lastValue = lastValue;
        }
        else
            lastValue = arg2;
    }
    else if (containsBrackets(arg2))
    {
        string _beforeBrackets(beforeBrackets(arg2)), _afterBrackets(afterBrackets(arg2));

        if (_beforeBrackets == "args")
        {
            vector<string> params = getBracketRange(_afterBrackets);

            if (isNumeric(params.at(0)))
            {
                if ((int)args.size() - 1 >= stoi(params.at(0)) && stoi(params.at(0)) >= 0)
                {
                    if (params.at(0) == "0")
                        lastValue = State.CurrentScript;
                    else
                        lastValue = args.at(stoi(params.at(0)));
                }
                else
                    lastValue = "";
            }
            else
                lastValue = "";
        }
        else if (listExists(_beforeBrackets))
        {
            _afterBrackets = subtractString(_afterBrackets, "]");

            if (getList(_beforeBrackets).size() >= stoi(_afterBrackets))
            {
                if (stoi(_afterBrackets) >= 0)
                    lastValue = getList(_beforeBrackets).at(stoi(_afterBrackets));
                else
                    lastValue = "";
            }
            else
                lastValue = "";
        }
    }
    else if (containsParams(arg2))
    {
        if (beforeParams(arg2).length() != 0)
        {
            executeTemplate(getMethod(arg2), getParams(arg2));

            lastValue = lastValue;
        }
        else
        {
            if (isStringStack(arg2))
                lastValue = getStringStack(arg2);
            else if (stackReady(arg2))
                lastValue = dtos(getStack(arg2));
        }
    }
    else
        lastValue = arg2;

    if (op == "+=")
        returnValue = (firstValue + lastValue);
    else if (op == "-=")
        returnValue = subtractString(firstValue, lastValue);
    else if (op == "*=")
    {
        if (isNumeric(lastValue))
        {
            string bigString("");

            for (int i = 0; i < (int)stod(lastValue); i++)
                bigString.append(firstValue);

            returnValue = bigString;
        }
    }
    else if (op == "/=")
        returnValue = subtractString(firstValue, lastValue);
    else if (op == "**=")
        returnValue = dtos(pow(stod(firstValue), stod(lastValue)));
    else if (op == "=")
        returnValue = lastValue;

    State.LastValue = returnValue;
    return returnValue;
}

double getNumberValue(string arg1, string op, string arg2)
{
    double firstValue = 0, lastValue = 0, returnValue = 0;

    if (variableExists(arg1))
    {
        if (isNumber(arg1))
            firstValue = getNumber(arg1);
    }

    if (variableExists(arg2))
    {
        if (isNumber(arg2))
            lastValue = getNumber(arg2);
        else
            lastValue = 0;
    }
    else if (methodExists(arg2))
    {
        parse(arg2);

        if (isNumeric(State.LastValue))
            lastValue = stod(State.LastValue);
        else
            lastValue = 0;
    }
    else if (!zeroDots(arg2))
    {
        string _beforeDot(beforeDot(arg2)), _afterDot(afterDot(arg2));
        if (_beforeDot == "env")
        {
            InternalGetEnv("", _afterDot, 2);
        }
        else if (_beforeDot == "args")
        {
            if (_afterDot == "size")
                lastValue = stod(itos(args.size()));
            else
                lastValue = 0;
        }
        else if (objectExists(_beforeDot))
        {
            executeTemplate(getObject(_beforeDot).getMethod(_afterDot), getParams(_afterDot));

            if (isNumeric(State.LastValue))
                lastValue = stod(State.LastValue);
            else
                lastValue = 0;
        }
        else
        {
            if (isNumeric(State.LastValue))
                lastValue = stod(arg2);
            else
                lastValue = 0;
        }
    }
    else if (containsBrackets(arg2))
    {
        string _beforeBrackets(beforeBrackets(arg2)), _afterBrackets(afterBrackets(arg2));

        if (listExists(_beforeBrackets))
        {
            _afterBrackets = subtractString(_afterBrackets, "]");

            if (getList(_beforeBrackets).size() >= stoi(_afterBrackets))
            {
                if (stoi(_afterBrackets) >= 0)
                {
                    if (isNumeric(getList(_beforeBrackets).at(stoi(_afterBrackets))))
                        lastValue = stod(getList(_beforeBrackets).at(stoi(_afterBrackets)));
                    else
                        lastValue = 0;
                }
                else
                    lastValue = 0;
            }
            else
                lastValue = 0;
        }
    }
    else if (containsParams(arg2))
    {
        if (beforeParams(arg2).length() != 0)
        {
            executeTemplate(getMethod(arg2), getParams(arg2));

            if (isNumeric(State.LastValue))

                lastValue = stod(State.LastValue);
            else
                lastValue = 0;
        }
        else
        {
            if (stackReady(arg2))
                lastValue = getStack(arg2);
            else
                lastValue = 0;
        }
    }
    else
    {
        if (isNumeric(arg2))
            lastValue = stod(arg2);
        else
            lastValue = 0;
    }

    if (op == "+=")
        returnValue = (firstValue + lastValue);
    else if (op == "-=")
        returnValue = (firstValue - lastValue);
    else if (op == "*=")
        returnValue = (firstValue * lastValue);
    else if (op == "/=")
        returnValue = (firstValue / lastValue);
    else if (op == "**=")
        returnValue = pow(firstValue, lastValue);
    else if (op == "=")
        returnValue = lastValue;

    State.LastValue = dtos(returnValue);
    return returnValue;
}

void whileLoop(Method m)
{
    for (int i = 0; i < m.size(); i++)
    {
        if (m.at(i) == "leave!")
            State.Breaking = true;
        else
            parse(m.at(i));
    }
}

void forLoop(Method m)
{
    State.DefaultLoopSymbol = "$";

    if (m.isListLoop())
    {
        int i = 0, stop = m.getList().size();

        while (i < stop)
        {
            for (int z = 0; z < m.size(); z++)
            {
                string cleaned(""), builder("");
                int len = m.at(z).length();
                bool buildSymbol = false, almostBuild = false, ended = false;

                for (int a = 0; a < len; a++)
                {
                    if (almostBuild)
                    {
                        if (m.at(z)[a] == '{')
                            buildSymbol = true;
                    }

                    if (buildSymbol)
                    {
                        if (m.at(z)[a] == '}')
                        {
                            almostBuild = false,
                            buildSymbol = false;
                            ended = true;

                            builder = subtractString(builder, "{");

                            if (builder == m.getSymbolString())
                            {
                                cleaned.append(m.getList().at(i));
                            }

                            builder.clear();
                        }
                        else
                        {
                            builder.push_back(m.at(z)[a]);
                        }
                    }

                    if (m.at(z)[a] == '$')
                    {
                        almostBuild = true;
                    }

                    if (!almostBuild && !buildSymbol)
                    {
                        if (ended)
                        {
                            ended = false;
                        }
                        else
                        {
                            cleaned.push_back(m.at(z)[a]);
                        }
                    }
                }

                parse(cleaned);
            }

            i++;

            if (State.Breaking == true)
            {
                State.Breaking = false;
                break;
            }
        }
    }
    else
    {
        if (m.isInfinite())
        {
            if (State.Negligence)
            {
                for (;;)
                {
                    for (int z = 0; z < m.size(); z++)
                        parse(m.at(z));

                    if (State.Breaking == true)
                    {
                        State.Breaking = false;
                        break;
                    }
                }
            }
            else
                error(ErrorMessage::INFINITE_LOOP, "", true);
        }
        else if (m.start() < m.stop())
        {
            int start = m.start(), stop = m.stop();

            while (start <= stop)
            {
                for (int z = 0; z < m.size(); z++)
                {
                    string cleanString(""), builder(""), tmp(m.at(z));
                    int l(tmp.length());
                    bool buildSymbol = false, almostBuild = false, ended = false;

                    for (int a = 0; a < l; a++)
                    {
                        if (almostBuild)
                        {
                            if (tmp[a] == '{')
                                buildSymbol = true;
                        }

                        if (buildSymbol)
                        {
                            if (tmp[a] == '}')
                            {
                                almostBuild = false,
                                buildSymbol = false;
                                ended = true;

                                builder = subtractString(builder, "{");

                                if (builder == m.getSymbolString())
                                    cleanString.append(itos(start));

                                builder.clear();
                            }
                            else
                                builder.push_back(tmp[a]);
                        }

                        if (tmp[a] == '$')
                            almostBuild = true;

                        if (!almostBuild && !buildSymbol)
                        {
                            if (ended)
                                ended = false;
                            else
                                cleanString.push_back(tmp[a]);
                        }
                    }

                    parse(cleanString);
                }

                start++;

                if (State.Breaking == true)
                {
                    State.Breaking = false;
                    break;
                }
            }
        }
        else if (m.start() > m.stop())
        {
            int start = m.start(), stop = m.stop();

            while (start >= stop)
            {
                for (int z = 0; z < m.size(); z++)
                {
                    string cleaned(""), builder(""), tmp(m.at(z));
                    int l(tmp.length());
                    bool buildSymbol = false, almostBuild = false, ended = false;

                    for (int a = 0; a < l; a++)
                    {
                        if (almostBuild)
                        {
                            if (tmp[a] == '{')
                                buildSymbol = true;
                        }

                        if (buildSymbol)
                        {
                            if (tmp[a] == '}')
                            {
                                almostBuild = false,
                                buildSymbol = false;
                                ended = true;

                                builder = subtractString(builder, "{");

                                if (builder == m.getSymbolString())
                                    cleaned.append(itos(start));

                                builder.clear();
                            }
                            else
                                builder.push_back(tmp[a]);
                        }

                        if (tmp[a] == '$')
                            almostBuild = true;

                        if (!almostBuild && !buildSymbol)
                        {
                            if (ended)
                                ended = false;
                            else
                                cleaned.push_back(tmp[a]);
                        }
                    }

                    parse(cleaned);
                }

                start--;

                if (State.Breaking == true)
                {
                    State.Breaking = false;
                    break;
                }
            }
        }
    }
}

void executeNest(Container n)
{
    State.DefiningNest = false;
    State.DefiningIfStatement = false;

    for (int i = 0; i < n.size(); i++)
    {
        if (State.FailedNest == false)
            parse(n.at(i));
        else
            break;
    }

    State.DefiningIfStatement = true;
}

void executeTemplate(Method m, vector<string> strings)
{
    vector<string> methodLines;

    State.ExecutedTemplate = true;
    State.DontCollectMethodVars = true;
    State.CurrentMethodObject = m.getObject();

    vector<Variable> methodVariables = m.getMethodVariables();

    for (int i = 0; i < (int)methodVariables.size(); i++)
    {
        if (variableExists(strings.at(i)))
        {
            if (isString(strings.at(i)))
                createVariable(methodVariables.at(i).name(), varString(strings.at(i)));
            else if (isNumber(strings.at(i)))
                createVariable(methodVariables.at(i).name(), varNumber(strings.at(i)));
        }
        else if (methodExists(strings.at(i)))
        {
            parse(strings.at(i));

            if (isNumeric(State.LastValue))
                createVariable(methodVariables.at(i).name(), stod(State.LastValue));
            else
                createVariable(methodVariables.at(i).name(), State.LastValue);
        }
        else
        {
            if (isNumeric(strings.at(i)))
                createVariable(methodVariables.at(i).name(), stod(strings.at(i)));
            else
                createVariable(methodVariables.at(i).name(), strings.at(i));
        }
    }

    for (int i = 0; i < (int)m.size(); i++)
    {
        string line = m.at(i), word("");
        int len = line.length();
        vector<string> words;

        for (int x = 0; x < len; x++)
        {
            if (line[x] == ' ')
            {
                words.push_back(word);
                word.clear();
            }
            else
                word.push_back(line[x]);
        }

        words.push_back(word);

        vector<string> newWords;

        for (int x = 0; x < (int)words.size(); x++)
        {
            bool found = false;

            for (int a = 0; a < (int)strings.size(); a++)
            {
                string variableString("$");
                variableString.append(itos(a));

                if (words.at(x) == variableString)
                {
                    found = true;

                    newWords.push_back(strings.at(a));
                }
            }

            if (!found)
                newWords.push_back(words.at(x));
        }

        string freshLine("");

        for (int b = 0; b < (int)newWords.size(); b++)
        {
            freshLine.append(newWords.at(b));

            if (b != (int)newWords.size() - 1)
                freshLine.push_back(' ');
        }

        methodLines.push_back(freshLine);
    }

    for (int i = 0; i < (int)methodLines.size(); i++)
        parse(methodLines.at(i));

    State.ExecutedTemplate = false, State.DontCollectMethodVars = false;

    collectGarbage(); // if (!State.DontCollectMethodVars)
}

void executeMethod(Method m)
{
    State.ExecutedMethod = true;
    State.CurrentMethodObject = m.getObject();

    if (State.DefiningParameterizedMethod)
    {
        vector<string> methodLines;

        for (int i = 0; i < (int)m.size(); i++)
        {
            string line = m.at(i), word("");
            int len = line.length();
            vector<string> words;

            for (int x = 0; x < len; x++)
            {
                if (line[x] == ' ')
                {
                    words.push_back(word);
                    word.clear();
                }
                else
                    word.push_back(line[x]);
            }

            words.push_back(word);

            vector<string> newWords;

            for (int x = 0; x < (int)words.size(); x++)
            {
                bool found = false;

                for (int a = 0; a < (int)m.getMethodVariables().size(); a++)
                {
                    string variableString("$");
                    variableString.append(itos(a));

                    if (words.at(x) == m.getMethodVariables().at(a).name())
                    {
                        found = true;

                        if (m.getMethodVariables().at(a).getString() != State.Null)
                            newWords.push_back(m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getNumber() != State.NullNum)
                            newWords.push_back(dtos(m.getMethodVariables().at(a).getNumber()));
                    }
                    else if (words.at(x) == variableString)
                    {
                        found = true;

                        if (m.getMethodVariables().at(a).getString() != State.Null)
                            newWords.push_back(m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getNumber() != State.NullNum)
                            newWords.push_back(dtos(m.getMethodVariables().at(a).getNumber()));
                    }
                }

                if (!found)
                    newWords.push_back(words.at(x));
            }

            string freshLine("");

            for (int b = 0; b < (int)newWords.size(); b++)
            {
                freshLine.append(newWords.at(b));

                if (b != (int)newWords.size() - 1)
                    freshLine.push_back(' ');
            }

            methodLines.push_back(freshLine);
        }

        for (int i = 0; i < (int)methodLines.size(); i++)
            parse(methodLines.at(i));
    }
    else
        for (int i = 0; i < m.size(); i++)
            parse(m.at(i));

    State.ExecutedMethod = false;

    collectGarbage();
}

void initializeVariable(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    string tmpObjName = beforeDot(arg0), tmpVarName = afterDot(arg0);
    bool tmpObjExists = objectExists(tmpObjName);
    if (tmpObjExists || startsWith(arg0, "@"))
    {
        if (tmpObjExists)
        {
            if (getObject(tmpObjName).getVariable(tmpVarName).getString() != State.Null)
            {
                string tempObjectVariableName("@ " + tmpObjName + tmpVarName + "_string");

                createVariable(tempObjectVariableName, getObject(tmpObjName).getVariable(tmpVarName).getString());

                twoSpace(tempObjectVariableName, arg1, arg2, "", command);

                getVar(tempObjectVariableName).setName(tmpVarName);

                getObject(tmpObjName).removeVariable(tmpVarName);
                getObject(tmpObjName).addVariable(getVar(tmpVarName));
                variables = removeVariable(variables, tmpVarName);
            }
            else if (getObject(tmpObjName).getVariable(tmpVarName).getNumber() != State.NullNum)
            {
                string tempObjectVariableName("@____" + beforeDot(arg0) + "___" + afterDot(arg0) + "_number");

                createVariable(tempObjectVariableName, getObject(beforeDot(arg0)).getVariable(afterDot(arg0)).getNumber());

                twoSpace(tempObjectVariableName, arg1, arg2, tempObjectVariableName + " " + arg1 + " " + arg2, command);

                getVar(tempObjectVariableName).setName(afterDot(arg0));

                getObject(beforeDot(arg0)).removeVariable(afterDot(arg0));
                getObject(beforeDot(arg0)).addVariable(getVar(afterDot(arg0)));
                variables = removeVariable(variables, afterDot(arg0));
            }
        }
        else if (arg1 == "=")
        {
            string before(beforeDot(arg2)), after(afterDot(arg2));

            if (containsBrackets(arg2) && (variableExists(beforeBrackets(arg2)) || listExists(beforeBrackets(arg2))))
            {
                string beforeBracket(beforeBrackets(arg2)), afterBracket(afterBrackets(arg2));

                afterBracket = subtractString(afterBracket, "]");

                if (listExists(beforeBracket))
                {
                    if (getList(beforeBracket).size() >= stoi(afterBracket))
                    {
                        if (getList(beforeBracket).at(stoi(afterBracket)) == "#!=no_line")
                            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
                        else
                        {
                            string listValue(getList(beforeBracket).at(stoi(afterBracket)));

                            if (isNumeric(listValue))
                            {
                                if (isNumber(arg0))
                                    setVariable(arg0, stod(listValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                            {
                                if (isString(arg0))
                                    setVariable(arg0, listValue);
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                        }
                    }
                }
                else if (isString(beforeBracket))
                    setSubString(arg0, arg2, beforeBracket);
                else
                    error(ErrorMessage::LIST_UNDEFINED, beforeBracket, false);
            }
            else if (before.length() != 0 && after.length() != 0)
            {
                if (containsParams(arg2))
                {
                    if (beforeParams(arg2) == "random")
                    {
                        if (!contains(arg2, ".."))
                        {
                            error(ErrorMessage::INVALID_SEQ_SEP, arg2, false);
                            return;
                        }

                        vector<string> range = getRange(arg2);
                        string s0(range.at(0)), s2(range.at(1));

                        if (isNumeric(s0) && isNumeric(s2))
                        {
                            if (isNumber(arg0))
                            {
                                double n0 = stod(s0), n2 = stod(s2);

                                if (n0 < n2)
                                    setVariable(arg0, (int)random(n0, n2));
                                else if (n0 > n2)
                                    setVariable(arg0, (int)random(n2, n0));
                                else
                                    setVariable(arg0, (int)random(n0, n2));
                            }
                            else if (isString(arg0))
                            {
                                double n0 = stod(s0), n2 = stod(s2);

                                if (n0 < n2)
                                    setVariable(arg0, itos((int)random(n0, n2)));
                                else if (n0 > n2)
                                    setVariable(arg0, itos((int)random(n2, n0)));
                                else
                                    setVariable(arg0, itos((int)random(n0, n2)));
                            }
                        }
                        else if (isAlpha(s0) && isAlpha(s2))
                        {
                            if (!isString(arg0))
                            {
                                error(ErrorMessage::NULL_STRING, arg0, false);
                                return;
                            }

                            if (get_alpha_num(s0[0]) < get_alpha_num(s2[0]))
                                setVariable(arg0, random(s0, s2));
                            else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0]))
                                setVariable(arg0, random(s2, s0));
                            else
                                setVariable(arg0, random(s2, s0));
                        }
                        else if (variableExists(s0) || variableExists(s2))
                        {
                            if (variableExists(s0))
                            {
                                if (isNumber(s0))
                                    s0 = dtos(varNumber(s0));
                                else if (isString(s0))
                                    s0 = varString(s0);
                            }

                            if (variableExists(s2))
                            {
                                if (isNumber(s2))
                                    s2 = varNumberString(s2);
                                else if (isString(s2))
                                    s2 = varString(s2);
                            }

                            if (isNumeric(s0) && isNumeric(s2))
                            {
                                if (isNumber(arg0))
                                {
                                    double n0 = stod(s0), n2 = stod(s2);

                                    if (n0 < n2)
                                        setVariable(arg0, (int)random(n0, n2));
                                    else if (n0 > n2)
                                        setVariable(arg0, (int)random(n2, n0));
                                    else
                                        setVariable(arg0, (int)random(n0, n2));
                                }
                                else if (isString(arg0))
                                {
                                    double n0 = stod(s0), n2 = stod(s2);

                                    if (n0 < n2)
                                        setVariable(arg0, itos((int)random(n0, n2)));
                                    else if (n0 > n2)
                                        setVariable(arg0, itos((int)random(n2, n0)));
                                    else
                                        setVariable(arg0, itos((int)random(n0, n2)));
                                }
                            }
                            else if (isAlpha(s0) && isAlpha(s2))
                            {
                                if (!isString(arg0))
                                {
                                    error(ErrorMessage::NULL_STRING, arg0, false);
                                    return;
                                }

                                if (get_alpha_num(s0[0]) < get_alpha_num(s2[0]))
                                    setVariable(arg0, random(s0, s2));
                                else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0]))
                                    setVariable(arg0, random(s2, s0));
                                else
                                    setVariable(arg0, random(s2, s0));
                            }
                        }
                        else
                            error(ErrorMessage::INVALID_SEQ, s0 + "_" + s2, false);
                    }
                }
                else if (listExists(before) && after == "size")
                {
                    if (isNumber(arg0))
                        setVariable(arg0, stod(itos(getList(before).size())));
                    else if (isString(arg0))
                        setVariable(arg0, itos(getList(before).size()));
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
                else if (before == "self")
                {
                    if (objectExists(State.CurrentMethodObject))
                        twoSpace(arg0, arg1, (State.CurrentMethodObject + "." + after), (arg0 + " " + arg1 + " " + (State.CurrentMethodObject + "." + after)), command);
                    else
                        twoSpace(arg0, arg1, after, (arg0 + " " + arg1 + " " + after), command);
                }
                else if (objectExists(before))
                {
                    if (getObject(before).variableExists(after))
                    {
                        if (getObject(before).getVariable(after).getString() != State.Null)
                            setVariable(arg0, getObject(before).getVariable(after).getString());
                        else if (getObject(before).getVariable(after).getNumber() != State.NullNum)
                            setVariable(arg0, getObject(before).getVariable(after).getNumber());
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else if (getObject(before).methodExists(after) && !containsParams(after))
                    {
                        parse(arg2);

                        if (isString(arg0))
                            setVariable(arg0, State.LastValue);
                        else if (isNumber(arg0))
                            setVariable(arg0, stod(State.LastValue));
                    }
                    else if (containsParams(after))
                    {
                        if (getObject(before).methodExists(beforeParams(after)))
                        {
                            executeTemplate(getObject(before).getMethod(beforeParams(after)), getParams(after));

                            if (isNumeric(State.LastValue))
                            {
                                if (isString(arg0))
                                    setVariable(arg0, State.LastValue);
                                else if (isNumber(arg0))
                                    setVariable(arg0, stod(State.LastValue));
                                else
                                    error(ErrorMessage::IS_NULL, arg0, false);
                            }
                            else
                            {
                                if (isString(arg0))
                                    setVariable(arg0, State.LastValue);
                                else if (isNumber(arg0))
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                                else
                                    error(ErrorMessage::IS_NULL, arg0, false);
                            }
                        }
                        else
                            Env::sysExec(s, command);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, arg2, false);
                }
                else if (before == "env")
                {
                    InternalGetEnv(arg0, after, 1);
                }
                else if (after == "to_int")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            setVariable(arg0, (int)varString(before)[0]);
                        else if (isNumber(before))
                        {
                            int i = (int)varNumber(before);
                            setVariable(arg0, (double)i);
                        }
                        else
                            error(ErrorMessage::IS_NULL, before, false);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before, false);
                }
                else if (after == "to_double")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            setVariable(arg0, (double)varString(before)[0]);
                        else if (isNumber(before))
                        {
                            double i = varNumber(before);
                            setVariable(arg0, (double)i);
                        }
                        else
                            error(ErrorMessage::IS_NULL, before, false);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before, false);
                }
                else if (after == "to_string")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            setVariable(arg0, dtos(varNumber(before)));
                        else
                            error(ErrorMessage::IS_NULL, before, false);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before, false);
                }
                else if (after == "to_number")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            setVariable(arg0, stod(varString(before)));
                        else
                            error(ErrorMessage::IS_NULL, before, false);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before, false);
                }
                else if (before == "readline")
                {
                    if (variableExists(after))
                    {
                        if (isString(after))
                        {
                            string line("");
                            write(cleanString(varString(after)));
                            getline(cin, line, '\n');

                            if (isNumber(arg0))
                            {
                                if (isNumeric(line))
                                    setVariable(arg0, stod(line));
                                else
                                    error(ErrorMessage::CONV_ERR, line, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, line);
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);
                        }
                        else
                        {
                            string line("");
                            IO::print("");
                            getline(cin, line, '\n');

                            if (isNumber(arg0))
                            {
                                if (isNumeric(line))
                                    setVariable(arg0, stod(line));
                                else
                                    error(ErrorMessage::CONV_ERR, line, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, line);
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);
                        }
                    }
                    else
                    {
                        string line("");
                        IO::print(cleanString(after));
                        getline(cin, line, '\n');

                        if (isNumeric(line))
                            setVariable(arg0, stod(line));
                        else
                            setVariable(arg0, line);
                    }
                }
                else if (before == "password")
                {
                    if (variableExists(after))
                    {
                        if (isString(after))
                        {
                            string line("");
                            line = getSilentOutput(varString(after));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(line))
                                    setVariable(arg0, stod(line));
                                else
                                    error(ErrorMessage::CONV_ERR, line, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, line);
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);

                            IO::println();
                        }
                        else
                        {
                            string line("");
                            line = getSilentOutput("password: ");

                            if (isNumber(arg0))
                            {
                                if (isNumeric(line))
                                    setVariable(arg0, stod(line));
                                else
                                    error(ErrorMessage::CONV_ERR, line, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, line);
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);

                            IO::println();
                        }
                    }
                    else
                    {
                        string line("");
                        line = getSilentOutput(cleanString(after));

                        if (isNumeric(line))
                            setVariable(arg0, stod(line));
                        else
                            setVariable(arg0, line);

                        IO::println();
                    }
                }
                else if (after == "cos")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, cos(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(cos(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "acos")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, acos(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(acos(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "cosh")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, cosh(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(cosh(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "log")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, log(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(log(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "sqrt")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, sqrt(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(sqrt(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "abs")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, abs(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(abs(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "floor")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, floor(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(floor(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "ceil")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, ceil(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(ceil(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "exp")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, exp(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(exp(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "sin")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, sin(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(sin(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "sinh")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, sinh(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(sinh(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "asin")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, asin(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(asin(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "tan")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, tan(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(tan(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "tanh")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, tanh(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(tanh(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "atan")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, atan(varNumber(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(atan(varNumber(before))));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "to_lower")
                {
                    if (variableExists(before))
                    {
                        if (isString(arg0))
                        {
                            if (isString(before))
                                setVariable(arg0, getLower(varString(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "read")
                {
                    if (isString(arg0))
                    {
                        if (variableExists(before))
                        {
                            if (isString(before))
                            {
                                if (Env::fileExists(varString(before)))
                                {
                                    ifstream file(varString(before).c_str());
                                    string line(""), bigString("");

                                    if (file.is_open())
                                    {
                                        while (!file.eof())
                                        {
                                            getline(file, line);
                                            bigString.append(line + "\r\n");
                                        }

                                        file.close();

                                        setVariable(arg0, bigString);
                                    }
                                    else
                                        error(ErrorMessage::READ_FAIL, varString(before), false);
                                }
                                else
                                    error(ErrorMessage::READ_FAIL, varString(before), false);
                            }
                            else
                                error(ErrorMessage::NULL_STRING, before, false);
                        }
                        else
                        {
                            if (Env::fileExists(before))
                            {
                                ifstream file(before.c_str());
                                string line(""), bigString("");

                                if (file.is_open())
                                {
                                    while (!file.eof())
                                    {
                                        getline(file, line);
                                        bigString.append(line + "\r\n");
                                    }

                                    file.close();

                                    setVariable(arg0, bigString);
                                }
                                else
                                    error(ErrorMessage::READ_FAIL, before, false);
                            }
                            else
                                error(ErrorMessage::READ_FAIL, before, false);
                        }
                    }
                    else
                        error(ErrorMessage::NULL_STRING, arg0, false);
                }
                else if (after == "to_upper")
                {
                    if (variableExists(before))
                    {
                        if (isString(arg0))
                        {
                            if (isString(before))
                                setVariable(arg0, getUpper(varString(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == "size")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(arg0))
                        {
                            if (isString(before))
                                setVariable(arg0, (double)varString(before).length());
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, (double)before.length());
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                }
                else if (after == "bytes")
                {
                    if (isNumber(arg0))
                    {
                        if (variableExists(before))
                        {
                            if (isString(before))
                            {
                                if (Env::fileExists(varString(before)))
                                    setVariable(arg0, getBytes(varString(before)));
                                else
                                    error(ErrorMessage::READ_FAIL, varString(before), false);
                            }
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                        {
                            if (Env::fileExists(before))
                                setVariable(arg0, getBytes(before));
                            else
                                error(ErrorMessage::READ_FAIL, before, false);
                        }
                    }
                    else
                        error(ErrorMessage::CONV_ERR, arg0, false);
                }
                else
                {
                    if (isNumber(arg0))
                    {
                        if (isNumeric(arg2))
                            setVariable(arg0, stod(arg2));
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else if (isString(arg0))
                        setVariable(arg0, arg2);
                    else if (getVar(arg0).waiting())
                    {
                        if (isNumeric(arg2))
                            setVariable(arg0, stod(before + "." + after));
                        else
                            setVariable(arg0, arg2);
                    }
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
            }
            else
            {
                if (getVar(arg0).waiting())
                {
                    if (isNumeric(arg2))
                        setVariable(arg0, stod(arg2));
                    else
                        setVariable(arg0, arg2);
                }
                else if (arg2 == "null")
                {
                    if (isString(arg0))
                        getVar(arg0).setNull();
                    else if (isNumber(arg0))
                        getVar(arg0).setNull();
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
                else if (constantExists(arg2))
                {
                    if (isString(arg0))
                    {
                        if (getConstant(arg2).ConstNumber())
                            setVariable(arg0, dtos(getConstant(arg2).getNumber()));
                        else if (getConstant(arg2).ConstString())
                            setVariable(arg0, getConstant(arg2).getString());
                    }
                    else if (isNumber(arg0))
                    {
                        if (getConstant(arg2).ConstNumber())
                            setVariable(arg0, getConstant(arg2).getNumber());
                        else
                            error(ErrorMessage::CONV_ERR, arg2, false);
                    }
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
                else if (methodExists(arg2))
                {
                    parse(arg2);

                    if (isString(arg0))
                        setVariable(arg0, State.LastValue);
                    else if (isNumber(arg0))
                        setVariable(arg0, stod(State.LastValue));
                }
                else if (variableExists(arg2))
                {
                    if (isString(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, varString(arg2));
                        else if (isNumber(arg0))
                            error(ErrorMessage::CONV_ERR, arg2, false);
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else if (isNumber(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, dtos(varNumber(arg2)));
                        else if (isNumber(arg0))
                            setVariable(arg0, varNumber(arg2));
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else
                        error(ErrorMessage::IS_NULL, arg2, false);
                }
                else if (arg2 == "password" || arg2 == "readline")
                {
                    if (arg2 == "password")
                    {
                        string passworder("");
                        passworder = getSilentOutput("");

                        if (isNumber(arg0))
                        {
                            if (isNumeric(passworder))
                                setVariable(arg0, stod(passworder));
                            else
                                error(ErrorMessage::CONV_ERR, passworder, false);
                        }
                        else if (isString(arg0))
                            setVariable(arg0, passworder);
                        else
                            setVariable(arg0, passworder);
                    }
                    else
                    {
                        string line("");
                        IO::print("");
                        getline(cin, line, '\n');

                        if (isNumeric(line))
                            createVariable(arg0, stod(line));
                        else
                            createVariable(arg0, line);
                    }
                }
                else if (containsParams(arg2))
                {
                    if (methodExists(beforeParams(arg2)))
                    {
                        // execute the method
                        executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));
                        // set the variable = last value
                        if (isString(arg0))
                        {
                            setVariable(arg0, State.LastValue);
                        }
                        else if (isNumber(arg0))
                        {
                            setVariable(arg0, stod(State.LastValue));
                        }
                    }
                    else if (isStringStack(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, getStringStack(arg2));
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else if (stackReady(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, dtos(getStack(arg2)));
                        else if (isNumber(arg0))
                            setVariable(arg0, getStack(arg2));
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else
                {
                    if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, stod(arg2));
                        else if (isString(arg0))
                            setVariable(arg0, arg2);
                    }
                    else
                    {
                        if (isNumber(arg0))
                            error(ErrorMessage::CONV_ERR, arg0, false);
                        else if (isString(arg0))
                            setVariable(arg0, cleanString(arg2));
                    }
                }
            }
        }
        else
        {
            if (arg1 == "+=")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg0))
                    {
                        if (isString(arg2))
                            setVariable(arg0, varString(arg0) + varString(arg2));
                        else if (isNumber(arg2))
                            setVariable(arg0, varString(arg0) + dtos(varNumber(arg2)));
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else if (isNumber(arg0))
                    {
                        if (isString(arg2))
                            error(ErrorMessage::CONV_ERR, arg2, false);
                        else if (isNumber(arg2))
                            setVariable(arg0, varNumber(arg0) + varNumber(arg2));
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (isStringStack(arg2))
                        {
                            if (isString(arg0))
                                setVariable(arg0, varString(arg0) + getStringStack(arg2));
                            else
                                error(ErrorMessage::CONV_ERR, arg0, false);
                        }
                        else if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, varNumber(arg0) + getStack(arg2));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isString(arg0))
                                setVariable(arg0, varString(arg0) + State.LastValue);
                            else if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, varNumber(arg0) + stod(State.LastValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isString(arg0))
                                setVariable(arg0, varString(arg0) + State.LastValue);
                            else if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, varNumber(arg0) + stod(State.LastValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isString(arg0))
                            setVariable(arg0, varString(arg0) + State.LastValue);
                        else if (isNumber(arg0))
                        {
                            if (isNumeric(State.LastValue))
                                setVariable(arg0, varNumber(arg0) + stod(State.LastValue));
                            else
                                error(ErrorMessage::CONV_ERR, arg0, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, varString(arg0) + arg2);
                        else if (isNumber(arg0))
                            setVariable(arg0, varNumber(arg0) + stod(arg2));
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else
                    {
                        if (isString(arg0))
                            setVariable(arg0, varString(arg0) + cleanString(arg2));
                        else if (isNumber(arg0))
                            error(ErrorMessage::CONV_ERR, arg0, false);
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
            }
            else if (arg1 == "-=")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg0))
                    {
                        if (isString(arg2))
                        {
                            if (varString(arg2).length() == 1)
                                setVariable(arg0, subtractChar(varString(arg0), varString(arg2)));
                            else
                                setVariable(arg0, subtractString(varString(arg0), varString(arg2)));
                        }
                        else if (isNumber(arg2))
                            setVariable(arg0, subtractString(varString(arg0), dtos(varNumber(arg2))));
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else if (isNumber(arg0))
                    {
                        if (isString(arg2))
                            error(ErrorMessage::CONV_ERR, arg2, false);
                        else if (isNumber(arg2))
                            setVariable(arg0, varNumber(arg0) - varNumber(arg2));
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (isStringStack(arg2))
                        {
                            if (isString(arg0))
                                setVariable(arg0, subtractString(varString(arg0), getStringStack(arg2)));
                            else
                                error(ErrorMessage::CONV_ERR, arg0, false);
                        }
                        else if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, varNumber(arg0) - getStack(arg2));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isString(arg0))
                                setVariable(arg0, subtractString(varString(arg0), State.LastValue));
                            else if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, varNumber(arg0) - stod(State.LastValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isString(arg0))
                                setVariable(arg0, subtractString(varString(arg0), State.LastValue));
                            else if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, varNumber(arg0) - stod(State.LastValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isString(arg0))
                            setVariable(arg0, subtractString(varString(arg0), State.LastValue));
                        else if (isNumber(arg0))
                        {
                            if (isNumeric(State.LastValue))
                                setVariable(arg0, varNumber(arg0) - stod(State.LastValue));
                            else
                                error(ErrorMessage::CONV_ERR, arg0, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isString(arg0))
                        {
                            if (arg2.length() == 1)
                                setVariable(arg0, subtractChar(varString(arg0), arg2));
                            else
                                setVariable(arg0, subtractString(varString(arg0), arg2));
                        }
                        else if (isNumber(arg0))
                            setVariable(arg0, varNumber(arg0) - stod(arg2));
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else
                    {
                        if (isString(arg0))
                        {
                            if (arg2.length() == 1)
                                setVariable(arg0, subtractChar(varString(arg0), arg2));
                            else
                                setVariable(arg0, subtractString(varString(arg0), cleanString(arg2)));
                        }
                        else if (isNumber(arg0))
                            error(ErrorMessage::CONV_ERR, arg0, false);
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
            }
            else if (arg1 == "*=")
            {
                if (variableExists(arg2))
                {
                    if (isNumber(arg2))
                        setVariable(arg0, varNumber(arg0) * varNumber(arg2));
                    else if (isString(arg2))
                        error(ErrorMessage::CONV_ERR, arg2, false);
                    else
                        error(ErrorMessage::IS_NULL, arg2, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, varNumber(arg0) * getStack(arg2));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, varNumber(arg0) * stod(State.LastValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::NULL_NUMBER, arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, varNumber(arg0) * stod(State.LastValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::NULL_NUMBER, arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isNumber(arg0))
                        {
                            if (isNumeric(State.LastValue))
                                setVariable(arg0, varNumber(arg0) * stod(State.LastValue));
                            else
                                error(ErrorMessage::CONV_ERR, arg0, false);
                        }
                        else
                            error(ErrorMessage::NULL_NUMBER, arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, varNumber(arg0) * stod(arg2));
                    }
                    else
                        setVariable(arg0, cleanString(arg2));
                }
            }
            else if (arg1 == "%=")
            {
                if (variableExists(arg2))
                {
                    if (isNumber(arg2))
                        setVariable(arg0, (int)varNumber(arg0) % (int)varNumber(arg2));
                    else if (isString(arg2))
                        error(ErrorMessage::CONV_ERR, arg2, false);
                    else
                        error(ErrorMessage::IS_NULL, arg2, false);
                }
                else if (methodExists(arg2))
                {
                    parse(arg2);

                    if (isNumber(arg0))
                    {
                        if (isNumeric(State.LastValue))
                            setVariable(arg0, (int)varNumber(arg0) % (int)stod(State.LastValue));
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else
                        error(ErrorMessage::NULL_NUMBER, arg0, false);
                }
                else
                {
                    if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, (int)varNumber(arg0) % (int)stod(arg2));
                    }
                    else
                        setVariable(arg0, cleanString(arg2));
                }
            }
            else if (arg1 == "**=")
            {
                if (variableExists(arg2))
                {
                    if (isNumber(arg2))
                        setVariable(arg0, pow(varNumber(arg0), varNumber(arg2)));
                    else if (isString(arg2))
                        error(ErrorMessage::CONV_ERR, arg2, false);
                    else
                        error(ErrorMessage::IS_NULL, arg2, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, pow(varNumber(arg0), (int)getStack(arg2)));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, pow(varNumber(arg0), (int)stod(State.LastValue)));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::NULL_NUMBER, arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, pow(varNumber(arg0), (int)stod(State.LastValue)));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::NULL_NUMBER, arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isNumber(arg0))
                        {
                            if (isNumeric(State.LastValue))
                                setVariable(arg0, pow(varNumber(arg0), (int)stod(State.LastValue)));
                            else
                                error(ErrorMessage::CONV_ERR, arg0, false);
                        }
                        else
                            error(ErrorMessage::NULL_NUMBER, arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, pow(varNumber(arg0), stod(arg2)));
                    }
                    else
                        setVariable(arg0, cleanString(arg2));
                }
            }
            else if (arg1 == "/=")
            {
                if (variableExists(arg2))
                {
                    if (isNumber(arg2))
                        setVariable(arg0, varNumber(arg0) / varNumber(arg2));
                    else if (isString(arg2))
                        error(ErrorMessage::CONV_ERR, arg2, false);
                    else
                        error(ErrorMessage::IS_NULL, arg2, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, varNumber(arg0) / getStack(arg2));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, varNumber(arg0) / stod(State.LastValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::NULL_NUMBER, arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(State.LastValue))
                                    setVariable(arg0, varNumber(arg0) / stod(State.LastValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                                error(ErrorMessage::NULL_NUMBER, arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isNumber(arg0))
                        {
                            if (isNumeric(State.LastValue))
                                setVariable(arg0, varNumber(arg0) / stod(State.LastValue));
                            else
                                error(ErrorMessage::CONV_ERR, arg0, false);
                        }
                        else
                            error(ErrorMessage::NULL_NUMBER, arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, varNumber(arg0) / stod(arg2));
                    }
                    else
                        setVariable(arg0, cleanString(arg2));
                }
            }
            else if (arg1 == "++=")
            {
                if (variableExists(arg2))
                {
                    if (isNumber(arg2))
                    {
                        if (isString(arg0))
                        {
                            int tempVarNumber((int)varNumber(arg2));
                            string tempVarString(varString(arg0));
                            int len(tempVarString.length());
                            string cleaned("");

                            for (int i = 0; i < len; i++)
                                cleaned.push_back((char)(((int)tempVarString[i]) + tempVarNumber));

                            setVariable(arg0, cleaned);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else
                        error(ErrorMessage::CONV_ERR, arg2, false);
                }
                else
                {
                    if (isNumeric(arg2))
                    {
                        int tempVarNumber(stoi(arg2));
                        string tempVarString(varString(arg0));

                        if (tempVarString != State.Null)
                        {
                            int len(tempVarString.length());
                            string cleaned("");

                            for (int i = 0; i < len; i++)
                                cleaned.push_back((char)(((int)tempVarString[i]) + tempVarNumber));

                            setVariable(arg0, cleaned);
                        }
                        else
                            error(ErrorMessage::IS_NULL, tempVarString, false);
                    }
                    else
                        error(ErrorMessage::CONV_ERR, arg2, false);
                }
            }
            else if (arg1 == "--=")
            {
                if (variableExists(arg2))
                {
                    if (isNumber(arg2))
                    {
                        if (isString(arg0))
                        {
                            int tempVarNumber((int)varNumber(arg2));
                            string tempVarString(varString(arg0));
                            int len(tempVarString.length());
                            string cleaned("");

                            for (int i = 0; i < len; i++)
                                cleaned.push_back((char)(((int)tempVarString[i]) - tempVarNumber));

                            setVariable(arg0, cleaned);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else
                        error(ErrorMessage::CONV_ERR, arg2, false);
                }
                else
                {
                    if (isNumeric(arg2))
                    {
                        int tempVarNumber(stoi(arg2));
                        string tempVarString(varString(arg0));

                        if (tempVarString != State.Null)
                        {
                            int len(tempVarString.length());
                            string cleaned("");

                            for (int i = 0; i < len; i++)
                                cleaned.push_back((char)(((int)tempVarString[i]) - tempVarNumber));

                            setVariable(arg0, cleaned);
                        }
                        else
                            error(ErrorMessage::IS_NULL, tempVarString, false);
                    }
                    else
                        error(ErrorMessage::CONV_ERR, arg2, false);
                }
            }
            else if (arg1 == "?")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, Env::getStdout(varString(arg2).c_str()));
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else
                        error(ErrorMessage::CONV_ERR, arg2, false);
                }
                else
                {
                    if (isString(arg0))
                        setVariable(arg0, Env::getStdout(cleanString(arg2).c_str()));
                    else
                        error(ErrorMessage::CONV_ERR, arg0, false);
                }
            }
            else if (arg1 == "!")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, getParsedOutput(varString(arg2).c_str()));
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else
                        error(ErrorMessage::CONV_ERR, arg2, false);
                }
                else
                {
                    if (isString(arg0))
                        setVariable(arg0, getParsedOutput(cleanString(arg2).c_str()));
                    else
                        error(ErrorMessage::CONV_ERR, arg0, false);
                }
            }
            else
            {
                error(ErrorMessage::INVALID_OPERATOR, arg1, false);
            }
        }
    }
}

void initializeListValues(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    string _b(beforeDot(arg2)), _a(afterDot(arg2)), __b(beforeParams(arg2));

    if (containsBrackets(arg0))
    {
        string after(afterBrackets(arg0)), before(beforeBrackets(arg0));
        after = subtractString(after, "]");

        if (getList(before).size() >= stoi(after))
        {
            if (stoi(after) == 0)
            {
                if (arg1 == "=")
                {
                    if (variableExists(arg2))
                    {
                        if (isString(arg2))
                            replaceElement(before, after, varString(arg2));
                        else if (isNumber(arg2))
                            replaceElement(before, after, dtos(varNumber(arg2)));
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else
                        replaceElement(before, after, arg2);
                }
            }
            else if (getList(before).at(stoi(after)) == "#!=no_line")
                error(ErrorMessage::OUT_OF_BOUNDS, arg0, false);
            else
            {
                if (arg1 == "=")
                {
                    if (variableExists(arg2))
                    {
                        if (isString(arg2))
                            replaceElement(before, after, varString(arg2));
                        else if (isNumber(arg2))
                            replaceElement(before, after, dtos(varNumber(arg2)));
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else
                        replaceElement(before, after, arg2);
                }
            }
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
    }
    else if (containsBrackets(arg2)) // INITIALIZE LIST FROM RANGE
    {
        string listName(beforeBrackets(arg2));

        if (!listExists(listName))
        {
            error(ErrorMessage::LIST_UNDEFINED, listName, false);
            return;
        }

        vector<string> listRange = getBracketRange(arg2);

        if (listRange.size() != 2)
        {
            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
            return;
        }

        string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

        if (!(rangeBegin.length() != 0 && rangeEnd.length() != 0))
        {
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
            return;
        }

        if (!(isNumeric(rangeBegin) && isNumeric(rangeEnd)))
        {
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
            return;
        }

        if (stoi(rangeBegin) < stoi(rangeEnd))
        {
            if (!(getList(listName).size() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) || !(stoi(rangeBegin) >= 0))
            {
                error(ErrorMessage::OUT_OF_BOUNDS, rangeEnd, false);
                return;
            }

            if (arg1 == "+=")
            {
                for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                    getList(arg0).add(getList(listName).at(i));
            }
            else if (arg1 == "=")
            {
                getList(arg0).clear();

                for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                    getList(arg0).add(getList(listName).at(i));
            }
            else
                error(ErrorMessage::INVALID_OPERATOR, arg1, false);
        }
        else if (stoi(rangeBegin) > stoi(rangeEnd))
        {
            if (!(getList(listName).size() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) || !(stoi(rangeBegin) >= 0))
            {
                error(ErrorMessage::OUT_OF_BOUNDS, rangeEnd, false);
                return;
            }

            if (arg1 == "+=")
            {
                for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                    getList(arg0).add(getList(listName).at(i));
            }
            else if (arg1 == "=")
            {
                getList(arg0).clear();

                for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                    getList(arg0).add(getList(listName).at(i));
            }
            else
                error(ErrorMessage::INVALID_OPERATOR, arg1, false);
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
    }
    else if (variableExists(_b) && contains(_a, "split") && arg1 == "=")
    {
        if (!isString(_b))
        {
            error(ErrorMessage::NULL_STRING, _b, false);
            return;
        }

        vector<string> params = getParams(_a);
        vector<string> elements;

        if (params.at(0) == "")
            elements = split(varString(_b), ' ');
        else
        {
            if (params.at(0)[0] == ';')
                elements = split(varString(_b), ';');
            else
                elements = split(varString(_b), params.at(0)[0]);
        }

        getList(arg0).clear();

        for (int i = 0; i < (int)elements.size(); i++)
            getList(arg0).add(elements.at(i));
    }
    else if (containsParams(arg2)) // ADD/REMOVE ARRAY FROM LIST
    {
        vector<string> params = getParams(arg2);

        if (arg1 == "=")
        {
            getList(arg0).clear();

            setList(arg0, arg2, params);
        }
        else if (arg1 == "+=")
            setList(arg0, arg2, params);
        else if (arg1 == "-=")
        {
            for (int i = 0; i < (int)params.size(); i++)
            {
                if (variableExists(params.at(i)))
                {
                    if (isString(params.at(i)))
                        getList(arg0).remove(varString(params.at(i)));
                    else if (isNumber(params.at(i)))
                        getList(arg0).remove(dtos(varNumber(params.at(i))));
                    else
                        error(ErrorMessage::IS_NULL, params.at(i), false);
                }
                else
                    getList(arg0).remove(params.at(i));
            }
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
    else if (variableExists(arg2)) // ADD/REMOVE VARIABLE VALUE TO/FROM LIST
    {
        if (arg1 == "+=")
        {
            if (isString(arg2))
                getList(arg0).add(varString(arg2));
            else if (isNumber(arg2))
                getList(arg0).add(dtos(varNumber(arg2)));
            else
                error(ErrorMessage::CONV_ERR, arg2, false);
        }
        else if (arg1 == "-=")
        {
            if (isString(arg2))
                getList(arg0).remove(varString(arg2));
            else if (isNumber(arg2))
                getList(arg0).remove(dtos(varNumber(arg2)));
            else
                error(ErrorMessage::CONV_ERR, arg2, false);
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
    else if (methodExists(arg2)) // INITIALIZE LIST FROM METHOD RETURN
    {
        parse(arg2);

        vector<string> _p = getParams(State.LastValue);

        if (arg1 == "=")
        {
            getList(arg0).clear();

            for (int i = 0; i < (int)_p.size(); i++)
                getList(arg0).add(_p.at(i));
        }
        else if (arg1 == "+=")
        {
            for (int i = 0; i < (int)_p.size(); i++)
                getList(arg0).add(_p.at(i));
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
    else // ADD/REMOVE STRING TO/FROM LIST
    {
        if (arg1 == "+=")
        {
            if (arg2.length() != 0)
                getList(arg0).add(arg2);
            else
                error(ErrorMessage::IS_EMPTY, arg2, false);
        }
        else if (arg1 == "-=")
        {
            if (arg2.length() != 0)
                getList(arg0).remove(arg2);
            else
                error(ErrorMessage::IS_EMPTY, arg2, false);
        }
    }
}

void createGlobalVariable(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    if (arg1 == "=")
    {
        string before(beforeDot(arg2)), after(afterDot(arg2));

        if (containsBrackets(arg2) && (variableExists(beforeBrackets(arg2)) || listExists(beforeBrackets(arg2))))
        {
            string beforeBracket(beforeBrackets(arg2)), afterBracket(afterBrackets(arg2));

            afterBracket = subtractString(afterBracket, "]");

            if (listExists(beforeBracket))
            {
                if (getList(beforeBracket).size() >= stoi(afterBracket))
                {
                    if (getList(beforeBracket).at(stoi(afterBracket)) == "#!=no_line")
                        error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
                    else
                    {
                        string listValue(getList(beforeBracket).at(stoi(afterBracket)));

                        if (isNumeric(listValue))
                            createVariable(arg0, stod(listValue));
                        else
                            createVariable(arg0, listValue);
                    }
                }
                else
                    error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
            }
            else if (variableExists(beforeBracket))
                setSubString(arg0, arg2, beforeBracket);
            else
                error(ErrorMessage::LIST_UNDEFINED, beforeBracket, false);
        }
        else if (listExists(before) && after == "size")
            createVariable(arg0, stod(itos(getList(before).size())));
        else if (before == "self")
        {
            if (objectExists(State.CurrentMethodObject))
                twoSpace(arg0, arg1, (State.CurrentMethodObject + "." + after), (arg0 + " " + arg1 + " " + (State.CurrentMethodObject + "." + after)), command);
            else
                twoSpace(arg0, arg1, after, (arg0 + " " + arg1 + " " + after), command);
        }
        else if (after == "to_integer")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isString(before))
                createVariable(arg0, (int)varString(before)[0]);
            else if (isNumber(before))
            {
                int i = (int)varNumber(before);
                createVariable(arg0, (double)i);
            }
            else
                error(ErrorMessage::IS_NULL, before, false);
        }
        else if (after == "to_double")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isString(before))
                createVariable(arg0, (double)varString(before)[0]);
            else if (isNumber(before))
            {
                double i = varNumber(before);
                createVariable(arg0, (double)i);
            }
            else
                error(ErrorMessage::IS_NULL, before, false);
        }
        else if (after == "to_string")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, dtos(varNumber(before)));
            else
                error(ErrorMessage::IS_NULL, before, false);
        }
        else if (after == "to_number")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isString(before))
                createVariable(arg0, stod(varString(before)));
            else
                error(ErrorMessage::IS_NULL, before, false);
        }
        else if (objectExists(before))
        {
            if (getObject(before).methodExists(after) && !containsParams(after))
            {
                parse(arg2);

                if (isNumeric(State.LastValue))
                    createVariable(arg0, stod(State.LastValue));
                else
                    createVariable(arg0, State.LastValue);
            }
            else if (containsParams(after))
            {
                if (!getObject(before).methodExists(beforeParams(after)))
                {
                    Env::sysExec(s, command);
                    return;
                }

                executeTemplate(getObject(before).getMethod(beforeParams(after)), getParams(after));

                if (isNumeric(State.LastValue))
                    createVariable(arg0, stod(State.LastValue));
                else
                    createVariable(arg0, State.LastValue);
            }
            else if (getObject(before).variableExists(after))
            {
                if (getObject(before).getVariable(after).getString() != State.Null)
                    createVariable(arg0, getObject(before).getVariable(after).getString());
                else if (getObject(before).getVariable(after).getNumber() != State.NullNum)
                    createVariable(arg0, getObject(before).getVariable(after).getNumber());
                else
                    error(ErrorMessage::IS_NULL, getObject(before).getVariable(after).name(), false);
            }
        }
        else if (variableExists(before) && after == "read")
        {
            if (!isString(before))
            {
                error(ErrorMessage::NULL_STRING, before, false);
                return;
            }

            if (!Env::fileExists(varString(before)))
            {
                error(ErrorMessage::READ_FAIL, varString(before), false);
                return;
            }

            ifstream file(varString(before).c_str());
            string line(""), bigString("");

            if (!file.is_open())
            {
                error(ErrorMessage::READ_FAIL, varString(before), false);
                return;
            }

            while (!file.eof())
            {
                getline(file, line);
                bigString.append(line + "\r\n");
            }

            file.close();

            createVariable(arg0, bigString);

        }
        else if (State.DefiningObject)
        {
            if (isNumeric(arg2))
            {
                Variable newVariable(arg0, stod(arg2));

                if (State.DefiningPrivateCode)
                    newVariable.setPrivate();
                else if (State.DefiningPublicCode)
                    newVariable.setPublic();

                getObject(State.CurrentObject).addVariable(newVariable);
            }
            else
            {
                Variable newVariable(arg0, arg2);

                if (State.DefiningPrivateCode)
                    newVariable.setPrivate();
                else if (State.DefiningPublicCode)
                    newVariable.setPublic();

                getObject(State.CurrentObject).addVariable(newVariable);
            }
        }
        else if (arg2 == "null")
            createVariable(arg0, arg2);
        else if (methodExists(arg2))
        {
            parse(arg2);

            if (isNumeric(State.LastValue))
                createVariable(arg0, stod(State.LastValue));
            else
                createVariable(arg0, State.LastValue);
        }
        else if (constantExists(arg2))
        {
            if (getConstant(arg2).ConstNumber())
                createVariable(arg0, getConstant(arg2).getNumber());
            else if (getConstant(arg2).ConstString())
                createVariable(arg0, getConstant(arg2).getString());
            else
                error(ErrorMessage::CONV_ERR, arg2, false);
        }
        else if (containsParams(arg2))
        {
            if (isStringStack(arg2))
                createVariable(arg0, getStringStack(arg2));
            else if (stackReady(arg2))
                createVariable(arg0, getStack(arg2));
            else if (beforeParams(arg2) == "random")
            {
                if (!contains(arg2, ".."))
                {
                    error(ErrorMessage::INVALID_RANGE_SEP, arg2, false);
                    return;
                }

                vector<string> range = getRange(arg2);
                string s0(range.at(0)), s2(range.at(1));

                if (isNumeric(s0) && isNumeric(s2))
                {
                    double n0 = stod(s0), n2 = stod(s2);

                    if (n0 < n2)
                        createVariable(arg0, (int)random(n0, n2));
                    else if (n0 > n2)
                        createVariable(arg0, (int)random(n2, n0));
                    else
                        createVariable(arg0, (int)random(n0, n2));
                }
                else if (isAlpha(s0) && isAlpha(s2))
                {
                    if (get_alpha_num(s0[0]) < get_alpha_num(s2[0]))
                        createVariable(arg0, random(s0, s2));
                    else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0]))
                        createVariable(arg0, random(s2, s0));
                    else
                        createVariable(arg0, random(s2, s0));
                }
                else if (variableExists(s0) || variableExists(s2))
                {
                    if (variableExists(s0))
                    {
                        if (isNumber(s0))
                            s0 = dtos(varNumber(s0));
                        else if (isString(s0))
                            s0 = varString(s0);
                    }

                    if (variableExists(s2))
                    {
                        if (isNumber(s2))
                            s2 = varNumberString(s2);
                        else if (isString(s2))
                            s2 = varString(s2);
                    }

                    if (isNumeric(s0) && isNumeric(s2))
                    {
                        double n0 = stod(s0), n2 = stod(s2);

                        if (n0 < n2)
                            createVariable(arg0, (int)random(n0, n2));
                        else if (n0 > n2)
                            createVariable(arg0, (int)random(n2, n0));
                        else
                            createVariable(arg0, (int)random(n0, n2));
                    }
                    else if (isAlpha(s0) && isAlpha(s2))
                    {
                        if (get_alpha_num(s0[0]) < get_alpha_num(s2[0]))
                            createVariable(arg0, random(s0, s2));
                        else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0]))
                            createVariable(arg0, random(s2, s0));
                        else
                            createVariable(arg0, random(s2, s0));
                    }
                }
                else
                    error(ErrorMessage::OUT_OF_BOUNDS, s0 + ".." + s2, false);
            }
            else
            {
                executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                if (isNumeric(State.LastValue))
                    createVariable(arg0, stod(State.LastValue));
                else
                    createVariable(arg0, State.LastValue);
            }
        }
        else if (variableExists(arg2))
        {
            if (isNumber(arg2))
                createVariable(arg0, varNumber(arg2));
            else if (isString(arg2))
                createVariable(arg0, varString(arg2));
            else
                createVariable(arg0, State.Null);
        }
        else if (arg2 == "password" || arg2 == "readline")
        {
            string line("");
            if (arg2 == "password")
            {
                line = getSilentOutput("");

                if (isNumeric(line))
                    createVariable(arg0, stod(line));
                else
                    createVariable(arg0, line);
            }
            else
            {
                IO::print("");
                getline(cin, line, '\n');

                if (isNumeric(line))
                    createVariable(arg0, stod(line));
                else
                    createVariable(arg0, line);
            }
        }
        else if (arg2 == "args.size")
            createVariable(arg0, (double)State.ArgumentCount);
        else if (before == "readline")
        {
            if (variableExists(after))
            {
                if (isString(after))
                {
                    string line("");
                    IO::print(cleanString(varString(after)));
                    getline(cin, line, '\n');

                    if (isNumeric(line))
                        createVariable(arg0, stod(line));
                    else
                        createVariable(arg0, line);
                }
                else
                {
                    string line("");
                    IO::print("");
                    getline(cin, line, '\n');

                    if (isNumeric(line))
                        createVariable(arg0, stod(line));
                    else
                        createVariable(arg0, line);
                }
            }
            else
            {
                string line("");
                IO::print(cleanString(after));
                getline(cin, line, '\n');

                if (isNumeric(line))
                    createVariable(arg0, stod(line));
                else
                    createVariable(arg0, line);
            }
        }
        else if (before == "password")
        {
            if (variableExists(after))
            {
                if (isString(after))
                {
                    string line("");
                    line = getSilentOutput(varString(after));

                    if (isNumeric(line))
                        createVariable(arg0, stod(line));
                    else
                        createVariable(arg0, line);

                    IO::println();
                }
                else
                {
                    string line("");
                    line = getSilentOutput("");

                    if (isNumeric(line))
                        createVariable(arg0, stod(line));
                    else
                        createVariable(arg0, line);

                    IO::println();
                }
            }
            else
            {
                string line("");
                line = getSilentOutput(cleanString(after));

                if (isNumeric(line))
                    createVariable(arg0, stod(line));
                else
                    createVariable(arg0, line);

                IO::println();
            }
        }
        else if (after == "size")
        {
            if (!variableExists(before))
            {
                createVariable(arg0, (double)before.length());
                return;
            }

            if (isString(before))
                createVariable(arg0, (double)varString(before).length());
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "sin")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, sin(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "sinh")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, sinh(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "asin")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, asin(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "tan")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, tan(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "tanh")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, tanh(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "atan")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, atan(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "cos")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, cos(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "acos")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, acos(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "cosh")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, cosh(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "log")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, log(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "sqrt")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, sqrt(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "abs")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, abs(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "floor")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, floor(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "ceil")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, ceil(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "exp")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isNumber(before))
                createVariable(arg0, exp(varNumber(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "to_upper")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isString(before))
                createVariable(arg0, getUpper(varString(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "to_lower")
        {
            if (!variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (isString(before))
                createVariable(arg0, getLower(varString(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == "bytes")
        {
            if (!variableExists(before))
            {
                if (Env::fileExists(before))
                    createVariable(arg0, getBytes(before));
                else
                    error(ErrorMessage::READ_FAIL, before, false);
                return;
            }

            if (!isString(before))
            {
                error(ErrorMessage::CONV_ERR, before, false);
                return;
            }

            if (Env::fileExists(varString(before)))
                createVariable(arg0, getBytes(varString(before)));
            else
                error(ErrorMessage::READ_FAIL, varString(before), false);
        }
        else if (before == "env")
        {
            InternalGetEnv(arg0, after, 0);
        }
        else
        {
            if (isNumeric(arg2))
                createVariable(arg0, stod(arg2));
            else
                createVariable(arg0, cleanString(arg2));
        }
    }
    else if (arg1 == "+=")
    {
        if (variableExists(arg2))
        {
            if (isString(arg2))
                createVariable(arg0, varString(arg2));
            else if (isNumber(arg2))
                createVariable(arg0, varNumber(arg2));
            else
                createVariable(arg0, State.Null);
        }
        else
        {
            if (isNumeric(arg2))
                createVariable(arg0, stod(arg2));
            else
                createVariable(arg0, cleanString(arg2));
        }
    }
    else if (arg1 == "-=")
    {
        if (variableExists(arg2))
        {
            if (isNumber(arg2))
                createVariable(arg0, 0 - varNumber(arg2));
            else
                createVariable(arg0, State.Null);
        }
        else
        {
            if (isNumeric(arg2))
                createVariable(arg0, stod(arg2));
            else
                createVariable(arg0, cleanString(arg2));
        }
    }
    else if (arg1 == "?")
    {
        if (!variableExists(arg2))
        {
            createVariable(arg0, Env::getStdout(cleanString(arg2)));
            return;
        }

        if (isString(arg2))
            createVariable(arg0, Env::getStdout(varString(arg2)));
        else
            error(ErrorMessage::CONV_ERR, arg2, false);
    }
    else if (arg1 == "!")
    {
        if (!variableExists(arg2))
        {
            createVariable(arg0, getParsedOutput(cleanString(arg2)));
            return;
        }

        if (isString(arg2))
            createVariable(arg0, getParsedOutput(varString(arg2)));
        else
            error(ErrorMessage::CONV_ERR, arg2, false);
    }
    else
        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
}

void createObjectVariable(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    string before = beforeDot(arg2),
           after = afterDot(arg2);

    if (objectExists(before))
    {
        if (arg1 == "=")
        {
            if (getObject(before).getVariable(after).getString() != State.Null)
                createVariable(arg0, getObject(before).getVariable(after).getString());
            else if (getObject(before).getVariable(after).getNumber() != State.NullNum)
                createVariable(arg0, getObject(before).getVariable(after).getNumber());
        }
    }
}

void copyObject(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    if (arg1 == "=")
    {
        vector<Method> objectMethods = getObject(arg2).getMethods();
        Object newObject(arg0);

        for (int i = 0; i < (int)objectMethods.size(); i++)
            newObject.addMethod(objectMethods.at(i));


        vector<Variable> objectVariables = getObject(arg2).getVariables();

        for (int i = 0; i < (int)objectVariables.size(); i++)
            newObject.addVariable(objectVariables.at(i));

        if (State.ExecutedMethod)
            newObject.collect();
        else
            newObject.dontCollect();

        objects.push_back(newObject);
        State.CurrentObject = arg1;
        State.DefiningObject = false;

        newObject.clear();
        objectMethods.clear();
    }
    else
        error(ErrorMessage::INVALID_OPERATOR, arg1, false);
}

void createConstant(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    if (!constantExists(arg0))
    {
        if (arg1 == "=")
        {
            if (isNumeric(arg2))
            {
                Constant newConstant(arg0, stod(arg2));
                constants.push_back(newConstant);
            }
            else
            {
                Constant newConstant(arg0, arg2);
                constants.push_back(newConstant);
            }
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
    else
        error(ErrorMessage::CONST_UNDEFINED, arg0, false);
}

void executeSimpleStatement(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    if (isNumeric(arg0) && isNumeric(arg2))
    {
        if (arg1 == "+")
            writeline(dtos(stod(arg0) + stod(arg2)));
        else if (arg1 == "-")
            writeline(dtos(stod(arg0) - stod(arg2)));
        else if (arg1 == "*")
            writeline(dtos(stod(arg0) * stod(arg2)));
        else if (arg1 == "/")
            writeline(dtos(stod(arg0) / stod(arg2)));
        else if (arg1 == "**")
            writeline(dtos(pow(stod(arg0), stod(arg2))));
        else if (arg1 == "%")
        {
            if ((int)stod(arg2) == 0)
                error(ErrorMessage::DIVIDED_BY_ZERO, s, false);
            else
                writeline(dtos((int)stod(arg0) % (int)stod(arg2)));
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
    else
    {
        if (arg1 == "+")
            writeline(arg0 + arg2);
        else if (arg1 == "-")
            writeline(subtractString(arg0, arg2));
        else if (arg1 == "*")
        {
            if (!zeroNumbers(arg2))
            {
                string bigstr("");
                for (int i = 0; i < stoi(arg2); i++)
                {
                    bigstr.append(arg0);
                    write(arg0);
                }

                State.LastValue = bigstr;
            }
            else
                error(ErrorMessage::INVALID_OP, s, false);
        }
        else if (arg1 == "/")
            writeline(subtractString(arg0, arg2));
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
}

void InternalEncryptDecrypt(string arg0, string arg1)
{
    Crypt c;
    string text = variableExists(arg1) ? (isString(arg1) ? varString(arg1) : varNumberString(arg1)) : arg1;
    write(arg0 == "encrypt" ? c.e(text) : c.d(text));
}

void InternalInspect(string arg0, string arg1, string before, string after)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (!objectExists(before))
        {
            error(ErrorMessage::OBJ_METHOD_UNDEFINED, before, false);
            return;
        }

        if (getObject(before).methodExists(after))
        {
            for (int i = 0; i < getObject(before).getMethod(after).size(); i++)
                write(getObject(before).getMethod(after).at(i));
        }
        else if (getObject(before).variableExists(after))
        {
            if (getObject(before).getVariable(after).getString() != State.Null)
                write(getObject(before).getVariable(after).getString());
            else if (getObject(before).getVariable(after).getNumber() != State.NullNum)
                write(dtos(getObject(before).getVariable(after).getNumber()));
            else
                write(State.Null);
        }
        else
            error(ErrorMessage::TARGET_UNDEFINED, arg1, false);

    }
    else
    {
        if (objectExists(arg1))
        {
            for (int i = 0; i < getObject(arg1).methodSize(); i++)
                write(getObject(arg1).getMethod(getObject(arg1).getMethodName(i)).name());
            for (int i = 0; i < getObject(arg1).variableSize(); i++)
                write(getObject(arg1).getVariable(getObject(arg1).getVariableName(i)).name());
        }
        else if (constantExists(arg1))
        {
            if (getConstant(arg1).ConstNumber())
                write(dtos(getConstant(arg1).getNumber()));
            else if (getConstant(arg1).ConstString())
                write(getConstant(arg1).getString());
        }
        else if (methodExists(arg1))
        {
            for (int i = 0; i < getMethod(arg1).size(); i++)
                write(getMethod(arg1).at(i));
        }
        else if (variableExists(arg1))
        {
            if (isString(arg1))
                write(varString(arg1));
            else if (isNumber(arg1))
                write(dtos(varNumber(arg1)));
        }
        else if (listExists(arg1))
        {
            for (int i = 0; i < getList(arg1).size(); i++)
                write(getList(arg1).at(i));
        }
        else if (arg1 == "variables")
        {
            for (int i = 0; i < (int)variables.size(); i++)
            {
                if (variables.at(i).getString() != State.Null)
                    write(variables.at(i).name() + ":\t" + variables.at(i).getString());
                else if (variables.at(i).getNumber() != State.NullNum)
                    write(variables.at(i).name() + ":\t" + dtos(variables.at(i).getNumber()));
                else
                    write(variables.at(i).name() + ":\tis_null");
            }
        }
        else if (arg1 == "lists")
        {
            for (int i = 0; i < (int)lists.size(); i++)
                write(lists.at(i).name());
        }
        else if (arg1 == "methods")
        {
            for (int i = 0; i < (int)methods.size(); i++)
                write(methods.at(i).name());
        }
        else if (arg1 == "objects")
        {
            for (int i = 0; i < (int)objects.size(); i++)
                write(objects.at(i).name());
        }
        else if (arg1 == "constants")
        {
            for (int i = 0; i < (int)constants.size(); i++)
                write(constants.at(i).name());
        }
        else if (arg1 == "os?")
            write(Env::getGuessedOS());
        else if (arg1 == "last")
            write(State.LastValue);
        else
            error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
    }
}

void InternalGlobalize(string arg0, string arg1)
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

void InternalCallMethod(string arg0, string arg1, string before, string after)
{
    if (State.DefiningObject)
    {
        if (getObject(State.CurrentObject).methodExists(arg1))
            executeMethod(getObject(State.CurrentObject).getMethod(arg1));
        else
            error(ErrorMessage::METHOD_UNDEFINED, arg1, false);
        return;
    }
    
    if (before.length() != 0 && after.length() != 0)
    {
        if (!objectExists(before))
        {
            error(ErrorMessage::OBJ_METHOD_UNDEFINED, before, true);
            return;
        }

        if (getObject(before).methodExists(after))
            executeMethod(getObject(before).getMethod(after));
        else
            error(ErrorMessage::METHOD_UNDEFINED, arg1, false);
    }
    else
    {
        if (methodExists(arg1))
            executeMethod(getMethod(arg1));
        else
            error(ErrorMessage::METHOD_UNDEFINED, arg1, true);
    }
}

void InternalCreateMethod(string arg0, string arg1)
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

void InternalCreateModule(string s)
{
    string moduleName = s;
    moduleName = subtractString(moduleName, "[");
    moduleName = subtractString(moduleName, "]");

    Module newModule(moduleName);
    modules.push_back(newModule);
    State.DefiningModule = true;
    State.CurrentModule = moduleName;
}

void InternalCreateObject(string arg0)
{
    if (objectExists(arg0))
    {
        State.DefiningObject = true;
        State.CurrentObject = arg0;
    }
    else
    {
        Object object(arg0);
        State.CurrentObject = arg0;
        object.dontCollect();
        objects.push_back(object);
        State.DefiningObject = true;
    }
}

void InternalForget(string arg0, string arg1)
{
    if (Env::fileExists(State.SavedVars))
    {
        string line(""), bigStr("");
        ifstream file(State.SavedVars.c_str());
        // REFACTOR HERE
        Crypt c;

        if (file.is_open())
        {
            while (!file.eof())
            {
                getline(file, line);
                bigStr.append(line);
            }

            file.close();

            int bigStrLength = bigStr.length();
            bool stop = false;
            string varName("");
            bigStr = c.d(bigStr);

            vector<string> varNames;
            vector<string> varValues;

            varNames.push_back("");
            varValues.push_back("");

            for (int i = 0; i < bigStrLength; i++)
            {
                switch (bigStr[i])
                {
                case '&':
                    stop = true;
                    break;

                case '#':
                    stop = false;
                    varNames.push_back("");
                    varValues.push_back("");
                    break;

                default:
                    if (!stop)
                        varNames.at((int)varNames.size() - 1).push_back(bigStr[i]);
                    else
                        varValues.at((int)varValues.size() - 1).push_back(bigStr[i]);
                    break;
                }
            }

            string new_saved("");

            for (int i = 0; i < (int)varNames.size(); i++)
            {
                if (varNames.at(i) != arg1)
                {
                    Variable newVariable(varNames.at(i), varValues.at(i));
                    variables.push_back(newVariable);

                    if (i != (int)varNames.size() - 1)
                        new_saved.append(varNames.at(i) + "&" + varValues.at(i) + "#");
                    else
                        new_saved.append(varNames.at(i) + "&" + varValues.at(i));
                }
            }

            varNames.clear();
            varValues.clear();

            Env::rm(State.SavedVars);
            Env::createFile(State.SavedVars);
            Env::app(State.SavedVars, c.e(new_saved));
        }
    }
}

//	modes:
//		0 = createVariable
//		1 = setVariable
//		2 = setLastValue
void InternalGetEnv(string arg0, string after, int mode)
{
    Crypt c;
    string defaultValue = c.e(DT::timeNow());
    string sValue(defaultValue);
    double dValue = 0;

    if (after == "cwd")
    {
        sValue = Env::cwd();
    }
    else if (after == "noctis")
    {
        sValue = State.Noctis;
    }
    else if (after == "os?")
    {
        sValue = Env::getGuessedOS();
    }
    else if (after == "user")
    {
        sValue = Env::getUser();
    }
    else if (after == "machine")
    {
        sValue = Env::getMachine();
    }
    else if (after == "init_dir" || after == "initial_directory")
    {
        sValue = NoctisEnv.InitialDirectory;
    }
    else if (after == "this_second")
    {
        dValue = (double)DT::secondNow();
    }
    else if (after == "this_minute")
    {
        dValue = (double)DT::minuteNow();
    }
    else if (after == "this_hour")
    {
        dValue = (double)DT::hourNow();
    }
    else if (after == "this_month")
    {
        dValue = (double)DT::monthNow();
    }
    else if (after == "this_year")
    {
        dValue = (double)DT::yearNow();
    }
    else if (after == "day_of_this_month")
    {
        dValue = (double)DT::dayOfTheMonth();
    }
    else if (after == "day_of_this_year")
    {
        dValue = (double)DT::dayOfTheYear();
    }
    else if (after == "day_of_this_week")
    {
        sValue = DT::dayOfTheWeek();
    }
    else if (after == "month_of_this_year")
    {
        sValue = DT::monthOfTheYear();
    }
    else if (after == "am_or_pm")
    {
        sValue = DT::amOrPm();
    }
    else if (after == "now")
    {
        sValue = DT::timeNow();
    }
    else if (after == "last_error")
    {
        sValue = State.LastError;
    }
    else if (after == "last_value")
    {
        sValue = State.LastValue;
    }
    else if (after == "empty_string")
    {
        sValue = "";
    }
    else if (after == "empty_number")
    {
        dValue = 0;
    }
    else
    {
        sValue = Env::getEnvironmentVariable(after);
    }

    switch (mode)
    {
    case 0:
        if (sValue != defaultValue)
        {
            createVariable(arg0, sValue);
        }
        else
        {
            createVariable(arg0, dValue);
        }
        break;
    case 1:
        if (sValue != defaultValue)
        {
            setVariable(arg0, sValue);
        }
        else
        {
            setVariable(arg0, dValue);
        }
        break;
    case 2:
        if (sValue != defaultValue)
        {
            State.LastValue = sValue;
        }
        else
        {
            State.LastValue = dtos(dValue);
        }
        break;
    case 3:
        if (sValue != defaultValue)
        {
            writeline(sValue);
        }
        else
        {
            writeline(dtos(dValue));
        }
    }
}

void InternalOutput(string arg0, string arg1)
{
    string text(arg1);
    bool is_say = (arg0 == "say");
    bool is_print = (arg0 == "print" || arg0 == "println");
    // if parameter is variable, get it's value
    if (variableExists(arg1))
    {
        // set the value
        if (!zeroDots(arg1))
        {
            if (getObject(beforeDot(arg1)).getVariable(afterDot(arg1)).getString() != State.Null)
                text = (getObject(beforeDot(arg1)).getVariable(afterDot(arg1)).getString());
            else if (getObject(beforeDot(arg1)).getVariable(afterDot(arg1)).getNumber() != State.NullNum)
                text = (dtos(getObject(beforeDot(arg1)).getVariable(afterDot(arg1)).getNumber()));
            else
            {
                error(ErrorMessage::IS_NULL, arg1, false);
                return;
            }
        }
        else
        {
            if (isString(arg1))
                text = (varString(arg1));
            else if (isNumber(arg1))
                text = (dtos(varNumber(arg1)));
            else
            {
                error(ErrorMessage::IS_NULL, arg1, false);
                return;
            }
        }
    }

    if (is_say)
    {
        writeline(text);
    }
    else if (is_print)
    {
        if (arg0 == "println")
        {
            IO::println(text);
        }
        else
        {
            IO::print(text);
        }
    }
    else
    {
        write(text);
    }
}

void InternalRemember(string arg0, string arg1)
{
    if (variableExists(arg1))
    {
        if (isString(arg1))
            saveVariable(arg1 + "&" + varString(arg1));
        else if (isNumber(arg1))
            saveVariable(arg1 + "&" + dtos(varNumber(arg1)));
        else
            error(ErrorMessage::IS_NULL, arg1, false);
    }
    else
        error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
}

bool InternalReturn(string arg0, string arg1, string before, string after)
{
    State.Returning = true;

    if (containsParams(arg1))
    {
        string before(beforeParams(arg1));

        if (methodExists(before))
        {
            executeTemplate(getMethod(before), getParams(arg1));

            parse("return " + State.LastValue);
        }
        else if (!zeroDots(arg1))
        {
            if (objectExists(before))
            {
                if (getObject(before).methodExists(beforeParams(after)))
                {
                    executeTemplate(getObject(before).getMethod(beforeParams(after)), getParams(arg1));
                    parse("return " + State.LastValue);
                }
                else
                    State.LastValue = arg1;
            }
            else
                State.LastValue = arg1;
        }
        else
        {
            if (isStringStack(arg1))
                State.LastValue = getStringStack(arg1);
            else if (stackReady(arg1))
                State.LastValue = dtos(getStack(arg1));
            else
            {
                arg1 = subtractString(arg1, "(");
                arg1 = subtractString(arg1, ")");

                return true;
            }
        }
    }
    else if (variableExists(arg1))
    {
        if (objectExists(beforeDot(arg1)))
        {
            if (getObject(beforeDot(arg1)).getVariable(afterDot(arg1)).getString() != State.Null)
                State.LastValue = getObject(beforeDot(arg1)).getVariable(afterDot(arg1)).getString();
            else if (getObject(beforeDot(arg1)).getVariable(afterDot(arg1)).getNumber() != State.NullNum)
                State.LastValue = dtos(getObject(beforeDot(arg1)).getVariable(afterDot(arg1)).getNumber());
            else
                State.LastValue = "null";
        }
        else
        {
            if (isString(arg1))
                State.LastValue = varString(arg1);
            else if (isNumber(arg1))
                State.LastValue = dtos(varNumber(arg1));
            else
                State.LastValue = "null";

            if (getVar(arg1).garbage())
                variables = removeVariable(variables, arg1);
        }
    }
    else if (listExists(arg1))
    {
        string bigString("(");

        for (int i = 0; i < (int)getList(arg1).size(); i++)
        {
            bigString.append(getList(arg1).at(i));

            if (i != (int)getList(arg1).size() - 1)
                bigString.push_back(',');
        }

        bigString.append(")");

        State.LastValue = bigString;

        if (getList(arg1).garbage())
            lists = removeList(lists, arg1);
    }
    else
        State.LastValue = arg1;

    return false;
}

void delay(int seconds)
{
    clock_t ct;
    ct = clock() + seconds * CLOCKS_PER_SEC;

    while(clock() < ct) {}
}

double random(double min, double max)
{
    double r = (double)rand() / (double)RAND_MAX;
    return min + (r * (max - min));
}

string random(string start, string sc)
{
    string s("");
    char c;
    c = start[0] == sc[0] ? sc[0] : (rand() % get_alpha_num(sc[0])) + start[0];
    s.push_back(c);

    return s;
}

void uninstall()
{
    if (Env::directoryExists(State.SavedVarsPath))
    {
        if (Env::fileExists(State.SavedVars))
            Env::rm(State.SavedVars);
        else
            IO::printerrln("...no remembered variables");

        Env::rd(State.SavedVarsPath);

        if (!Env::directoryExists(State.SavedVarsPath) && !Env::fileExists(State.SavedVars))
            IO::println("...removed successfully");
        else
            IO::printerrln("...failed to remove");
    }
    else
        IO::printerrln("...found nothing to remove");
}

double getBytes(string path)
{
    int bytes;

    ifstream file(path.c_str());

    if(!file.is_open())
    {
        error(ErrorMessage::READ_FAIL, path, false);

        return -DBL_MAX;
    }

    long begin, end;

    begin = file.tellg();
    file.seekg(0, ios::end);
    end = file.tellg();

    file.close();

    bytes = (end - begin);

    return bytes;
}

#ifdef __linux__

string getSilentOutput(string text)
{
    char * s = getpass(cleanString(text).c_str());

    return s;
}

#elif defined _WIN32 || defined _WIN64

string getSilentOutput(string text)
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

    IO::print(cleanString(text));

    string s("");
    getline(cin, s);

    SetConsoleMode(hStdin, mode);

    return s;
}

#endif

void setup()
{
    State.BadMethodCount = 0,
    State.BadObjectCount = 0,
    State.BadVarCount = 0,
    State.CurrentLineNumber = 0,
    State.IfStatementCount = 0,
    State.ForLoopCount = 0,
    State.WhileLoopCount = 0,
    State.ParamVarCount = 0;
    State.CaptureParse = false,
    State.IsCommented = false,
    State.UseCustomPrompt = false,
    State.DontCollectMethodVars = false,
    State.FailedIfStatement = false,
    State.GoToLabel = false,
    State.ExecutedIfStatement = false,
    State.InDefaultCase = false,
    State.ExecutedMethod = false,
    State.DefiningSwitchBlock = false,
    State.DefiningIfStatement = false,
    State.DefiningForLoop = false,
    State.DefiningWhileLoop = false,
    State.DefiningModule = false,
    State.DefiningPrivateCode = false,
    State.DefiningPublicCode = false,
    State.DefiningScript = false,
    State.ExecutedTemplate = false, // remove
    State.ExecutedTryBlock = false,
    State.Breaking = false,
    State.DefiningMethod = false,
    State.IsMultilineComment = false,
    State.Negligence = false,
    State.FailedNest = false,
    State.DefiningNest = false,
    State.DefiningObject = false,
    State.DefiningObjectMethod = false,
    State.DefiningParameterizedMethod = false,
    State.Returning = false,
    State.SkipCatchBlock = false,
    State.RaiseCatchBlock = false,
    State.DefiningLocalSwitchBlock = false,
    State.DefiningLocalWhileLoop = false,
    State.DefiningLocalForLoop = false;

    State.CurrentObject = "",
    State.CurrentMethodObject = "",
    State.CurrentModule = "",
    State.CurrentScript = "",
    State.ErrorVarName = "",
    State.GoTo = "",
    State.LastError = "",
    State.LastValue = "",
    State.ParsedOutput = "",
    State.PreviousScript = "",
    State.CurrentScriptName = "",
    State.SwitchVarName = "",
    State.CurrentLine = "",
    State.DefaultLoopSymbol = "$";

    State.Null = "[null]";

    State.ArgumentCount = 0,
    State.NullNum = -DBL_MAX;

    if (contains(Env::getEnvironmentVariable("HOMEPATH"), "Users"))
    {
        NoctisEnv.GuessedOS = OS_WIN64;
        State.SavedVarsPath = (Env::getEnvironmentVariable("HOMEPATH") + "\\AppData") + "\\.State.SavedVarsPath", State.SavedVars = State.SavedVarsPath + "\\.State.SavedVars";
    }
    else if (contains(Env::getEnvironmentVariable("HOMEPATH"), "Documents"))
    {
        NoctisEnv.GuessedOS = OS_WIN32;
        State.SavedVarsPath = Env::getEnvironmentVariable("HOMEPATH") + "\\Application Data\\.State.SavedVarsPath", State.SavedVars = State.SavedVarsPath + "\\.State.SavedVars";
    }
    else if (startsWith(Env::getEnvironmentVariable("HOME"), "/"))
    {
        NoctisEnv.GuessedOS = OS_NIX;
        State.SavedVarsPath = Env::getEnvironmentVariable("HOME") + "/.State.SavedVarsPath", State.SavedVars = State.SavedVarsPath + "/.State.SavedVars";
    }
    else
    {
        NoctisEnv.GuessedOS = OS_UNKNOWN;
        State.SavedVarsPath = "\\.State.SavedVarsPath", State.SavedVars = State.SavedVarsPath + "\\.State.SavedVars";
    }
}

#endif
