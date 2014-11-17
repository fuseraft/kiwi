/**
 * Project:		Unorthodox Scripting Language (USL)
 * Version:		0.0.0.1
 *
 * Filename:	main.cpp
 *
 * Author:		Scott Christopher Stauffer
 * Email:		scstauf@gmail.com
 * Date:		Friday, August 23, 2013
 *
 * Copyright 2013
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
//#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <locale>
#include <cmath>
#include <string.h>
#include <cfloat>

#ifdef __linux__
#include <unistd.h>
#endif

using namespace std;

const int MAX_BUFFER = 1024;

#include "methods.h"
#include "objects.h"

#include <ctime>

int secondNow(),
    minuteNow(),
    hourNow(),
    monthNow(),
    yearNow(),
    dayOfTheMonth(),
    dayOfTheYear();

string  amOrPm(),
        dayOfTheWeek(),
        monthOfTheYear(),
        timeNow(),
        getPrompt();

int random(int low, int high);
double random(double min, double max);
string random(string start, string sc);

vector<Method>      methods;
vector<Object>      objects;
vector<Variable>    variables;
vector<List>        lists;
vector<Constant>    constants;
vector<Method>      ifStatements;
vector<Method>      forLoops;
vector<Method>      whileLoops;
vector<string>      args;
vector<Module>      modules;
vector<Script>      scripts;

Switch mainSwitch;

vector<Constant>    removeConstant(vector<Constant> v, string target);
vector<List>        removeList(vector<List> v, string target);
vector<Method>      removeMethod(vector<Method> v, string target);
vector<Object>      removeObject(vector<Object> v, string target);
vector<Variable>    removeVariable(vector<Variable> v, string target);

void InternalCallMethod(string arg0, string arg1, string before, string after);
void InternalCreateMethod(string arg0, string arg1);
void InternalCreateModule(string s);
void InternalCreateObject(string arg0);
void InternalEncryptDecrypt(string arg0, string arg1);
void InternalForget(string arg0, string arg1);
void InternalInspect(string arg0, string arg1, string before, string after);
void InternalGetEnv(string arg1, string after, int mode);
void InternalGlobalize(string arg0, string arg1);
void InternalOutput(string arg0, string arg1);
void InternalRemember(string arg0, string arg1);
bool InternalReturn(string arg0, string arg1, string before, string after);

string	__CurrentLine,
        __CurrentMethodObject,
        __CurrentModule,
        __CurrentObject,
        __CurrentScript,
        __CurrentScriptName,
        __ErrorVarName,
        __GoTo,
        __GuessedOS,
        __InitialDirectory,
        __LastError,
        __LastValue,
        __LogFile,
        __Noctis,
        __ParsedOutput,
        __PreviousScript,
        __PromptStyle,
        __SavedVarsPath,
        __SavedVars,
        __SwitchVarName,
        __DefaultLoopSymbol,
        __Null,
        cleanString(string st),
        getParsedOutput(string cmd),
        getSilentOutput(string text);

int __ArgumentCount,
    __BadMethodCount,
    __BadObjectCount,
    __BadVarCount,
    __CurrentLineNumber,
    __IfStatementCount,
    __ForLoopCount,
    __ParamVarCount,
    __WhileLoopCount;

double __NullNum;

bool    __Breaking,
        __CaptureParse,
        __DefiningIfStatement,
        __DefiningForLoop,
        __DefiningLocalForLoop,
        __DefiningLocalSwitchBlock,
        __DefiningLocalWhileLoop,
        __DefiningMethod,
        __DefiningModule,
        __DefiningNest,
        __DefiningObject,
        __DefiningObjectMethod,
        __DefiningParameterizedMethod,
        __DefiningPrivateCode,
        __DefiningPublicCode,
        __DefiningScript,
        __DefiningSwitchBlock,
        __DefiningWhileLoop,
        __DontCollectMethodVars,
        __ExecutedIfStatement,
        __ExecutedMethod,
        __ExecutedTemplate,
        __ExecutedTryBlock,
        __FailedIfStatement,
        __FailedNest,
        __GoToLabel,
        __InDefaultCase,
        __Logging,
        __Negligence,
        __Returning,
        __SkipCatchBlock,
        __SkipDefaultBlock,
        __RaiseCatchBlock,
        __UseCustomPrompt;

List        getDirectoryList(string before, bool filesOnly);
Method      getMethod(string s);
Object      getObject(string s);
Variable    getVariable(string s);

int indexOfConstant(string s),
    indexOfList(string s),
    indexOfMethod(string s),
    indexOfModule(string s),
    indexOfObject(string s),
    indexOfScript(string s),
    indexOfVariable(string s);

void    cd(string p),
        clearAll(),
        clearArgs(),
        clearConstants(),
        clearFor(),
        clearIf(),
        clearLists(),
        clearMethods(),
        clearObjects(),
        clearVariables(),
        clearWhile(),
        delay(int seconds),
        displayVersion(),
        error(string e, bool quit),
        executeMethod(Method m),
        executeNest(Container n),
        failedIfStatement(),
        failedFor(),
        failedWhile(),
        forLoop(Method m),
        help(string app),
        loadSavedVars(Crypt c, string &bs),
        loadScript(string script),
        loop(bool skip),
        parse(string s),
        redefine(string target, string name),
        replaceElement(string before, string after, string replacement),
        saveVariable(string var),
        say(string st),
        setLastValue(string s),
        setup(),
        setVariable(string name, string value),
        setVariable(string name, double value),
        successfulFor(),
        successfulFor(double a, double b, string op),
        successfulIF(),
        successfullWhile(string v1, string op, string v2),
        successfulFor(List list),
        executeTemplate(Method m, vector<string> vs),
        uninstall(),
        whileLoop(Method m),
        __stdout(string st),
        __true(),
        __false();

void    zeroSpace(string arg0, string s, vector<string> command),
        oneSpace(string arg0, string arg1, string s, vector<string> command),
        twoSpace(string arg0, string arg1, string arg2, string s, vector<string> command),
        threeSpace(string arg0, string arg1, string arg2, string arg3, string s, vector<string> command);

void appendText(string arg1, string arg2, bool newLine);
void __fwrite(string file, string contents);

bool is(string s, string si),
     isNumber(string varName),
     isScript(string path),
     isString(string varName),
     methodExists(string s),
     objectExists(string s),
     variableExists(string s),
     listExists(string s),
     moduleExists(string s),
     noLists(),
     noMethods(),
     noObjects(),
     noVariables(),
     notStandardZeroSpace(string s),
     notStandardOneSpace(string arg1),
     notStandardTwoSpace(string arg1);

double  getBytes(string path),
        getKBytes(string path),
        getMBytes(string path),
        getGBytes(string path),
        getTBytes(string path),
        getStack(string arg2);

bool isStringStack(string arg2);
bool stackReady(string arg2);
string getStringStack(string arg2);

void setLastValue(string s)
{
    __LastValue = s;
}

void setList(string arg1, string arg2, vector<string> params)
{
    if (methodExists(beforeParams(arg2)))
    {
        executeTemplate(getMethod(beforeParams(arg2)), params);

        if (containsParams(__LastValue))
        {
            vector<string> last_params = getParams(__LastValue);

            for (int i = 0; i < (int)last_params.size(); i++)
                lists.at(indexOfList(arg1)).add(last_params.at(i));
        }
        else
            lists.at(indexOfList(arg1)).add(__LastValue);
    }
    else if (objectExists(beforeDot(beforeParams(arg2))))
    {
        executeTemplate(objects.at(indexOfObject(beforeDot(beforeParams(arg2)))).getMethod(afterDot(beforeParams(arg2))), params);

        if (containsParams(__LastValue))
        {
            vector<string> last_params = getParams(__LastValue);

            for (int i = 0; i < (int)last_params.size(); i++)
                lists.at(indexOfList(arg1)).add(last_params.at(i));
        }
        else
            lists.at(indexOfList(arg1)).add(__LastValue);
    }
    else
    {
        for (int i = 0; i < (int)params.size(); i++)
        {
            if (variableExists(params.at(i)))
            {
                if (isString(params.at(i)))
                    lists.at(indexOfList(arg1)).add(variables.at(indexOfVariable(params.at(i))).getString());
                else if (isNumber(params.at(i)))
                    lists.at(indexOfList(arg1)).add(dtos(variables.at(indexOfVariable(params.at(i))).getNumber()));
                else
                    error("is_null:" + params.at(i), false);
            }
            else
                lists.at(indexOfList(arg1)).add(params.at(i));
        }
    }
}

void setVariable(string name, string value)
{
    variables.at(indexOfVariable(name)).setVariable(value);
    setLastValue(value);
}

void setVariable(string name, double value)
{
    if (isString(name))
        variables.at(indexOfVariable(name)).setVariable(dtos(value));
    else if (isNumber(name))
        variables.at(indexOfVariable(name)).setVariable(value);
    else
    {
        if (variables.at(indexOfVariable(name)).waiting())
            variables.at(indexOfVariable(name)).stopWait();

        variables.at(indexOfVariable(name)).setVariable(value);
    }

    setLastValue(dtos(value));
}

void createVariable(string name, string value)
{
    Variable newVariable(name, value);

    if (__ExecutedTemplate || __ExecutedMethod || __ExecutedTryBlock)
        newVariable.collect();
    else
        newVariable.dontCollect();

    variables.push_back(newVariable);
    setLastValue(value);
}

///	Creates a double type variable
void createVariable(string name, double value)
{
    Variable newVariable(name, value);

    if (__ExecutedTemplate || __ExecutedMethod || __ExecutedTryBlock)
        newVariable.collect();
    else
        newVariable.dontCollect();

    variables.push_back(newVariable);
    setLastValue(dtos(value));
}

void replaceElement(string before, string after, string replacement)
{
    vector<string> newList;

    for (int i = 0; i < (int)lists.at(indexOfList(before)).size(); i++)
    {
        if (i == stoi(after))
            newList.push_back(replacement);
        else
            newList.push_back(lists.at(indexOfList(before)).at(i));
    }

    lists.at(indexOfList(before)).clear();

    for (int i = 0; i < (int)newList.size(); i++)
        lists.at(indexOfList(before)).add(newList.at(i));

    newList.clear();
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;

    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }

    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;

    return split(s, delim, elems);
}

void appendText(string arg1, string arg2, bool newLine)
{
    if (variableExists(arg1))
    {
        if (isString(arg1))
        {
            if (fileExists(variables.at(indexOfVariable(arg1)).getString()))
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                    {
                        if (newLine)
                            app(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg2)).getString() + "\r\n");
                        else
                            app(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg2)).getString());
                    }
                    else if (isNumber(arg2))
                    {
                        if (newLine)
                            app(variables.at(indexOfVariable(arg1)).getString(), dtos(variables.at(indexOfVariable(arg2)).getNumber()) + "\r\n");
                        else
                            app(variables.at(indexOfVariable(arg1)).getString(), dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                    }
                    else
                        error("is_null:" + arg2, false);
                }
                else
                {
                    if (newLine)
                        app(variables.at(indexOfVariable(arg1)).getString(), arg2 + "\r\n");
                    else
                        app(variables.at(indexOfVariable(arg1)).getString(), arg2);
                }
            }
            else
                error("read_fail:" + variables.at(indexOfVariable(arg1)).getString(), false);
        }
        else
            error("conversion_error:" + arg1, false);
    }
    else
    {
        if (variableExists(arg2))
        {
            if (isString(arg2))
            {
                if (fileExists(arg1))
                {
                    if (newLine)
                        app(arg1, variables.at(indexOfVariable(arg2)).getString() + "\r\n");
                    else
                        app(arg1, variables.at(indexOfVariable(arg2)).getString());
                }
                else
                    error("read_fail:" + variables.at(indexOfVariable(arg2)).getString(), false);
            }
            else
                error("conversion_error:" + arg2, false);
        }
        else
        {
            if (fileExists(arg1))
            {
                if (newLine)
                    app(arg1, arg2 + "\r\n");
                else
                    app(arg1, arg2);
            }
            else
                error("read_fail:" + arg1, false);
        }
    }
}

void __fwrite(string arg1, string arg2)
{
    if (variableExists(arg1))
    {
        if (isString(arg1))
        {
            if (fileExists(variables.at(indexOfVariable(arg1)).getString()))
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                    {
                        app(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg2)).getString() + "\r\n");
                        __LastValue = "0";
                    }
                    else if (isNumber(arg2))
                    {
                        app(variables.at(indexOfVariable(arg1)).getString(), dtos(variables.at(indexOfVariable(arg2)).getNumber()) + "\r\n");
                        __LastValue = "0";
                    }
                    else
                    {
                        error("is_null:" + arg2, false);
                        __LastValue = "-1";
                    }
                }
                else
                {
                    app(variables.at(indexOfVariable(arg1)).getString(), arg2 + "\r\n");
                    __LastValue = "0";
                }
            }
            else
            {
                touch(variables.at(indexOfVariable(arg1)).getString());

                if (isString(arg2))
                {
                    app(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg2)).getString() + "\r\n");
                    __LastValue = "1";
                }
                else if (isNumber(arg2))
                {
                    app(variables.at(indexOfVariable(arg1)).getString(), dtos(variables.at(indexOfVariable(arg2)).getNumber()) + "\r\n");
                    __LastValue = "1";
                }
                else
                {
                    error("is_null:" + arg2, false);
                    __LastValue = "-1";
                }

                __LastValue = "1";
            }
        }
        else
        {
            error("conversion_error:" + arg1, false);
            __LastValue = "-1";
        }
    }
    else
    {
        if (variableExists(arg2))
        {
            if (isString(arg2))
            {
                if (fileExists(arg1))
                {
                    app(arg1, variables.at(indexOfVariable(arg2)).getString() + "\r\n");
                    __LastValue = "0";
                }
                else
                {
                    touch(variables.at(indexOfVariable(arg2)).getString());
                    app(arg1, variables.at(indexOfVariable(arg2)).getString() + "\r\n");
                    __LastValue = "1";
                }
            }
            else
            {
                error("conversion_error:" + arg2, false);
                __LastValue = "-1";
            }
        }
        else
        {
            if (fileExists(arg1))
            {
                app(arg1, arg2 + "\r\n");
                __LastValue = "0";
            }
            else
            {
                touch(arg1);
                app(arg1, arg2 + "\r\n");
                __LastValue = "1";
            }
        }
    }
}

string getPrompt()
{
    string new_style("");
    int length = __PromptStyle.length();
    char prevChar = 'a';

    for (int i = 0; i < length; i++)
    {
        switch (__PromptStyle[i])
        {
        case 'u':
            if (prevChar == '\\')
                new_style.append(getUser());
            else
                new_style.push_back('u');
            break;

        case 'm':
            if (prevChar == '\\')
                new_style.append(getMachine());
            else
                new_style.push_back('m');
            break;

        case 'w':
            if (prevChar == '\\')
                new_style.append(cwd());
            else
                new_style.push_back('w');
            break;

        case '\\':
            break;

        default:
            new_style.push_back(__PromptStyle[i]);
            break;
        }

        prevChar = __PromptStyle[i];
    }

    return (new_style);
}

void cd(string p)
{
    if (containsTilde(p))
    {
        string cleaned("");
        int l = p.length();

        for (int i = 0; i < l; i++)
        {
            if (p[i] == '~')
            {
                if (__GuessedOS == "UNIXMacorLINUX")
                    cleaned.append(getEnvironmentVariable("HOME"));
                else if (__GuessedOS == "Win2000NTorXP")
                    cleaned.append(getEnvironmentVariable("HOMEPATH"));
                else if (__GuessedOS == "Win7orVista")
                    cleaned.append(getEnvironmentVariable("HOMEPATH"));
                else if (__GuessedOS == "UnknownWindowsOS")
                    cleaned.append(getEnvironmentVariable("HOME"));
                else
                    error("undefined_os", false);
            }
            else
                cleaned.push_back(p[i]);
        }

        if (directoryExists(cleaned))
            cd(cleaned);
        else
            error("read_fail:" + p, false);
    }
    else
    {
        if (p == "~")
        {
            if (__GuessedOS == "UNIXMacorLINUX" || __GuessedOS == "UnknownWindowsOS")
                cd(getEnvironmentVariable("HOME"));
            else
                cd(getEnvironmentVariable("HOMEPATH"));
        }
        else if (p == "init_dir" || p == "initial_directory")
            cd(__InitialDirectory);
        else
        {
            if (chdir(p.c_str()) != 0)
                error("read_fail:" + p, false);
        }
    }
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
                        cleaned.append(variables.at(indexOfVariable(builder)).getString());
                    else if (isNumber(builder))
                        cleaned.append(dtos(variables.at(indexOfVariable(builder)).getNumber()));
                    else
                        cleaned.append("null");
                }
                else if (methodExists(builder))
                {
                    parse(builder);

                    cleaned.append(__LastValue);
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
                            if (objects.at(indexOfObject(before)).methodExists(beforeParams(after)))
                            {
                                executeTemplate(objects.at(indexOfObject(before)).getMethod(beforeParams(after)), getParams(after));

                                cleaned.append(__LastValue);
                            }
                            else
                                error("invalid_operation:method_undefined:" + before + "." + beforeParams(after), false);
                        }
                        else
                            error("invalid_operation:object_undefined:" + before, false);
                    }
                    else if (methodExists(beforeParams(builder)))
                    {
                        executeTemplate(methods.at(indexOfMethod(beforeParams(builder))), getParams(builder));

                        cleaned.append(__LastValue);
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
                            string tempString(variables.at(indexOfVariable(_beforeBrackets)).getString());

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
                                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
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
                                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                    }
                                    else
                                        error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
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
                                        error("invalid_operation:index_out_of_bounds:" + afterBrackets, false);
                                }
                                else
                                    error("invalid_operation:invalid_range:" + afterBrackets, false);
                            }
                            else
                                error("invalid_operation:invalid_range:" + afterBrackets, false);
                        }
                    }
                    else if (listExists(_beforeBrackets))
                    {
                        if (listRange.size() == 2)
                        {
                            rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                            if (isNumeric(rangeBegin) && isNumeric(rangeEnd))
                            {
                                if (stoi(rangeBegin) < stoi(rangeEnd))
                                {
                                    if (lists.at(indexOfList(_beforeBrackets)).size() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                                    {
                                        string bigString("(");

                                        for (int z = stoi(rangeBegin); z <= stoi(rangeEnd); z++)
                                        {
                                            bigString.append("\"" + lists.at(indexOfList(_beforeBrackets)).at(z) + "\"");

                                            if (z < stoi(rangeEnd))
                                                bigString.push_back(',');
                                        }

                                        bigString.push_back(')');

                                        cleaned.append(bigString);
                                    }
                                    else
                                        error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                }
                                else if (stoi(rangeBegin) > stoi(rangeEnd))
                                {
                                    if (lists.at(indexOfList(_beforeBrackets)).size() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                                    {
                                        string bigString("(");

                                        for (int z = stoi(rangeBegin); z >= stoi(rangeEnd); z--)
                                        {
                                            bigString.append("\"" + lists.at(indexOfList(_beforeBrackets)).at(z) + "\"");

                                            if (z > stoi(rangeEnd))
                                                bigString.push_back(',');
                                        }

                                        bigString.push_back(')');

                                        cleaned.append(bigString);
                                    }
                                    else
                                        error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                }
                                else
                                    error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                            }
                            else
                                error("invalid_operationg:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                        }
                        else if (listRange.size() == 1)
                        {
                            rangeBegin = listRange.at(0);

                            if (isNumeric(rangeBegin))
                            {
                                if (stoi(rangeBegin) <= (int)lists.at(indexOfList(_beforeBrackets)).size() - 1 && stoi(rangeBegin) >= 0)
                                    cleaned.append(lists.at(indexOfList(_beforeBrackets)).at(stoi(rangeBegin)));
                                else
                                    error("invalid_operation:index_out_of_bounds:" + afterBrackets, false);
                            }
                            else
                                error("invalid_operation:invalid_range:" + afterBrackets, false);
                        }
                        else
                            error("invalid_operation:invalid_range:" + afterBrackets, false);
                    }
                    else
                        cleaned.append("null");
                }
                else if (!zeroDots(builder))
                {
                    string before(beforeDot(builder)), after(afterDot(builder));

                    if (objectExists(before))
                    {
                        if (objects.at(indexOfObject(before)).methodExists(after))
                        {
                            parse(before + "." + after);

                            cleaned.append(__LastValue);
                        }
                        else if (objects.at(indexOfObject(before)).variableExists(after))
                        {
                            if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                                cleaned.append(objects.at(indexOfObject(before)).getVariable(after).getString());
                            else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                                cleaned.append(dtos(objects.at(indexOfObject(before)).getVariable(after).getNumber()));
                            else
                                cleaned.append("null");
                        }
                        else
                            error("invalid_operation:variable_undefined:" + before + "." + after, false);
                    }
                    else
                        error("invalid_operation:object_undefined:" + before, false);
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
            if (st[i] == '\\' && st[i + 1] == 'n') // if begin new-line
                cleaned.push_back('\r');
            else if (st[i] == 'n' && st[i - 1] == '\\') // if end new-line
                cleaned.push_back('\n');
            else if (st[i] == '\\' && st[i + 1] == 't') // if begin tab
                doNothing();
            else if (st[i] == 't' && st[i - 1] == '\\') // if end tab
                cleaned.push_back('\t');
            else if (st[i] == '\\' && st[i + 1] == ';') // if begin semi-colon
                doNothing();
            else if (st[i] == ';' && st[i - 1] == '\\') // if end semi-colon
                cleaned.push_back(';');
            else if (st[i] == '\\' && st[i + 1] == '\'') // if begin apost
                doNothing();
            else if (st[i] == '\'' && st[i - 1] == '\\') // if end apost
                cleaned.push_back('\'');
            else if (st[i] == '\\' && st[i + 1] == '{') // if begin symbol
                buildSymbol = true;
            else
                cleaned.push_back(st[i]);
        }
    }

    return (cleaned);
}

void __stdout(string st)
{
    if (__CaptureParse)
        __ParsedOutput.append(cleanString(st));
    else
        cout << cleanString(st);

    if (__Logging)
        app(__LogFile, "[stdout]:" + st + "\r\n");
}

void say(string st)
{
    setLastValue(st);
    if (__GuessedOS == "UNIXMacorLINUX")
        __stdout(cleanString(st) + "\n");
    else
        __stdout(cleanString(st) + "\r\n");
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

void displayVersion()
{
    cout << "\r\nnoctis v0.0.0 by <scstauf@gmail.com>\r\n" << endl;
}

void error(string e, bool quit)
{
    if (__ExecutedTryBlock)
    {
        __RaiseCatchBlock = true;
        __LastError = e + "(" + __CurrentLine + ")";
    }
    else
    {
        if (__CaptureParse)
            __ParsedOutput.append("#!=" + itos(__CurrentLineNumber) + ":" + e + "(" + __CurrentLine + ")\r\n");
        else
            cerr << "#!=" << __CurrentLineNumber << ":" << e << "(" << __CurrentLine << ")" << endl;
    }

    if (__Logging)
        app(__LogFile, "#!=" + itos(__CurrentLineNumber) + ":" + e + "(" + __CurrentLine + ")\r\n");

    if (!__Negligence)
    {
        if (quit)
        {
            clearAll();
            exit(0);
        }
    }
}

string getParsedOutput(string cmd)
{
    __CaptureParse = true;
    parse(cmd);
    string ret = __ParsedOutput;
    __ParsedOutput.clear();
    __CaptureParse = false;

    return (ret);
}

List getDirectoryList(string before, bool filesOnly)
{
    List newList;
    int i = 1;

    DIR *pd;
    struct dirent *pe;

    string meat = variables.at(indexOfVariable(before)).getString();

    if ((pd = opendir(meat.c_str())) == NULL)
        __DefiningForLoop = false;
    else
    {
        while ((pe = readdir(pd)) != NULL)
        {
            if (string(pe->d_name) != "." && string(pe->d_name) != "..")
            {
                string tmp("");

                if (meat == "/")
                    meat = "";

                if (__GuessedOS == "UNIXMacorLINUX")
                    tmp = meat + "/" + string(pe->d_name);
                else
                    tmp = meat + "\\" + string(pe->d_name);

                if (filesOnly)
                {
                    if (fileExists(tmp))
                    {
                        newList.add(tmp);
                        i++;
                    }
                }
                else
                {
                    if (directoryExists(tmp))
                    {
                        newList.add(tmp);
                        i++;
                    }
                }
            }
        }
    }

    closedir(pd);

    return (newList);
}

Method getMethod(string s)
{
    Method bad_meth("[bad_meth#" + itos(__BadMethodCount) + "]");

    if (methodExists(s))
        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == s)
                return (methods.at(i));

    __BadMethodCount++;
    return (bad_meth);
}

Object getObject(string s)
{
    Object bad_obj("[bad_obj#" + itos(__BadObjectCount) + "]");

    if (objectExists(s))
        for (int i = 0; i < (int)objects.size(); i++)
            if (objects.at(i).name() == s)
                return (objects.at(i));

    __BadObjectCount++;

    return (bad_obj);
}

Variable getVariable(string s)
{
    Variable bad_var("[bad_var#" + itos(__BadVarCount) + "]");

    if (variableExists(s))
        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == s)
                return (variables.at(i));

    __BadVarCount++;

    return (bad_var);
}

bool listExists(string s)
{
    for (int i = 0; i < (int)lists.size(); i++)
        if (lists.at(i).name() == s)
            return (true);

    return (false);
}

bool methodExists(string s)
{
    if (!zeroDots(s))
    {
        if (objectExists(beforeDot(s)))
        {
            if (objects.at(indexOfObject(beforeDot(s))).methodExists(afterDot(s)))
                return (true);
            else
                return (false);
        }
    }
    else
        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == s)
                return (true);

    return (false);
}

bool objectExists(string s)
{
    for (int i = 0; i < (int)objects.size(); i++)
        if (objects.at(i).name() == s)
            return (true);

    return (false);
}

bool variableExists(string s)
{
    if (!zeroDots(s))
    {
        string before(beforeDot(s)), after(afterDot(s));

        if (objectExists(before))
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
                return (true);
            else
                return (false);
        }
        else
            return (false);
    }
    else
        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == s)
                return (true);

    return (false);
}

bool moduleExists(string s)
{
    for (int i = 0; i < (int)modules.size(); i++)
        if (modules.at(i).name() == s)
            return (true);

    return (false);
}

bool constantExists(string s)
{
    for (int i = 0; i < (int)constants.size(); i++)
        if (constants.at(i).name() == s)
            return (true);

    return (false);
}

bool noLists()
{
    if (lists.empty())
        return (true);

    return (false);
}

bool noMethods()
{
    if (methods.empty())
        return (true);

    return (false);
}

bool noObjects()
{
    if (objects.empty())
        return (true);

    return (false);
}

bool noVariables()
{
    if (variables.empty())
        return (true);

    return (false);
}

bool notObjectMethod(string s)
{
    if (zeroDots(s))
        return (true);
    else
    {
        string before(beforeDot(s));

        if (objectExists(before))
            return (false);
        else
            return (true);
    }

    return (true);
}

void __true()
{
    __stdout("true");
    setLastValue("true");
}

void __false()
{
    __stdout("false");
    setLastValue("false");
}

void saveVariable(string variableName)
{
    Crypt c;

    if (!fileExists(__SavedVars))
    {
        if (!directoryExists(__SavedVarsPath))
            md(__SavedVarsPath);

        touch(__SavedVars);
        app(__SavedVars, c.e(variableName));
    }
    else
    {
        string line, bigStr("");
        ifstream file(__SavedVars.c_str());

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
            rm(__SavedVars);
            touch(__SavedVars);
            app(__SavedVars, c.e(bigStr + "#" + variableName));
            file.close();
        }
        else
            error("read_fail:" + __SavedVars, false);
    }
}

vector<Method> removeMethod(vector<Method> v, string target)
{
    vector<Method> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return (cleanedVector);
}

vector<Object> removeObject(vector<Object> v, string target)
{
    vector<Object> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return (cleanedVector);
}

vector<Variable> removeVariable(vector<Variable> v, string target)
{
    vector<Variable> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return (cleanedVector);
}

vector<List> removeList(vector<List> v, string target)
{
    vector<List> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return (cleanedVector);
}

vector<Module> removeModule(vector<Module> v, string target)
{
    vector<Module> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return (cleanedVector);
}

vector<Constant> removeConstant(vector<Constant> v, string target)
{
    vector<Constant> cleanedVector;

    for (int i = 0; i < (int)v.size(); i++)
        if (v.at(i).name() != target)
            cleanedVector.push_back(v.at(i));

    return (cleanedVector);
}

void help(string app)
{
    cout << "\r\nnoctis by <scstauf@gmail.com>" << endl << endl
         << "usage:\t" << app << "\t\t\t// start the shell" << endl
         << "\t" << app << " {args}\t\t// ditto, with parameters" << endl
         << "\t" << app << " {script}\t\t// interpret a script" << endl
         << "\t" << app << " {script} {args}\t// ditto, with parameters" << endl
         << "\t" << app << " -n, --__Negligence\t// do not terminate on parse errors" << endl
         << "\t" << app << " -sl, --skipload\t// start the shell, with fresh memory" << endl
         << "\t" << app << " -l, --log {path}\t// create a shell log" << endl
         << "\t" << app << " -u, --uninstall\t// remove $HOME/.__SavedVarsPath" << endl
         << "\t" << app << " -v, --version\t// display current version" << endl
         << "\t" << app << " -p, --parse\t// parse a command" << endl
         << "\t" << app << " -h, --help\t// display this message" << endl << endl;
}

bool notStandardZeroSpace(string arg)
{
    if (arg != "caught" &&
            arg != "clear_all!" &&
            arg != "clear_lists!" &&
            arg != "clear_methods!" &&
            arg != "clear_objects!" &&
            arg != "clear_variables!" &&
            arg != "clear_constants!" &&
            arg != "exit" &&
            arg != "else" &&
            arg != "failif" &&
            arg != "help" &&
            arg != "leave!" && arg != "break" &&
            arg != "no_methods?" &&
            arg != "no_objects?" &&
            arg != "no_variables?" &&
            arg != "end" &&
            arg != "}" &&
            arg != "parser" &&
            arg != "private" &&
            arg != "public" &&
            arg != "try" &&
            arg != "pass")
        return (true);

    return (false);
}

bool notStandardOneSpace(string arg)
{
    if (arg != "!" &&
            arg != "?" &&
            arg != "__begin__" &&
            arg != "cd" && arg != "chdir" &&
            arg != "collect?" && arg!= "garbage?" &&
            arg != "decrypt" &&
            arg != "delay" &&
            arg != "encrypt" &&
            arg != "err" &&
            arg != "error" &&
            arg != "for" &&
            arg != "forget" &&
            arg != "globalize" &&
            arg != "goto" &&
            arg != "help" &&
            arg != "lose" &&
            arg != "init_dir" && arg != "intial_directory" &&
            arg != "is_method?" && arg != "method?" &&
            arg != "is_object?" && arg != "object?" &&
            arg != "is_variable?" && arg != "variable?" && arg != "var?" &&
            arg != "is_list?" && arg != "list?" &&
            arg != "is_directory?" && arg != "dir?" && arg != "directory?" &&
            arg != "is_file?" && arg != "file?" &&
            arg != "is_number?" && arg != "number?" &&
            arg != "is_string?" && arg != "string?" &&
            arg != "lowercase?" && arg != "lower?" && arg != "is_lowercase?" &&
            arg != "uppercase?" && arg != "upper?" && arg != "is_uppercase?" &&
            arg != "list" &&
            arg != "load" &&
            arg != "lock" &&
            arg != "unlock" &&
            arg != "loop" &&
            arg != "method" &&
            arg != "[method]" &&
            arg != "call_method" &&
            arg != "object" &&
            arg != "out" &&
            arg != "print" &&
            arg != "println" &&
            arg != "prompt" &&
            arg != "remember" &&
            arg != "remove" &&
            arg != "return" &&
            arg != "save" &&
            arg != "say" &&
            arg != "see" &&
            arg != "see_string" &&
            arg != "see_number" &&
            arg != "stdout" &&
            arg != "switch" &&
            arg != "template" &&
            arg != "fpush" &&
            arg != "fpop" &&
            arg != "dpush" &&
            arg != "dpop")
        return (true);

    return (false);
}

bool notStandardTwoSpace(string arg)
{
    if (arg != "=" &&
            arg != "+=" &&
            arg != "-=" &&
            arg != "*=" &&
            arg != "%=" &&
            arg != "/=" &&
            arg != "**=" &&
            arg != "+" &&
            arg != "-" &&
            arg != "*" &&
            arg != "**" &&
            arg != "/" &&
            arg != "%" &&
            arg != "++=" &&
            arg != "--=" &&
            arg != "?" &&
            arg != "!")
        return (true);

    return (false);
}

int indexOfMethod(string s)
{
    for (int i = 0; i < (int)methods.size(); i++)
    {
        if (methods.at(i).name() == s)
            return (i);
    }

    return (-1);
}

int indexOfObject(string s)
{
    for (int i = 0; i < (int)objects.size(); i++)
    {
        if (objects.at(i).name() == s)
            return (i);
    }

    return (-1);
}

int indexOfVariable(string s)
{
    for (int i = 0; i < (int)variables.size(); i++)
    {
        if (variables.at(i).name() == s)
            return (i);
    }

    return (-1);
}

int indexOfList(string s)
{
    for (int i = 0; i < (int)lists.size(); i++)
    {
        if (lists.at(i).name() == s)
            return (i);
    }

    return (-1);
}

int indexOfModule(string s)
{
    for (int i = 0; i < (int)modules.size(); i++)
    {
        if (modules.at(i).name() == s)
            return (i);
    }

    return (-1);
}

int indexOfScript(string s)
{
    for (int i = 0; i < (int)scripts.size(); i++)
    {
        if (scripts.at(i).name() == s)
            return (i);
    }

    return (-1);
}

int indexOfConstant(string s)
{
    for (int i = 0; i < (int)constants.size(); i++)
    {
        if (constants.at(i).name() == s)
            return (i);
    }

    return (-1);
}

bool is(string s, string si)
{
    if (s == ("-" + si) || s == ("--" + si) || s == ("/" + si))
        return (true);

    return (false);
}

bool isScript(string path)
{
    if (endsWith(path, ".us"))
        return (true);

    return (false);
}

void loadSavedVars(Crypt c, string &bigStr)
{
    string line("");
    ifstream file(__SavedVars.c_str());

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
        error("read_fail:" + __SavedVars, false);
}

void runScript()
{
    for (int i = 0; i < scripts.at(indexOfScript(__CurrentScript)).size(); i++)
    {
        __CurrentLineNumber = i + 1;

        if (!__GoToLabel)
            parse(scripts.at(indexOfScript(__CurrentScript)).at(i));
        else
        {
            bool startParsing = false;
            __DefiningIfStatement = false;
            __DefiningForLoop = false;
            __GoToLabel = false;

            for (int z = 0; z < scripts.at(indexOfScript(__CurrentScript)).size(); z++)
            {
                if (endsWith(scripts.at(indexOfScript(__CurrentScript)).at(z), "::"))
                {
                    string s(scripts.at(indexOfScript(__CurrentScript)).at(z));
                    s = subtractString(s, "::");

                    if (s == __GoTo)
                        startParsing = true;
                }

                if (startParsing)
                    parse(scripts.at(indexOfScript(__CurrentScript)).at(z));
            }
        }
    }

    __CurrentScript = __PreviousScript;
}

void loadScript(string script)
{
    string s("");
    ifstream f(script.c_str());
    __CurrentScript = script;

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
                    newScript.add(trim_leading_whitespace(c));
                    delete[] c;
                }
                else
                {
                    char * c = new char[s.size() + 1];
                    copy(s.begin(), s.end(), c);
                    c[s.size()] = '\0';
                    newScript.add(trim_leading_whitespace(c));
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

        if (fileExists(__SavedVars))
            loadSavedVars(c, bigStr);
    }

    while (active)
    {
        s.clear();

        if (__UseCustomPrompt)
        {
            if (__PromptStyle == "bash")
                cout << getUser() << "@" << getMachine() << "(" << cwd() << ")" << "$ ";
            else if (__PromptStyle == "empty")
                doNothing();
            else
                cout << getPrompt();
        }
        else
            cout << "> ";

        getline(cin, s, '\n');

        if (s[0] == '\t')
            s.erase(remove(s.begin(), s.end(), '\t'), s.end());

        if (s == "exit")
        {
            if (!__DefiningObject && !__DefiningMethod)
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
            parse(trim_leading_whitespace(c));
            delete[] c;
        }
    }
}

void whileLoop(Method m)
{
    for (int i = 0; i < m.size(); i++)
    {
        if (m.at(i) == "leave!")
            __Breaking = true;
        else
            parse(m.at(i));
    }
}

void forLoop(Method m)
{
    __DefaultLoopSymbol = "$";

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

            if (__Breaking == true)
            {
                __Breaking = false;
                break;
            }
        }
    }
    else
    {
        if (m.isInfinite())
        {
            if (__Negligence)
            {
                for (;;)
                {
                    for (int z = 0; z < m.size(); z++)
                        parse(m.at(z));

                    if (__Breaking == true)
                    {
                        __Breaking = false;
                        break;
                    }
                }
            }
            else
                error("infinite_loop", true);
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

                if (__Breaking == true)
                {
                    __Breaking = false;
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

                if (__Breaking == true)
                {
                    __Breaking = false;
                    break;
                }
            }
        }
        else
            error("special_error(5)", false);
    }
}

void executeNest(Container n)
{
    __DefiningNest = false;
    __DefiningIfStatement = false;

    for (int i = 0; i < n.size(); i++)
    {
        if (__FailedNest == false)
            parse(n.at(i));
        else
            break;
    }

    __DefiningIfStatement = true;
}

void collectGarbage()
{
    vector<string> garbageVars;

    for (int i = 0; i < (int)variables.size(); i++)
        if (variables.at(i).garbage() && !__ExecutedIfStatement)
            if (!__DontCollectMethodVars)
                garbageVars.push_back(variables.at(i).name());

    for (int i = 0; i < (int)garbageVars.size(); i++)
        variables = removeVariable(variables, garbageVars.at(i));

    vector<string> garbageLists;

    for (int i = 0; i < (int)lists.size(); i++)
        if (lists.at(i).garbage() && !__ExecutedIfStatement)
            garbageLists.push_back(lists.at(i).name());

    for (int i = 0; i < (int)garbageLists.size(); i++)
        lists = removeList(lists, garbageLists.at(i));

    vector<string> garbageObjects;

    for (int i = 0; i < (int)objects.size(); i++)
        if (objects.at(i).garbage() && !__ExecutedIfStatement)
            garbageObjects.push_back(objects.at(i).name());

    for (int i = 0; i < (int)garbageObjects.size(); i++)
        objects = removeObject(objects, garbageObjects.at(i));
}

void executeTemplate(Method m, vector<string> strings)
{
    vector<string> methodLines;

    __ExecutedTemplate = true;
    __DontCollectMethodVars = true;
    __CurrentMethodObject = m.getObject();

    vector<Variable> methodVariables = m.getMethodVariables();

    for (int i = 0; i < (int)methodVariables.size(); i++)
    {
        if (variableExists(strings.at(i)))
        {
            if (isString(strings.at(i)))
                createVariable(methodVariables.at(i).name(), variables.at(indexOfVariable(strings.at(i))).getString());
            else if (isNumber(strings.at(i)))
                createVariable(methodVariables.at(i).name(), variables.at(indexOfVariable(strings.at(i))).getNumber());
        }
        else if (methodExists(strings.at(i)))
        {
            parse(strings.at(i));

            if (isNumeric(__LastValue))
                createVariable(methodVariables.at(i).name(), stod(__LastValue));
            else
                createVariable(methodVariables.at(i).name(), __LastValue);
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

    __ExecutedTemplate = false, __DontCollectMethodVars = false;

    collectGarbage(); // if (!__DontCollectMethodVars)
}

void executeMethod(Method m)
{
    __ExecutedMethod = true;
    __CurrentMethodObject = m.getObject();

    if (__DefiningParameterizedMethod)
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

                        if (m.getMethodVariables().at(a).getString() != __Null)
                            newWords.push_back(m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getNumber() != __NullNum)
                            newWords.push_back(dtos(m.getMethodVariables().at(a).getNumber()));
                    }
                    else if (words.at(x) == variableString)
                    {
                        found = true;

                        if (m.getMethodVariables().at(a).getString() != __Null)
                            newWords.push_back(m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getNumber() != __NullNum)
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

    __ExecutedMethod = false;

    collectGarbage();
}

bool suc_stat = false;

bool success()
{
    return (suc_stat);
}

void failedFor()
{
    Method forMethod("[for#" + itos(__ForLoopCount) + "]");
    forMethod.setFor(false);
    __DefiningForLoop = true;
    forLoops.push_back(forMethod);
    __DefaultLoopSymbol = "$";
    suc_stat = false;
}

void failedWhile()
{
    Method whileMethod("[while#" + itos(__WhileLoopCount) + "]");
    whileMethod.setWhile(false);
    __DefiningWhileLoop = true;
    whileLoops.push_back(whileMethod);
}

void successfullWhile(string v1, string op, string v2)
{
    Method whileMethod("[while#" + itos(__WhileLoopCount) + "]");
    whileMethod.setWhile(true);
    whileMethod.setWhileValues(v1, op, v2);
    __DefiningWhileLoop = true;
    whileLoops.push_back(whileMethod);
    __WhileLoopCount++;
}

void successfulFor(List list)
{
    Method forMethod("[for#" + itos(__ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setForList(list);
    forMethod.setListLoop();
    forMethod.setSymbol(__DefaultLoopSymbol);
    __DefiningForLoop = true;
    forLoops.push_back(forMethod);
    __ForLoopCount++;
    suc_stat = true;
}

void successfulFor(double a, double b, string op)
{
    Method forMethod("[for#" + itos(__ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setSymbol(__DefaultLoopSymbol);

    if (op == "<=")
        forMethod.setForValues((int)a, (int)b);
    else if (op == ">=")
        forMethod.setForValues((int)a, (int)b);
    else if (op == "<")
        forMethod.setForValues((int)a, (int)b - 1);
    else if (op == ">")
        forMethod.setForValues((int)a, (int)b + 1);

    __DefiningForLoop = true;
    forLoops.push_back(forMethod);
    __ForLoopCount++;
    suc_stat = true;
}

void successfulFor()
{
    Method forMethod("[for#" + itos(__ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setInfinite();
    __DefiningForLoop = true;
    forLoops.push_back(forMethod);
    __ForLoopCount++;
    suc_stat = true;
}

void failedIfStatement()
{
    __LastValue = "false";

    if (!__DefiningNest)
    {
        Method ifMethod("[failif]");
        ifMethod.setBool(false);
        __DefiningIfStatement = true;
        ifStatements.push_back(ifMethod);
        __FailedIfStatement = true;
        __FailedNest = true;
    }
    else
        __FailedNest = true;
}

void successfulIF()
{
    __LastValue = "true";

    if (__DefiningNest)
    {
        ifStatements.at((int)ifStatements.size() - 1).buildNest();
        __FailedNest = false;
    }
    else
    {
        Method ifMethod("[if#" + itos(__IfStatementCount) +"]");
        ifMethod.setBool(true);
        __DefiningIfStatement = true;
        ifStatements.push_back(ifMethod);
        __IfStatementCount++;
        __FailedIfStatement = false;
        __FailedNest = false;
    }
}

bool stackReady(string arg2)
{
    if (contains(arg2, "+") || contains(arg2, "-") || contains(arg2, "*") || contains(arg2, "/") || contains(arg2, "%") || contains(arg2, "^"))
        return (true);

    return (false);
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
                        return (true);
                }
                else if (methodExists(temporaryBuild))
                {
                    parse(temporaryBuild);

                    if (isNumeric(__LastValue))
                        temporaryBuild.clear();
                    else
                        return (true);
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
                    return (true);
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            }
            else if (!isNumeric(temporaryBuild))
                return (true);
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
                    return (true);
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            }
            else if (!isNumeric(temporaryBuild))
                return (true);
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
                    return (true);
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            }
            else if (!isNumeric(temporaryBuild))
                return (true);
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
                    return (true);
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            }
            else if (!isNumeric(temporaryBuild))
                return (true);
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
                    return (true);
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            }
            else if (!isNumeric(temporaryBuild))
                return (true);
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
                    return (true);
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            }
            else if (!isNumeric(temporaryBuild))
                return (true);
            else
                temporaryBuild.clear();
        }
        else
            temporaryBuild.push_back(tempArgTwo[i]);
    }

    return (false);
}

string getStringStack(string arg2)
{
    string tempArgTwo = arg2, temporaryBuild("");
    tempArgTwo = subtractChar(tempArgTwo, "(");
    tempArgTwo = subtractChar(tempArgTwo, ")");

    string stackValue("");

    vector<string> vars;
    vector<string> contents;

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
                        contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                        temporaryBuild.clear();
                    }
                    else if (isString(temporaryBuild))
                    {
                        vars.push_back(temporaryBuild);
                        contents.push_back(variables.at(indexOfVariable(temporaryBuild)).getString());
                        temporaryBuild.clear();
                    }
                }
                else if (methodExists(temporaryBuild))
                {
                    parse(temporaryBuild);

                    contents.push_back(__LastValue);
                    temporaryBuild.clear();
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("+");
                }
                else if (isString(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(variables.at(indexOfVariable(temporaryBuild)).getString());
                    temporaryBuild.clear();
                    contents.push_back("+");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                contents.push_back(__LastValue);
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("-");
                }
                else if (isString(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(variables.at(indexOfVariable(temporaryBuild)).getString());
                    temporaryBuild.clear();
                    contents.push_back("-");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                contents.push_back(__LastValue);
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("*");
                }
                else if (isString(temporaryBuild))
                {
                    vars.push_back(temporaryBuild);
                    contents.push_back(variables.at(indexOfVariable(temporaryBuild)).getString());
                    temporaryBuild.clear();
                    contents.push_back("*");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                contents.push_back(__LastValue);
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
            contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
            temporaryBuild.clear();
        }
        else if (isString(temporaryBuild))
        {
            vars.push_back(temporaryBuild);
            contents.push_back(variables.at(indexOfVariable(temporaryBuild)).getString());
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

    if (__Returning)
    {
        for (int i = 0; i < (int)vars.size(); i++)
            variables = removeVariable(variables, vars.at(i));

        __Returning = false;
    }

    return (stackValue);
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
                        contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                        temporaryBuild.clear();
                    }
                }
                else if (methodExists(temporaryBuild))
                {
                    parse(temporaryBuild);

                    if (isNumeric(__LastValue))
                    {
                        contents.push_back(__LastValue);
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("+");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                {
                    contents.push_back(__LastValue);
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("-");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                {
                    contents.push_back(__LastValue);
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("*");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                {
                    contents.push_back(__LastValue);
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("/");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                {
                    contents.push_back(__LastValue);
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("%");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                {
                    contents.push_back(__LastValue);
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
                    contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("^");
                }
            }
            else if (methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (isNumeric(__LastValue))
                {
                    contents.push_back(__LastValue);
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
            contents.push_back(dtos(variables.at(indexOfVariable(temporaryBuild)).getNumber()));
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

    if (__Returning)
    {
        for (int i = 0; i < (int)vars.size(); i++)
            variables = removeVariable(variables, vars.at(i));

        __Returning = false;
    }

    return (stackValue);
}

bool __IsCommented, __MultilineComment;

int sysExec(string s, vector<string> command)
{
//    string _cleaned;
//	_cleaned = cleanString(s);
    for (int i = 0; i < (int)methods.size(); i++)
    {
        if (command.at(0) == methods.at(i).name())
        {
            if ((int)command.size() - 1 == (int)methods.at(i).getMethodVariables().size())
            {
                // work
            }
        }
    }
    return system(cleanString(s).c_str());
}

/**
	The heart of it all. Parse a string and send for interpretation.
**/
void parse(string s)
{
    vector<string> command; // a tokenized command container
    int length = s.length(), //	length of the line
        count = 0, // command token counter
        size = 0; // final size of tokenized command container
    bool quoted = false, // flag: parsing string literals
         broken = false, // flag: end of a command
         uncomment = false, // flag: end a command
         parenthesis = false; // flag: parsing contents within parentheses
    char prevChar = 'a'; // previous character in string

    StringContainer stringContainer; // contains separate commands
    string bigString(""); // a string to build upon

    __CurrentLine = s; // store a copy of the current line
    if (__Logging) app(__LogFile, s + "\r\n"); // if __Logging a session, log the line

    command.push_back(""); // push back an empty string to begin.
    // iterate each char in the initial string
    for (int i = 0; i < length; i++)
    {
        switch (s[i])
        {
        case ' ':
            /**
            	we can push a space onto the string if:
            		parsing a string literal AND not within parentheses AND not in comment mode
            **/
            if (quoted && !parenthesis)
            {
                if (!__IsCommented)
                    command.at(count).push_back(' ');
            }
            else if (parenthesis && !quoted)
                doNothing();
            else if (parenthesis && quoted)
            {
                if (!__IsCommented)
                    command.at(count).push_back(' ');
            }
            else
            {
                if (!__IsCommented)
                {
                    if (prevChar != ' ')
                    {
                        command.push_back("");
                        count++;
                    }
                }
            }

            bigString.push_back(' ');
            break;

        case '\"':
            if (!quoted)
                quoted = true;
            else
                quoted = false;

            bigString.push_back('\"');
            break;

        case '(':
            if (!parenthesis)
                parenthesis = true;

            command.at(count).push_back('(');

            bigString.push_back('(');
            break;

        case ')':
            if (parenthesis)
                parenthesis = false;

            command.at(count).push_back(')');
            bigString.push_back(')');
            break;

        case '\\':
            if (quoted || parenthesis)
            {
                if (!__IsCommented)
                    command.at(count).push_back('\\');
            }

            bigString.push_back('\\');
            break;

        case '\'':
            if (quoted || parenthesis)
            {
                if (prevChar == '\\')
                    command.at(count).append("\'");
                else
                    command.at(count).append("\"");

                bigString.push_back('\'');
            }
            break;

        case '#':
            if (quoted || parenthesis)
                command.at(count).push_back('#');
            else if (prevChar == '#' && __MultilineComment == false)
            {
                __MultilineComment = true;
                __IsCommented = true;
                uncomment = false;
            }
            else if (prevChar == '#' && __MultilineComment == true)
                uncomment = true;
            else if (prevChar != '#' && __MultilineComment == false)
            {
                __IsCommented = true;
                uncomment = true;
            }

            bigString.push_back('#');
            break;

        case '~':
            if (!__IsCommented)
            {
                if (prevChar == '\\')
                    command.at(count).push_back('~');
                else
                {
                    if (__GuessedOS == "UNIXMacorLinux")
                        command.at(count).append(getEnvironmentVariable("HOME"));
                    else
                        command.at(count).append(getEnvironmentVariable("HOMEPATH"));
                }
            }
            bigString.push_back('~');
            break;

        case ';':
            if (!quoted)
            {
                if (!__IsCommented)
                {
                    broken = true;
                    stringContainer.add(bigString);
                    bigString = "";
                    count = 0;
                    command.clear();
                    command.push_back("");
                }
            }
            else
            {
                bigString.push_back(';');
                command.at(count).push_back(';');
            }
            break;

        default:
            if (!__IsCommented)
                command.at(count).push_back(s[i]);
            bigString.push_back(s[i]);
            break;
        }

        prevChar = s[i];
    }

    size = (int)command.size();

    if (command.at(size - 1) == "{" && size != 1)
        command.pop_back();

    size = (int)command.size();

    if (!__IsCommented)
    {
        if (!broken)
        {
            for (int i = 0; i < size; i++)
            {
                // handle arguments
                // args[0], args[1], ..., args[n-1]
                if (contains(command.at(i), "args") && command.at(i) != "args.size")
                {
                    vector<string> params = getBracketRange(command.at(i));

                    if (isNumeric(params.at(0)))
                    {
                        if ((int)args.size() - 1 >= stoi(params.at(0)) && stoi(params.at(0)) >= 0)
                        {
                            if (params.at(0) == "0")
                                command.at(i) = __CurrentScript;
                            else
                                command.at(i) = args.at(stoi(params.at(0)));
                        }
                        else
                            error("invalid_operation:index_out_of_bounds:" + command.at(i), false);
                    }
                    else
                        error("invalid_operation:invalid_range:" + command.at(i), false);
                }
            }

            if (__DefiningSwitchBlock)
            {
                if (s == "{")
                    doNothing();
                else if (startsWith(s, "case"))
                    mainSwitch.addCase(command.at(1));
                else if (s == "default")
                    __InDefaultCase = true;
                else if (s == "end" || s == "}")
                {
                    string switch_value("");

                    if (isString(__SwitchVarName))
                        switch_value = variables.at(indexOfVariable(__SwitchVarName)).getString();
                    else if (isNumber(__SwitchVarName))
                        switch_value = dtos(variables.at(indexOfVariable(__SwitchVarName)).getNumber());
                    else
                        switch_value = "";

                    Container rightCase = mainSwitch.rightCase(switch_value);

                    __InDefaultCase = false;
                    __DefiningSwitchBlock = false;

                    for (int i = 0; i < (int)rightCase.size(); i++)
                        parse(rightCase.at(i));

                    mainSwitch.clear();
                }
                else
                {
                    if (__InDefaultCase)
                        mainSwitch.addToDefault(s);
                    else
                        mainSwitch.addToCase(s);
                }
            }
            else if (__DefiningModule)
            {
                if (s == ("[/" + __CurrentModule + "]"))
                {
                    __DefiningModule = false;
                    __CurrentModule = "";
                }
                else
                    modules.at(indexOfModule(__CurrentModule)).add(s);
            }
            else if (__DefiningScript)
            {
                if (s == "__end__")
                {
                    __CurrentScriptName = "";
                    __DefiningScript = false;
                }
                else
                    app(__CurrentScriptName, s + "\n");
            }
            else
            {
                if (__RaiseCatchBlock)
                {
                    if (s == "catch")
                        __RaiseCatchBlock = false;
                }
                else if (__ExecutedTryBlock && s == "catch")
                    __SkipCatchBlock = true;
                else if (__ExecutedTryBlock && __SkipCatchBlock)
                {
                    if (s == "caught")
                    {
                        __SkipCatchBlock = false;
                        parse("caught");
                    }
                }
                else if (__DefiningMethod)
                {
                    if (contains(s, "while"))
                        __DefiningLocalWhileLoop = true;

                    if (contains(s, "switch"))
                        __DefiningLocalSwitchBlock = true;

                    if (__DefiningParameterizedMethod)
                    {
                        if (s == "{")
                            doNothing();
                        else if (s == "end" || s == "}")
                        {
                            if (__DefiningLocalWhileLoop)
                            {
                                __DefiningLocalWhileLoop = false;

                                if (__DefiningObject)
                                    objects.at(indexOfObject(__CurrentObject)).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                            else if (__DefiningLocalSwitchBlock)
                            {
                                __DefiningLocalSwitchBlock = false;

                                if (__DefiningObject)
                                    objects.at(indexOfObject(__CurrentObject)).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                            else
                            {
                                __DefiningMethod = false;

                                if (__DefiningObject)
                                {
                                    __DefiningObjectMethod = false;
                                    objects.at(objects.size() - 1).setCurrentMethod("");
                                }
                            }
                        }
                        else
                        {
                            int _len = s.length();
                            vector<string> words;
                            string word("");

                            for (int z = 0; z < _len; z++)
                            {
                                if (s[z] == ' ')
                                {
                                    words.push_back(word);
                                    word.clear();
                                }
                                else
                                    word.push_back(s[z]);
                            }

                            words.push_back(word);

                            string freshLine("");

                            for (int z = 0; z < (int)words.size(); z++)
                            {
                                if (variableExists(words.at(z)))
                                {
                                    if (isString(words.at(z)))
                                        freshLine.append(variables.at(indexOfVariable(words.at(z))).getString());
                                    else if (isNumber(words.at(z)))
                                        freshLine.append(dtos(variables.at(indexOfVariable(words.at(z))).getNumber()));
                                }
                                else
                                    freshLine.append(words.at(z));

                                if (z != (int)words.size() - 1)
                                    freshLine.push_back(' ');
                            }

                            if (__DefiningObject)
                            {
                                objects.at(indexOfObject(__CurrentObject)).addToCurrentMethod(freshLine);

                                if (__DefiningPublicCode)
                                    objects.at(indexOfObject(__CurrentObject)).setPublic();
                                else if (__DefiningPrivateCode)
                                    objects.at(indexOfObject(__CurrentObject)).setPrivate();
                                else
                                    objects.at(indexOfObject(__CurrentObject)).setPublic();
                            }
                            else
                                methods.at(methods.size() - 1).add(freshLine);
                        }
                    }
                    else
                    {
                        if (s == "{")
                            doNothing();
                        else if (s == "end" || s == "}")
                        {
                            if (__DefiningLocalWhileLoop)
                            {
                                __DefiningLocalWhileLoop = false;

                                if (__DefiningObject)
                                    objects.at(objects.size() - 1).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                            else if (__DefiningLocalSwitchBlock)
                            {
                                __DefiningLocalSwitchBlock = false;

                                if (__DefiningObject)
                                    objects.at(objects.size() - 1).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                            else
                            {
                                __DefiningMethod = false;

                                if (__DefiningObject)
                                {
                                    __DefiningObjectMethod = false;
                                    objects.at(objects.size() - 1).setCurrentMethod("");
                                }
                            }
                        }
                        else
                        {
                            if (__DefiningObject)
                            {
                                objects.at(objects.size() - 1).addToCurrentMethod(s);

                                if (__DefiningPublicCode)
                                    objects.at(objects.size() - 1).setPublic();
                                else if (__DefiningPrivateCode)
                                    objects.at(objects.size() - 1).setPrivate();
                                else
                                    objects.at(objects.size() - 1).setPublic();
                            }
                            else
                            {
                                if (__DefiningObjectMethod)
                                {
                                    objects.at(objects.size() - 1).addToCurrentMethod(s);

                                    if (__DefiningPublicCode)
                                        objects.at(objects.size() - 1).setPublic();
                                    else if (__DefiningPrivateCode)
                                        objects.at(objects.size() - 1).setPrivate();
                                    else
                                        objects.at(objects.size() - 1).setPublic();
                                }
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                        }
                    }
                }
                else if (__DefiningIfStatement)
                {
                    if (__DefiningNest)
                    {
                        if (command.at(0) == "endif")
                            executeNest(ifStatements.at((int)ifStatements.size() - 1).getNest());
                        else
                            ifStatements.at((int)ifStatements.size() - 1).inNest(s);
                    }
                    else
                    {
                        if (command.at(0) == "if")
                        {
                            __DefiningNest = true;

                            if (size == 4)
                                threeSpace("if", command.at(1), command.at(2), command.at(3), s, command);
                            else
                            {
                                failedIfStatement();
                                __DefiningNest = false;
                            }
                        }
                        else if (command.at(0) == "endif")
                        {
                            __DefiningIfStatement = false;
                            __ExecutedIfStatement = true;

                            for (int i = 0; i < (int)ifStatements.size(); i++)
                            {
                                if (ifStatements.at(i).isIF())
                                {
                                    executeMethod(ifStatements.at(i));

                                    if (__FailedIfStatement == false)
                                        break;
                                }
                            }

                            __ExecutedIfStatement = false;

                            ifStatements.clear();

                            __IfStatementCount = 0;
                            __FailedIfStatement = false;
                        }
                        else if (command.at(0) == "elsif" || command.at(0) == "elif")
                        {
                            if (size == 4)
                                threeSpace("if", command.at(1), command.at(2), command.at(3), s, command);
                            else
                                failedIfStatement();
                        }
                        else if (s == "else")
                            threeSpace("if", "true", "is", "true", "if true is true", command);
                        else if (s == "failif")
                        {
                            if (__FailedIfStatement == true)
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                            ifStatements.at((int)ifStatements.size() - 1).add(s);
                    }
                }
                else
                {
                    if (__DefiningWhileLoop)
                    {
                        if (s == "{")
                            doNothing();
                        else if (command.at(0) == "end" || command.at(0) == "}")
                        {
                            __DefiningWhileLoop = false;

                            string v1 = whileLoops.at(whileLoops.size() - 1).valueOne(),
                                   v2 = whileLoops.at(whileLoops.size() - 1).valueTwo(),
                                   op = whileLoops.at(whileLoops.size() - 1).logicOperator();

                            if (variableExists(v1) && variableExists(v2))
                            {
                                if (op == "==" || op == "is")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() == variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == "<")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() < variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == ">")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() > variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == "<=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() <= variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == ">=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() >= variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == "!=" || op == "not")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() != variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                            }
                            else if (variableExists(v1))
                            {
                                if (op == "==" || op == "is")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() == stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == "<")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() < stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == ">")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() > stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == "<=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() <= stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == ">=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() >= stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                                else if (op == "!=" || op == "not")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() != stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (__Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    __WhileLoopCount = 0;
                                }
                            }
                            else
                                error("special_error(8)", false);
                        }
                        else
                            whileLoops.at(whileLoops.size() - 1).add(s);
                    }
                    else if (__DefiningForLoop)
                    {
                        if (command.at(0) == "next" || command.at(0) == "endfor")
                        {
                            __DefiningForLoop = false;

                            for (int i = 0; i < (int)forLoops.size(); i++)
                                if (forLoops.at(i).isForLoop())
                                    forLoop(forLoops.at(i));

                            forLoops.clear();

                            __ForLoopCount = 0;
                        }
                        else
                        {
                            if (s == "{")
                                doNothing();
                            else
                                forLoops.at(forLoops.size() - 1).add(s);
                        }
                    }
                    else
                    {
                        if (size == 1)
                        {
                            if (notStandardZeroSpace(command.at(0)))
                            {
                                string before(beforeDot(s)), after(afterDot(s));

                                if (before.length() != 0 && after.length() != 0)
                                {
                                    if (objectExists(before) && after.length() != 0)
                                    {
                                        if (containsParams(after))
                                        {
                                            s = subtractChar(s, "\"");

                                            if (objects.at(indexOfObject(before)).methodExists(beforeParams(after)))
                                                executeTemplate(objects.at(indexOfObject(before)).getMethod(beforeParams(after)), getParams(after));
                                            else
                                                sysExec(s, command);
                                        }
                                        else if (objects.at(indexOfObject(before)).methodExists(after))
                                            executeMethod(objects.at(indexOfObject(before)).getMethod(after));
                                        else if (objects.at(indexOfObject(before)).variableExists(after))
                                        {
                                            if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                                                say(objects.at(indexOfObject(before)).getVariable(after).getString());
                                            else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                                                say(dtos(objects.at(indexOfObject(before)).getVariable(after).getNumber()));
                                            else
                                                error("is_null", false);
                                        }
                                        else if (after == "clear")
                                            objects.at(indexOfObject(before)).clear();
                                        else
                                            error("undefined", false);
                                    }
                                    else
                                    {
                                        if (before == "env")
                                        {
                                            InternalGetEnv("", after, 3);
                                        }
                                        else if (variableExists(before))
                                        {
                                            if (after == "clear")
                                                parse(before + " = __Null");
                                        }
                                        else if (listExists(before))
                                        {
                                            // REFACTOR HERE
                                            if (after == "clear")
                                                lists.at(indexOfList(before)).clear();
                                            else if (after == "sort")
                                                lists.at(indexOfList(before)).listSort();
                                            else if (after == "reverse")
                                                lists.at(indexOfList(before)).listReverse();
                                            else if (after == "revert")
                                                lists.at(indexOfList(before)).listRevert();
                                        }
                                        else if (before == "self")
                                        {
                                            if (__ExecutedMethod)
                                                executeMethod(objects.at(indexOfObject(__CurrentMethodObject)).getMethod(after));
                                        }
                                        else
                                            sysExec(s, command);
                                    }
                                }
                                else if (endsWith(s, "::"))
                                {
                                    if (__CurrentScript != "")
                                    {
                                        string newMark(s);
                                        newMark = subtractString(s, "::");
                                        scripts.at(indexOfScript(__CurrentScript)).addMark(newMark);
                                    }
                                }
                                else if (methodExists(s))
                                    executeMethod(getMethod(s));
                                else if (startsWith(s, "[") && endsWith(s, "]"))
                                {
                                    InternalCreateModule(s);
                                }
                                else
                                {
                                    s = subtractChar(s, "\"");

                                    if (methodExists(beforeParams(s)))
                                        executeTemplate(getMethod(beforeParams(s)), getParams(s));
                                    else
                                        sysExec(s, command);
                                }
                            }
                            else
                                zeroSpace(command.at(0), s, command);
                        }
                        else if (size == 2)
                        {
                            if (notStandardOneSpace(command.at(0)))
                                sysExec(s, command);
                            else
                            {
                                oneSpace(command.at(0), command.at(1), s, command);
                            }
                        }
                        else if (size == 3)
                        {
                            if (notStandardTwoSpace(command.at(1)))
                            {
                                if (command.at(0) == "append")
                                    appendText(command.at(1), command.at(2), false);
                                else if (command.at(0) == "appendl")
                                    appendText(command.at(1), command.at(2), true);
                                else if ((command.at(0) == "fwrite"))
                                    __fwrite(command.at(1), command.at(2));
                                else if (command.at(0) == "redefine")
                                    redefine(command.at(1), command.at(2));
                                else if (command.at(0) == "loop")
                                {
                                    if (containsParams(command.at(2)))
                                    {
                                        __DefaultLoopSymbol = command.at(2);
                                        __DefaultLoopSymbol = subtractChar(__DefaultLoopSymbol, "(");
                                        __DefaultLoopSymbol = subtractChar(__DefaultLoopSymbol, ")");

                                        oneSpace(command.at(0), command.at(1), subtractString(s, command.at(2)), command);
                                        __DefaultLoopSymbol = "$";
                                    }
                                    else
                                        sysExec(s, command);
                                }
                                else
                                    sysExec(s, command);
                            }
                            else
                                twoSpace(command.at(0), command.at(1), command.at(2), s, command);
                        }
                        else if (size == 4)
                            threeSpace(command.at(0), command.at(1), command.at(2), command.at(3), s, command);
                        else if (size == 5)
                        {
                            if (command.at(0) == "for")
                            {
                                if (containsParams(command.at(4)))
                                {
                                    __DefaultLoopSymbol = command.at(4);
                                    __DefaultLoopSymbol = subtractChar(__DefaultLoopSymbol, "(");
                                    __DefaultLoopSymbol = subtractChar(__DefaultLoopSymbol, ")");

                                    threeSpace(command.at(0), command.at(1), command.at(2), command.at(3), subtractString(s, command.at(4)), command);
                                    __DefaultLoopSymbol = "$";
                                }
                                else
                                    sysExec(s, command);
                            }
                            else
                                sysExec(s, command);
                        }
                        else
                            sysExec(s, command);
                    }
                }
            }
        }
        else
        {
            stringContainer.add(bigString);

            for (int i = 0; i < (int)stringContainer.get().size(); i++)
                parse(stringContainer.at(i));
        }
    }
    else
    {
        if (__MultilineComment)
        {
            if (uncomment)
            {
                __IsCommented = false;
                __MultilineComment = false;
            }
        }
        else
        {
            if (uncomment)
            {
                __IsCommented = false;
                uncomment = false;

                if (!broken)
                {
                    string commentString("");

                    bool commentFound = false;

                    for (int i = 0; i < (int)bigString.length(); i++)
                    {
                        if (bigString[i] == '#')
                            commentFound = true;

                        if (!commentFound)
                            commentString.push_back(bigString[i]);
                    }

                    char * c = new char[commentString.size() + 1];
                    copy(commentString.begin(), commentString.end(), c);
                    c[commentString.size()] = '\0';
                    parse(trim_leading_whitespace(c));
                    delete[] c;
                }
                else
                {
                    string commentString("");

                    bool commentFound = false;

                    for (int i = 0; i < (int)bigString.length(); i++)
                    {
                        if (bigString[i] == '#')
                            commentFound = true;

                        if (!commentFound)
                            commentString.push_back(bigString[i]);
                    }

                    char * c = new char[commentString.size() + 1];
                    copy(commentString.begin(), commentString.end(), c);
                    c[commentString.size()] = '\0';

                    stringContainer.add(trim_leading_whitespace(c));

                    delete[] c;

                    for (int i = 0; i < (int)stringContainer.get().size(); i++)
                        parse(stringContainer.at(i));
                }
            }
        }
    }
}

/**
	Give a new name to anything.
**/
void redefine(string target, string name)
{
    if (variableExists(target))
    {
        if (fileExists(variables.at(indexOfVariable(target)).getString()) || directoryExists(variables.at(indexOfVariable(target)).getString()))
        {
            string old_name(variables.at(indexOfVariable(target)).getString()), new_name("");

            if (variableExists(name))
            {
                if (isString(name))
                {
                    new_name = variables.at(indexOfVariable(name)).getString();

                    if (fileExists(old_name))
                    {
                        if (!fileExists(new_name))
                        {
                            if (fileExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error("invalid_operation:file_undefined:" + old_name, false);
                        }
                        else
                            error("invalid_operation:file_defined:" + new_name, false);
                    }
                    else if (directoryExists(old_name))
                    {
                        if (!directoryExists(new_name))
                        {
                            if (directoryExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error("invalid_operation:directory_undefined:" + old_name, false);
                        }
                        else
                            error("invalid_operation:directory_defined:" + new_name, false);
                    }
                    else
                        error("invalid_operation:target_undefined:" + old_name, false);
                }
                else
                    error("invalid_operation:__Null_string:" + name, false);
            }
            else
            {
                if (fileExists(old_name))
                {
                    if (!fileExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error("invalid_operation:file_defined:" + name, false);
                }
                else if (directoryExists(old_name))
                {
                    if (!directoryExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error("invalid_operation:directory_defined:" + name, false);
                }
                else
                    error("invalid_operation:target_undefined:" + old_name, false);
            }
        }
        else
        {
            if (startsWith(name, "@"))
            {
                if (!variableExists(name))
                    variables.at(indexOfVariable(target)).setName(name);
                else
                    error("invalid_operation:variable_defined:" + name, false);
            }
            else
                error("invalid_operation:invalid_variable_declaration:" + name, false);
        }
    }
    else if (listExists(target))
    {
        if (!listExists(name))
            lists.at(indexOfList(target)).setName(name);
        else
            error("invalid_operation:list_undefined:" + name, false);
    }
    else if (objectExists(target))
    {
        if (!objectExists(name))
            objects.at(indexOfObject(target)).setName(name);
        else
            error("invalid_operation:object_undefined:" + name, false);
    }
    else if (methodExists(target))
    {
        if (!methodExists(name))
            methods.at(indexOfMethod(target)).setName(name);
        else
            error("invalid_operation:method_undefined:" + name, false);
    }
    else if (fileExists(target) || directoryExists(target))
        rename(target.c_str(), name.c_str());
    else
        error("invalid_operation:target_undefined:" + target, false);
}

void setup()
{
    __BadMethodCount = 0,
    __BadObjectCount = 0,
    __BadVarCount = 0,
    __CurrentLineNumber = 0,
    __IfStatementCount = 0,
    __ForLoopCount = 0,
    __WhileLoopCount = 0,
    __ParamVarCount = 0;
    __CaptureParse = false,
    __IsCommented = false,
    __UseCustomPrompt = false,
    __DontCollectMethodVars = false,
    __FailedIfStatement = false,
    __GoToLabel = false,
    __ExecutedIfStatement = false,
    __InDefaultCase = false,
    __ExecutedMethod = false,
    __DefiningSwitchBlock = false,
    __DefiningIfStatement = false,
    __DefiningForLoop = false,
    __DefiningWhileLoop = false,
    __DefiningModule = false,
    __DefiningPrivateCode = false,
    __DefiningPublicCode = false,
    __DefiningScript = false,
    __ExecutedTemplate = false, // remove
    __ExecutedTryBlock = false,
    __Breaking = false,
    __DefiningMethod = false,
    __MultilineComment = false,
    __Negligence = false,
    __FailedNest = false,
    __DefiningNest = false,
    __DefiningObject = false,
    __DefiningObjectMethod = false,
    __DefiningParameterizedMethod = false,
    __Returning = false,
    __SkipCatchBlock = false,
    __RaiseCatchBlock = false,
    __DefiningLocalSwitchBlock = false,
    __DefiningLocalWhileLoop = false,
    __DefiningLocalForLoop = false;

    __CurrentObject = "",
    __CurrentMethodObject = "",
    __CurrentModule = "",
    __CurrentScript = "",
    __ErrorVarName = "",
    __GoTo = "",
    __LastError = "",
    __LastValue = "",
    __ParsedOutput = "",
    __PreviousScript = "",
    __CurrentScriptName = "",
    __SwitchVarName = "",
    __CurrentLine = "",
    __DefaultLoopSymbol = "$";

    __Null = "[__Null]";

    __ArgumentCount = 0,
    __NullNum = -DBL_MAX;

    if (contains(getEnvironmentVariable("HOMEPATH"), "Users"))
    {
        __GuessedOS = "Win7orVista";
        __SavedVarsPath = (getEnvironmentVariable("HOMEPATH") + "\\AppData") + "\\.__SavedVarsPath", __SavedVars = __SavedVarsPath + "\\.__SavedVars";
    }
    else if (contains(getEnvironmentVariable("HOMEPATH"), "Documents"))
    {
        __GuessedOS = "Win2000NTorXP";
        __SavedVarsPath = getEnvironmentVariable("HOMEPATH") + "\\Application Data\\.__SavedVarsPath", __SavedVars = __SavedVarsPath + "\\.__SavedVars";
    }
    else if (startsWith(getEnvironmentVariable("HOME"), "/"))
    {
        __GuessedOS = "UNIXMacorLINUX";
        __SavedVarsPath = getEnvironmentVariable("HOME") + "/.__SavedVarsPath", __SavedVars = __SavedVarsPath + "/.__SavedVars";
    }
    else
    {
        __GuessedOS = "UnknownWindowsOS";
        __SavedVarsPath = "\\.__SavedVarsPath", __SavedVars = __SavedVarsPath + "\\.__SavedVars";
    }
}

string getSubString(string arg1, string arg2, string beforeBracket)
{
    string returnValue("");

    if (isString(beforeBracket))
    {
        vector<string> listRange = getBracketRange(arg2);

        string variableString = variables.at(indexOfVariable(beforeBracket)).getString();

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
                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
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
                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                    }
                    else
                        error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                }
                else
                    error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
            }
            else
                error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
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
            error("invalid_operation:invalid_range:" + arg2, false);
    }
    else
        error("invalid_operation:__Null_string:" + beforeBracket, false);

    return (returnValue);
}

void setSubString(string arg1, string arg2, string beforeBracket)
{
    if (isString(beforeBracket))
    {
        vector<string> listRange = getBracketRange(arg2);

        string variableString = variables.at(indexOfVariable(beforeBracket)).getString();

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
                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
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
                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                    }
                    else
                        error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                }
                else
                    error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
            }
            else
                error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
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
            error("invalid_operation:invalid_range:" + arg2, false);
    }
    else
        error("invalid_operation:__Null_string:" + beforeBracket, false);
}

void zeroSpace(string arg0, string s, vector<string> command)
{
    if (arg0 == "pass")
    {
        return;
    }
    else if (arg0 == "caught")
    {
        string to_remove = "remove ";
        to_remove.append(__ErrorVarName);

        parse(to_remove);

        __ExecutedTryBlock = false,
        __RaiseCatchBlock = false;
        __LastError = "";
        __ErrorVarName = "";
    }
    else if (arg0 == "clear_methods!")
        clearMethods();
    else if (arg0 == "clear_objects!")
        clearObjects();
    else if (arg0 == "clear_variables!")
        clearVariables();
    else if (arg0 == "clear_lists!")
        clearLists();
    else if (arg0 == "clear_all!")
        clearAll();
    else if (arg0 == "clear_constants!")
        clearConstants();
    else if (arg0 == "help")
        printUSLHelp();
    else if (arg0 == "exit")
    {
        clearAll();
        exit(0);
    }
    else if (arg0 == "break" || arg0 == "leave!")
        __Breaking = true;
    else if (arg0 == "no_methods?")
    {
        if (noMethods())
            __true();
        else
            __false();
    }
    else if (arg0 == "no_objects?")
    {
        if (noObjects())
            __true();
        else
            __false();
    }
    else if (arg0 == "no_variables?")
    {
        if (noVariables())
            __true();
        else
            __false();
    }
    else if (arg0 == "no_lists?")
    {
        if (noLists())
            __true();
        else
            __false();
    }
    else if (arg0 == "end" || arg0 == "}")
    {
        __DefiningPrivateCode = false,
        __DefiningPublicCode = false;
        __DefiningObject = false;
        __DefiningObjectMethod = false;
        __CurrentObject = "";
    }
    else if (arg0 == "parser")
        loop(false);
    else if (arg0 == "private")
    {
        __DefiningPrivateCode = true;
        __DefiningPublicCode = false;
    }
    else if (arg0 == "public")
    {
        __DefiningPrivateCode = false;
        __DefiningPublicCode = true;
    }
    else if (arg0 == "try")
        __ExecutedTryBlock = true;
    else if (arg0 == "failif")
    {
        if (__FailedIfStatement == true)
            successfulIF();
        else
            failedIfStatement();
    }
    else
        sysExec(s, command);
}

string &replace(string &subj, string old, string neu)
{
    size_t uiui = subj.find(old);

    if (uiui != string::npos)
    {
        subj.erase(uiui, old.size());
        subj.insert(uiui, neu);
    }

    return subj;
}

void oneSpace(string arg0, string arg1, string s, vector<string> command)
{
    string before(beforeDot(arg1)), after(afterDot(arg1));

    if (contains(arg1, "self."))
    {
        arg1 = replace(arg1, "self", __CurrentMethodObject);
    }

    if (arg0 == "return")
    {
        if (!InternalReturn(arg0, arg1, before, after))
            oneSpace("return", arg1, "return " + arg1, command);
    }
    else if (arg0 == "switch")
    {
        if (variableExists(arg1))
        {
            __DefiningSwitchBlock = true;
            __SwitchVarName = arg1;
        }
        else
            error("invalid_operation:variable_undefined:" + arg1, false);
    }
    else if (arg0 == "goto")
    {
        if (__CurrentScript != "")
        {
            if (scripts.at(indexOfScript(__CurrentScript)).markExists(arg1))
            {
                __GoTo = arg1;
                __GoToLabel = true;
            }
        }
    }
    else if (arg0 == "help")
        comprehensiveHelp(arg1);
    else if (arg0 == "prompt")
    {
        if (arg1 == "bash")
        {
            __UseCustomPrompt = true;
            __PromptStyle = "bash";
        }
        else if (arg1 == "!")
        {
            if (__UseCustomPrompt == true)
                __UseCustomPrompt = false;
            else
                __UseCustomPrompt = true;
        }
        else if (arg1 == "empty")
        {
            __UseCustomPrompt = true;
            __PromptStyle = "empty";
        }
        else
        {
            __UseCustomPrompt = true;
            __PromptStyle = arg1;
        }
    }
    else if (arg0 == "err" || arg0 == "error")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
                cerr << variables.at(indexOfVariable(arg1)).getString() << endl;
            else if (isNumber(arg1))
                cerr << variables.at(indexOfVariable(arg1)).getNumber() << endl;
            else
                error("is_null:" + arg1, false);
        }
        else
            cerr << arg1 << endl;
    }
    else if (arg0 == "delay")
    {
        if (isNumeric(arg1))
            delay(stoi(arg1));
        else
            error("conversion_error:" + arg1, false);
    }
    else if (arg0 == "loop")
        threeSpace("for", "var", "in", arg1, "for var in " + arg1, command); // REFACTOR HERE
    else if (arg0 == "for" && arg1 == "infinity")
        successfulFor();
    else if (arg0 == "remove")
    {
        if (containsParams(arg1))
        {
            vector<string> params = getParams(arg1);

            for (int i = 0; i < (int)params.size(); i++)
            {
                if (variableExists(params.at(i)))
                    variables = removeVariable(variables, params.at(i));
                else if (listExists(params.at(i)))
                    lists = removeList(lists, params.at(i));
                else if (objectExists(params.at(i)))
                    objects = removeObject(objects, params.at(i));
                else if (methodExists(params.at(i)))
                    methods = removeMethod(methods, params.at(i));
                else
                    error("invalid_operation:target_undefined:" + params.at(i), false);
            }
        }
        else if (variableExists(arg1))
            variables = removeVariable(variables, arg1);
        else if (listExists(arg1))
            lists = removeList(lists, arg1);
        else if (objectExists(arg1))
            objects = removeObject(objects, arg1);
        else if (methodExists(arg1))
            methods = removeMethod(methods, arg1);
        else
            error("invalid_operation:target_undefined:" + arg1, false);
    }
    else if (arg0 == "see_string")
    {
        if (variableExists(arg1))
            __stdout(variables.at(indexOfVariable(arg1)).getString());
        else
            error("invalid_operation:variable_undefined:" + arg1, false);
    }
    else if (arg0 == "see_number")
    {
        if (variableExists(arg1))
            __stdout(dtos(variables.at(indexOfVariable(arg1)).getNumber()));
        else
            error("invalid_operation:variable_undefined:" + arg1, false);
    }
    else if (arg0 == "__begin__")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (!fileExists(variables.at(indexOfVariable(arg1)).getString()))
                {
                    touch(variables.at(indexOfVariable(arg1)).getString());
                    __DefiningScript = true;
                    __CurrentScriptName = variables.at(indexOfVariable(arg1)).getString();
                }
                else
                    error("invalid_operation:file_defined:" + variables.at(indexOfVariable(arg1)).getString(), false);
            }
        }
        else if (!fileExists(arg1))
        {
            touch(arg1);
            __DefiningScript = true;
            __CurrentScriptName = arg1;
        }
        else
            error("invalid_operation:file_defined:" + arg1, false);
    }
    else if (arg0 == "encrypt" || arg0 == "decrypt")
    {
        InternalEncryptDecrypt(arg0, arg1);
    }
    else if (arg0 == "globalize")
    {
        InternalGlobalize(arg0, arg1);
    }
    else if (arg0 == "remember" || arg0 == "save")
    {
        InternalRemember(arg0, arg1);
    }
    else if (arg0 == "forget"  || arg0 == "lose")
    {
        InternalForget(arg0, arg1);
    }
    else if (arg0 == "load")
    {
        if (fileExists(arg1))
        {
            if (isScript(arg1))
            {
                __PreviousScript = __CurrentScript;
                loadScript(arg1);
            }
            else
                error("bad_load:" + arg1, true);
        }
        else if (moduleExists(arg1))
        {
            vector<string> lines = modules.at(indexOfModule(arg1)).get();

            for (int i = 0; i < (int)lines.size(); i++)
                parse(lines.at(i));
        }
        else
            error("bad_load:" + arg1, true);
    }
    else if (arg0 == "say" || arg0 == "stdout" || arg0 == "out" || arg0 == "print" || arg0 == "println")
    {
        InternalOutput(arg0, arg1);
    }
    else if (arg0 == "cd" || arg0 == "chdir")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                    cd(variables.at(indexOfVariable(arg1)).getString());
                else
                    error("read_fail:" + variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error("invalid_operation:__Null_string:" + arg1, false);
        }
        else
        {
            if (arg1 == "init_dir" || arg1 == "initial_directory")
                cd(__InitialDirectory);
            else if (directoryExists(arg1))
                cd(arg1);
            else
                cd(arg1);
        }
    }
    else if (arg0 == "list")
    {
        if (listExists(arg1))
            lists.at(indexOfList(arg1)).clear();
        else
        {
            List newList(arg1);

            if (__ExecutedTemplate || __ExecutedMethod)
                newList.collect();
            else
                newList.dontCollect();

            lists.push_back(newList);
        }
    }
    else if (arg0 == "!")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
                parse(variables.at(indexOfVariable(arg1)).getString().c_str());
            else
                error("is_null:" + arg1, false);
        }
        else
            parse(arg1.c_str());
    }
    else if (arg0 == "?")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
                sysExec(variables.at(indexOfVariable(arg1)).getString(), command);
            else
                error("is_null:" + arg1, false);
        }
        else
            sysExec(arg1, command);
    }
    else if (arg0 == "init_dir" || arg0 == "initial_directory")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                {
                    __InitialDirectory = variables.at(indexOfVariable(arg1)).getString();
                    cd(__InitialDirectory);
                }
                else
                    error("read_fail:" + __InitialDirectory, false);
            }
            else
                error("invalid_operation:__Null_string:" + arg1, false);
        }
        else
        {
            if (directoryExists(arg1))
            {
                if (arg1 == ".")
                    __InitialDirectory = cwd();
                else if (arg1 == "..")
                    __InitialDirectory = cwd() + "\\..";
                else
                    __InitialDirectory = arg1;

                cd(__InitialDirectory);
            }
            else
                error("read_fail:" + __InitialDirectory, false);
        }
    }
    else if (arg0 == "is_method?" || arg0 == "method?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).methodExists(after))
                __true();
            else
                __false();
        }
        else
        {
            if (methodExists(arg1))
                __true();
            else
                __false();
        }
    }
    else if (arg0 == "is_object?" || arg0 == "object?")
    {
        if (objectExists(arg1))
            __true();
        else
            __false();
    }
    else if (arg0 == "is_variable?" || arg0 == "var?" || arg0 == "variable?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
                __true();
            else
                __false();
        }
        else
        {
            if (variableExists(arg1))
                __true();
            else
                __false();
        }
    }
    else if (arg0 == "is_list?" || arg0 == "list?")
    {
        if (listExists(arg1))
            __true();
        else
            __false();
    }
    else if (arg0 == "is_directory?" || arg0 == "dir?" || arg0 == "directory?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (directoryExists(objects.at(indexOfObject(before)).getVariable(after).getString()))
                    __true();
                else
                    __false();
            }
            else
                error("invalid_operation:target_undefined:" + arg1, false);
        }
        else
        {
            if (variableExists(arg1))
            {
                if (isString(arg1))
                {
                    if (directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        __true();
                    else
                        __false();
                }
                else
                    error("invalid_operation:__Null_string:" + arg1, false);
            }
            else
            {
                if (directoryExists(arg1))
                    __true();
                else
                    __false();
            }
        }
    }
    else if (arg0 == "is_file?" || arg0 == "file?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (fileExists(objects.at(indexOfObject(before)).getVariable(after).getString()))
                    __true();
                else
                    __false();
            }
            else
                error("invalid_operation:target_undefined:" + arg1, false);
        }
        else
        {
            if (variableExists(arg1))
            {
                if (isString(arg1))
                {
                    if (fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        __true();
                    else
                        __false();
                }
                else
                    __false();
            }
            else
            {
                if (fileExists(arg1))
                    __true();
                else
                    __false();
            }
        }
    }
    else if (arg0 == "collect?" || arg0 == "garbage?")
    {
        if (variableExists(arg1))
        {
            if (variables.at(indexOfVariable(arg1)).garbage())
                __true();
            else
                __false();
        }
        else
            cout << "under construction..." << endl;
    }
    else if (arg0 == "is_number?" || arg0 == "number?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                    __true();
                else
                    __false();
            }
            else
                error("invalid_operation:target_undefined:" + arg1, false);
        }
        else
        {
            if (variableExists(arg1))
            {
                if (isNumber(arg1))
                    __true();
                else
                    __false();
            }
            else
            {
                if (isNumeric(arg1))
                    __true();
                else
                    __false();
            }
        }
    }
    else if (arg0 == "is_string?" || arg0 == "string?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                    __true();
                else
                    __false();
            }
            else
                error("invalid_operation:target_undefined:" + arg1, false);
        }
        else
        {
            if (variableExists(arg1))
            {
                if (isString(arg1))
                    __true();
                else
                    __false();
            }
            else
            {
                if (isNumeric(arg1))
                    __false();
                else
                    __true();
            }
        }
    }
    else if (arg0 == "is_uppercase?" || arg0 == "upper?" || arg0 == "uppercase?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (isUpper(objects.at(indexOfObject(before)).getVariable(after).getString()))
                    __true();
                else
                    __false();
            }
            else
                error("invalid_operation:target_undefined:" + arg1, false);
        }
        else
        {
            if (variableExists(arg1))
            {
                if (isString(arg1))
                {
                    if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                        __true();
                    else
                        __false();
                }
                else
                    __false();
            }
            else
            {
                if (isNumeric(arg1))
                    __false();
                else
                {
                    if (isUpper(arg1))
                        __true();
                    else
                        __false();
                }
            }
        }
    }
    else if (arg0 == "is_lowercase?" || arg0 == "lower?" || arg0 == "lowercase?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (isLower(objects.at(indexOfObject(before)).getVariable(after).getString()))
                    __true();
                else
                    __false();
            }
            else
                error("invalid_operation:target_undefined:" + arg1, false);
        }
        else
        {
            if (variableExists(arg1))
            {
                if (isString(arg1))
                {
                    if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                        __true();
                    else
                        __false();
                }
                else
                    __false();
            }
            else
            {
                if (isNumeric(arg1))
                    __false();
                else
                {
                    if (isLower(arg1))
                        __true();
                    else
                        __false();
                }
            }
        }
    }
    else if (arg0 == "see")
    {
        InternalInspect(arg0, arg1, before, after);
    }
    else if (arg0 == "template")
    {
        if (methodExists(arg1))
            error("invalid_operation:method_defined:" + arg1, false);
        else
        {
            if (containsParams(arg1))
            {
                vector<string> params = getParams(arg1);
                Method method(beforeParams(arg1), true);

                method.setTemplateSize((int)params.size());

                methods.push_back(method);

                __DefiningMethod = true;
            }
        }
    }
    else if (arg0 == "lock")
    {
        if (variableExists(arg1))
            variables.at(indexOfVariable(arg1)).setIndestructible();
        else if (methodExists(arg1))
            methods.at(indexOfMethod(arg1)).setIndestructible();
    }
    else if (arg0 == "unlock")
    {
        if (variableExists(arg1))
            variables.at(indexOfVariable(arg1)).setDestructible();
        else if (methodExists(arg1))
            methods.at(indexOfMethod(arg1)).setDestructible();
    }
    else if (arg0 == "method" || arg0 == "[method]")
    {
        InternalCreateMethod(arg0, arg1);
    }
    else if (arg0 == "call_method")
    {
        InternalCallMethod(arg0, arg1, before, after);
    }
    else if (arg0 == "object")
    {
        InternalCreateObject(arg1);
    }
    else if (arg0 == "fpush")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (!fileExists(variables.at(indexOfVariable(arg1)).getString()))
                    touch(variables.at(indexOfVariable(arg1)).getString());
                else
                    error("invalid_operation:file_defined:" + variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error("invalid_operation:__Null_string:" + arg1, false);
        }
        else
        {
            if (!fileExists(arg1))
                touch(arg1);
            else
                error("invalid_operation:file_defined:" + arg1, false);
        }
    }
    else if (arg0 == "fpop")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (fileExists(variables.at(indexOfVariable(arg1)).getString()))
                    rm(variables.at(indexOfVariable(arg1)).getString());
                else
                    error("invalid_operation:file_undefined:" + variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error("invalid_operation:__Null_string:" + arg1, false);
        }
        else
        {
            if (fileExists(arg1))
                rm(arg1);
            else
                error("invalid_operation:file_undefined:" + arg1, false);
        }
    }
    else if (arg0 == "dpush")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (!directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                    md(variables.at(indexOfVariable(arg1)).getString());
                else
                    error("invalid_operation:directory_defined:" + variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error("invalid_operation:__Null_string:" + arg1, false);
        }
        else
        {
            if (!directoryExists(arg1))
                md(arg1);
            else
                error("invalid_operation:directory_defined:" + arg1, false);
        }
    }
    else if (arg0 == "dpop")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                    rd(variables.at(indexOfVariable(arg1)).getString());
                else
                    error("invalid_operation:directory_undefined:" + variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error("invalid_operation:__Null_string:" + arg1, false);
        }
        else
        {
            if (directoryExists(arg1))
                rd(arg1);
            else
                error("invalid_operation:directory_undefined:" + arg1, false);
        }
    }
    else
        sysExec(s, command);
}

bool isNumber(string varName)
{
    return variables.at(indexOfVariable(varName)).getNumber() != __NullNum;
}

bool isString(string varName)
{
    return variables.at(indexOfVariable(varName)).getString() != __Null;
}

double getNumber(string varName)
{
    return variables.at(indexOfVariable(varName)).getNumber();
}

string getString(string varName)
{
    return variables.at(indexOfVariable(varName)).getString();
}

bool secondIsNumber(string s)
{
    if (variableExists(s))
    {
        if (isNumber(s))
            return (true);
    }
    else if (stackReady(s))
    {
        if (!isStringStack(s))
            return (true);
    }
    else
    {
        if (isNumeric(s))
            return (true);
    }

    return (false);
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
            executeTemplate(objects.at(indexOfObject(_beforeDot)).getMethod(_afterDot), getParams(_afterDot));

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
                        lastValue = __CurrentScript;
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

            if (lists.at(indexOfList(_beforeBrackets)).size() >= stoi(_afterBrackets))
            {
                if (stoi(_afterBrackets) >= 0)
                    lastValue = lists.at(indexOfList(_beforeBrackets)).at(stoi(_afterBrackets));
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
            executeTemplate(methods.at(indexOfMethod(arg2)), getParams(arg2));

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

    setLastValue(returnValue);
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

        if (isNumeric(__LastValue))
            lastValue = stod(__LastValue);
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
            executeTemplate(objects.at(indexOfObject(_beforeDot)).getMethod(_afterDot), getParams(_afterDot));

            if (isNumeric(__LastValue))
                lastValue = stod(__LastValue);
            else
                lastValue = 0;
        }
        else
        {
            if (isNumeric(__LastValue))
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

            if (lists.at(indexOfList(_beforeBrackets)).size() >= stoi(_afterBrackets))
            {
                if (stoi(_afterBrackets) >= 0)
                {
                    if (isNumeric(lists.at(indexOfList(_beforeBrackets)).at(stoi(_afterBrackets))))
                        lastValue = stod(lists.at(indexOfList(_beforeBrackets)).at(stoi(_afterBrackets)));
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
            executeTemplate(methods.at(indexOfMethod(arg2)), getParams(arg2));

            if (isNumeric(__LastValue))

                lastValue = stod(__LastValue);
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

    setLastValue(dtos(returnValue));
    return (returnValue);
}

/**

@s = "This is a string."
@n = 3.14 # and that's a number.

**/
void twoSpace(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    string last_val = "";

    if (contains(arg2, "self."))
        arg2 = replace(arg2, "self", __CurrentMethodObject);

    if (contains(arg0, "self."))
        arg0 = replace(arg0, "self", __CurrentMethodObject);

    if (variableExists(arg0))
    {
        if (objectExists(beforeDot(arg0)) || startsWith(arg0, "@"))
        {
            if (objectExists(beforeDot(arg0)))
            {
                if (objects.at(indexOfObject(beforeDot(arg0))).getVariable(afterDot(arg0)).getString() != __Null)
                {
                    string tempObjectVariableName("@____" + beforeDot(arg0) + "___" + afterDot(arg0) + "__string_var");

                    createVariable(tempObjectVariableName, objects.at(indexOfObject(beforeDot(arg0))).getVariable(afterDot(arg0)).getString());

                    twoSpace(tempObjectVariableName, arg1, arg2, tempObjectVariableName + " " + arg1 + " " + arg2, command);

                    variables.at(indexOfVariable(tempObjectVariableName)).setName(afterDot(arg0));

                    objects.at(indexOfObject(beforeDot(arg0))).removeVariable(afterDot(arg0));
                    objects.at(indexOfObject(beforeDot(arg0))).addVariable(variables.at(indexOfVariable(afterDot(arg0))));
                    variables = removeVariable(variables, afterDot(arg0));
                }
                else if (objects.at(indexOfObject(beforeDot(arg0))).getVariable(afterDot(arg0)).getNumber() != __NullNum)
                {
                    string tempObjectVariableName("@____" + beforeDot(arg0) + "___" + afterDot(arg0) + "__number_var");

                    createVariable(tempObjectVariableName, objects.at(indexOfObject(beforeDot(arg0))).getVariable(afterDot(arg0)).getNumber());

                    twoSpace(tempObjectVariableName, arg1, arg2, tempObjectVariableName + " " + arg1 + " " + arg2, command);

                    variables.at(indexOfVariable(tempObjectVariableName)).setName(afterDot(arg0));

                    objects.at(indexOfObject(beforeDot(arg0))).removeVariable(afterDot(arg0));
                    objects.at(indexOfObject(beforeDot(arg0))).addVariable(variables.at(indexOfVariable(afterDot(arg0))));
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
                        if (lists.at(indexOfList(beforeBracket)).size() >= stoi(afterBracket))
                        {
                            if (lists.at(indexOfList(beforeBracket)).at(stoi(afterBracket)) == "#!=no_line")
                                error("invalid_operation:index_out_of_bounds:" + arg2, false);
                            else
                            {
                                string listValue(lists.at(indexOfList(beforeBracket)).at(stoi(afterBracket)));

                                if (isNumeric(listValue))
                                {
                                    if (isNumber(arg0))
                                        setVariable(arg0, stod(listValue));
                                    else
                                        error("conversion_error:" + arg0, false);
                                }
                                else
                                {
                                    if (isString(arg0))
                                        setVariable(arg0, listValue);
                                    else
                                        error("conversion_error:" + arg0, false);
                                }
                            }
                        }
                    }
                    else if (isString(beforeBracket))
                        setSubString(arg0, arg2, beforeBracket);
                    else
                        error("invalid_operation:list_undefined:" + beforeBracket, false);
                }
                else if (before.length() != 0 && after.length() != 0)
                {
                    if (containsParams(arg2))
                    {
                        if (beforeParams(arg2) == "random")
                        {
                            if (contains(arg2, ".."))
                            {
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
                                    else
                                        error("special_error(7)", false);
                                }
                                else if (isAlpha(s0) && isAlpha(s2))
                                {
                                    if (isString(arg0))
                                    {
                                        if (get_alpha_num(s0[0]) < get_alpha_num(s2[0]))
                                            setVariable(arg0, random(s0, s2));
                                        else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0]))
                                            setVariable(arg0, random(s2, s0));
                                        else
                                            setVariable(arg0, random(s2, s0));
                                    }
                                    else
                                        error("invalid_operation:__Null_string:" + arg0, false);
                                }
                                else if (variableExists(s0) || variableExists(s2))
                                {
                                    if (variableExists(s0))
                                    {
                                        if (isNumber(s0))
                                            s0 = dtos(variables.at(indexOfVariable(s0)).getNumber());
                                        else if (isString(s0))
                                            s0 = variables.at(indexOfVariable(s0)).getString();
                                    }

                                    if (variableExists(s2))
                                    {
                                        if (isNumber(s2))
                                            s2 = dtos(variables.at(indexOfVariable(s2)).getNumber());
                                        else if (isString(s2))
                                            s2 = variables.at(indexOfVariable(s2)).getString();
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
                                        else
                                            error("special_error(7)", false);
                                    }
                                    else if (isAlpha(s0) && isAlpha(s2))
                                    {
                                        if (isString(arg0))
                                        {
                                            if (get_alpha_num(s0[0]) < get_alpha_num(s2[0]))
                                                setVariable(arg0, random(s0, s2));
                                            else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0]))
                                                setVariable(arg0, random(s2, s0));
                                            else
                                                setVariable(arg0, random(s2, s0));
                                        }
                                        else
                                            error("invalid_operation:__Null_string:" + arg0, false);
                                    }
                                }
                                else
                                    error("invalid_operation:invalid_sequence:" + s0 + "_" + s2, false);
                            }
                            else
                                error("invalid_operation:invalid_sequence_separator:" + arg2, false);
                        }
                    }
                    else if (listExists(before) && after == "size")
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, stod(itos(lists.at(indexOfList(before)).size())));
                        else if (isString(arg0))
                            setVariable(arg0, itos(lists.at(indexOfList(before)).size()));
                        else
                            error("is_null:" + arg0, false);
                    }
                    else if (before == "self")
                    {
                        if (objectExists(__CurrentMethodObject))
                            twoSpace(arg0, arg1, (__CurrentMethodObject + "." + after), (arg0 + " " + arg1 + " " + (__CurrentMethodObject + "." + after)), command);
                        else
                            twoSpace(arg0, arg1, after, (arg0 + " " + arg1 + " " + after), command);
                    }
                    else if (objectExists(before))
                    {
                        if (objects.at(indexOfObject(before)).variableExists(after))
                        {
                            if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                                setVariable(arg0, objects.at(indexOfObject(before)).getVariable(after).getString());
                            else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                                setVariable(arg0, objects.at(indexOfObject(before)).getVariable(after).getNumber());
                            else
                                error("is_null:" + arg2, false);
                        }
                        else if (objects.at(indexOfObject(before)).methodExists(after) && !containsParams(after))
                        {
                            parse(arg2);

                            if (isString(arg0))
                                setVariable(arg0, __LastValue);
                            else if (isNumber(arg0))
                                setVariable(arg0, stod(__LastValue));
                        }
                        else if (containsParams(after))
                        {
                            if (objects.at(indexOfObject(before)).methodExists(beforeParams(after)))
                            {
                                executeTemplate(objects.at(indexOfObject(before)).getMethod(beforeParams(after)), getParams(after));

                                if (isNumeric(__LastValue))
                                {
                                    if (isString(arg0))
                                        setVariable(arg0, __LastValue);
                                    else if (isNumber(arg0))
                                        setVariable(arg0, stod(__LastValue));
                                    else
                                        error("is_null:" + arg0, false);
                                }
                                else
                                {
                                    if (isString(arg0))
                                        setVariable(arg0, __LastValue);
                                    else if (isNumber(arg0))
                                        error("conversion_error:" + arg0, false);
                                    else
                                        error("is_null:" + arg0, false);
                                }
                            }
                            else
                                sysExec(s, command);
                        }
                        else
                            error("invalid_operation:variable_undefined:" + arg2, false);
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
                                setVariable(arg0, (int)variables.at(indexOfVariable(before)).getString()[0]);
                            else if (isNumber(before))
                            {
                                int i = (int)variables.at(indexOfVariable(before)).getNumber();
                                setVariable(arg0, (double)i);
                            }
                            else
                                error("is_null:" + before, false);
                        }
                        else
                            error("invalid_operation:variable_undefined:" + before, false);
                    }
                    else if (after == "to_double")
                    {
                        if (variableExists(before))
                        {
                            if (isString(before))
                                setVariable(arg0, (double)variables.at(indexOfVariable(before)).getString()[0]);
                            else if (isNumber(before))
                            {
                                double i = variables.at(indexOfVariable(before)).getNumber();
                                setVariable(arg0, (double)i);
                            }
                            else
                                error("is_null:" + before, false);
                        }
                        else
                            error("invalid_operation:variable_undefined:" + before, false);
                    }
                    else if (after == "to_string")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(variables.at(indexOfVariable(before)).getNumber()));
                            else
                                error("is_null:" + before, false);
                        }
                        else
                            error("invalid_operation:variable_undefined:" + before, false);
                    }
                    else if (after == "to_number")
                    {
                        if (variableExists(before))
                        {
                            if (isString(before))
                                setVariable(arg0, stod(variables.at(indexOfVariable(before)).getString()));
                            else
                                error("is_null:" + before, false);
                        }
                        else
                            error("invalid_operation:variable_undefined:" + before, false);
                    }
                    else if (before == "readline")
                    {
                        if (variableExists(after))
                        {
                            if (isString(after))
                            {
                                string line("");
                                __stdout(cleanString(variables.at(indexOfVariable(after)).getString()));
                                getline(cin, line, '\n');

                                if (isNumber(arg0))
                                {
                                    if (isNumeric(line))
                                        setVariable(arg0, stod(line));
                                    else
                                        error("conversion_error:" + line, false);
                                }
                                else if (isString(arg0))
                                    setVariable(arg0, line);
                                else
                                    error("is_null:" + arg0, false);
                            }
                            else
                            {
                                string line("");
                                cout << "readline: ";
                                getline(cin, line, '\n');

                                if (isNumber(arg0))
                                {
                                    if (isNumeric(line))
                                        setVariable(arg0, stod(line));
                                    else
                                        error("conversion_error:" + line, false);
                                }
                                else if (isString(arg0))
                                    setVariable(arg0, line);
                                else
                                    error("is_null:" + arg0, false);
                            }
                        }
                        else
                        {
                            string line("");
                            cout << cleanString(after);
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
                                line = getSilentOutput(variables.at(indexOfVariable(after)).getString());

                                if (isNumber(arg0))
                                {
                                    if (isNumeric(line))
                                        setVariable(arg0, stod(line));
                                    else
                                        error("conversion_error:" + line, false);
                                }
                                else if (isString(arg0))
                                    setVariable(arg0, line);
                                else
                                    error("is_null:" + arg0, false);

                                cout << endl;
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
                                        error("conversion_error:" + line, false);
                                }
                                else if (isString(arg0))
                                    setVariable(arg0, line);
                                else
                                    error("is_null:" + arg0, false);

                                cout << endl;
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

                            cout << endl;
                        }
                    }
                    else if (after == "cos")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, cos(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(cos(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "acos")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, acos(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(acos(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "cosh")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, cosh(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(cosh(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "log")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, log(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(log(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "sqrt")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, sqrt(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(sqrt(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "abs")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, abs(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(abs(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "floor")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, floor(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(floor(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "ceil")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, ceil(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(ceil(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "exp")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, exp(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(exp(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "sin")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, sin(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(sin(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "sinh")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, sinh(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(sinh(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "asin")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, asin(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(asin(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "tan")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, tan(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(tan(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "tanh")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, tanh(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(tanh(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "atan")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, atan(variables.at(indexOfVariable(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else if (isString(arg0))
                            {
                                if (isNumber(before))
                                    setVariable(arg0, dtos(atan(variables.at(indexOfVariable(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "to_lower")
                    {
                        if (variableExists(before))
                        {
                            if (isString(arg0))
                            {
                                if (isString(before))
                                    setVariable(arg0, getLower(variables.at(indexOfVariable(before)).getString()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
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
                                    if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                    {
                                        ifstream file(variables.at(indexOfVariable(before)).getString().c_str());
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
                                            error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                    }
                                    else
                                        error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                }
                                else
                                    error("invalid_operation:__Null_string:" + before, false);
                            }
                            else
                            {
                                if (fileExists(before))
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
                                        error("read_fail:" + before, false);
                                }
                                else
                                    error("read_fail:" + before, false);
                            }
                        }
                        else
                            error("invalid_operation:__Null_string:" + arg0, false);
                    }
                    else if (after == "to_upper")
                    {
                        if (variableExists(before))
                        {
                            if (isString(arg0))
                            {
                                if (isString(before))
                                    setVariable(arg0, getUpper(variables.at(indexOfVariable(before)).getString()));
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (after == "size")
                    {
                        if (variableExists(before))
                        {
                            if (isNumber(arg0))
                            {
                                if (isString(before))
                                    setVariable(arg0, (double)variables.at(indexOfVariable(before)).getString().length());
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, (double)before.length());
                            else
                                error("conversion_error:" + arg0, false);
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
                                    if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                        setVariable(arg0, getBytes(variables.at(indexOfVariable(before)).getString()));
                                    else
                                        error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                }
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                            {
                                if (fileExists(before))
                                    setVariable(arg0, getBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        }
                        else
                            error("conversion_error:" + arg0, false);
                    }
                    else if (after == "kbytes")
                    {
                        if (isNumber(arg0))
                        {
                            if (variableExists(before))
                            {
                                if (isString(before))
                                {
                                    if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                        setVariable(arg0, getKBytes(variables.at(indexOfVariable(before)).getString()));
                                    else
                                        error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                }
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                            {
                                if (fileExists(before))
                                    setVariable(arg0, getKBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        }
                        else
                            error("conversion_error:" + arg0, false);
                    }
                    else if (after == "mbytes")
                    {
                        if (isNumber(arg0))
                        {
                            if (variableExists(before))
                            {
                                if (isString(before))
                                {
                                    if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                        setVariable(arg0, getMBytes(variables.at(indexOfVariable(before)).getString()));
                                    else
                                        error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                }
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                            {
                                if (fileExists(before))
                                    setVariable(arg0, getMBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        }
                        else
                            error("conversion_error:" + arg0, false);
                    }
                    else if (after == "gbytes")
                    {
                        if (isNumber(arg0))
                        {
                            if (variableExists(before))
                            {
                                if (isString(before))
                                {
                                    if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                        setVariable(arg0, getGBytes(variables.at(indexOfVariable(before)).getString()));
                                    else
                                        error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                }
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                            {
                                if (fileExists(before))
                                    setVariable(arg0, getGBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        }
                        else
                            error("conversion_error:" + arg0, false);
                    }
                    else if (after == "tbytes")
                    {
                        if (isNumber(arg0))
                        {
                            if (variableExists(before))
                            {
                                if (isString(before))
                                {
                                    if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                        setVariable(arg0, getTBytes(variables.at(indexOfVariable(before)).getString()));
                                    else
                                        error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                }
                                else
                                    error("conversion_error:" + before, false);
                            }
                            else
                            {
                                if (fileExists(before))
                                    setVariable(arg0, getTBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        }
                        else
                            error("conversion_error:" + arg0, false);
                    }
                    else
                    {
                        if (isNumber(arg0))
                        {
                            if (isNumeric(arg2))
                                setVariable(arg0, stod(arg2));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else if (isString(arg0))
                            setVariable(arg0, arg2);
                        else if (variables.at(indexOfVariable(arg0)).waiting())
                        {
                            if (isNumeric(arg2))
                                setVariable(arg0, stod(before + "." + after));
                            else
                                setVariable(arg0, arg2);
                        }
                        else
                            error("is_null:" + arg0, false);
                    }
                }
                else
                {
                    if (variables.at(indexOfVariable(arg0)).waiting())
                    {
                        if (isNumeric(arg2))
                            setVariable(arg0, stod(arg2));
                        else
                            setVariable(arg0, arg2);
                    }
                    else if (arg2 == "null")
                    {
                        if (isString(arg0))
                            variables.at(indexOfVariable(arg0)).setNull();
                        else if (isNumber(arg0))
                            variables.at(indexOfVariable(arg0)).setNull();
                        else
                            error("is_null:" + arg0, false);
                    }
                    else if (constantExists(arg2))
                    {
                        if (isString(arg0))
                        {
                            if (constants.at(indexOfConstant(arg2)).ConstNumber())
                                setVariable(arg0, dtos(constants.at(indexOfConstant(arg2)).getNumber()));
                            else if (constants.at(indexOfConstant(arg2)).ConstString())
                                setVariable(arg0, constants.at(indexOfConstant(arg2)).getString());
                        }
                        else if (isNumber(arg0))
                        {
                            if (constants.at(indexOfConstant(arg2)).ConstNumber())
                                setVariable(arg0, constants.at(indexOfConstant(arg2)).getNumber());
                            else
                                error("conversion_error:__Null_number:" + arg2, false);
                        }
                        else
                            error("is_null:" + arg0, false);
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isString(arg0))
                            setVariable(arg0, __LastValue);
                        else if (isNumber(arg0))
                            setVariable(arg0, stod(__LastValue));
                    }
                    else if (variableExists(arg2))
                    {
                        if (isString(arg2))
                        {
                            if (isString(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg2)).getString());
                            else if (isNumber(arg0))
                                error("conversion_error:" + arg2, false);
                            else
                                error("is_null:" + arg0, false);
                        }
                        else if (isNumber(arg2))
                        {
                            if (isString(arg0))
                                setVariable(arg0, dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                            else if (isNumber(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg2)).getNumber());
                            else
                                error("is_null:" + arg0, false);
                        }
                        else
                            error("is_null:" + arg2, false);
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
                                    error("conversion_error:" + passworder, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, passworder);
                            else
                                setVariable(arg0, passworder);
                        }
                        else
                        {
                            string line("");
                            cout << "readline: ";
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
                                setVariable(arg0, __LastValue);
                            }
                            else if (isNumber(arg0))
                            {
                                setVariable(arg0, stod(__LastValue));
                            }
                        }
                        else if (isStringStack(arg2))
                        {
                            if (isString(arg0))
                                setVariable(arg0, getStringStack(arg2));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else if (stackReady(arg2))
                        {
                            if (isString(arg0))
                                setVariable(arg0, dtos(getStack(arg2)));
                            else if (isNumber(arg0))
                                setVariable(arg0, getStack(arg2));
                            else
                                error("is_null:" + arg0, false);
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
                                error("conversion_error:" + arg0, false);
                            else if (isString(arg0))
                                setVariable(arg0, cleanString(arg2));
                        }
                    }
                }
            }
            else if (arg1 == "+=")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg0))
                    {
                        if (isString(arg2))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + variables.at(indexOfVariable(arg2)).getString());
                        else if (isNumber(arg2))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                        else
                            error("is_null:" + arg2, false);
                    }
                    else if (isNumber(arg0))
                    {
                        if (isString(arg2))
                            error("conversion_error:" + arg2, false);
                        else if (isNumber(arg2))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() + variables.at(indexOfVariable(arg2)).getNumber());
                        else
                            error("is_null:" + arg2, false);
                    }
                    else
                        error("is_null:" + arg0, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (isStringStack(arg2))
                        {
                            if (isString(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + getStringStack(arg2));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() + getStack(arg2));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isString(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + __LastValue);
                            else if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() + stod(__LastValue));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isString(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + __LastValue);
                            else if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() + stod(__LastValue));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isString(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + __LastValue);
                        else if (isNumber(arg0))
                        {
                            if (isNumeric(__LastValue))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() + stod(__LastValue));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else
                            error("is_null:" + arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + arg2);
                        else if (isNumber(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() + stod(arg2));
                        else
                            error("is_null:" + arg0, false);
                    }
                    else
                    {
                        if (isString(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + cleanString(arg2));
                        else if (isNumber(arg0))
                            error("conversion_error:" + arg0, false);
                        else
                            error("is_null:" + arg0, false);
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
                            if (variables.at(indexOfVariable(arg2)).getString().length() == 1)
                                setVariable(arg0, subtractChar(variables.at(indexOfVariable(arg0)).getString(), variables.at(indexOfVariable(arg2)).getString()));
                            else
                                setVariable(arg0, subtractString(variables.at(indexOfVariable(arg0)).getString(), variables.at(indexOfVariable(arg2)).getString()));
                        }
                        else if (isNumber(arg2))
                            setVariable(arg0, subtractString(variables.at(indexOfVariable(arg0)).getString(), dtos(variables.at(indexOfVariable(arg2)).getNumber())));
                        else
                            error("is_null:" + arg2, false);
                    }
                    else if (isNumber(arg0))
                    {
                        if (isString(arg2))
                            error("conversion_error:" + arg2, false);
                        else if (isNumber(arg2))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() - variables.at(indexOfVariable(arg2)).getNumber());
                        else
                            error("is_null:" + arg2, false);
                    }
                    else
                        error("is_null:" + arg0, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (isStringStack(arg2))
                        {
                            if (isString(arg0))
                                setVariable(arg0, subtractString(variables.at(indexOfVariable(arg0)).getString(), getStringStack(arg2)));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() - getStack(arg2));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isString(arg0))
                                setVariable(arg0, subtractString(variables.at(indexOfVariable(arg0)).getString(), __LastValue));
                            else if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() - stod(__LastValue));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isString(arg0))
                                setVariable(arg0, subtractString(variables.at(indexOfVariable(arg0)).getString(), __LastValue));
                            else if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() - stod(__LastValue));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("is_null:" + arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isString(arg0))
                            setVariable(arg0, subtractString(variables.at(indexOfVariable(arg0)).getString(), __LastValue));
                        else if (isNumber(arg0))
                        {
                            if (isNumeric(__LastValue))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() - stod(__LastValue));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else
                            error("is_null:" + arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isString(arg0))
                        {
                            if (arg2.length() == 1)
                                setVariable(arg0, subtractChar(variables.at(indexOfVariable(arg0)).getString(), arg2));
                            else
                                setVariable(arg0, subtractString(variables.at(indexOfVariable(arg0)).getString(), arg2));
                        }
                        else if (isNumber(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() - stod(arg2));
                        else
                            error("is_null:" + arg0, false);
                    }
                    else
                    {
                        if (isString(arg0))
                        {
                            if (arg2.length() == 1)
                                setVariable(arg0, subtractChar(variables.at(indexOfVariable(arg0)).getString(), arg2));
                            else
                                setVariable(arg0, subtractString(variables.at(indexOfVariable(arg0)).getString(), cleanString(arg2)));
                        }
                        else if (isNumber(arg0))
                            error("conversion_error:" + arg0, false);
                        else
                            error("is_null:" + arg0, false);
                    }
                }
            }
            else if (arg1 == "*=")
            {
                if (variableExists(arg2))
                {
                    if (isNumber(arg2))
                        setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() * variables.at(indexOfVariable(arg2)).getNumber());
                    else if (isString(arg2))
                        error("conversion_error:" + arg2, false);
                    else
                        error("is_null:" + arg2, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() * getStack(arg2));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() * stod(__LastValue));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("invalid_operation:__Null_number:" + arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() * stod(__LastValue));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("invalid_operation:__Null_number:" + arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isNumber(arg0))
                        {
                            if (isNumeric(__LastValue))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() * stod(__LastValue));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else
                            error("invalid_operation:__Null_number:" + arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() * stod(arg2));
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
                        setVariable(arg0, (int)variables.at(indexOfVariable(arg0)).getNumber() % (int)variables.at(indexOfVariable(arg2)).getNumber());
                    else if (isString(arg2))
                        error("conversion_error:" + arg2, false);
                    else
                        error("is_null:" + arg2, false);
                }
                else if (methodExists(arg2))
                {
                    parse(arg2);

                    if (isNumber(arg0))
                    {
                        if (isNumeric(__LastValue))
                            setVariable(arg0, (int)variables.at(indexOfVariable(arg0)).getNumber() % (int)stod(__LastValue));
                        else
                            error("conversion_error:" + arg0, false);
                    }
                    else
                        error("invalid_operation:__Null_number:" + arg0, false);
                }
                else
                {
                    if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, (int)variables.at(indexOfVariable(arg0)).getNumber() % (int)stod(arg2));
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
                        setVariable(arg0, pow(variables.at(indexOfVariable(arg0)).getNumber(), variables.at(indexOfVariable(arg2)).getNumber()));
                    else if (isString(arg2))
                        error("conversion_error:" + arg2, false);
                    else
                        error("is_null:" + arg2, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, pow(variables.at(indexOfVariable(arg0)).getNumber(), (int)getStack(arg2)));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, pow(variables.at(indexOfVariable(arg0)).getNumber(), (int)stod(__LastValue)));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("invalid_operation:__Null_number:" + arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, pow(variables.at(indexOfVariable(arg0)).getNumber(), (int)stod(__LastValue)));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("invalid_operation:__Null_number:" + arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isNumber(arg0))
                        {
                            if (isNumeric(__LastValue))
                                setVariable(arg0, pow(variables.at(indexOfVariable(arg0)).getNumber(), (int)stod(__LastValue)));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else
                            error("invalid_operation:__Null_number:" + arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, pow(variables.at(indexOfVariable(arg0)).getNumber(), stod(arg2)));
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
                        setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() / variables.at(indexOfVariable(arg2)).getNumber());
                    else if (isString(arg2))
                        error("conversion_error:" + arg2, false);
                    else
                        error("is_null:" + arg2, false);
                }
                else
                {
                    if (containsParams(arg2))
                    {
                        if (stackReady(arg2))
                        {
                            if (isNumber(arg0))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() / getStack(arg2));
                        }
                        else if (methodExists(beforeParams(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() / stod(__LastValue));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("invalid_operation:__Null_number:" + arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() / stod(__LastValue));
                                else
                                    error("conversion_error:" + arg0, false);
                            }
                            else
                                error("invalid_operation:__Null_number:" + arg0, false);
                        }
                    }
                    else if (methodExists(arg2))
                    {
                        parse(arg2);

                        if (isNumber(arg0))
                        {
                            if (isNumeric(__LastValue))
                                setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() / stod(__LastValue));
                            else
                                error("conversion_error:" + arg0, false);
                        }
                        else
                            error("invalid_operation:__Null_number:" + arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() / stod(arg2));
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
                            int tempVarNumber((int)variables.at(indexOfVariable(arg2)).getNumber());
                            string tempVarString(variables.at(indexOfVariable(arg0)).getString());
                            int len(tempVarString.length());
                            string cleaned("");

                            for (int i = 0; i < len; i++)
                                cleaned.push_back((char)(((int)tempVarString[i]) + tempVarNumber));

                            setVariable(arg0, cleaned);
                        }
                        else
                            error("is_null:" + arg0, false);
                    }
                    else
                        error("conversion_error:" + arg2, false);
                }
                else
                {
                    if (isNumeric(arg2))
                    {
                        int tempVarNumber(stoi(arg2));
                        string tempVarString(variables.at(indexOfVariable(arg0)).getString());

                        if (tempVarString != __Null)
                        {
                            int len(tempVarString.length());
                            string cleaned("");

                            for (int i = 0; i < len; i++)
                                cleaned.push_back((char)(((int)tempVarString[i]) + tempVarNumber));

                            setVariable(arg0, cleaned);
                        }
                        else
                            error("is_null:" + tempVarString, false);
                    }
                    else
                        error("conversion_error:" + arg2, false);
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
                            int tempVarNumber((int)variables.at(indexOfVariable(arg2)).getNumber());
                            string tempVarString(variables.at(indexOfVariable(arg0)).getString());
                            int len(tempVarString.length());
                            string cleaned("");

                            for (int i = 0; i < len; i++)
                                cleaned.push_back((char)(((int)tempVarString[i]) - tempVarNumber));

                            setVariable(arg0, cleaned);
                        }
                        else
                            error("is_null:" + arg0, false);
                    }
                    else
                        error("conversion_error:" + arg2, false);
                }
                else
                {
                    if (isNumeric(arg2))
                    {
                        int tempVarNumber(stoi(arg2));
                        string tempVarString(variables.at(indexOfVariable(arg0)).getString());

                        if (tempVarString != __Null)
                        {
                            int len(tempVarString.length());
                            string cleaned("");

                            for (int i = 0; i < len; i++)
                                cleaned.push_back((char)(((int)tempVarString[i]) - tempVarNumber));

                            setVariable(arg0, cleaned);
                        }
                        else
                            error("is_null:" + tempVarString, false);
                    }
                    else
                        error("conversion_error:" + arg2, false);
                }
            }
            else if (arg1 == "?")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, getStdout(variables.at(indexOfVariable(arg2)).getString().c_str()));
                        else
                            error("conversion_error:" + arg0, false);
                    }
                    else
                        error("conversion_error:" + arg2, false);
                }
                else
                {
                    if (isString(arg0))
                        setVariable(arg0, getStdout(cleanString(arg2).c_str()));
                    else
                        error("conversion_error:" + arg0, false);
                }
            }
            else if (arg1 == "!")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, getParsedOutput(variables.at(indexOfVariable(arg2)).getString().c_str()));
                        else
                            error("conversion_error:" + arg0, false);
                    }
                    else
                        error("conversion_error:" + arg2, false);
                }
                else
                {
                    if (isString(arg0))
                        setVariable(arg0, getParsedOutput(cleanString(arg2).c_str()));
                    else
                        error("conversion_error:" + arg0, false);
                }
            }
            else
                error("invalid_operator:" + arg1, false);
        }
        else
            error("special_error(4)", false);
    }
    else if (listExists(arg0) || listExists(beforeBrackets(arg0)))
    {
        string _b(beforeDot(arg2)), _a(afterDot(arg2)),
               __b(beforeParams(arg2));

        if (containsBrackets(arg0))
        {
            string after(afterBrackets(arg0)), before(beforeBrackets(arg0));

            after = subtractString(after, "]");

            if (lists.at(indexOfList(before)).size() >= stoi(after))
            {
                if (stoi(after) == 0)
                {
                    if (arg1 == "=")
                    {
                        if (variableExists(arg2))
                        {
                            if (isString(arg2))
                                replaceElement(before, after, variables.at(indexOfVariable(arg2)).getString());
                            else if (isNumber(arg2))
                                replaceElement(before, after, dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                            else
                                error("is_null:" + arg2, false);
                        }
                        else
                            replaceElement(before, after, arg2);
                    }
                }
                else if (lists.at(indexOfList(before)).at(stoi(after)) == "#!=no_line")
                    error("invalid_operation:index_out_of_bounds:" + arg0, false);
                else
                {
                    if (arg1 == "=")
                    {
                        if (variableExists(arg2))
                        {
                            if (isString(arg2))
                                replaceElement(before, after, variables.at(indexOfVariable(arg2)).getString());
                            else if (isNumber(arg2))
                                replaceElement(before, after, dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                            else
                                error("is_null:" + arg2, false);
                        }
                        else
                            replaceElement(before, after, arg2);
                    }
                }
            }
            else
                error("invalid_operation:index_out_of_bounds:" + arg2, false);
        }
        else if (containsBrackets(arg2))
        {
            string listName(beforeBrackets(arg2));

            if (listExists(listName))
            {
                vector<string> listRange = getBracketRange(arg2);

                if (listRange.size() == 2)
                {
                    string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

                    if (rangeBegin.length() != 0 && rangeEnd.length() != 0)
                    {
                        if (isNumeric(rangeBegin) && isNumeric(rangeEnd))
                        {
                            if (stoi(rangeBegin) < stoi(rangeEnd))
                            {
                                if (lists.at(indexOfList(listName)).size() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                                {
                                    if (stoi(rangeBegin) >= 0)
                                    {
                                        if (arg1 == "+=")
                                        {
                                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                                lists.at(indexOfList(arg0)).add(lists.at(indexOfList(listName)).at(i));
                                        }
                                        else if (arg1 == "=")
                                        {
                                            lists.at(indexOfList(arg0)).clear();

                                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                                lists.at(indexOfList(arg0)).add(lists.at(indexOfList(listName)).at(i));
                                        }
                                        else
                                            error("invalid_operator:" + arg1, false);
                                    }
                                    else
                                        error("invalid_operation:index_invalid:" + rangeBegin, false);
                                }
                                else
                                    error("invalid_operation:index_out_of_bounds:" + rangeEnd, false);
                            }
                            else if (stoi(rangeBegin) > stoi(rangeEnd))
                            {
                                if (lists.at(indexOfList(listName)).size() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                                {
                                    if (stoi(rangeBegin) >= 0)
                                    {
                                        if (arg1 == "+=")
                                        {
                                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                                lists.at(indexOfList(arg0)).add(lists.at(indexOfList(listName)).at(i));
                                        }
                                        else if (arg1 == "=")
                                        {
                                            lists.at(indexOfList(arg0)).clear();

                                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                                lists.at(indexOfList(arg0)).add(lists.at(indexOfList(listName)).at(i));
                                        }
                                        else
                                            error("invalid_operator:" + arg1, false);
                                    }
                                    else
                                        error("invalid_operation:index_invalid:" + rangeBegin, false);
                                }
                                else
                                    error("invalid_operation:index_out_of_bounds:" + rangeEnd, false);
                            }
                            else
                                error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                        }
                        else
                            error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                    }
                    else
                        error("invalid_operation:invalid_range_lengths:" + rangeBegin + ".." + rangeEnd, false);
                }
                else
                    error("invalid_operation:invalid_range:" + arg2, false);
            }
            else
                error("invalid_operation:list_undefined:" + listName, false);
        }
        else if (variableExists(_b) && contains(_a, "split") && arg1 == "=")
        {
            if (isString(_b))
            {
                vector<string> params = getParams(_a);
                vector<string> elements;

                if (params.at(0) == "")
                    elements = split(variables.at(indexOfVariable(_b)).getString(), ' ');
                else
                {
                    if (params.at(0)[0] == ';')
                        elements = split(variables.at(indexOfVariable(_b)).getString(), ';');
                    else
                        elements = split(variables.at(indexOfVariable(_b)).getString(), params.at(0)[0]);
                }

                lists.at(indexOfList(arg0)).clear();

                for (int i = 0; i < (int)elements.size(); i++)
                    lists.at(indexOfList(arg0)).add(elements.at(i));
            }
            else
                error("invalid_operation:__Null_string:" + _b, false);
        }
        else if (containsParams(arg2))
        {
            vector<string> params = getParams(arg2);

            if (arg1 == "=")
            {
                lists.at(indexOfList(arg0)).clear();

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
                            lists.at(indexOfList(arg0)).remove(variables.at(indexOfVariable(params.at(i))).getString());
                        else if (isNumber(params.at(i)))
                            lists.at(indexOfList(arg0)).remove(dtos(variables.at(indexOfVariable(params.at(i))).getNumber()));
                        else
                            error("is_null:" + params.at(i), false);
                    }
                    else
                        lists.at(indexOfList(arg0)).remove(params.at(i));
                }
            }
            else
                error("invalid_operator:" + arg1, false);
        }
        else if (variableExists(arg2))
        {
            if (arg1 == "+=")
            {
                if (isString(arg2))
                    lists.at(indexOfList(arg0)).add(variables.at(indexOfVariable(arg2)).getString());
                else if (isNumber(arg2))
                    lists.at(indexOfList(arg0)).add(dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                else
                    error("conversion_error:" + arg2, false);
            }
            else if (arg1 == "-=")
            {
                if (isString(arg2))
                    lists.at(indexOfList(arg0)).remove(variables.at(indexOfVariable(arg2)).getString());
                else if (isNumber(arg2))
                    lists.at(indexOfList(arg0)).remove(dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                else
                    error("conversion_error:" + arg2, false);
            }
            else
                error("invalid_operator:" + arg1, false);
        }
        else if (methodExists(arg2))
        {
            parse(arg2);

            vector<string> _p = getParams(__LastValue);

            if (arg1 == "=")
            {
                lists.at(indexOfList(arg0)).clear();

                for (int i = 0; i < (int)_p.size(); i++)
                    lists.at(indexOfList(arg0)).add(_p.at(i));
            }
            else if (arg1 == "+=")
            {
                for (int i = 0; i < (int)_p.size(); i++)
                    lists.at(indexOfList(arg0)).add(_p.at(i));
            }
            else
                error("invalid_operator:" + arg1, false);
        }
        else
        {
            if (arg1 == "+=")
            {
                if (arg2.length() != 0)
                    lists.at(indexOfList(arg0)).add(arg2);
                else
                    error("is_empty:" + arg2, false);
            }
            else if (arg1 == "-=")
            {
                if (arg2.length() != 0)
                    lists.at(indexOfList(arg0)).remove(arg2);
                else
                    error("is_empty:" + arg2, false);
            }
        }
    }
    else
    {
        if (startsWith(arg0, "@") && zeroDots(arg0))
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
                        if (lists.at(indexOfList(beforeBracket)).size() >= stoi(afterBracket))
                        {
                            if (lists.at(indexOfList(beforeBracket)).at(stoi(afterBracket)) == "#!=no_line")
                                error("invalid_operation:index_out_of_bounds:" + arg2, false);
                            else
                            {
                                string listValue(lists.at(indexOfList(beforeBracket)).at(stoi(afterBracket)));

                                if (isNumeric(listValue))
                                    createVariable(arg0, stod(listValue));
                                else
                                    createVariable(arg0, listValue);
                            }
                        }
                        else
                            error("invalid_operation:index_out_of_bounds:" + arg2, false);
                    }
                    else if (variableExists(beforeBracket))
                        setSubString(arg0, arg2, beforeBracket);
                    else
                        error("invalid_operation:list_undefined:" + beforeBracket, false);
                }
                else if (listExists(before) && after == "size")
                    createVariable(arg0, stod(itos(lists.at(indexOfList(before)).size())));
                else if (before == "self")
                {
                    if (objectExists(__CurrentMethodObject))
                        twoSpace(arg0, arg1, (__CurrentMethodObject + "." + after), (arg0 + " " + arg1 + " " + (__CurrentMethodObject + "." + after)), command);
                    else
                        twoSpace(arg0, arg1, after, (arg0 + " " + arg1 + " " + after), command);
                }
                else if (after == "to_int" || after == "integer")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            createVariable(arg0, (int)variables.at(indexOfVariable(before)).getString()[0]);
                        else if (isNumber(before))
                        {
                            int i = (int)variables.at(indexOfVariable(before)).getNumber();
                            createVariable(arg0, (double)i);
                        }
                        else
                            error("is_null:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "to_double" || after == "double")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            createVariable(arg0, (double)variables.at(indexOfVariable(before)).getString()[0]);
                        else if (isNumber(before))
                        {
                            double i = variables.at(indexOfVariable(before)).getNumber();
                            createVariable(arg0, (double)i);
                        }
                        else
                            error("is_null:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "to_string")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, dtos(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("is_null:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "to_number")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            createVariable(arg0, stod(variables.at(indexOfVariable(before)).getString()));
                        else
                            error("is_null:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (objectExists(before))
                {
                    if (objects.at(indexOfObject(before)).methodExists(after) && !containsParams(after))
                    {
                        parse(arg2);

                        if (isNumeric(__LastValue))
                            createVariable(arg0, stod(__LastValue));
                        else
                            createVariable(arg0, __LastValue);
                    }
                    else if (containsParams(after))
                    {
                        if (objects.at(indexOfObject(before)).methodExists(beforeParams(after)))
                        {
                            executeTemplate(objects.at(indexOfObject(before)).getMethod(beforeParams(after)), getParams(after));

                            if (isNumeric(__LastValue))
                                createVariable(arg0, stod(__LastValue));
                            else
                                createVariable(arg0, __LastValue);
                        }
                        else
                            sysExec(s, command);
                    }
                    else if (objects.at(indexOfObject(before)).variableExists(after))
                    {
                        if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                            createVariable(arg0, objects.at(indexOfObject(before)).getVariable(after).getString());
                        else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                            createVariable(arg0, objects.at(indexOfObject(before)).getVariable(after).getNumber());
                        else
                            error("is_null:" + objects.at(indexOfObject(before)).getVariable(after).name(), false);
                    }
                }
                else if (variableExists(before) && after == "read")
                {
                    if (isString(before))
                    {
                        if (fileExists(variables.at(indexOfVariable(before)).getString()))
                        {
                            ifstream file(variables.at(indexOfVariable(before)).getString().c_str());
                            string line(""), bigString("");

                            if (file.is_open())
                            {
                                while (!file.eof())
                                {
                                    getline(file, line);
                                    bigString.append(line + "\r\n");
                                }

                                file.close();

                                createVariable(arg0, bigString);
                            }
                            else
                                error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                        }
                        else
                            error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                    }
                    else
                        error("invalid_operation:__Null_string:" + before, false);
                }
                else if (__DefiningObject)
                {
                    if (isNumeric(arg2))
                    {
                        Variable newVariable(arg0, stod(arg2));

                        if (__DefiningPrivateCode)
                            newVariable.setPrivate();
                        else if (__DefiningPublicCode)
                            newVariable.setPublic();

                        objects.at(indexOfObject(__CurrentObject)).addVariable(newVariable);
                    }
                    else
                    {
                        Variable newVariable(arg0, arg2);

                        if (__DefiningPrivateCode)
                            newVariable.setPrivate();
                        else if (__DefiningPublicCode)
                            newVariable.setPublic();

                        objects.at(indexOfObject(__CurrentObject)).addVariable(newVariable);
                    }
                }
                else if (arg2 == "null")
                    createVariable(arg0, arg2);
                else if (methodExists(arg2))
                {
                    parse(arg2);

                    if (isNumeric(__LastValue))
                        createVariable(arg0, stod(__LastValue));
                    else
                        createVariable(arg0, __LastValue);
                }
                else if (constantExists(arg2))
                {
                    if (constants.at(indexOfConstant(arg2)).ConstNumber())
                        createVariable(arg0, constants.at(indexOfConstant(arg2)).getNumber());
                    else if (constants.at(indexOfConstant(arg2)).ConstString())
                        createVariable(arg0, constants.at(indexOfConstant(arg2)).getString());
                    else
                        error("conversion_error:__Null_constant:" + arg2, false);
                }
                else if (containsParams(arg2))
                {
                    if (isStringStack(arg2))
                        createVariable(arg0, getStringStack(arg2));
                    else if (stackReady(arg2))
                        createVariable(arg0, getStack(arg2));
                    else if (beforeParams(arg2) == "random")
                    {
                        if (contains(arg2, ".."))
                        {
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
                                        s0 = dtos(variables.at(indexOfVariable(s0)).getNumber());
                                    else if (isString(s0))
                                        s0 = variables.at(indexOfVariable(s0)).getString();
                                }

                                if (variableExists(s2))
                                {
                                    if (isNumber(s2))
                                        s2 = dtos(variables.at(indexOfVariable(s2)).getNumber());
                                    else if (isString(s2))
                                        s2 = variables.at(indexOfVariable(s2)).getString();
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
                                error("invalid_operation:invalid_range:" + s0 + ".." + s2, false);
                        }
                        else
                            error("invalid_operation:invalid_range_separator:" + arg2, false);
                    }
                    else
                    {
                        executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                        if (isNumeric(__LastValue))
                            createVariable(arg0, stod(__LastValue));
                        else
                            createVariable(arg0, __LastValue);
                    }
                }
                else if (variableExists(arg2))
                {
                    if (isNumber(arg2))
                        createVariable(arg0, variables.at(indexOfVariable(arg2)).getNumber());
                    else if (isString(arg2))
                        createVariable(arg0, variables.at(indexOfVariable(arg2)).getString());
                    else
                        createVariable(arg0, __Null);
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
                        cout << "readline: ";
                        getline(cin, line, '\n');

                        if (isNumeric(line))
                            createVariable(arg0, stod(line));
                        else
                            createVariable(arg0, line);
                    }
                }
                else if (arg2 == "args.size")
                    createVariable(arg0, (double)__ArgumentCount);
                else if (before == "readline")
                {
                    if (variableExists(after))
                    {
                        if (isString(after))
                        {
                            string line("");
                            cout << cleanString(variables.at(indexOfVariable(after)).getString());
                            getline(cin, line, '\n');

                            if (isNumeric(line))
                                createVariable(arg0, stod(line));
                            else
                                createVariable(arg0, line);
                        }
                        else
                        {
                            string line("");
                            cout << "readline: ";
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
                        cout << cleanString(after);
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
                            line = getSilentOutput(variables.at(indexOfVariable(after)).getString());

                            if (isNumeric(line))
                                createVariable(arg0, stod(line));
                            else
                                createVariable(arg0, line);

                            cout << endl;
                        }
                        else
                        {
                            string line("");
                            line = getSilentOutput("password: ");

                            if (isNumeric(line))
                                createVariable(arg0, stod(line));
                            else
                                createVariable(arg0, line);

                            cout << endl;
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

                        cout << endl;
                    }
                }
                else if (after == "size")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            createVariable(arg0, (double)variables.at(indexOfVariable(before)).getString().length());
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        createVariable(arg0, (double)before.length());
                }
                else if (after == "sin")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, sin(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "sinh")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, sinh(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "asin")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, asin(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "tan")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, tan(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "tanh")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, tanh(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "atan")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, atan(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "cos")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, cos(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "acos")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, acos(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "cosh")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, cosh(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "log")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, log(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "sqrt")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, sqrt(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "abs")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, abs(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "floor")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, floor(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "ceil")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, ceil(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "exp")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            createVariable(arg0, exp(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "to_upper")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            createVariable(arg0, getUpper(variables.at(indexOfVariable(before)).getString()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "to_lower")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            createVariable(arg0, getLower(variables.at(indexOfVariable(before)).getString()));
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                        error("invalid_operation:variable_undefined:" + before, false);
                }
                else if (after == "bytes")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                        {
                            if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                createVariable(arg0, getBytes(variables.at(indexOfVariable(before)).getString()));
                            else
                                error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                        }
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                    {
                        if (fileExists(before))
                            createVariable(arg0, getBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                }
                else if (after == "kbytes")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                        {
                            if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                createVariable(arg0, getKBytes(variables.at(indexOfVariable(before)).getString()));
                            else
                                error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                        }
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                    {
                        if (fileExists(before))
                            createVariable(arg0, getKBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                }
                else if (after == "mbytes")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                        {
                            if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                createVariable(arg0, getMBytes(variables.at(indexOfVariable(before)).getString()));
                            else
                                error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                        }
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                    {
                        if (fileExists(before))
                            createVariable(arg0, getMBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                }
                else if (after == "gbytes")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                        {
                            if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                createVariable(arg0, getGBytes(variables.at(indexOfVariable(before)).getString()));
                            else
                                error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                        }
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                    {
                        if (fileExists(before))
                            createVariable(arg0, getGBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                }
                else if (after == "tbytes")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                        {
                            if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                createVariable(arg0, getTBytes(variables.at(indexOfVariable(before)).getString()));
                            else
                                error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                        }
                        else
                            error("conversion_error:" + before, false);
                    }
                    else
                    {
                        if (fileExists(before))
                            createVariable(arg0, getTBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
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
                        createVariable(arg0, variables.at(indexOfVariable(arg2)).getString());
                    else if (isNumber(arg2))
                        createVariable(arg0, variables.at(indexOfVariable(arg2)).getNumber());
                    else
                        createVariable(arg0, __Null);
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
                        createVariable(arg0, 0 - variables.at(indexOfVariable(arg2)).getNumber());
                    else
                        createVariable(arg0, __Null);
                }
                else
                {
                    if (is_num(arg2))
                        createVariable(arg0, stod(arg2));
                    else
                        createVariable(arg0, cleanString(arg2));
                }
            }
            else if (arg1 == "?")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                        createVariable(arg0, getStdout(variables.at(indexOfVariable(arg2)).getString()));
                    else
                        error("conversion_error:" + arg2, false);
                }
                else
                    createVariable(arg0, getStdout(cleanString(arg2)));
            }
            else if (arg1 == "!")
            {
                if (variableExists(arg2))
                {
                    if (isString(arg2))
                        createVariable(arg0, getParsedOutput(variables.at(indexOfVariable(arg2)).getString()));
                    else
                        error("conversion_error:" + arg2, false);
                }
                else
                    createVariable(arg0, getParsedOutput(cleanString(arg2)));
            }
            else
                error("invalid_operator:" + arg2, false);
        }
        else if (startsWith(arg0, "@") && !zeroDots(arg2))
        {
            string before = beforeDot(arg2),
                   after = afterDot(arg2);

            if (objectExists(before))
            {
                if (arg1 == "=")
                {
                    if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                        createVariable(arg0, objects.at(indexOfObject(before)).getVariable(after).getString());
                    else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                        createVariable(arg0, objects.at(indexOfObject(before)).getVariable(after).getNumber());
                }
            }
        }
        else if (!objectExists(arg0) && objectExists(arg2))
        {
            if (arg1 == "=")
            {
                vector<Method> objectMethods = objects.at(indexOfObject(arg2)).getMethods();
                Object newObject(arg0);

                for (int i = 0; i < (int)objectMethods.size(); i++)
                    newObject.addMethod(objectMethods.at(i));


                vector<Variable> objectVariables = objects.at(indexOfObject(arg2)).getVariables();

                for (int i = 0; i < (int)objectVariables.size(); i++)
                    newObject.addVariable(objectVariables.at(i));

                if (__ExecutedMethod)
                    newObject.collect();
                else
                    newObject.dontCollect();

                objects.push_back(newObject);
                __CurrentObject = arg1;
                __DefiningObject = false;

                newObject.clear();
                objectMethods.clear();
            }
            else
                error("invalid_operator:" + arg1, false);
        }
        else if (isUpperConstant(arg0))
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
                    error("invalid_operator:" + arg1, false);
            }
            else
                error("invalid_operation:constant_defined:" + arg0, false);
        }
        else
        {
            if (isNumeric(arg0) && isNumeric(arg2))
            {
                if (arg1 == "+")
                    say(dtos(stod(arg0) + stod(arg2)));
                else if (arg1 == "-")
                    say(dtos(stod(arg0) - stod(arg2)));
                else if (arg1 == "*")
                    say(dtos(stod(arg0) * stod(arg2)));
                else if (arg1 == "/")
                    say(dtos(stod(arg0) / stod(arg2)));
                else if (arg1 == "**")
                    say(dtos(pow(stod(arg0), stod(arg2))));
                else if (arg1 == "%")
                {
                    if ((int)stod(arg2) == 0)
                        error("segfault:" + s, false);
                    else
                        say(dtos((int)stod(arg0) % (int)stod(arg2)));
                }
                else
                    error("invalid_operator:" + arg1, false);
            }
            else
            {
                if (arg1 == "+")
                    say(arg0 + arg2);
                else if (arg1 == "-")
                    say(subtractString(arg0, arg2));
                else if (arg1 == "*")
                {
                    if (!zeroNumbers(arg2))
                    {
                        string bigstr(arg0);
                        for (int i = 1; i <= stoi(arg2); i++)
                        {
                            bigstr.append(arg0);
                            __stdout(arg0);
                        }

                        setLastValue(bigstr);
                    }
                    else
                        error("invalid_operation:" + s, false);
                }
                else if (arg1 == "/")
                    say(subtractString(arg0, arg2));
                else
                    error("invalid_operator:" + arg1, false);
            }
        }
    }
}

void threeSpace(string arg0, string arg1, string arg2, string arg3, string s, vector<string> command)
{
    // isNumber(arg3)
    // isString(arg3)

    if (arg0 == "object")
    {
        if (objectExists(arg1))
        {
            __DefiningObject = true;
            __CurrentObject = arg1;
        }
        else
        {
            if (objectExists(arg3))
            {
                if (arg2 == "=")
                {
                    vector<Method> objectMethods = objects.at(indexOfObject(arg3)).getMethods();
                    Object newObject(arg1);

                    for (int i = 0; i < (int)objectMethods.size(); i++)
                    {
                        if (objectMethods.at(i).isPublic())
                            newObject.addMethod(objectMethods.at(i));
                    }

                    objects.push_back(newObject);
                    __CurrentObject = arg1;
                    __DefiningObject = true;

                    newObject.clear();
                    objectMethods.clear();
                }
                else
                    error("invalid_operator:" + arg2, false);
            }
            else
                error("invalid_operation:object_undefined:" + arg3, false);
        }
    }
    else if (arg0 == "unless")
    {
        if (listExists(arg3))
        {
            if (arg2 == "in")
            {
                string testString("[none]");

                if (variableExists(arg1))
                {
                    if (isString(arg1))
                        testString = variables.at(indexOfVariable(arg1)).getString();
                    else if (isNumber(arg1))
                        testString = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                    else
                        error("is_null:" + arg1, false);
                }
                else
                    testString = arg1;

                if (testString != "[none]")
                {
                    bool elementFound = false;
                    for (int i = 0; i < (int)lists.at(indexOfList(arg3)).size(); i++)
                    {
                        if (lists.at(indexOfList(arg3)).at(i) == testString)
                        {
                            elementFound = true;
                            failedIfStatement();
                            __LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        successfulIF();
                }
                else
                    successfulIF();
            }
        }
        else if (variableExists(arg1) && variableExists(arg3))
        {
            if (isString(arg1) && isString(arg3))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (variables.at(indexOfVariable(arg1)).getString() == variables.at(indexOfVariable(arg3)).getString())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (variables.at(indexOfVariable(arg1)).getString() != variables.at(indexOfVariable(arg3)).getString())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() > variables.at(indexOfVariable(arg3)).getString().length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() < variables.at(indexOfVariable(arg3)).getString().length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() <= variables.at(indexOfVariable(arg3)).getString().length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() >= variables.at(indexOfVariable(arg3)).getString().length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "contains")
                {
                    if (contains(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "ends_with")
                {
                    if (endsWith(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "begins_with")
                {
                    if (startsWith(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    successfulIF();
                }
            }
            else if (isNumber(arg1) && isNumber(arg3))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() == variables.at(indexOfVariable(arg3)).getNumber())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() != variables.at(indexOfVariable(arg3)).getNumber())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() > variables.at(indexOfVariable(arg3)).getNumber())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() >= variables.at(indexOfVariable(arg3)).getNumber())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() < variables.at(indexOfVariable(arg3)).getNumber())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() <= variables.at(indexOfVariable(arg3)).getNumber())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    successfulIF();
                }
            }
            else
            {
                error("conversion_error:" + s, false);
                successfulIF();
            }
        }
        else if ((variableExists(arg1) && !variableExists(arg3)) && !methodExists(arg3) && notObjectMethod(arg3) && !containsParams(arg3))
        {
            if (isNumber(arg1))
            {
                if (isNumeric(arg3))
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() == stod(arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() != stod(arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() > stod(arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() < stod(arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
                else if (arg3 == "number?")
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                        error("invalid_operator:" + arg2, false);
                }
                else
                {
                    error("conversion_error:" + s, false);
                    successfulIF();
                }
            }
            else if (isString(arg1))
            {
                if (arg3 == "string?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                            failedIfStatement();
                        else if (arg2 == "!=" || arg2 == "not")
                            successfulIF();
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "!")
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                }
                else if (arg3 == "number?")
                {
                    if (isNumber(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                            failedIfStatement();
                        else if (arg2 == "!=" || arg2 == "not")
                            successfulIF();
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=" || arg2 == "not")
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                }
                else if (arg3 == "upper?" || arg3 == "uppercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isUpper(arg2))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                            successfulIF();
                    }
                }
                else if (arg3 == "lower?" || arg3 == "lowercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isLower(arg2))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                            successfulIF();
                    }
                }
                else if (arg3 == "file?")
                {
                    if (isString(arg1))
                    {
                        if (fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==" || arg2 == "is")
                                failedIfStatement();
                            else if (arg2 == "!=" || arg2 == "not")
                                successfulIF();
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=" || arg2 == "not")
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                    }
                    else
                    {
                        error("is_null:" + arg1, false);
                        successfulIF();
                    }
                }
                else if (arg3 == "dir?" || arg3 == "directory?")
                {
                    if (isString(arg1))
                    {
                        if (directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==" || arg2 == "is")
                                failedIfStatement();
                            else if (arg2 == "!=" || arg2 == "not")
                                successfulIF();
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=" || arg2 == "not")
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                    }
                    else
                    {
                        error("is_null:" + arg1, false);
                        successfulIF();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() == arg3)
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() != arg3)
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() > arg3.length())
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() < arg3.length())
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() >= arg3.length())
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() <= arg3.length())
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
            }
            else
            {
                error("special_error(0)", false); // variable is neither string nor number
                successfulIF();
            }
        }
        else if ((variableExists(arg1) && !variableExists(arg3)) && !methodExists(arg3) && notObjectMethod(arg3) && containsParams(arg3))
        {
            string stackValue("");

            if (isStringStack(arg3))
                stackValue = getStringStack(arg3);
            else if (stackReady(arg3))
                stackValue = dtos(getStack(arg3));
            else
                stackValue = arg3;

            if (isNumber(arg1))
            {
                if (isNumeric(stackValue))
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() == stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() != stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() > stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() < stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
                else if (stackValue == "is_number?" || stackValue == "number?")
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                        error("invalid_operator:" + arg2, false);
                }
                else
                {
                    error("conversion_error:" + s, false);
                    successfulIF();
                }
            }
            else if (isString(arg1))
            {
                if (stackValue == "is_string?" || stackValue == "string?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                            failedIfStatement();
                        else if (arg2 == "!=" || arg2 == "not")
                            successfulIF();
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=" || arg2 == "not")
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                }
                else if (stackValue == "is_number?" || stackValue == "number?")
                {
                    if (isNumber(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                            failedIfStatement();
                        else if (arg2 == "!=" || arg2 == "not")
                            successfulIF();
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=" || arg2 == "not")
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                }
                else if (stackValue == "uppercase?" || stackValue == "upper?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isUpper(arg2))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                            successfulIF();
                    }
                }
                else if (stackValue == "lowercase?" || stackValue == "lower?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isLower(arg2))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                            successfulIF();
                    }
                }
                else if (stackValue == "file?")
                {
                    if (isString(arg1))
                    {
                        if (fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==" || arg2 == "is")
                                failedIfStatement();
                            else if (arg2 == "!=" || arg2 == "not")
                                successfulIF();
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=" || arg2 == "not")
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                    }
                    else
                    {
                        error("is_null:" + arg1, false);
                        successfulIF();
                    }
                }
                else if (stackValue == "dir?" || stackValue == "directory?")
                {
                    if (isString(arg1))
                    {
                        if (directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==" || arg2 == "is")
                                failedIfStatement();
                            else if (arg2 == "!=" || arg2 == "not")
                                successfulIF();
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=" || arg2 == "not")
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                    }
                    else
                    {
                        error("is_null:" + arg1, false);
                        successfulIF();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() == stackValue)
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() != stackValue)
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() > stackValue.length())
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() < stackValue.length())
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() >= stackValue.length())
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() <= stackValue.length())
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
            }
            else
            {
                error("special_error(0)", false); // variable is neither string nor number
                successfulIF();
            }
        }
        else if ((!variableExists(arg1) && variableExists(arg3)) && !methodExists(arg1) && notObjectMethod(arg1) && !containsParams(arg1))
        {
            if (isNumber(arg3))
            {
                if (isNumeric(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() == stod(arg1))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() != stod(arg1))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() > stod(arg1))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() < stod(arg1))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() >= stod(arg1))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() <= stod(arg1))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
                else
                {
                    error("conversion_error:" + s, false);
                    successfulIF();
                }
            }
            else if (isString(arg3))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() == arg1)
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() != arg1)
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() > arg1.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() < arg1.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() >= arg1.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() <= arg1.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    successfulIF();
                }
            }
            else
            {
                error("special_error(1)", false); // variable is neither string nor number
                successfulIF();
            }
        }
        else if ((!variableExists(arg1) && variableExists(arg3)) && !methodExists(arg1) && notObjectMethod(arg1) && containsParams(arg1))
        {
            string stackValue("");

            if (isStringStack(arg1))
                stackValue = getStringStack(arg1);
            else if (stackReady(arg1))
                stackValue = dtos(getStack(arg1));
            else
                stackValue = arg1;

            if (isNumber(arg3))
            {
                if (isNumeric(stackValue))
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() == stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() != stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() > stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() < stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() >= stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() <= stod(stackValue))
                            failedIfStatement();
                        else
                            successfulIF();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
                else
                {
                    error("conversion_error:" + s, false);
                    successfulIF();
                }
            }
            else if (isString(arg3))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() == stackValue)
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() != stackValue)
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() > stackValue.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() < stackValue.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() >= stackValue.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() <= stackValue.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    successfulIF();
                }
            }
            else
            {
                error("special_error(1)", false); // variable is neither string nor number
                successfulIF();
            }
        }
        else if (containsParams(arg1) || containsParams(arg3))
        {
            if (containsParams(arg1) && containsParams(arg3))
            {
                if (!zeroDots(arg1) && !zeroDots(arg3))
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1)),
                           arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    string arg1Result(""), arg3Result("");

                    if (objectExists(arg1before) && objectExists(arg3before))
                    {
                        if (objects.at(indexOfObject(arg1before)).methodExists(beforeParams(arg1after)))
                            executeTemplate(objects.at(indexOfObject(arg1before)).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = __LastValue;

                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = __LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (arg1Result == arg3Result)
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (arg1Result != arg3Result)
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                    }
                    else
                    {
                        if (!objectExists(arg1before))
                            error("invalid_operation:object_undefined:" + arg1before, false);

                        if (!objectExists(arg3before))
                            error("invalid_operation:object_undefined:" + arg3before, false);

                        successfulIF();
                    }
                }
                else if (!zeroDots(arg1) && zeroDots(arg3))
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    string arg1Result(""), arg3Result("");

                    if (objectExists(arg1before))
                    {
                        if (objects.at(indexOfObject(arg1before)).methodExists(beforeParams(arg1after)))
                            executeTemplate(objects.at(indexOfObject(arg1before)).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = __LastValue;

                        if (methodExists(beforeParams(arg3)))
                            executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                        arg3Result = __LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (arg1Result == arg3Result)
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (arg1Result != arg3Result)
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:object_undefined:" + arg1before, false);
                        successfulIF();
                    }
                }
                else if (zeroDots(arg1) && !zeroDots(arg3))
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    string arg1Result(""), arg3Result("");

                    if (objectExists(arg3before))
                    {
                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = __LastValue;

                        if (methodExists(beforeParams(arg1)))
                            executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                        arg1Result = __LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (arg1Result == arg3Result)
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (arg1Result != arg3Result)
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:object_undefined:" + arg3before, false);
                        successfulIF();
                    }
                }
                else if (zeroDots(arg1) && zeroDots(arg3))
                {
                    string arg1Result(""), arg3Result("");

                    if (methodExists(beforeParams(arg1)))
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                    arg1Result = __LastValue;

                    if (methodExists(beforeParams(arg3)))
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                    arg3Result = __LastValue;

                    if (isNumeric(arg1Result) && isNumeric(arg3Result))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (stod(arg1Result) == stod(arg3Result))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (stod(arg1Result) != stod(arg3Result))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == "<")
                        {
                            if (stod(arg1Result) < stod(arg3Result))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == ">")
                        {
                            if (stod(arg1Result) > stod(arg3Result))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == "<=")
                        {
                            if (stod(arg1Result) <= stod(arg3Result))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == ">=")
                        {
                            if (stod(arg1Result) >= stod(arg3Result))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                    else
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (arg1Result == arg3Result)
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (arg1Result != arg3Result)
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            successfulIF();
                        }
                    }
                }
                else
                {
                    error("special_error(9)", false);
                    successfulIF();
                }
            }
            else if (containsParams(arg1) && !containsParams(arg3))
            {
                string arg1Result(""), arg3Result("");

                bool pass = true;

                if (zeroDots(arg1))
                {
                    if (methodExists(beforeParams(arg1)))
                    {
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                        arg1Result = __LastValue;

                        if (methodExists(arg3))
                        {
                            parse(arg3);
                            arg3Result = __LastValue;
                        }
                        else if (variableExists(arg3))
                        {
                            if (isString(arg3))
                                arg3Result = variables.at(indexOfVariable(arg3)).getString();
                            else if (isNumber(arg3))
                                arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                            else
                            {
                                pass = false;
                                error("is_null:" + arg3, false);
                                successfulIF();
                            }
                        }
                        else
                            arg3Result = arg3;

                        if (pass)
                        {
                            if (isNumeric(arg1Result) && isNumeric(arg3Result))
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (stod(arg1Result) == stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    successfulIF();
                                }
                            }
                            else
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (arg1Result == arg3Result)
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (arg1Result != arg3Result)
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    successfulIF();
                                }
                            }
                        }
                        else
                            successfulIF();
                    }
                    else
                    {
                        error("invalid_operation:method_undefined:" + beforeParams(arg1), false);
                        successfulIF();
                    }
                }
                else
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    if (objectExists(arg1before))
                    {
                        if (objects.at(indexOfObject(arg1before)).methodExists(beforeParams(arg1after)))
                            executeTemplate(objects.at(indexOfObject(arg1before)).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = __LastValue;

                        if (variableExists(arg3))
                        {
                            if (isString(arg3))
                                arg3Result = variables.at(indexOfVariable(arg3)).getString();
                            else if (isNumber(arg3))
                                arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                            else
                            {
                                pass = false;
                                error("is_null:" + arg3, false);
                                successfulIF();
                            }
                        }
                        else if (methodExists(arg3))
                        {
                            parse(arg3);

                            arg3Result = __LastValue;
                        }
                        else
                            arg3Result = arg3;

                        if (pass)
                        {
                            if (isNumeric(arg1Result) && isNumeric(arg3Result))
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (stod(arg1Result) == stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    successfulIF();
                                }
                            }
                            else
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (arg1Result == arg3Result)
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (arg1Result != arg3Result)
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    successfulIF();
                                }
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:object_undefined:" + arg1before, false);
                        successfulIF();
                    }
                }
            }
            else if (!containsParams(arg1) && containsParams(arg3))
            {
                string arg1Result(""), arg3Result("");

                bool pass = true;

                if (zeroDots(arg3))
                {
                    if (methodExists(beforeParams(arg3)))
                    {
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                        arg3Result = __LastValue;

                        if (methodExists(arg1))
                        {
                            parse(arg1);
                            arg1Result = __LastValue;
                        }
                        else if (variableExists(arg1))
                        {
                            if (isString(arg1))
                                arg1Result = variables.at(indexOfVariable(arg1)).getString();
                            else if (isNumber(arg1))
                                arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                            else
                            {
                                pass = false;
                                error("is_null:" + arg1, false);
                                successfulIF();
                            }
                        }
                        else
                            arg1Result = arg1;

                        if (pass)
                        {
                            if (isNumeric(arg3Result) && isNumeric(arg1Result))
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (stod(arg3Result) == stod(arg1Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (stod(arg3Result) != stod(arg1Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg3Result) < stod(arg1Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg3Result) > stod(arg1Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg3Result) <= stod(arg1Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg3Result) >= stod(arg1Result))
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    successfulIF();
                                }
                            }
                            else
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (arg3Result == arg1Result)
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (arg3Result != arg1Result)
                                        failedIfStatement();
                                    else
                                        successfulIF();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    successfulIF();
                                }
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:method_undefined:" + beforeParams(arg3), false);
                        successfulIF();
                    }
                }
                else
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    if (objectExists(arg3before))
                    {
                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = __LastValue;

                        if (variableExists(arg1))
                        {
                            if (isString(arg1))
                                arg1Result = variables.at(indexOfVariable(arg1)).getString();
                            else if (isNumber(arg3))
                                arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                            else
                            {
                                error("is_null:" + arg1, false);
                                successfulIF();
                            }
                        }
                        else if (methodExists(arg1))
                        {
                            parse(arg1);

                            arg1Result = __LastValue;
                        }
                        else
                            arg1Result = arg1;

                        if (isNumeric(arg3Result) && isNumeric(arg1Result))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(arg3Result) == stod(arg1Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(arg3Result) != stod(arg1Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg3Result) < stod(arg1Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg3Result) > stod(arg1Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg3Result) <= stod(arg1Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg3Result) >= stod(arg1Result))
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (arg3Result == arg1Result)
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (arg3Result != arg1Result)
                                    failedIfStatement();
                                else
                                    successfulIF();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                successfulIF();
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:object_undefined:" + arg3before, false);
                        successfulIF();
                    }
                }
            }
        }
        else if ((methodExists(arg1) && arg3 != "method?")|| methodExists(arg3))
        {
            string arg1Result(""), arg3Result("");

            if (methodExists(arg1))
            {
                parse(arg1);
                arg1Result = __LastValue;
            }
            else if (variableExists(arg1))
            {
                if (isString(arg1))
                    arg1Result = variables.at(indexOfVariable(arg1)).getString();
                else if (isNumber(arg1))
                    arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                else
                {
                    error("is_null:" + arg1, false);
                    successfulIF();
                }
            }
            else
                arg1Result = arg1;

            if (methodExists(arg3))
            {
                parse(arg3);
                arg3Result = __LastValue;
            }
            else if (variableExists(arg3))
            {
                if (isString(arg3))
                    arg3Result = variables.at(indexOfVariable(arg3)).getString();
                else if (isNumber(arg3))
                    arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                else
                {
                    error("is_null:" + arg3, false);
                    successfulIF();
                }
            }
            else
                arg3Result = arg3;

            if (isNumeric(arg1Result) && isNumeric(arg3Result))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (stod(arg1Result) == stod(arg3Result))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (stod(arg1Result) != stod(arg3Result))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<")
                {
                    if (stod(arg1Result) < stod(arg3Result))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">")
                {
                    if (stod(arg1Result) > stod(arg3Result))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "<=")
                {
                    if (stod(arg1Result) <= stod(arg3Result))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == ">=")
                {
                    if (stod(arg1Result) >= stod(arg3Result))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    successfulIF();
                }
            }
            else
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (arg1Result == arg3Result)
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (arg1Result != arg3Result)
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    successfulIF();
                }
            }
        }
        else
        {
            if (arg3 == "object?")
            {
                if (objectExists(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
            }
            else if (arg3 == "var?" || arg3 == "variable?")
            {
                if (variableExists(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
                else
                {
                    if (arg2 == "=")
                        successfulIF();
                    else if (arg2 == "!")
                        failedIfStatement();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
            }
            else if (arg3 == "method?")
            {
                if (methodExists(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
            }
            else if (arg3 == "list?")
            {
                if (listExists(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        successfulIF();
                    }
                }
            }
            else if (arg2 == "==" || arg2 == "is")
            {
                if (arg1 == arg3)
                    failedIfStatement();
                else
                    successfulIF();
            }
            else if (arg2 == "!=" || arg2 == "not")
            {
                if (arg1 != arg3)
                    failedIfStatement();
                else
                    successfulIF();
            }
            else if (arg2 == ">")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) > stod(arg3))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    if (arg1.length() > arg3.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
            }
            else if (arg2 == "<")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) < stod(arg3))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    if (arg1.length() < arg3.length())
                        failedIfStatement();
                    else
                        successfulIF();
                }
            }
            else if (arg2 == ">=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) >= stod(arg3))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    error("invalid_operator:>=", false);
                    successfulIF();
                }
            }
            else if (arg2 == "<=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) <= stod(arg3))
                        failedIfStatement();
                    else
                        successfulIF();
                }
                else
                {
                    error("invalid_operator:<=", false);
                    failedIfStatement();
                }
            }
            else if (arg2 == "begins_with")
            {
                if (startsWith(arg1, arg3))
                    failedIfStatement();
                else
                    successfulIF();
            }
            else if (arg2 == "ends_with")
            {
                if (endsWith(arg1, arg3))
                    failedIfStatement();
                else
                    successfulIF();
            }
            else if (arg2 == "contains")
            {
                if (contains(arg1, arg3))
                    failedIfStatement();
                else
                    successfulIF();
            }
            else
            {
                error("invalid_operator:" + arg2, false);
                successfulIF();
            }
        }
    }
    else if (arg0 == "if")
    {
        if (listExists(arg3))
        {
            if (arg2 == "in")
            {
                string testString("[none]");

                if (variableExists(arg1))
                {
                    if (isString(arg1))
                        testString = variables.at(indexOfVariable(arg1)).getString();
                    else if (isNumber(arg1))
                        testString = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                    else
                        error("is_null:" + arg1, false);
                }
                else
                    testString = arg1;

                if (testString != "[none]")
                {
                    bool elementFound = false;
                    for (int i = 0; i < (int)lists.at(indexOfList(arg3)).size(); i++)
                    {
                        if (lists.at(indexOfList(arg3)).at(i) == testString)
                        {
                            elementFound = true;
                            successfulIF();
                            __LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        failedIfStatement();
                }
                else
                    failedIfStatement();
            }
        }
        else if (listExists(arg1) && arg3 != "list?")
        {
            if (arg2 == "contains")
            {
                string testString("[none]");

                if (variableExists(arg3))
                {
                    if (isString(arg3))
                        testString = variables.at(indexOfVariable(arg3)).getString();
                    else if (isNumber(arg3))
                        testString = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                    else
                        error("is_null:" + arg3, false);
                }
                else
                    testString = arg3;

                if (testString != "[none]")
                {
                    bool elementFound = false;
                    for (int i = 0; i < (int)lists.at(indexOfList(arg1)).size(); i++)
                    {
                        if (lists.at(indexOfList(arg1)).at(i) == testString)
                        {
                            elementFound = true;
                            successfulIF();
                            __LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        failedIfStatement();
                }
                else
                    failedIfStatement();
            }
        }
        else if (variableExists(arg1) && variableExists(arg3))
        {
            if (isString(arg1) && isString(arg3))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (variables.at(indexOfVariable(arg1)).getString() == variables.at(indexOfVariable(arg3)).getString())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (variables.at(indexOfVariable(arg1)).getString() != variables.at(indexOfVariable(arg3)).getString())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() > variables.at(indexOfVariable(arg3)).getString().length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() < variables.at(indexOfVariable(arg3)).getString().length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() <= variables.at(indexOfVariable(arg3)).getString().length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() >= variables.at(indexOfVariable(arg3)).getString().length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "contains")
                {
                    if (contains(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "ends_with")
                {
                    if (endsWith(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "begins_with")
                {
                    if (startsWith(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    failedIfStatement();
                }
            }
            else if (isNumber(arg1) && isNumber(arg3))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() == variables.at(indexOfVariable(arg3)).getNumber())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() != variables.at(indexOfVariable(arg3)).getNumber())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() > variables.at(indexOfVariable(arg3)).getNumber())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() >= variables.at(indexOfVariable(arg3)).getNumber())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() < variables.at(indexOfVariable(arg3)).getNumber())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() <= variables.at(indexOfVariable(arg3)).getNumber())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    failedIfStatement();
                }
            }
            else
            {
                error("conversion_error:" + s, false);
                failedIfStatement();
            }
        }
        else if ((variableExists(arg1) && !variableExists(arg3)) && !methodExists(arg3) && notObjectMethod(arg3) && !containsParams(arg3))
        {
            if (isNumber(arg1))
            {
                if (isNumeric(arg3))
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() == stod(arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() != stod(arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() > stod(arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() < stod(arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
                else if (arg3 == "number?")
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                        error("invalid_operator:" + arg2, false);
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedIfStatement();
                }
            }
            else if (isString(arg1))
            {
                if (arg3 == "string?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                            successfulIF();
                        else if (arg2 == "!=" || arg2 == "not")
                            failedIfStatement();
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                }
                else if (arg3 == "number?")
                {
                    if (isNumber(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                            successfulIF();
                        else if (arg2 == "!=" || arg2 == "not")
                            failedIfStatement();
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                }
                else if (arg3 == "upper?" || arg3 == "uppercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                            failedIfStatement();
                    }
                }
                else if (arg3 == "lower?" || arg3 == "lowercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                            failedIfStatement();
                    }
                }
                else if (arg3 == "file?")
                {
                    if (isString(arg1))
                    {
                        if (fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==" || arg2 == "is")
                                successfulIF();
                            else if (arg2 == "!=" || arg2 == "not")
                                failedIfStatement();
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                    }
                    else
                    {
                        error("is_null:" + arg1, false);
                        failedIfStatement();
                    }
                }
                else if (arg3 == "dir?" || arg3 == "directory?")
                {
                    if (isString(arg1))
                    {
                        if (directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==" || arg2 == "is")
                                successfulIF();
                            else if (arg2 == "!=" || arg2 == "not")
                                failedIfStatement();
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                    }
                    else
                    {
                        error("is_null:" + arg1, false);
                        failedIfStatement();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() == arg3)
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() != arg3)
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() > arg3.length())
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() < arg3.length())
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() >= arg3.length())
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() <= arg3.length())
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
            }
            else
            {
                error("special_error(0)", false); // variable is neither string nor number
                failedIfStatement();
            }
        }
        else if ((variableExists(arg1) && !variableExists(arg3)) && !methodExists(arg3) && notObjectMethod(arg3) && containsParams(arg3))
        {
            string stackValue("");

            if (isStringStack(arg3))
                stackValue = getStringStack(arg3);
            else if (stackReady(arg3))
                stackValue = dtos(getStack(arg3));
            else
                stackValue = arg3;

            if (isNumber(arg1))
            {
                if (isNumeric(stackValue))
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() == stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() != stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() > stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() < stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
                else if (stackValue == "number?")
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                        error("invalid_operator:" + arg2, false);
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedIfStatement();
                }
            }
            else if (isString(arg1))
            {
                if (stackValue == "string?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                            successfulIF();
                        else if (arg2 == "!=" || arg2 == "not")
                            failedIfStatement();
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                }
                else if (stackValue == "number?")
                {
                    if (isNumber(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                            successfulIF();
                        else if (arg2 == "!=" || arg2 == "not")
                            failedIfStatement();
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                }
                else if (stackValue == "upper?" || stackValue == "uppercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                            failedIfStatement();
                    }
                }
                else if (stackValue == "lower?" || stackValue == "lowercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                failedIfStatement();
                            else
                                successfulIF();
                        }
                        else
                            failedIfStatement();
                    }
                }
                else if (stackValue == "file?")
                {
                    if (isString(arg1))
                    {
                        if (fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==" || arg2 == "is")
                                successfulIF();
                            else if (arg2 == "!=" || arg2 == "not")
                                failedIfStatement();
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                    }
                    else
                    {
                        error("is_null:" + arg1, false);
                        failedIfStatement();
                    }
                }
                else if (stackValue == "dir?" || stackValue == "directory?")
                {
                    if (isString(arg1))
                    {
                        if (directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==" || arg2 == "is")
                                successfulIF();
                            else if (arg2 == "!=" || arg2 == "not")
                                failedIfStatement();
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                    }
                    else
                    {
                        error("is_null:" + arg1, false);
                        failedIfStatement();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() == stackValue)
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() != stackValue)
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() > stackValue.length())
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() < stackValue.length())
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() >= stackValue.length())
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() <= stackValue.length())
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
            }
            else
            {
                error("special_error(0)", false); // variable is neither string nor number
                failedIfStatement();
            }
        }
        else if ((!variableExists(arg1) && variableExists(arg3)) && !methodExists(arg1) && notObjectMethod(arg1) && !containsParams(arg1))
        {
            if (isNumber(arg3))
            {
                if (isNumeric(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() == stod(arg1))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() != stod(arg1))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() > stod(arg1))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() < stod(arg1))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() >= stod(arg1))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() <= stod(arg1))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedIfStatement();
                }
            }
            else if (isString(arg3))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() == arg1)
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() != arg1)
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() > arg1.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() < arg1.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() >= arg1.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() <= arg1.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    failedIfStatement();
                }
            }
            else
            {
                error("special_error(1)", false); // variable is neither string nor number
                failedIfStatement();
            }
        }
        else if ((!variableExists(arg1) && variableExists(arg3)) && !methodExists(arg1) && notObjectMethod(arg1) && containsParams(arg1))
        {
            string stackValue("");

            if (isStringStack(arg1))
                stackValue = getStringStack(arg1);
            else if (stackReady(arg1))
                stackValue = dtos(getStack(arg1));
            else
                stackValue = arg1;

            if (isNumber(arg3))
            {
                if (isNumeric(stackValue))
                {
                    if (arg2 == "==" || arg2 == "is")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() == stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "!=" || arg2 == "not")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() != stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() > stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() < stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() >= stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() <= stod(stackValue))
                            successfulIF();
                        else
                            failedIfStatement();
                    }
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedIfStatement();
                }
            }
            else if (isString(arg3))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() == stackValue)
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() != stackValue)
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() > stackValue.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() < stackValue.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() >= stackValue.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() <= stackValue.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    failedIfStatement();
                }
            }
            else
            {
                error("special_error(1)", false); // variable is neither string nor number
                failedIfStatement();
            }
        }
        else if (containsParams(arg1) || containsParams(arg3))
        {
            if (containsParams(arg1) && containsParams(arg3))
            {
                if (!zeroDots(arg1) && !zeroDots(arg3))
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1)),
                           arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    string arg1Result(""), arg3Result("");

                    if (objectExists(arg1before) && objectExists(arg3before))
                    {
                        if (objects.at(indexOfObject(arg1before)).methodExists(beforeParams(arg1after)))
                            executeTemplate(objects.at(indexOfObject(arg1before)).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = __LastValue;

                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = __LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (arg1Result == arg3Result)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (arg1Result != arg3Result)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                    }
                    else
                    {
                        if (!objectExists(arg1before))
                            error("invalid_operation:object_undefined:" + arg1before, false);

                        if (!objectExists(arg3before))
                            error("invalid_operation:object_undefined:" + arg3before, false);

                        failedIfStatement();
                    }
                }
                else if (!zeroDots(arg1) && zeroDots(arg3))
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    string arg1Result(""), arg3Result("");

                    if (objectExists(arg1before))
                    {
                        if (objects.at(indexOfObject(arg1before)).methodExists(beforeParams(arg1after)))
                            executeTemplate(objects.at(indexOfObject(arg1before)).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = __LastValue;

                        if (methodExists(beforeParams(arg3)))
                            executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                        arg3Result = __LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (arg1Result == arg3Result)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (arg1Result != arg3Result)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:object_undefined:" + arg1before, false);
                        failedIfStatement();
                    }
                }
                else if (zeroDots(arg1) && !zeroDots(arg3))
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    string arg1Result(""), arg3Result("");

                    if (objectExists(arg3before))
                    {
                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = __LastValue;

                        if (methodExists(beforeParams(arg1)))
                            executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                        arg1Result = __LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (arg1Result == arg3Result)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (arg1Result != arg3Result)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:object_undefined:" + arg3before, false);
                        failedIfStatement();
                    }
                }
                else if (zeroDots(arg1) && zeroDots(arg3))
                {
                    string arg1Result(""), arg3Result("");

                    if (methodExists(beforeParams(arg1)))
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                    arg1Result = __LastValue;

                    if (methodExists(beforeParams(arg3)))
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                    arg3Result = __LastValue;

                    if (isNumeric(arg1Result) && isNumeric(arg3Result))
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (stod(arg1Result) == stod(arg3Result))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (stod(arg1Result) != stod(arg3Result))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == "<")
                        {
                            if (stod(arg1Result) < stod(arg3Result))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == ">")
                        {
                            if (stod(arg1Result) > stod(arg3Result))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == "<=")
                        {
                            if (stod(arg1Result) <= stod(arg3Result))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == ">=")
                        {
                            if (stod(arg1Result) >= stod(arg3Result))
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                    else
                    {
                        if (arg2 == "==" || arg2 == "is")
                        {
                            if (arg1Result == arg3Result)
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else if (arg2 == "!=" || arg2 == "not")
                        {
                            if (arg1Result != arg3Result)
                                successfulIF();
                            else
                                failedIfStatement();
                        }
                        else
                        {
                            error("invalid_operator:" + arg2, false);
                            failedIfStatement();
                        }
                    }
                }
                else
                {
                    error("special_error(9)", false);
                    failedIfStatement();
                }
            }
            else if (containsParams(arg1) && !containsParams(arg3))
            {
                string arg1Result(""), arg3Result("");

                bool pass = true;

                if (zeroDots(arg1))
                {
                    if (methodExists(beforeParams(arg1)))
                    {
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                        arg1Result = __LastValue;

                        if (methodExists(arg3))
                        {
                            parse(arg3);
                            arg3Result = __LastValue;
                        }
                        else if (variableExists(arg3))
                        {
                            if (isString(arg3))
                                arg3Result = variables.at(indexOfVariable(arg3)).getString();
                            else if (isNumber(arg3))
                                arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                            else
                            {
                                pass = false;
                                error("is_null:" + arg3, false);
                                failedIfStatement();
                            }
                        }
                        else
                            arg3Result = arg3;

                        if (pass)
                        {
                            if (isNumeric(arg1Result) && isNumeric(arg3Result))
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (stod(arg1Result) == stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    failedIfStatement();
                                }
                            }
                            else
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (arg1Result == arg3Result)
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (arg1Result != arg3Result)
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    failedIfStatement();
                                }
                            }
                        }
                        else
                            failedIfStatement();
                    }
                    else if (stackReady(arg1))
                    {
                        string stackValue("");

                        if (isStringStack(arg1))
                            stackValue = getStringStack(arg1);
                        else
                            stackValue = dtos(getStack(arg1));

                        string comp("");

                        if (variableExists(arg3))
                        {
                            if (isString(arg3))
                                comp = variables.at(indexOfVariable(arg3)).getString();
                            else if (isNumber(arg3))
                                comp = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                        }
                        else if (methodExists(arg3))
                        {
                            parse(arg3);

                            comp = __LastValue;
                        }
                        else if (containsParams(arg3))
                        {
                            executeTemplate(getMethod(beforeParams(arg3)), getParams(arg3));

                            comp = __LastValue;
                        }
                        else
                            comp = arg3;

                        if (isNumeric(stackValue) && isNumeric(comp))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(stackValue) == stod(comp))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(stackValue) != stod(comp))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(stackValue) < stod(comp))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(stackValue) > stod(comp))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(stackValue) <= stod(comp))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(stackValue) >= stod(comp))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stackValue == comp)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stackValue != comp)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:method_undefined:" + beforeParams(arg1), false);
                        failedIfStatement();
                    }
                }
                else
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    if (objectExists(arg1before))
                    {
                        if (objects.at(indexOfObject(arg1before)).methodExists(beforeParams(arg1after)))
                            executeTemplate(objects.at(indexOfObject(arg1before)).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = __LastValue;

                        if (variableExists(arg3))
                        {
                            if (isString(arg3))
                                arg3Result = variables.at(indexOfVariable(arg3)).getString();
                            else if (isNumber(arg3))
                                arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                            else
                            {
                                pass = false;
                                error("is_null:" + arg3, false);
                                failedIfStatement();
                            }
                        }
                        else if (methodExists(arg3))
                        {
                            parse(arg3);

                            arg3Result = __LastValue;
                        }
                        else
                            arg3Result = arg3;

                        if (pass)
                        {
                            if (isNumeric(arg1Result) && isNumeric(arg3Result))
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (stod(arg1Result) == stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    failedIfStatement();
                                }
                            }
                            else
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (arg1Result == arg3Result)
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (arg1Result != arg3Result)
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    failedIfStatement();
                                }
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:object_undefined:" + arg1before, false);
                        failedIfStatement();
                    }
                }
            }
            else if (!containsParams(arg1) && containsParams(arg3))
            {
                string arg1Result(""), arg3Result("");

                bool pass = true;

                if (zeroDots(arg3))
                {
                    if (methodExists(beforeParams(arg3)))
                    {
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                        arg3Result = __LastValue;

                        if (methodExists(arg1))
                        {
                            parse(arg1);
                            arg1Result = __LastValue;
                        }
                        else if (variableExists(arg1))
                        {
                            if (isString(arg1))
                                arg1Result = variables.at(indexOfVariable(arg1)).getString();
                            else if (isNumber(arg1))
                                arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                            else
                            {
                                pass = false;
                                error("is_null:" + arg1, false);
                                failedIfStatement();
                            }
                        }
                        else
                            arg1Result = arg1;

                        if (pass)
                        {
                            if (isNumeric(arg3Result) && isNumeric(arg1Result))
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (stod(arg3Result) == stod(arg1Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (stod(arg3Result) != stod(arg1Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg3Result) < stod(arg1Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg3Result) > stod(arg1Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg3Result) <= stod(arg1Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg3Result) >= stod(arg1Result))
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    failedIfStatement();
                                }
                            }
                            else
                            {
                                if (arg2 == "==" || arg2 == "is")
                                {
                                    if (arg3Result == arg1Result)
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else if (arg2 == "!=" || arg2 == "not")
                                {
                                    if (arg3Result != arg1Result)
                                        successfulIF();
                                    else
                                        failedIfStatement();
                                }
                                else
                                {
                                    error("invalid_operator:" + arg2, false);
                                    failedIfStatement();
                                }
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:method_undefined:" + beforeParams(arg3), false);
                        failedIfStatement();
                    }
                }
                else
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    if (objectExists(arg3before))
                    {
                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = __LastValue;

                        if (variableExists(arg1))
                        {
                            if (isString(arg1))
                                arg1Result = variables.at(indexOfVariable(arg1)).getString();
                            else if (isNumber(arg3))
                                arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                            else
                            {
                                error("is_null:" + arg1, false);
                                failedIfStatement();
                            }
                        }
                        else if (methodExists(arg1))
                        {
                            parse(arg1);

                            arg1Result = __LastValue;
                        }
                        else
                            arg1Result = arg1;

                        if (isNumeric(arg3Result) && isNumeric(arg1Result))
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (stod(arg3Result) == stod(arg1Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (stod(arg3Result) != stod(arg1Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg3Result) < stod(arg1Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg3Result) > stod(arg1Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg3Result) <= stod(arg1Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg3Result) >= stod(arg1Result))
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                        else
                        {
                            if (arg2 == "==" || arg2 == "is")
                            {
                                if (arg3Result == arg1Result)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else if (arg2 == "!=" || arg2 == "not")
                            {
                                if (arg3Result != arg1Result)
                                    successfulIF();
                                else
                                    failedIfStatement();
                            }
                            else
                            {
                                error("invalid_operator:" + arg2, false);
                                failedIfStatement();
                            }
                        }
                    }
                    else
                    {
                        error("invalid_operation:object_undefined:" + arg3before, false);
                        failedIfStatement();
                    }
                }
            }
        }
        else if ((methodExists(arg1) && arg3 != "method?") || methodExists(arg3))
        {
            string arg1Result(""), arg3Result("");

            if (methodExists(arg1))
            {
                parse(arg1);
                arg1Result = __LastValue;
            }
            else if (variableExists(arg1))
            {
                if (isString(arg1))
                    arg1Result = variables.at(indexOfVariable(arg1)).getString();
                else if (isNumber(arg1))
                    arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                else
                {
                    error("is_null:" + arg1, false);
                    failedIfStatement();
                }
            }
            else
                arg1Result = arg1;

            if (methodExists(arg3))
            {
                parse(arg3);
                arg3Result = __LastValue;
            }
            else if (variableExists(arg3))
            {
                if (isString(arg3))
                    arg3Result = variables.at(indexOfVariable(arg3)).getString();
                else if (isNumber(arg3))
                    arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                else
                {
                    error("is_null:" + arg3, false);
                    failedIfStatement();
                }
            }
            else
                arg3Result = arg3;

            if (isNumeric(arg1Result) && isNumeric(arg3Result))
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (stod(arg1Result) == stod(arg3Result))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (stod(arg1Result) != stod(arg3Result))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<")
                {
                    if (stod(arg1Result) < stod(arg3Result))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">")
                {
                    if (stod(arg1Result) > stod(arg3Result))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "<=")
                {
                    if (stod(arg1Result) <= stod(arg3Result))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == ">=")
                {
                    if (stod(arg1Result) >= stod(arg3Result))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    failedIfStatement();
                }
            }
            else
            {
                if (arg2 == "==" || arg2 == "is")
                {
                    if (arg1Result == arg3Result)
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else if (arg2 == "!=" || arg2 == "not")
                {
                    if (arg1Result != arg3Result)
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    error("invalid_operator:" + arg2, false);
                    failedIfStatement();
                }
            }
        }
        else
        {
            if (arg3 == "object?")
            {
                if (objectExists(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
            }
            else if (arg3 == "var?" || arg3 == "variable?")
            {
                if (variableExists(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
            }
            else if (arg3 == "method?")
            {
                if (methodExists(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
            }
            else if (arg3 == "list?")
            {
                if (listExists(arg1))
                {
                    if (arg2 == "==" || arg2 == "is")
                        successfulIF();
                    else if (arg2 == "!=" || arg2 == "not")
                        failedIfStatement();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
                else
                {
                    if (arg2 == "==" || arg2 == "is")
                        failedIfStatement();
                    else if (arg2 == "!=" || arg2 == "not")
                        successfulIF();
                    else
                    {
                        error("invalid_operator:" + arg2, false);
                        failedIfStatement();
                    }
                }
            }
            else if (arg2 == "==" || arg2 == "is")
            {
                if (arg1 == arg3)
                    successfulIF();
                else
                    failedIfStatement();
            }
            else if (arg2 == "!=" || arg2 == "not")
            {
                if (arg1 != arg3)
                    successfulIF();
                else
                    failedIfStatement();
            }
            else if (arg2 == ">")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) > stod(arg3))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    if (arg1.length() > arg3.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
            }
            else if (arg2 == "<")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) < stod(arg3))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    if (arg1.length() < arg3.length())
                        successfulIF();
                    else
                        failedIfStatement();
                }
            }
            else if (arg2 == ">=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) >= stod(arg3))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    error("invalid_operator:>=", false);
                    failedIfStatement();
                }
            }
            else if (arg2 == "<=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) <= stod(arg3))
                        successfulIF();
                    else
                        failedIfStatement();
                }
                else
                {
                    error("invalid_operator:<=", false);
                    failedIfStatement();
                }
            }
            else if (arg2 == "begins_with")
            {
                if (startsWith(arg1, arg3))
                    successfulIF();
                else
                    failedIfStatement();
            }
            else if (arg2 == "ends_with")
            {
                if (endsWith(arg1, arg3))
                    successfulIF();
                else
                    failedIfStatement();
            }
            else if (arg2 == "contains")
            {
                if (contains(arg1, arg3))
                    successfulIF();
                else
                    failedIfStatement();
            }
            else
            {
                error("invalid_operator:" + arg2, false);
                failedIfStatement();
            }
        }
    }
    else if (arg0 == "for")
    {
        if (arg2 == "<")
        {
            if (variableExists(arg1) && variableExists(arg3))
            {
                if (isNumber(arg1) && isNumber(arg3))
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() < variables.at(indexOfVariable(arg3)).getNumber())
                        successfulFor(variables.at(indexOfVariable(arg1)).getNumber(), variables.at(indexOfVariable(arg3)).getNumber(), "<");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (variableExists(arg1) && !variableExists(arg3))
            {
                if (isNumber(arg1) && isNumeric(arg3))
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() < stod(arg3))
                        successfulFor(variables.at(indexOfVariable(arg1)).getNumber(), stod(arg3), "<");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (!variableExists(arg1) && variableExists(arg3))
            {
                if (isNumeric(arg1) && isNumber(arg3))
                {
                    if (stod(arg1) < variables.at(indexOfVariable(arg3)).getNumber())
                        successfulFor(stod(arg1), variables.at(indexOfVariable(arg3)).getNumber(), "<");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (!variableExists(arg1) && !variableExists(arg3))
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) < stod(arg3))
                        successfulFor(stod(arg1), stod(arg3), "<");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else
            {
                error("special_error(2)", false); // impossible operation
                failedFor();
            }
        }
        else if (arg2 == ">")
        {
            if (variableExists(arg1) && variableExists(arg3))
            {
                if (isNumber(arg1) && isNumber(arg3))
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() > variables.at(indexOfVariable(arg3)).getNumber())
                        successfulFor(variables.at(indexOfVariable(arg1)).getNumber(), variables.at(indexOfVariable(arg3)).getNumber(), ">");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (variableExists(arg1) && !variableExists(arg3))
            {
                if (isNumber(arg1) && isNumeric(arg3))
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() > stod(arg3))
                        successfulFor(variables.at(indexOfVariable(arg1)).getNumber(), stod(arg3), ">");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (!variableExists(arg1) && variableExists(arg3))
            {
                if (isNumeric(arg1) && isNumber(arg3))
                {
                    if (stod(arg1) > variables.at(indexOfVariable(arg3)).getNumber())
                        successfulFor(stod(arg1), variables.at(indexOfVariable(arg3)).getNumber(), ">");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (!variableExists(arg1) && !variableExists(arg3))
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) > stod(arg3))
                        successfulFor(stod(arg1), stod(arg3), ">");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else
            {
                error("special_error(3)", false); // impossible operation
                failedFor();
            }
        }
        else if (arg2 == "<=")
        {
            if (variableExists(arg1) && variableExists(arg3))
            {
                if (isNumber(arg1) && isNumber(arg3))
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() <= variables.at(indexOfVariable(arg3)).getNumber())
                        successfulFor(variables.at(indexOfVariable(arg1)).getNumber(), variables.at(indexOfVariable(arg3)).getNumber(), "<=");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (variableExists(arg1) && !variableExists(arg3))
            {
                if (isNumber(arg1) && isNumeric(arg3))
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(arg3))
                        successfulFor(variables.at(indexOfVariable(arg1)).getNumber(), stod(arg3), "<=");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (!variableExists(arg1) && variableExists(arg3))
            {
                if (isNumeric(arg1) && isNumber(arg3))
                {
                    if (stod(arg1) <= variables.at(indexOfVariable(arg3)).getNumber())
                        successfulFor(stod(arg1), variables.at(indexOfVariable(arg3)).getNumber(), "<=");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (!variableExists(arg1) && !variableExists(arg3))
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) <= stod(arg3))
                        successfulFor(stod(arg1), stod(arg3), "<=");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else
            {
                error("special_error(2)", false); // impossible operation
                failedFor();
            }
        }
        else if (arg2 == ">=")
        {
            if (variableExists(arg1) && variableExists(arg3))
            {
                if (isNumber(arg1) && isNumber(arg3))
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() >= variables.at(indexOfVariable(arg3)).getNumber())
                        successfulFor(variables.at(indexOfVariable(arg1)).getNumber(), variables.at(indexOfVariable(arg3)).getNumber(), ">=");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (variableExists(arg1) && !variableExists(arg3))
            {
                if (isNumber(arg1) && isNumeric(arg3))
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(arg3))
                        successfulFor(variables.at(indexOfVariable(arg1)).getNumber(), stod(arg3), ">=");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (!variableExists(arg1) && variableExists(arg3))
            {
                if (isNumeric(arg1) && isNumber(arg3))
                {
                    if (stod(arg1) >= variables.at(indexOfVariable(arg3)).getNumber())
                        successfulFor(stod(arg1), variables.at(indexOfVariable(arg3)).getNumber(), ">=");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else if (!variableExists(arg1) && !variableExists(arg3))
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) >= stod(arg3))
                        successfulFor(stod(arg1), stod(arg3), ">=");
                    else
                        failedFor();
                }
                else
                {
                    error("conversion_error:" + s, false);
                    failedFor();
                }
            }
            else
            {
                error("special_error(3)", false); // impossible operation
                failedFor();
            }
        }
        else if (arg2 == "in")
        {
            if (arg1 == "var")
            {
                string before(beforeDot(arg3)), after(afterDot(arg3));

                if (before == "args" && after == "size")
                {
                    List newList;

                    for (int i = 0; i < (int)args.size(); i++)
                        newList.add(args.at(i));

                    successfulFor(newList);
                }
                else if (objectExists(before) && (after == "get_methods" || after == "methods"))
                {
                    List newList;

                    vector<Method> objMethods = objects.at(indexOfObject(before)).getMethods();

                    for (int i = 0; i < (int)objMethods.size(); i++)
                        newList.add(objMethods.at(i).name());

                    successfulFor(newList);
                }
                else if (objectExists(before) && (after == "get_variables" || after == "variables"))
                {
                    List newList;

                    vector<Variable> objVars = objects.at(indexOfObject(before)).getVariables();

                    for (int i = 0; i < (int)objVars.size(); i++)
                        newList.add(objVars.at(i).name());

                    successfulFor(newList);
                }
                else if (variableExists(before) && (after == "size" || after == "length"))
                {
                    if (isString(before))
                    {
                        List newList;
                        string tempVarStr = variables.at(indexOfVariable(before)).getString();
                        int len = tempVarStr.length();

                        for (int i = 0; i < len; i++)
                        {
                            string tempStr("");
                            tempStr.push_back(tempVarStr[i]);
                            newList.add(tempStr);
                        }

                        successfulFor(newList);
                    }
                }
                else
                {
                    if (before.length() != 0 && after.length() != 0)
                    {
                        if (variableExists(before))
                        {
                            if (after == "read_dirs" || after == "directories")
                            {
                                if (directoryExists(variables.at(indexOfVariable(before)).getString()))
                                    successfulFor(getDirectoryList(before, false));
                                else
                                {
                                    error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                    failedFor();
                                }
                            }
                            else if (after == "read_files" || after == "files")
                            {
                                if (directoryExists(variables.at(indexOfVariable(before)).getString()))
                                    successfulFor(getDirectoryList(before, true));
                                else
                                {
                                    error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                    failedFor();
                                }
                            }
                            else if (after == "read")
                            {
                                if (fileExists(variables.at(indexOfVariable(before)).getString()))
                                {
                                    List newList;

                                    ifstream file(variables.at(indexOfVariable(before)).getString().c_str());
                                    string line("");

                                    if (file.is_open())
                                    {
                                        while (!file.eof())
                                        {
                                            getline(file, line);
                                            newList.add(line);
                                        }

                                        file.close();

                                        successfulFor(newList);
                                    }
                                    else
                                    {
                                        error("read_fail:" + variables.at(indexOfVariable(before)).getString(), false);
                                        failedFor();
                                    }
                                }
                            }
                            else
                            {
                                error("invalid_operation:method_undefined:" + after, false);
                                failedFor();
                            }
                        }
                        else
                        {
                            error("invalid_operation:variable_undefined:" + before, false);
                            failedFor();
                        }
                    }
                    else
                    {
                        if (listExists(arg3))
                            successfulFor(lists.at(indexOfList(arg3)));
                        else
                        {
                            error("invalid_operation:list_undefined:" + arg3, false);
                            failedFor();
                        }
                    }
                }
            }
            else if (containsParams(arg3))
            {
                vector<string> rangeSpecifiers;

                rangeSpecifiers = getRange(arg3);

                if (rangeSpecifiers.size() == 2)
                {
                    string firstRangeSpecifier(rangeSpecifiers.at(0)), lastRangeSpecifier(rangeSpecifiers.at(1));

                    if (variableExists(firstRangeSpecifier))
                    {
                        if (isNumber(firstRangeSpecifier))
                            firstRangeSpecifier = dtos(variables.at(indexOfVariable(firstRangeSpecifier)).getNumber());
                        else
                            failedFor();
                    }

                    if (variableExists(lastRangeSpecifier))
                    {
                        if (isNumber(lastRangeSpecifier))
                            lastRangeSpecifier = dtos(variables.at(indexOfVariable(lastRangeSpecifier)).getNumber());
                        else
                            failedFor();
                    }

                    if (isNumeric(firstRangeSpecifier) && isNumeric(lastRangeSpecifier))
                    {
                        __DefaultLoopSymbol = arg1;

                        int ifrs = stoi(firstRangeSpecifier), ilrs(stoi(lastRangeSpecifier));

                        if (ifrs < ilrs)
                            successfulFor(stod(firstRangeSpecifier), stod(lastRangeSpecifier), "<=");
                        else if (ifrs > ilrs)
                            successfulFor(stod(firstRangeSpecifier), stod(lastRangeSpecifier), ">=");
                        else
                            failedFor();
                    }
                    else
                        failedFor();
                }
            }
            else if (containsBrackets(arg3))
            {
                string before(beforeBrackets(arg3));

                if (variableExists(before))
                {
                    if (isString(before))
                    {
                        string tempVarString(variables.at(indexOfVariable(before)).getString());

                        vector<string> range = getBracketRange(arg3);

                        if (range.size() == 2)
                        {
                            string rangeBegin(range.at(0)), rangeEnd(range.at(1));

                            if (rangeBegin.length() != 0 && rangeEnd.length() != 0)
                            {
                                if (isNumeric(rangeBegin) && isNumeric(rangeEnd))
                                {
                                    if (stoi(rangeBegin) < stoi(rangeEnd))
                                    {
                                        if ((int)tempVarString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                                        {
                                            List newList("&l&i&s&t&");

                                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                            {
                                                string tempString("");
                                                tempString.push_back(tempVarString[i]);
                                                newList.add(tempString);
                                            }

                                            __DefaultLoopSymbol = arg1;

                                            successfulFor(newList);

                                            lists = removeList(lists, "&l&i&s&t&");
                                        }
                                        else
                                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                    }
                                    else if (stoi(rangeBegin) > stoi(rangeEnd))
                                    {
                                        if ((int)tempVarString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
                                        {
                                            List newList("&l&i&s&t&");

                                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                            {
                                                string tempString("");
                                                tempString.push_back(tempVarString[i]);
                                                newList.add(tempString);
                                            }

                                            __DefaultLoopSymbol = arg1;

                                            successfulFor(newList);

                                            lists = removeList(lists, "&l&i&s&t&");
                                        }
                                        else
                                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                    }
                                    else
                                        error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                                }
                                else
                                    error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                            }
                            else
                                error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                        }
                        else
                            error("invalid_operation:invalid_range:" + arg3, false);
                    }
                    else
                    {
                        error("invalid_operation:__Null_string:" + before, false);
                        failedFor();
                    }
                }
            }
            else if (listExists(arg3))
            {
                __DefaultLoopSymbol = arg1;

                successfulFor(lists.at(indexOfList(arg3)));
            }
            else if (!zeroDots(arg3))
            {
                string _b(beforeDot(arg3)), _a(afterDot(arg3));

                if (_b == "args" && _a == "size")
                {
                    List newList;

                    __DefaultLoopSymbol = arg1;

                    for (int i = 0; i < (int)args.size(); i++)
                        newList.add(args.at(i));

                    successfulFor(newList);
                }
                else if (_b == "env" && (_a == "get_members" || _a == "members"))
                {
                    List newList;

                    newList.add("cwd");
                    newList.add("usl");
                    newList.add("os?");
                    newList.add("user");
                    newList.add("machine");
                    newList.add("init_dir");
                    newList.add("initial_directory");
                    newList.add("am_or_pm");
                    newList.add("now");
                    newList.add("day_of_this_week");
                    newList.add("day_of_this_month");
                    newList.add("day_of_this_year");
                    newList.add("month_of_this_year");
                    newList.add("this_second");
                    newList.add("this_minute");
                    newList.add("this_hour");
                    newList.add("this_month");
                    newList.add("this_year");
                    newList.add("empty_string");
                    newList.add("empty_number");
                    newList.add("last_error");
                    newList.add("last_value");
                    newList.add("get_members");
                    newList.add("members");

                    __DefaultLoopSymbol = arg1;
                    successfulFor(newList);
                }
                else if (objectExists(_b) && (_a == "get_methods" || _a == "methods"))
                {
                    List newList;

                    vector<Method> objMethods = objects.at(indexOfObject(_b)).getMethods();

                    for (int i = 0; i < (int)objMethods.size(); i++)
                        newList.add(objMethods.at(i).name());

                    __DefaultLoopSymbol = arg1;
                    successfulFor(newList);
                }
                else if (objectExists(_b) && (_a == "get_variables" || _a == "variables"))
                {
                    List newList;

                    vector<Variable> objVars = objects.at(indexOfObject(_b)).getVariables();

                    for (int i = 0; i < (int)objVars.size(); i++)
                        newList.add(objVars.at(i).name());

                    __DefaultLoopSymbol = arg1;
                    successfulFor(newList);
                }
                else if (variableExists(_b) && (_a == "size" || _a == "length"))
                {
                    if (isString(_b))
                    {
                        __DefaultLoopSymbol = arg1;
                        List newList;
                        string _t = variables.at(indexOfVariable(_b)).getString();
                        int _l = _t.length();

                        for (int i = 0; i < _l; i++)
                        {
                            string tmpStr("");
                            tmpStr.push_back(_t[i]);
                            newList.add(tmpStr);
                        }

                        successfulFor(newList);
                    }
                }
                else
                {
                    if (_b.length() != 0 && _a.length() != 0)
                    {
                        if (variableExists(_b))
                        {
                            if (_a == "read_dirs" || _a == "directories")
                            {
                                if (directoryExists(variables.at(indexOfVariable(_b)).getString()))
                                {
                                    __DefaultLoopSymbol = arg1;
                                    successfulFor(getDirectoryList(_b, false));
                                }
                                else
                                {
                                    error("read_fail:" + variables.at(indexOfVariable(_b)).getString(), false);
                                    failedFor();
                                }
                            }
                            else if (_a == "read_files" || _a == "files")
                            {
                                if (directoryExists(variables.at(indexOfVariable(_b)).getString()))
                                {
                                    __DefaultLoopSymbol = arg1;
                                    successfulFor(getDirectoryList(_b, true));
                                }
                                else
                                {
                                    error("read_fail:" + variables.at(indexOfVariable(_b)).getString(), false);
                                    failedFor();
                                }
                            }
                            else if (_a == "read")
                            {
                                if (fileExists(variables.at(indexOfVariable(_b)).getString()))
                                {
                                    List newList;

                                    ifstream file(variables.at(indexOfVariable(_b)).getString().c_str());
                                    string line("");

                                    if (file.is_open())
                                    {
                                        while (!file.eof())
                                        {
                                            getline(file, line);
                                            newList.add(line);
                                        }

                                        file.close();

                                        __DefaultLoopSymbol = arg1;
                                        successfulFor(newList);
                                    }
                                    else
                                    {
                                        error("read_fail:" + variables.at(indexOfVariable(_b)).getString(), false);
                                        failedFor();
                                    }
                                }
                            }
                            else
                            {
                                error("invalid_operation:method_undefined:" + _a, false);
                                failedFor();
                            }
                        }
                        else
                        {
                            error("invalid_operation:variable_undefined:" + _b, false);
                            failedFor();
                        }
                    }
                }
            }
            else
            {
                error("invalid_operation:" + s, false);
                failedFor();
            }
        }
        else
        {
            error("invalid_operation:" + s, false);
            failedFor();
        }
    }
    else if (arg0 == "while")
    {
        if (variableExists(arg1) && variableExists(arg3))
        {
            if (isNumber(arg1) && isNumber(arg3))
            {
                if (arg2 == "<" || arg2 == "<=" || arg2 == ">=" || arg2 == ">" || arg2 == "==" || arg2 == "!=")
                    successfullWhile(arg1, arg2, arg3);
                else
                {
                    error("invalid_operation:" + s, false);
                    failedWhile();
                }
            }
            else
            {
                error("conversion_error:__Null_number:" + arg1 + arg2 + arg3, false);
                failedWhile();
            }
        }
        else if (isNumeric(arg3) && variableExists(arg1))
        {
            if (isNumber(arg1))
            {
                if (arg2 == "<" || arg2 == "<=" || arg2 == ">=" || arg2 == ">" || arg2 == "==" || arg2 == "!=")
                    successfullWhile(arg1, arg2, arg3);
                else
                {
                    error("invalid_operation:" + s, false);
                    failedWhile();
                }
            }
            else
            {
                error("conversion_error:__Null_number:" + arg1 + arg2 + arg3, false);
                failedWhile();
            }
        }
        else if (isNumeric(arg1) && isNumeric(arg3))
        {
            if (arg2 == "<" || arg2 == "<=" || arg2 == ">=" || arg2 == ">" || arg2 == "==" || arg2 == "!=")
                successfullWhile(arg1, arg2, arg3);
            else
            {
                error("invalid_operation:" + s, false);
                failedWhile();
            }
        }
        else
        {
            error("invalid_operation:" + s, false);
            failedWhile();
        }
    }
    else
        sysExec(s, command);
}
// AAAA
void InternalEncryptDecrypt(string arg0, string arg1)
{
    Crypt c;
    string text = variableExists(arg1) ? (isString(arg1) ? getVariable(arg1).getString() : dtos(getVariable(arg1).getNumber())) : arg1;
    __stdout(arg0 == "encrypt" ? c.e(text) : c.d(text));
}

void InternalInspect(string arg0, string arg1, string before, string after)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (objectExists(before))
        {
            if (objects.at(indexOfObject(before)).methodExists(after))
            {
                for (int i = 0; i < objects.at(indexOfObject(before)).getMethod(after).size(); i++)
                    __stdout(objects.at(indexOfObject(before)).getMethod(after).at(i));
            }
            else if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                    __stdout(objects.at(indexOfObject(before)).getVariable(after).getString());
                else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                    __stdout(dtos(objects.at(indexOfObject(before)).getVariable(after).getNumber()));
                else
                    __stdout(__Null);
            }
            else
                error("invalid_operation:target_undefined:" + arg1, false);
        }
        else
            error("invalid_operation:object_undefined:" + before, false);
    }
    else
    {
        if (objectExists(arg1))
        {
            for (int i = 0; i < objects.at(indexOfObject(arg1)).methodSize(); i++)
                __stdout(objects.at(indexOfObject(arg1)).getMethod(objects.at(indexOfObject(arg1)).getMethodName(i)).name());
            for (int i = 0; i < objects.at(indexOfObject(arg1)).variableSize(); i++)
                __stdout(objects.at(indexOfObject(arg1)).getVariable(objects.at(indexOfObject(arg1)).getVariableName(i)).name());
        }
        else if (constantExists(arg1))
        {
            if (constants.at(indexOfConstant(arg1)).ConstNumber())
                __stdout(dtos(constants.at(indexOfConstant(arg1)).getNumber()));
            else if (constants.at(indexOfConstant(arg1)).ConstString())
                __stdout(constants.at(indexOfConstant(arg1)).getString());
        }
        else if (methodExists(arg1))
        {
            for (int i = 0; i < methods.at(indexOfMethod(arg1)).size(); i++)
                __stdout(methods.at(indexOfMethod(arg1)).at(i));
        }
        else if (variableExists(arg1))
        {
            if (isString(arg1))
                __stdout(variables.at(indexOfVariable(arg1)).getString());
            else if (isNumber(arg1))
                __stdout(dtos(variables.at(indexOfVariable(arg1)).getNumber()));
        }
        else if (listExists(arg1))
        {
            for (int i = 0; i < lists.at(indexOfList(arg1)).size(); i++)
                __stdout(lists.at(indexOfList(arg1)).at(i));
        }
        else if (arg1 == "variables")
        {
            for (int i = 0; i < (int)variables.size(); i++)
            {
                if (variables.at(i).getString() != __Null)
                    __stdout(variables.at(i).name() + ":\t" + variables.at(i).getString());
                else if (variables.at(i).getNumber() != __NullNum)
                    __stdout(variables.at(i).name() + ":\t" + dtos(variables.at(i).getNumber()));
                else
                    __stdout(variables.at(i).name() + ":\tis_null");
            }
        }
        else if (arg1 == "lists")
        {
            for (int i = 0; i < (int)lists.size(); i++)
                __stdout(lists.at(i).name());
        }
        else if (arg1 == "methods")
        {
            for (int i = 0; i < (int)methods.size(); i++)
                __stdout(methods.at(i).name());
        }
        else if (arg1 == "objects")
        {
            for (int i = 0; i < (int)objects.size(); i++)
                __stdout(objects.at(i).name());
        }
        else if (arg1 == "constants")
        {
            for (int i = 0; i < (int)constants.size(); i++)
                __stdout(constants.at(i).name());
        }
        else if (arg1 == "os?")
            __stdout(__GuessedOS);
        else if (arg1 == "last")
            __stdout(__LastValue);
        else
            error("invalid_operation:target_undefined:" + arg1, false);
    }
}

void InternalGlobalize(string arg0, string arg1)
{
    if (contains(arg1, ".") && methodExists(arg1) && !methodExists(afterDot(arg1)))
    {
        Method method(afterDot(arg1));

        vector<string> lines = getObject(beforeDot(arg1)).getMethod(afterDot(arg1)).getLines();

        for (int i = 0; i < (int)lines.size(); i++)
            method.add(lines[i]);

        methods.push_back(method);
    }
    else
        error("invalid_operation:object_method_undefined:" + arg1, false);
}

void InternalCallMethod(string arg0, string arg1, string before, string after)
{
    if (__DefiningObject)
    {
        if (objects.at(indexOfObject(__CurrentObject)).methodExists(arg1))
            executeMethod(objects.at(indexOfObject(__CurrentObject)).getMethod(arg1));
        else
            error("invalid_operation:method_undefined:" + arg1, false);
    }
    else
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objectExists(before))
            {
                if (objects.at(indexOfObject(before)).methodExists(after))
                    executeMethod(objects.at(indexOfObject(before)).getMethod(after));
                else
                    error("invalid_operation:method_undefined:" + arg1, false);
            }
            else
                error("invalid_operation:object_undefined:" + before, true);
        }
        else
        {
            if (methodExists(arg1))
                executeMethod(methods.at(indexOfMethod(arg1)));
            else
                error("invalid_operation:method_undefined:" + arg1, true);
        }
    }
}

void InternalCreateMethod(string arg0, string arg1)
{
    bool indestructable = false;

    if (arg0 == "[method]")
        indestructable = true;

    if (__DefiningObject)
    {
        if (objects.at(indexOfObject(__CurrentObject)).methodExists(arg1))
            error("invalid_operation:method_defined:" + arg1, false);
        else
        {
            if (containsParams(arg1))
            {
                vector<string> params = getParams(arg1);

                Method method(beforeParams(arg1));

                if (__DefiningPublicCode)
                    method.setPublic();
                else if (__DefiningPrivateCode)
                    method.setPrivate();

                method.setObject(__CurrentObject);

                for (int i = 0; i < (int)params.size(); i++)
                {
                    if (variableExists(params.at(i)))
                    {
                        if (!zeroDots(params.at(i)))
                        {
                            string before(beforeDot(params.at(i))), after(afterDot(params.at(i)));

                            if (objectExists(before))
                            {
                                if (objects.at(indexOfObject(before)).variableExists(after))
                                {
                                    if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                                        method.addMethodVariable(objects.at(indexOfObject(before)).getVariable(after).getString(), after);
                                    else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                                        method.addMethodVariable(objects.at(indexOfObject(before)).getVariable(after).getNumber(), after);
                                    else
                                        error("is_null:" + params.at(i), false);
                                }
                                else
                                    error("invalid_operation:object_variable_undefined:" + after, false);
                            }
                            else
                                error("invalid_operation:object_undefined:" + before, false);
                        }
                        else
                        {
                            if (isString(params.at(i)))
                                method.addMethodVariable(variables.at(indexOfVariable(params.at(i))).getString(), variables.at(indexOfVariable(params.at(i))).name());
                            else if (isNumber(params.at(i)))
                                method.addMethodVariable(variables.at(indexOfVariable(params.at(i))).getNumber(), variables.at(indexOfVariable(params.at(i))).name());
                            else
                                error("is_null:" + params.at(i), false);
                        }
                    }
                    else
                    {
                        if (isAlpha(params.at(i)))
                        {
                            Variable newVariable("@[pm#" + itos(__ParamVarCount) + "]", params.at(i));
                            method.addMethodVariable(newVariable);
                            __ParamVarCount++;
                        }
                        else
                        {
                            Variable newVariable("@[pm#" + itos(__ParamVarCount) + "]", stod(params.at(i)));
                            method.addMethodVariable(newVariable);
                            __ParamVarCount++;
                        }
                    }
                }

                objects.at(indexOfObject(__CurrentObject)).addMethod(method);
                objects.at(indexOfObject(__CurrentObject)).setCurrentMethod(beforeParams(arg1));
                __DefiningMethod = true;
                __DefiningParameterizedMethod = true;
                __DefiningObjectMethod = true;
            }
            else
            {
                Method method(arg1);

                if (__DefiningPublicCode)
                    method.setPublic();
                else if (__DefiningPrivateCode)
                    method.setPrivate();

                method.setObject(__CurrentObject);
                objects.at(indexOfObject(__CurrentObject)).addMethod(method);
                objects.at(indexOfObject(__CurrentObject)).setCurrentMethod(arg1);
                __DefiningMethod = true;
                __DefiningObjectMethod = true;
            }
        }
    }
    else
    {
        if (methodExists(arg1))
            error("invalid_operation:method_defined:" + arg1, false);
        else
        {
            if (!zeroDots(arg1))
            {
                string before(beforeDot(arg1)), after(afterDot(arg1));

                if (objectExists(before))
                {
                    Method method(after);

                    if (__DefiningPublicCode)
                        method.setPublic();
                    else if (__DefiningPrivateCode)
                        method.setPrivate();

                    method.setObject(before);
                    objects.at(indexOfObject(before)).addMethod(method);
                    objects.at(indexOfObject(before)).setCurrentMethod(after);
                    __DefiningMethod = true;
                    __DefiningObjectMethod = true;
                }
                else
                    error("invalid_operation:object_undefined", false);
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
                                if (objects.at(indexOfObject(before)).variableExists(after))
                                {
                                    if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                                        method.addMethodVariable(objects.at(indexOfObject(before)).getVariable(after).getString(), after);
                                    else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                                        method.addMethodVariable(objects.at(indexOfObject(before)).getVariable(after).getNumber(), after);
                                    else
                                        error("is_null:" + params.at(i), false);
                                }
                                else
                                    error("invalid_operation:object_variable_undefined:" + after, false);
                            }
                            else
                                error("invalid_operation:object_undefined:" + before, false);
                        }
                        else
                        {
                            if (isString(params.at(i)))
                                method.addMethodVariable(variables.at(indexOfVariable(params.at(i))).getString(), variables.at(indexOfVariable(params.at(i))).name());
                            else if (isNumber(params.at(i)))
                                method.addMethodVariable(variables.at(indexOfVariable(params.at(i))).getNumber(), variables.at(indexOfVariable(params.at(i))).name());
                            else
                                error("is_null:" + params.at(i), false);
                        }
                    }
                    else
                    {
                        if (isAlpha(params.at(i)))
                        {
                            Variable newVariable("@" + params.at(i), "");
                            newVariable.setNull();
                            method.addMethodVariable(newVariable);
                            __ParamVarCount++;
                        }
                        else
                        {
                            Variable newVariable("@" + params.at(i), 0);
                            newVariable.setNull();
                            method.addMethodVariable(newVariable);
                            __ParamVarCount++;
                        }
                    }
                }

                methods.push_back(method);
                __DefiningMethod = true;
                __DefiningParameterizedMethod = true;
            }
            else
            {
                Method method(arg1);

                if (indestructable)
                    method.setIndestructible();

                methods.push_back(method);
                __DefiningMethod = true;
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
    __DefiningModule = true;
    __CurrentModule = moduleName;
}

void InternalCreateObject(string arg0)
{
    if (objectExists(arg0))
    {
        __DefiningObject = true;
        __CurrentObject = arg0;
    }
    else
    {
        Object object(arg0);
        __CurrentObject = arg0;
        object.dontCollect();
        objects.push_back(object);
        __DefiningObject = true;
    }
}

void InternalForget(string arg0, string arg1)
{
    if (fileExists(__SavedVars))
    {
        string line(""), bigStr("");
        ifstream file(__SavedVars.c_str());
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

            rm(__SavedVars);
            touch(__SavedVars);
            app(__SavedVars, c.e(new_saved));
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
    string defaultValue = c.e(timeNow());
    string sValue(defaultValue);
    double dValue = 0;

    if (after == "cwd")
    {
        sValue = cwd();
    }
    else if (after == "usl")
    {
        sValue = __Noctis;
    }
    else if (after == "os?")
    {
        sValue = __GuessedOS;
    }
    else if (after == "user")
    {
        sValue = getUser();
    }
    else if (after == "machine")
    {
        sValue = getMachine();
    }
    else if (after == "init_dir" || after == "initial_directory")
    {
        sValue = __InitialDirectory;
    }
    else if (after == "this_second")
    {
        dValue = (double)secondNow();
    }
    else if (after == "this_minute")
    {
        dValue = (double)minuteNow();
    }
    else if (after == "this_hour")
    {
        dValue = (double)hourNow();
    }
    else if (after == "this_month")
    {
        dValue = (double)monthNow();
    }
    else if (after == "this_year")
    {
        dValue = (double)yearNow();
    }
    else if (after == "day_of_this_month")
    {
        dValue = (double)dayOfTheMonth();
    }
    else if (after == "day_of_this_year")
    {
        dValue = (double)dayOfTheYear();
    }
    else if (after == "day_of_this_week")
    {
        sValue = dayOfTheWeek();
    }
    else if (after == "month_of_this_year")
    {
        sValue = monthOfTheYear();
    }
    else if (after == "am_or_pm")
    {
        sValue = amOrPm();
    }
    else if (after == "now")
    {
        sValue = timeNow();
    }
    else if (after == "last_error")
    {
        sValue = __LastError;
    }
    else if (after == "last_value")
    {
        sValue = __LastValue;
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
        sValue = getEnvironmentVariable(after);
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
            setLastValue(sValue);
        }
        else
        {
            setLastValue(dtos(dValue));
        }
        break;
    case 3:
        if (sValue != defaultValue)
        {
            say(sValue);
        }
        else
        {
            say(dtos(dValue));
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
            if (objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getString() != __Null)
                text = (objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getString());
            else if (objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getNumber() != __NullNum)
                text = (dtos(objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getNumber()));
            else
                text = ("is_null:" + arg1, false);
        }
        else
        {
            if (isString(arg1))
                text = (variables.at(indexOfVariable(arg1)).getString());
            else if (isNumber(arg1))
                text = (dtos(variables.at(indexOfVariable(arg1)).getNumber()));
            else
                text = ("is_null:" + arg1, false);
        }
    }

    if (is_say)
    {
        say(text);
    }
    else if (is_print)
    {
        if (arg0 == "println")
        {
            cout << text << endl;
        }
        else
        {
            cout << text;
        }
    }
    else
    {
        __stdout(text);
    }
}

void InternalRemember(string arg0, string arg1)
{
    if (variableExists(arg1))
    {
        if (isString(arg1))
            saveVariable(arg1 + "&" + variables.at(indexOfVariable(arg1)).getString());
        else if (isNumber(arg1))
            saveVariable(arg1 + "&" + dtos(variables.at(indexOfVariable(arg1)).getNumber()));
        else
            error("is_null:" + arg1, false);
    }
    else
        error("invalid_operation:target_undefined:" + arg1, false);
}

bool InternalReturn(string arg0, string arg1, string before, string after)
{
    __Returning = true;

    if (containsParams(arg1))
    {
        string before(beforeParams(arg1));

        if (methodExists(before))
        {
            executeTemplate(getMethod(before), getParams(arg1));

            parse("return " + __LastValue);
        }
        else if (!zeroDots(arg1))
        {
            if (objectExists(before))
            {
                if (objects.at(indexOfObject(before)).methodExists(beforeParams(after)))
                {
                    executeTemplate(objects.at(indexOfObject(before)).getMethod(beforeParams(after)), getParams(arg1));
                    parse("return " + __LastValue);
                }
                else
                    __LastValue = arg1;
            }
            else
                __LastValue = arg1;
        }
        else
        {
            if (isStringStack(arg1))
                __LastValue = getStringStack(arg1);
            else if (stackReady(arg1))
                __LastValue = dtos(getStack(arg1));
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
            if (objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getString() != __Null)
                __LastValue = objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getString();
            else if (objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getNumber() != __NullNum)
                __LastValue = dtos(objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getNumber());
            else
                __LastValue = "null";
        }
        else
        {
            if (isString(arg1))
                __LastValue = variables.at(indexOfVariable(arg1)).getString();
            else if (isNumber(arg1))
                __LastValue = dtos(variables.at(indexOfVariable(arg1)).getNumber());
            else
                __LastValue = "null";

            if (variables.at(indexOfVariable(arg1)).garbage())
                variables = removeVariable(variables, arg1);
        }
    }
    else if (listExists(arg1))
    {
        string bigString("(");

        for (int i = 0; i < (int)lists.at(indexOfList(arg1)).size(); i++)
        {
            bigString.append(lists.at(indexOfList(arg1)).at(i));

            if (i != (int)lists.at(indexOfList(arg1)).size() - 1)
                bigString.push_back(',');
        }

        bigString.append(")");

        __LastValue = bigString;

        if (lists.at(indexOfList(arg1)).garbage())
            lists = removeList(lists, arg1);
    }
    else
        __LastValue = arg1;

    return false;
}

int secondNow()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    return (t->tm_sec + 1);
}

int minuteNow()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    return (t->tm_min);
}

int hourNow()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    if (t->tm_hour <= 11)
    {
        if (t->tm_hour == 0)
            return (12);
        else
            return (t->tm_hour);
    }

    return (t->tm_hour - 12);
}

string amOrPm()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    if (t->tm_hour > 11)
        return ("PM");

    return ("AM");
}

string timeNow()
{
    string now("");

    now = itos(monthNow()) + "/" + itos(dayOfTheMonth()) + "/" + itos(yearNow()) + " ";

    if (hourNow() < 10)
        now.append("0" + itos(hourNow()));
    else
        now.append(itos(hourNow()));

    now.append(":");

    if (minuteNow() < 10)
        now.append("0" + itos(minuteNow()));
    else
        now.append(itos(minuteNow()));

    now.append(":");

    if (secondNow() < 10)
        now.append("0" + itos(secondNow()));
    else
        now.append(itos(secondNow()));

    now.append(" " + amOrPm());

    return (now);
}

int monthNow()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    return (t->tm_mon + 1);
}

int yearNow()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    return (t->tm_year + 1900);
}

int dayOfTheMonth()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    return (t->tm_mday);
}

int dayOfTheYear()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    return (t->tm_yday + 2);
}

string dayOfTheWeek()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    string day("");

    switch (t->tm_wday)
    {
    case 0:
        day = "Sunday";
        break;
    case 1:
        day = "Monday";
        break;
    case 2:
        day = "Tuesday";
        break;
    case 3:
        day = "Wednesday";
        break;
    case 4:
        day = "Thursday";
        break;
    case 5:
        day = "Friday";
        break;
    case 6:
        day = "Saturday";
        break;
    default:
        cout << "defaulted: " << t->tm_mday << endl;
        break;
    }

    return (day);
}

string monthOfTheYear()
{
    time_t currently;

    time(&currently);

    struct tm * t = localtime(&currently);

    string month("");

    switch (t->tm_mon)
    {
    case 0:
        month = "January";
        break;
    case 1:
        month = "February";
        break;
    case 2:
        month = "March";
        break;
    case 3:
        month = "April";
        break;
    case 4:
        month = "May";
        break;
    case 5:
        month = "June";
        break;
    case 6:
        month = "July";
        break;
    case 7:
        month = "August";
        break;
    case 8:
        month = "September";
        break;
    case 9:
        month = "October";
        break;
    case 10:
        month = "November";
        break;
    case 11:
        month = "December";
        break;
    default:
        month = "Unknown";
        break;
    }

    return (month);
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

    return (min + (r * (max - min)));
}

string random(string start, string sc)
{
    string s("");
    char c;
    c = (rand() % get_alpha_num(sc[0])) + start[0];
    s.push_back(c);

    return (s);
}

void uninstall()
{
    if (directoryExists(__SavedVarsPath))
    {
        if (fileExists(__SavedVars))
            rm(__SavedVars);
        else
            cerr << "...no remembered variables" << endl;

        rd(__SavedVarsPath);

        if (!directoryExists(__SavedVarsPath) && !fileExists(__SavedVars))
            cout << "...removed successfully" << endl;
        else
            cerr << "...failed to remove" << endl;
    }
    else
        cerr << "...found nothing to remove" << endl;
}

double getBytes(string path)
{
    int bytes;

    ifstream file(path.c_str());

    if(!file.is_open())
    {
        error("read_fail:" + path, false);

        return (-DBL_MAX);
    }

    long begin, end;

    begin = file.tellg();
    file.seekg(0, ios::end);
    end = file.tellg();

    file.close();

    bytes = (end - begin);

    return (bytes);
}

double getKBytes(string path)
{
    return (getBytes(path) / 1024.0);
}

double getMBytes(string path)
{
    return (getBytes(path) / 1048576.0);
}

double getGBytes(string path)
{
    return (getBytes(path) / 1073741824.0);
}

double getTBytes(string path)
{
    return (getBytes(path) / 1099511627776.0);
}

#ifdef __linux__

string getSilentOutput(string text)
{
    char * s = getpass(cleanString(text).c_str());

    return (s);
}

#elif defined _WIN32 || defined _WIN64

string getSilentOutput(string text)
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

    cout << cleanString(text);

    string s("");
    getline(cin, s);

    SetConsoleMode(hStdin, mode);

    return (s);
}

#endif

int main(int c, char ** v)
{
    string app = v[0];
    setup();
    __Noctis = app;
    __InitialDirectory = cwd();
    __Logging = false;

#ifdef _WIN32
    SetConsoleTitle("USL");
#endif

    srand((unsigned int)time(NULL));

    if (c == 1)
    {
        __CurrentScript = app;
        args.push_back(app);
        __ArgumentCount = (int)args.size();
        loop(false);
    }
    else if (c == 2)
    {
        string opt = v[1];

        if (isScript(opt))
        {
            __CurrentScript = opt;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loadScript(opt);
        }
        else if (is(opt, "h") || is(opt, "help"))
            help(app);
        else if (is(opt, "u") || is(opt, "uninstall"))
            uninstall();
        else if (is(opt, "sl") || is(opt, "skipload"))
        {
            __CurrentScript = app;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loop(true);
        }
        else if (is(opt, "n") || is(opt, "__Negligence"))
        {
            __Negligence = true;
            __CurrentScript = app;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loop(true);
        }
        else if (is(opt, "v") || is(opt, "version"))
            displayVersion();
        else
        {
            __CurrentScript = app;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loop(false);
        }
    }
    else if (c == 3)
    {
        string opt = v[1], script = v[2];

        if (is(opt, "sl") || is(opt, "skipload"))
        {
            __CurrentScript = app;

            if (isScript(script))
            {
                __CurrentScript = script;
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loadScript(script);
            }
            else
            {
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loop(true);
            }
        }
        else if (is(opt, "n") || is(opt, "__Negligence"))
        {
            if (isScript(script))
            {
                __Negligence = true;
                __CurrentScript = script;
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loadScript(script);
            }
            else
            {
                __Negligence = true;
                __CurrentScript = app;
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loop(true);
            }
        }
        else if (is(opt, "l") || is(opt, "log"))
        {
            __LogFile = script;

            if (fileExists(__LogFile))
            {
                __Logging = true;
                loop(false);
            }
            else
            {
                touch(__LogFile);
                __Logging = true;
                loop(false);
            }
        }
        else if (is(opt, "p") || is(opt, "parse"))
        {
            string stringBuilder("");

            for (int i = 0; i < (int)script.length(); i++)
            {
                if (script[i] == '\'')
                    stringBuilder.push_back('\"');
                else
                    stringBuilder.push_back(script[i]);
            }

            parse(stringBuilder);
        }
        else
        {
            if (isScript(opt))
            {
                __CurrentScript = opt;
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loadScript(opt);
            }
            else
            {
                __CurrentScript = app;
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loop(false);
            }
        }
    }
    else if (c > 3)
    {
        string opt = v[1];

        if (isScript(opt))
        {
            for (int i = 2; i < c; i++)
            {
                string tmpStr = v[i];
                args.push_back(tmpStr);
            }

            __ArgumentCount = (int)args.size();

            loadScript(opt);
        }
        else
        {
            for (int i = 1; i < c; i++)
            {
                string tmpStr = v[i];
                args.push_back(tmpStr);
            }

            __ArgumentCount = (int)args.size();

            __CurrentScript = app;
            loop(false);
        }
    }
    else
        help(app);

    clearAll();

    return (0);
}
