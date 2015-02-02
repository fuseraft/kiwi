/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#include <iostream>
//#include <iomanip>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <algorithm>
#include <cfloat>

#ifdef __linux__
#include <vector>
#include <sys/stat.h>
// #include <stdio.h>
// #include <functional>
// #include <locale>
#include <cmath>
#include <string.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

const int OS_UNKNOWN	= 0x0000;
const int OS_NIX	    = 0x0001;
const int OS_WIN64	    = 0x0002;
const int OS_WIN32	    = 0x0004;

const int MAX_BUFFER    = 1024;

#include "methods.h"
#include "objects.h"

#include "noctis_datetime.h"

string getPrompt();

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

string	__CurrentLine;
string __CurrentMethodObject;
string __CurrentModule;
string __CurrentObject;
string __CurrentScript;
string __CurrentScriptName;
string __ErrorVarName;
string __GoTo;
string __InitialDirectory;
string __LastError;
string __LastValue;
string __LogFile;
string __Noctis;
string __ParsedOutput;
string __PreviousScript;
string __PromptStyle;
string __SavedVarsPath;
string __SavedVars;
string __SwitchVarName;
string __DefaultLoopSymbol;
string __Null;
double __NullNum;

string cleanString(string st);
string getGuessedOS(string os);
string getParsedOutput(string cmd);
string getSilentOutput(string text);
int sysExec(string s, vector<string> command);

int __ArgumentCount;
int __BadMethodCount;
int __BadObjectCount;
int __BadVarCount;
int __CurrentLineNumber;
int __IfStatementCount;
int __ForLoopCount;
int __ParamVarCount;
int __WhileLoopCount;
int __GuessedOS;

bool __IsCommented;
bool __MultilineComment;

bool __Breaking;
bool __CaptureParse;
bool __DefiningIfStatement;
bool __DefiningForLoop;
bool __DefiningLocalForLoop;
bool __DefiningLocalSwitchBlock;
bool __DefiningLocalWhileLoop;
bool __DefiningMethod;
bool __DefiningModule;
bool __DefiningNest;
bool __DefiningObject;
bool __DefiningObjectMethod;
bool __DefiningParameterizedMethod;
bool __DefiningPrivateCode;
bool __DefiningPublicCode;
bool __DefiningScript;
bool __DefiningSwitchBlock;
bool __DefiningWhileLoop;
bool __DontCollectMethodVars;
bool __ExecutedIfStatement;
bool __ExecutedMethod;
bool __ExecutedTemplate;
bool __ExecutedTryBlock;
bool __FailedIfStatement;
bool __FailedNest;
bool __GoToLabel;
bool __InDefaultCase;
bool __Negligence;
bool __Returning;
bool __SkipCatchBlock;
bool __SkipDefaultBlock;
bool __RaiseCatchBlock;
bool __UseCustomPrompt;

bool suc_stat = false;

List        getDirectoryList(string before, bool filesOnly);
Method      getMethod(string s);
Object      getObject(string s);
Variable    getVariable(string s);

int indexOfConstant(string s);
int indexOfList(string s);
int indexOfMethod(string s);
int indexOfModule(string s);
int indexOfObject(string s);
int indexOfScript(string s);
int indexOfVariable(string s);

void cd(string p);
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
void delay(int seconds);
void displayVersion();
void executeMethod(Method m);
void executeNest(Container n);
void setFalseIf();
void failedFor();
void failedWhile();
void forLoop(Method m);
void help(string app);
void loadSavedVars(Crypt c, string &bs);
void loadScript(string script);
void loop(bool skip);
void redefine(string target, string name);
void replaceElement(string before, string after, string replacement);
void saveVariable(string var);
void writeline(string st);
void setLastValue(string s);
void setup();
void setVariable(string name, string value);
void setVariable(string name, double value);
void successfulFor();
void successfulFor(double a, double b, string op);
void setTrueIf();
void successfullWhile(string v1, string op, string v2);
void successfulFor(List list);
void executeTemplate(Method m, vector<string> vs);
void uninstall();
void whileLoop(Method m);
void write(string st);
void __true();
void __false();

void initializeVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeListValues(string arg0, string arg1, string arg2, string s, vector<string> command);
void createGlobalVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void createObjectVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void copyObject(string arg0, string arg1, string arg2, string s, vector<string> command);
void createConstant(string arg0, string arg1, string arg2, string s, vector<string> command);
void executeSimpleStatement(string arg0, string arg1, string arg2, string s, vector<string> command);

void appendText(string arg1, string arg2, bool newLine);
void __fwrite(string file, string contents);

bool is(string s, string si);
bool isNumber(string varName);
bool isNumber(Variable var);
bool isScript(string path);
bool isString(string varName);
bool isString(Variable var);
bool methodExists(string s);
bool objectExists(string s);
bool variableExists(string s);
bool listExists(string s);
bool moduleExists(string s);
bool noLists();
bool noMethods();
bool noObjects();
bool noVariables();
bool notObjectMethod(string s);
bool notStandardZeroSpace(string s);
bool notStandardOneSpace(string arg1);
bool notStandardTwoSpace(string arg1);

double getBytes(string path);
double getStack(string arg2);

bool isStringStack(string arg2);
bool stackReady(string arg2);
string getStringStack(string arg2);

void error(int errorType, string errorMessage, bool quit);
string getErrorString(int errorType);

const int IS_NULL =	0;
const int BAD_LOAD = 1;
const int CONV_ERR = 2;
const int INFINITE_LOOP = 3;
const int INVALID_OP = 4;
const int DIR_EXISTS = 5;
const int DIR_NOT_FOUND = 6;
const int FILE_EXISTS =	7;
const int FILE_NOT_FOUND = 8;
const int OUT_OF_BOUNDS = 9;
const int INVALID_RANGE_SEP = 10;
const int INVALID_SEQ = 11;
const int INVALID_SEQ_SEP = 12;
const int INVALID_VAR_DECL = 13;
const int LIST_UNDEFINED = 14;
const int METHOD_DEFINED = 15;
const int METHOD_UNDEFINED = 16;
const int NULL_NUMBER = 17;
const int NULL_STRING = 18;
const int OBJ_METHOD_UNDEFINED = 19;
const int OBJ_UNDEFINED = 20;
const int OBJ_VAR_UNDEFINED = 21;
const int VAR_DEFINED = 22;
const int VAR_UNDEFINED = 23;
const int TARGET_UNDEFINED = 24;
const int CONST_UNDEFINED = 25;
const int INVALID_OPERATOR = 26;
const int IS_EMPTY = 27;
const int READ_FAIL = 28;
const int DIVIDED_BY_ZERO = 29;
const int UNDEFINED = 30;
const int UNDEFINED_OS = 31;

#include "parser.h"

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
                    error(IS_NULL, params.at(i), false);
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
                        error(IS_NULL, arg2, false);
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
                error(READ_FAIL, variables.at(indexOfVariable(arg1)).getString(), false);
        }
        else
            error(CONV_ERR, arg1, false);
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
                    error(READ_FAIL, variables.at(indexOfVariable(arg2)).getString(), false);
            }
            else
                error(CONV_ERR, arg2, false);
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
                error(READ_FAIL, arg1, false);
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
                        error(IS_NULL, arg2, false);
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
                createFile(variables.at(indexOfVariable(arg1)).getString());

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
                    error(IS_NULL, arg2, false);
                    __LastValue = "-1";
                }

                __LastValue = "1";
            }
        }
        else
        {
            error(CONV_ERR, arg1, false);
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
                    createFile(variables.at(indexOfVariable(arg2)).getString());
                    app(arg1, variables.at(indexOfVariable(arg2)).getString() + "\r\n");
                    __LastValue = "1";
                }
            }
            else
            {
                error(CONV_ERR, arg2, false);
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
                createFile(arg1);
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
                switch (__GuessedOS)
                {
                case OS_NIX:
                    cleaned.append(getEnvironmentVariable("HOME"));
                    break;
                case OS_WIN32:
                case OS_WIN64:
                    cleaned.append(getEnvironmentVariable("HOMEPATH"));
                    break;
                case OS_UNKNOWN:
                    cleaned.append(getEnvironmentVariable("HOME"));
                    break;
                default:
                    error(UNDEFINED_OS, "", false);
                    break;
                }
            }
            else
                cleaned.push_back(p[i]);
        }

        if (directoryExists(cleaned))
            cd(cleaned);
        else
            error(READ_FAIL, p, false);
    }
    else
    {
        if (p == "~")
        {
            if (__GuessedOS == OS_NIX || __GuessedOS == OS_UNKNOWN)
                cd(getEnvironmentVariable("HOME"));
            else
                cd(getEnvironmentVariable("HOMEPATH"));
        }
        else if (p == "init_dir" || p == "initial_directory")
            cd(__InitialDirectory);
        else
        {
            if (chdir(p.c_str()) != 0)
                error(READ_FAIL, p, false);
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
                                error(METHOD_UNDEFINED, before + "." + beforeParams(after), false);
                        }
                        else
                            error(OBJ_METHOD_UNDEFINED, before, false);
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
                                            error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
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
                                            error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                                    }
                                    else
                                        error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
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
                                        error(OUT_OF_BOUNDS, afterBrackets, false);
                                }
                                else
                                    error(OUT_OF_BOUNDS, afterBrackets, false);
                            }
                            else
                                error(OUT_OF_BOUNDS, afterBrackets, false);
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
                                        error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
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
                                        error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                                }
                                else
                                    error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                            }
                            else
                                error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                        }
                        else if (listRange.size() == 1)
                        {
                            rangeBegin = listRange.at(0);

                            if (isNumeric(rangeBegin))
                            {
                                if (stoi(rangeBegin) <= (int)lists.at(indexOfList(_beforeBrackets)).size() - 1 && stoi(rangeBegin) >= 0)
                                    cleaned.append(lists.at(indexOfList(_beforeBrackets)).at(stoi(rangeBegin)));
                                else
                                    error(OUT_OF_BOUNDS, afterBrackets, false);
                            }
                            else
                                error(OUT_OF_BOUNDS, afterBrackets, false);
                        }
                        else
                            error(OUT_OF_BOUNDS, afterBrackets, false);
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
                            error(VAR_UNDEFINED, before + "." + after, false);
                    }
                    else
                        error(OBJ_METHOD_UNDEFINED, before, false);
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

    return (cleaned);
}

void write(string st)
{
    if (__CaptureParse)
        __ParsedOutput.append(cleanString(st));
    else
        cout << cleanString(st);

    setLastValue(st);
}

void writeline(string st)
{
    write(st);
    cout << (__GuessedOS == OS_NIX ? "\n" : "\r\n");
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

void displayVersion()
{
    cout << "\r\nnoctis v0.0.0 by <scstauf@gmail.com>\r\n" << endl;
}

string getErrorString(int errorType)
{
    string errorString("");

    switch (errorType)
    {
    case IS_NULL:
        errorString.append("is null");
        break;
    case BAD_LOAD:
        errorString.append("bad load");
        break;
    case CONV_ERR:
        errorString.append("conversion error");
        break;
    case INFINITE_LOOP:
        errorString.append("infinite loop");
    case INVALID_OP:
        errorString.append("invalid operation");
        break;
    case DIR_EXISTS:
        errorString.append("directory already exists");
        break;
    case DIR_NOT_FOUND:
        errorString.append("directory does not exist");
        break;
    case FILE_EXISTS:
        errorString.append("file already exists");
        break;
    case FILE_NOT_FOUND:
        errorString.append("file does not exist");
        break;
    case OUT_OF_BOUNDS:
        errorString.append("index out of bounds");
        break;
    case INVALID_RANGE_SEP:
        errorString.append("invalid range separator");
        break;
    case INVALID_SEQ:
        errorString.append("invalid sequence");
        break;
    case INVALID_SEQ_SEP:
        errorString.append("invalid sequence separator");
        break;
    case INVALID_VAR_DECL:
        errorString.append("invalid variable declaration");
        break;
    case LIST_UNDEFINED:
        errorString.append("list undefined");
        break;
    case METHOD_DEFINED:
        errorString.append("method defined");
        break;
    case METHOD_UNDEFINED:
        errorString.append("method undefined");
        break;
    case NULL_NUMBER:
        errorString.append("null number");
        break;
    case NULL_STRING:
        errorString.append("null string");
        break;
    case OBJ_METHOD_UNDEFINED:
        errorString.append("object method undefined");
        break;
    case OBJ_UNDEFINED:
        errorString.append("object undefined");
        break;
    case OBJ_VAR_UNDEFINED:
        errorString.append("object variable undefined");
        break;
    case VAR_DEFINED:
        errorString.append("variable defined");
        break;
    case VAR_UNDEFINED:
        errorString.append("variable undefined");
        break;
    case TARGET_UNDEFINED:
        errorString.append("target undefined");
        break;
    case CONST_UNDEFINED:
        errorString.append("constant defined");
        break;
    case INVALID_OPERATOR:
        errorString.append("invalid operator");
        break;
    case IS_EMPTY:
        errorString.append("is empty");
        break;
    case READ_FAIL:
        errorString.append("read failure");
        break;
    case DIVIDED_BY_ZERO:
        errorString.append("cannot divide by zero");
        break;
    case UNDEFINED:
        errorString.append("undefined");
        break;
    case UNDEFINED_OS:
        errorString.append("undefined_os");
        break;
    }

    return errorString;
}

void error(int errorType, string errorInfo, bool quit)
{
    string completeError("##\n# error:\t");
    completeError.append(getErrorString(errorType));
    completeError.append(":\t");
    completeError.append(errorInfo);
    completeError.append("\n# line ");
    completeError.append(itos(__CurrentLineNumber));
    completeError.append(":\t");
    completeError.append(__CurrentLine);
    completeError.append("\n##\n");

    if (__ExecutedTryBlock)
    {
        __RaiseCatchBlock = true;
        __LastError = completeError;
    }
    else
    {
        if (__CaptureParse)
            __ParsedOutput.append(completeError);
        else
            cerr << completeError;
    }

    if (!__Negligence)
    {
        if (quit)
        {
            clearAll();
            exit(0);
        }
    }
}

string getGuessedOS()
{
    string guessedOS("");

    switch (__GuessedOS)
    {
    case OS_NIX:
        guessedOS = "OS_NIX";
        break;
    case OS_WIN32:
        guessedOS = "OS_WIN32";
        break;
    case OS_WIN64:
        guessedOS = "OS_WIN64";
        break;
    case OS_UNKNOWN:
        guessedOS = "OS_UNKNOWN";
        break;
    }

    return guessedOS;
}

string getParsedOutput(string cmd)
{
    __CaptureParse = true;
    parse(cmd);
    string ret = __ParsedOutput;
    __ParsedOutput.clear();
    __CaptureParse = false;
	
    return ret.length() == 0 ? __LastValue : ret;
}

List getDirectoryList(string before, bool filesOnly)
{
    List newList;
    int i = 1;

    DIR *pd;
    struct dirent *pe;

    string dir = variables.at(indexOfVariable(before)).getString();

    if ((pd = opendir(dir.c_str())) == NULL)
        __DefiningForLoop = false;
    else
    {
        while ((pe = readdir(pd)) != NULL)
        {
            if (string(pe->d_name) != "." && string(pe->d_name) != "..")
            {
                string tmp("");

                if (dir == "/")
                    dir = "";

                if (__GuessedOS == OS_NIX)
                    tmp = dir + "/" + string(pe->d_name);
                else
                    tmp = dir + "\\" + string(pe->d_name);

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
    setLastValue("true");
}

void __false()
{
    setLastValue("false");
}

void saveVariable(string variableName)
{
    Crypt c;

    if (!fileExists(__SavedVars))
    {
        if (!directoryExists(__SavedVarsPath))
            md(__SavedVarsPath);

        createFile(__SavedVars);
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
            createFile(__SavedVars);
            app(__SavedVars, c.e(bigStr + "#" + variableName));
            file.close();
        }
        else
            error(READ_FAIL, __SavedVars, false);
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
         << "\t" << app << " {args}\t\t// start the shell, with parameters" << endl
         << "\t" << app << " {script}\t\t// interpret a script" << endl
         << "\t" << app << " {script} {args}\t// interpret a script, with parameters" << endl
         << "\t" << app << " -n, --negligence\t// do not terminate on parse errors" << endl
         << "\t" << app << " -sl, --skipload\t// start the shell, skip loading saved vars" << endl
         << "\t" << app << " -u, --uninstall\t// remove $HOME/.savedVarsPath" << endl
         << "\t" << app << " -v, --version\t// display current version" << endl
         << "\t" << app << " -p, --parse\t\t// parse a command" << endl
         << "\t" << app << " -h, --help\t\t// display this message" << endl << endl;
}

// TODO:
//		how can I clean up these stupidly long if-statements?
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
    return (s == ("-" + si) || s == ("--" + si) || s == ("/" + si));
}

bool isScript(string path)
{
    return endsWith(path, ".ns");
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
        error(READ_FAIL, __SavedVars, false);
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
            parse(trimLeadingWhitespace(c));
            delete[] c;
        }
    }
}

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

void setFalseIf()
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

void setTrueIf()
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

int sysExec(string s, vector<string> command)
{
	/*string _cleaned;
	_cleaned = cleanstring(s);
    for (int i = 0; i < (int)methods.size(); i++)
    {
        if (command.at(0) == methods.at(i).name())
        {
            if ((int)command.size() - 1 == (int)methods.at(i).getmethodvariables().size())
            {
                // work
            }
        }
    }*/
	exec(cleanString(s));
	return 0;
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
                                error(FILE_NOT_FOUND, old_name, false);
                        }
                        else
                            error(FILE_EXISTS, new_name, false);
                    }
                    else if (directoryExists(old_name))
                    {
                        if (!directoryExists(new_name))
                        {
                            if (directoryExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error(DIR_NOT_FOUND, old_name, false);
                        }
                        else
                            error(DIR_EXISTS, new_name, false);
                    }
                    else
                        error(TARGET_UNDEFINED, old_name, false);
                }
                else
                    error(NULL_STRING, name, false);
            }
            else
            {
                if (fileExists(old_name))
                {
                    if (!fileExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error(FILE_EXISTS, name, false);
                }
                else if (directoryExists(old_name))
                {
                    if (!directoryExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error(DIR_EXISTS, name, false);
                }
                else
                    error(TARGET_UNDEFINED, old_name, false);
            }
        }
        else
        {
            if (startsWith(name, "@"))
            {
                if (!variableExists(name))
                    variables.at(indexOfVariable(target)).setName(name);
                else
                    error(VAR_DEFINED, name, false);
            }
            else
                error(INVALID_VAR_DECL, name, false);
        }
    }
    else if (listExists(target))
    {
        if (!listExists(name))
            lists.at(indexOfList(target)).setName(name);
        else
            error(LIST_UNDEFINED, name, false);
    }
    else if (objectExists(target))
    {
        if (!objectExists(name))
            objects.at(indexOfObject(target)).setName(name);
        else
            error(OBJ_METHOD_UNDEFINED, name, false);
    }
    else if (methodExists(target))
    {
        if (!methodExists(name))
            methods.at(indexOfMethod(target)).setName(name);
        else
            error(METHOD_UNDEFINED, name, false);
    }
    else if (fileExists(target) || directoryExists(target))
        rename(target.c_str(), name.c_str());
    else
        error(TARGET_UNDEFINED, target, false);
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
                            error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
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
                            error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                    }
                    else
                        error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                }
                else
                    error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
            }
            else
                error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
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
            error(OUT_OF_BOUNDS, arg2, false);
    }
    else
        error(NULL_STRING, beforeBracket, false);

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
                            error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
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
                            error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                    }
                    else
                        error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                }
                else
                    error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
            }
            else
                error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
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
            error(OUT_OF_BOUNDS, arg2, false);
    }
    else
        error(NULL_STRING, beforeBracket, false);
}

bool isNumber(Variable var)
{
	return var.getNumber() != __NullNum;
}

bool isNumber(string varName)
{
    return variables.at(indexOfVariable(varName)).getNumber() != __NullNum;
}

bool isString(Variable var)
{
	return var.getString() != __Null;
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
                error(INFINITE_LOOP, "", true);
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

void initializeVariable(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    string tmpObjName = beforeDot(arg0), tmpVarName = afterDot(arg0);
    bool tmpObjExists = objectExists(tmpObjName);
    if (tmpObjExists || startsWith(arg0, "@"))
    {
        if (tmpObjExists)
        {
            if (objects.at(indexOfObject(tmpObjName)).getVariable(tmpVarName).getString() != __Null)
            {
                string tempObjectVariableName("@ " + tmpObjName + tmpVarName + "_string");

                createVariable(tempObjectVariableName, objects.at(indexOfObject(tmpObjName)).getVariable(tmpVarName).getString());

                twoSpace(tempObjectVariableName, arg1, arg2, "", command);

                variables.at(indexOfVariable(tempObjectVariableName)).setName(tmpVarName);

                objects.at(indexOfObject(tmpObjName)).removeVariable(tmpVarName);
                objects.at(indexOfObject(tmpObjName)).addVariable(variables.at(indexOfVariable(tmpVarName)));
                variables = removeVariable(variables, tmpVarName);
            }
            else if (objects.at(indexOfObject(tmpObjName)).getVariable(tmpVarName).getNumber() != __NullNum)
            {
                string tempObjectVariableName("@____" + beforeDot(arg0) + "___" + afterDot(arg0) + "_number");

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
                            error(OUT_OF_BOUNDS, arg2, false);
                        else
                        {
                            string listValue(lists.at(indexOfList(beforeBracket)).at(stoi(afterBracket)));

                            if (isNumeric(listValue))
                            {
                                if (isNumber(arg0))
                                    setVariable(arg0, stod(listValue));
                                else
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                            {
                                if (isString(arg0))
                                    setVariable(arg0, listValue);
                                else
                                    error(CONV_ERR, arg0, false);
                            }
                        }
                    }
                }
                else if (isString(beforeBracket))
                    setSubString(arg0, arg2, beforeBracket);
                else
                    error(LIST_UNDEFINED, beforeBracket, false);
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
                                    error(NULL_STRING, arg0, false);
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
                                        error(NULL_STRING, arg0, false);
                                }
                            }
                            else
                                error(INVALID_SEQ, s0 + "_" + s2, false);
                        }
                        else
                            error(INVALID_SEQ_SEP, arg2, false);
                    }
                }
                else if (listExists(before) && after == "size")
                {
                    if (isNumber(arg0))
                        setVariable(arg0, stod(itos(lists.at(indexOfList(before)).size())));
                    else if (isString(arg0))
                        setVariable(arg0, itos(lists.at(indexOfList(before)).size()));
                    else
                        error(IS_NULL, arg0, false);
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
                            error(IS_NULL, arg2, false);
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
                                    error(IS_NULL, arg0, false);
                            }
                            else
                            {
                                if (isString(arg0))
                                    setVariable(arg0, __LastValue);
                                else if (isNumber(arg0))
                                    error(CONV_ERR, arg0, false);
                                else
                                    error(IS_NULL, arg0, false);
                            }
                        }
                        else
                            sysExec(s, command);
                    }
                    else
                        error(VAR_UNDEFINED, arg2, false);
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
                            error(IS_NULL, before, false);
                    }
                    else
                        error(VAR_UNDEFINED, before, false);
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
                            error(IS_NULL, before, false);
                    }
                    else
                        error(VAR_UNDEFINED, before, false);
                }
                else if (after == "to_string")
                {
                    if (variableExists(before))
                    {
                        if (isNumber(before))
                            setVariable(arg0, dtos(variables.at(indexOfVariable(before)).getNumber()));
                        else
                            error(IS_NULL, before, false);
                    }
                    else
                        error(VAR_UNDEFINED, before, false);
                }
                else if (after == "to_number")
                {
                    if (variableExists(before))
                    {
                        if (isString(before))
                            setVariable(arg0, stod(variables.at(indexOfVariable(before)).getString()));
                        else
                            error(IS_NULL, before, false);
                    }
                    else
                        error(VAR_UNDEFINED, before, false);
                }
                else if (before == "readline")
                {
                    if (variableExists(after))
                    {
                        if (isString(after))
                        {
                            string line("");
                            write(cleanString(variables.at(indexOfVariable(after)).getString()));
                            getline(cin, line, '\n');

                            if (isNumber(arg0))
                            {
                                if (isNumeric(line))
                                    setVariable(arg0, stod(line));
                                else
                                    error(CONV_ERR, line, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, line);
                            else
                                error(IS_NULL, arg0, false);
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
                                    error(CONV_ERR, line, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, line);
                            else
                                error(IS_NULL, arg0, false);
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
                                    error(CONV_ERR, line, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, line);
                            else
                                error(IS_NULL, arg0, false);

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
                                    error(CONV_ERR, line, false);
                            }
                            else if (isString(arg0))
                                setVariable(arg0, line);
                            else
                                error(IS_NULL, arg0, false);

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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(cos(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(acos(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(cosh(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(log(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(sqrt(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(abs(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(floor(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(ceil(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(exp(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(sin(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(sinh(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(asin(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(tan(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(tanh(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else if (isString(arg0))
                        {
                            if (isNumber(before))
                                setVariable(arg0, dtos(atan(variables.at(indexOfVariable(before)).getNumber())));
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                        error(READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
                                }
                                else
                                    error(READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
                            }
                            else
                                error(NULL_STRING, before, false);
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
                                    error(READ_FAIL, before, false);
                            }
                            else
                                error(READ_FAIL, before, false);
                        }
                    }
                    else
                        error(NULL_STRING, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, before, false);
                        }
                        else
                            error(CONV_ERR, arg0, false);
                    }
                    else
                    {
                        if (isNumber(arg0))
                            setVariable(arg0, (double)before.length());
                        else
                            error(CONV_ERR, arg0, false);
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
                                    error(READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
                            }
                            else
                                error(CONV_ERR, before, false);
                        }
                        else
                        {
                            if (fileExists(before))
                                setVariable(arg0, getBytes(before));
                            else
                                error(READ_FAIL, before, false);
                        }
                    }
                    else
                        error(CONV_ERR, arg0, false);
                }
                else
                {
                    if (isNumber(arg0))
                    {
                        if (isNumeric(arg2))
                            setVariable(arg0, stod(arg2));
                        else
                            error(CONV_ERR, arg0, false);
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
                        error(IS_NULL, arg0, false);
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
                        error(IS_NULL, arg0, false);
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
                            error(CONV_ERR, arg2, false);
                    }
                    else
                        error(IS_NULL, arg0, false);
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
                            error(CONV_ERR, arg2, false);
                        else
                            error(IS_NULL, arg0, false);
                    }
                    else if (isNumber(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                        else if (isNumber(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg2)).getNumber());
                        else
                            error(IS_NULL, arg0, false);
                    }
                    else
                        error(IS_NULL, arg2, false);
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
                                error(CONV_ERR, passworder, false);
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
                            error(CONV_ERR, arg0, false);
                    }
                    else if (stackReady(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, dtos(getStack(arg2)));
                        else if (isNumber(arg0))
                            setVariable(arg0, getStack(arg2));
                        else
                            error(IS_NULL, arg0, false);
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
                            error(CONV_ERR, arg0, false);
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
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + variables.at(indexOfVariable(arg2)).getString());
                        else if (isNumber(arg2))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + dtos(variables.at(indexOfVariable(arg2)).getNumber()));
                        else
                            error(IS_NULL, arg2, false);
                    }
                    else if (isNumber(arg0))
                    {
                        if (isString(arg2))
                            error(CONV_ERR, arg2, false);
                        else if (isNumber(arg2))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() + variables.at(indexOfVariable(arg2)).getNumber());
                        else
                            error(IS_NULL, arg2, false);
                    }
                    else
                        error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, arg0, false);
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
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(IS_NULL, arg0, false);
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
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, arg0, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
                    }
                    else if (isNumeric(arg2))
                    {
                        if (isString(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + arg2);
                        else if (isNumber(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() + stod(arg2));
                        else
                            error(IS_NULL, arg0, false);
                    }
                    else
                    {
                        if (isString(arg0))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getString() + cleanString(arg2));
                        else if (isNumber(arg0))
                            error(CONV_ERR, arg0, false);
                        else
                            error(IS_NULL, arg0, false);
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
                            error(IS_NULL, arg2, false);
                    }
                    else if (isNumber(arg0))
                    {
                        if (isString(arg2))
                            error(CONV_ERR, arg2, false);
                        else if (isNumber(arg2))
                            setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() - variables.at(indexOfVariable(arg2)).getNumber());
                        else
                            error(IS_NULL, arg2, false);
                    }
                    else
                        error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, arg0, false);
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
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(IS_NULL, arg0, false);
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
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(IS_NULL, arg0, false);
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
                                error(CONV_ERR, arg0, false);
                        }
                        else
                            error(IS_NULL, arg0, false);
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
                            error(IS_NULL, arg0, false);
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
                            error(CONV_ERR, arg0, false);
                        else
                            error(IS_NULL, arg0, false);
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
                        error(CONV_ERR, arg2, false);
                    else
                        error(IS_NULL, arg2, false);
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
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(NULL_NUMBER, arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() * stod(__LastValue));
                                else
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(NULL_NUMBER, arg0, false);
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
                                error(CONV_ERR, arg0, false);
                        }
                        else
                            error(NULL_NUMBER, arg0, false);
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
                        error(CONV_ERR, arg2, false);
                    else
                        error(IS_NULL, arg2, false);
                }
                else if (methodExists(arg2))
                {
                    parse(arg2);

                    if (isNumber(arg0))
                    {
                        if (isNumeric(__LastValue))
                            setVariable(arg0, (int)variables.at(indexOfVariable(arg0)).getNumber() % (int)stod(__LastValue));
                        else
                            error(CONV_ERR, arg0, false);
                    }
                    else
                        error(NULL_NUMBER, arg0, false);
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
                        error(CONV_ERR, arg2, false);
                    else
                        error(IS_NULL, arg2, false);
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
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(NULL_NUMBER, arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, pow(variables.at(indexOfVariable(arg0)).getNumber(), (int)stod(__LastValue)));
                                else
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(NULL_NUMBER, arg0, false);
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
                                error(CONV_ERR, arg0, false);
                        }
                        else
                            error(NULL_NUMBER, arg0, false);
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
                        error(CONV_ERR, arg2, false);
                    else
                        error(IS_NULL, arg2, false);
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
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(NULL_NUMBER, arg0, false);
                        }
                        else if (objectExists(beforeDot(arg2)))
                        {
                            executeTemplate(getMethod(beforeParams(arg2)), getParams(arg2));

                            if (isNumber(arg0))
                            {
                                if (isNumeric(__LastValue))
                                    setVariable(arg0, variables.at(indexOfVariable(arg0)).getNumber() / stod(__LastValue));
                                else
                                    error(CONV_ERR, arg0, false);
                            }
                            else
                                error(NULL_NUMBER, arg0, false);
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
                                error(CONV_ERR, arg0, false);
                        }
                        else
                            error(NULL_NUMBER, arg0, false);
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
                            error(IS_NULL, arg0, false);
                    }
                    else
                        error(CONV_ERR, arg2, false);
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
                            error(IS_NULL, tempVarString, false);
                    }
                    else
                        error(CONV_ERR, arg2, false);
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
                            error(IS_NULL, arg0, false);
                    }
                    else
                        error(CONV_ERR, arg2, false);
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
                            error(IS_NULL, tempVarString, false);
                    }
                    else
                        error(CONV_ERR, arg2, false);
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
                            error(CONV_ERR, arg0, false);
                    }
                    else
                        error(CONV_ERR, arg2, false);
                }
                else
                {
                    if (isString(arg0))
                        setVariable(arg0, getStdout(cleanString(arg2).c_str()));
                    else
                        error(CONV_ERR, arg0, false);
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
                            error(CONV_ERR, arg0, false);
                    }
                    else
                        error(CONV_ERR, arg2, false);
                }
                else
                {
                    if (isString(arg0))
                        setVariable(arg0, getParsedOutput(cleanString(arg2).c_str()));
                    else
                        error(CONV_ERR, arg0, false);
                }
            }
            else
            {
                error(INVALID_OPERATOR, arg1, false);
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
                            error(IS_NULL, arg2, false);
                    }
                    else
                        replaceElement(before, after, arg2);
                }
            }
            else if (lists.at(indexOfList(before)).at(stoi(after)) == "#!=no_line")
                error(OUT_OF_BOUNDS, arg0, false);
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
                            error(IS_NULL, arg2, false);
                    }
                    else
                        replaceElement(before, after, arg2);
                }
            }
        }
        else
            error(OUT_OF_BOUNDS, arg2, false);
    }
    else if (containsBrackets(arg2)) // INITIALIZE LIST FROM RANGE
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
                                        error(INVALID_OPERATOR, arg1, false);
                                }
                                else
                                    error(OUT_OF_BOUNDS, rangeBegin, false);
                            }
                            else
                                error(OUT_OF_BOUNDS, rangeEnd, false);
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
                                        error(INVALID_OPERATOR, arg1, false);
                                }
                                else
                                    error(OUT_OF_BOUNDS, rangeBegin, false);
                            }
                            else
                                error(OUT_OF_BOUNDS, rangeEnd, false);
                        }
                        else
                            error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                    }
                    else
                        error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
                }
                else
                    error(OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
            }
            else
                error(OUT_OF_BOUNDS, arg2, false);
        }
        else
            error(LIST_UNDEFINED, listName, false);
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
            error(NULL_STRING, _b, false);
    }
    else if (containsParams(arg2)) // ADD/REMOVE ARRAY FROM LIST
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
                        error(IS_NULL, params.at(i), false);
                }
                else
                    lists.at(indexOfList(arg0)).remove(params.at(i));
            }
        }
        else
            error(INVALID_OPERATOR, arg1, false);
    }
    else if (variableExists(arg2)) // ADD/REMOVE VARIABLE VALUE TO/FROM LIST
    {
        if (arg1 == "+=")
        {
            if (isString(arg2))
                lists.at(indexOfList(arg0)).add(variables.at(indexOfVariable(arg2)).getString());
            else if (isNumber(arg2))
                lists.at(indexOfList(arg0)).add(dtos(variables.at(indexOfVariable(arg2)).getNumber()));
            else
                error(CONV_ERR, arg2, false);
        }
        else if (arg1 == "-=")
        {
            if (isString(arg2))
                lists.at(indexOfList(arg0)).remove(variables.at(indexOfVariable(arg2)).getString());
            else if (isNumber(arg2))
                lists.at(indexOfList(arg0)).remove(dtos(variables.at(indexOfVariable(arg2)).getNumber()));
            else
                error(CONV_ERR, arg2, false);
        }
        else
            error(INVALID_OPERATOR, arg1, false);
    }
    else if (methodExists(arg2)) // INITIALIZE LIST FROM METHOD RETURN
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
            error(INVALID_OPERATOR, arg1, false);
    }
    else // ADD/REMOVE STRING TO/FROM LIST
    {
        if (arg1 == "+=")
        {
            if (arg2.length() != 0)
                lists.at(indexOfList(arg0)).add(arg2);
            else
                error(IS_EMPTY, arg2, false);
        }
        else if (arg1 == "-=")
        {
            if (arg2.length() != 0)
                lists.at(indexOfList(arg0)).remove(arg2);
            else
                error(IS_EMPTY, arg2, false);
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
                if (lists.at(indexOfList(beforeBracket)).size() >= stoi(afterBracket))
                {
                    if (lists.at(indexOfList(beforeBracket)).at(stoi(afterBracket)) == "#!=no_line")
                        error(OUT_OF_BOUNDS, arg2, false);
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
                    error(OUT_OF_BOUNDS, arg2, false);
            }
            else if (variableExists(beforeBracket))
                setSubString(arg0, arg2, beforeBracket);
            else
                error(LIST_UNDEFINED, beforeBracket, false);
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
        else if (after == "to_integer")
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
                    error(IS_NULL, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "to_double")
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
                    error(IS_NULL, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "to_string")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, dtos(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(IS_NULL, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "to_number")
        {
            if (variableExists(before))
            {
                if (isString(before))
                    createVariable(arg0, stod(variables.at(indexOfVariable(before)).getString()));
                else
                    error(IS_NULL, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
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
                    error(IS_NULL, objects.at(indexOfObject(before)).getVariable(after).name(), false);
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
                        error(READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
                }
                else
                    error(READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
            }
            else
                error(NULL_STRING, before, false);
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
                error(CONV_ERR, arg2, false);
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
                        error(OUT_OF_BOUNDS, s0 + ".." + s2, false);
                }
                else
                    error(INVALID_RANGE_SEP, arg2, false);
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
                    error(CONV_ERR, before, false);
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
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "sinh")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, sinh(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "asin")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, asin(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "tan")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, tan(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "tanh")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, tanh(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "atan")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, atan(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "cos")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, cos(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "acos")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, acos(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "cosh")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, cosh(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "log")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, log(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "sqrt")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, sqrt(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "abs")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, abs(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "floor")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, floor(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "ceil")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, ceil(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "exp")
        {
            if (variableExists(before))
            {
                if (isNumber(before))
                    createVariable(arg0, exp(variables.at(indexOfVariable(before)).getNumber()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "to_upper")
        {
            if (variableExists(before))
            {
                if (isString(before))
                    createVariable(arg0, getUpper(variables.at(indexOfVariable(before)).getString()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
        }
        else if (after == "to_lower")
        {
            if (variableExists(before))
            {
                if (isString(before))
                    createVariable(arg0, getLower(variables.at(indexOfVariable(before)).getString()));
                else
                    error(CONV_ERR, before, false);
            }
            else
                error(VAR_UNDEFINED, before, false);
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
                        error(READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
                }
                else
                    error(CONV_ERR, before, false);
            }
            else
            {
                if (fileExists(before))
                    createVariable(arg0, getBytes(before));
                else
                    error(READ_FAIL, before, false);
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
            if (isNumeric(arg2))
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
                error(CONV_ERR, arg2, false);
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
                error(CONV_ERR, arg2, false);
        }
        else
            createVariable(arg0, getParsedOutput(cleanString(arg2)));
    }
    else
        error(INVALID_OPERATOR, arg2, false);
}

void createObjectVariable(string arg0, string arg1, string arg2, string s, vector<string> command)
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

void copyObject(string arg0, string arg1, string arg2, string s, vector<string> command)
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
        error(INVALID_OPERATOR, arg1, false);
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
            error(INVALID_OPERATOR, arg1, false);
    }
    else
        error(CONST_UNDEFINED, arg0, false);
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
                error(DIVIDED_BY_ZERO, s, false);
            else
                writeline(dtos((int)stod(arg0) % (int)stod(arg2)));
        }
        else
            error(INVALID_OPERATOR, arg1, false);
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

                setLastValue(bigstr);
            }
            else
                error(INVALID_OP, s, false);
        }
        else if (arg1 == "/")
            writeline(subtractString(arg0, arg2));
        else
            error(INVALID_OPERATOR, arg1, false);
    }
}

void InternalEncryptDecrypt(string arg0, string arg1)
{
    Crypt c;
    string text = variableExists(arg1) ? (isString(arg1) ? getVariable(arg1).getString() : dtos(getVariable(arg1).getNumber())) : arg1;
    write(arg0 == "encrypt" ? c.e(text) : c.d(text));
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
                    write(objects.at(indexOfObject(before)).getMethod(after).at(i));
            }
            else if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (objects.at(indexOfObject(before)).getVariable(after).getString() != __Null)
                    write(objects.at(indexOfObject(before)).getVariable(after).getString());
                else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != __NullNum)
                    write(dtos(objects.at(indexOfObject(before)).getVariable(after).getNumber()));
                else
                    write(__Null);
            }
            else
                error(TARGET_UNDEFINED, arg1, false);
        }
        else
            error(OBJ_METHOD_UNDEFINED, before, false);
    }
    else
    {
        if (objectExists(arg1))
        {
            for (int i = 0; i < objects.at(indexOfObject(arg1)).methodSize(); i++)
                write(objects.at(indexOfObject(arg1)).getMethod(objects.at(indexOfObject(arg1)).getMethodName(i)).name());
            for (int i = 0; i < objects.at(indexOfObject(arg1)).variableSize(); i++)
                write(objects.at(indexOfObject(arg1)).getVariable(objects.at(indexOfObject(arg1)).getVariableName(i)).name());
        }
        else if (constantExists(arg1))
        {
            if (constants.at(indexOfConstant(arg1)).ConstNumber())
                write(dtos(constants.at(indexOfConstant(arg1)).getNumber()));
            else if (constants.at(indexOfConstant(arg1)).ConstString())
                write(constants.at(indexOfConstant(arg1)).getString());
        }
        else if (methodExists(arg1))
        {
            for (int i = 0; i < methods.at(indexOfMethod(arg1)).size(); i++)
                write(methods.at(indexOfMethod(arg1)).at(i));
        }
        else if (variableExists(arg1))
        {
            if (isString(arg1))
                write(variables.at(indexOfVariable(arg1)).getString());
            else if (isNumber(arg1))
                write(dtos(variables.at(indexOfVariable(arg1)).getNumber()));
        }
        else if (listExists(arg1))
        {
            for (int i = 0; i < lists.at(indexOfList(arg1)).size(); i++)
                write(lists.at(indexOfList(arg1)).at(i));
        }
        else if (arg1 == "variables")
        {
            for (int i = 0; i < (int)variables.size(); i++)
            {
                if (variables.at(i).getString() != __Null)
                    write(variables.at(i).name() + ":\t" + variables.at(i).getString());
                else if (variables.at(i).getNumber() != __NullNum)
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
            write(getGuessedOS());
        else if (arg1 == "last")
            write(__LastValue);
        else
            error(TARGET_UNDEFINED, arg1, false);
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
        error(OBJ_METHOD_UNDEFINED, arg1, false);
}

void InternalCallMethod(string arg0, string arg1, string before, string after)
{
    if (__DefiningObject)
    {
        if (objects.at(indexOfObject(__CurrentObject)).methodExists(arg1))
            executeMethod(objects.at(indexOfObject(__CurrentObject)).getMethod(arg1));
        else
            error(METHOD_UNDEFINED, arg1, false);
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
                    error(METHOD_UNDEFINED, arg1, false);
            }
            else
                error(OBJ_METHOD_UNDEFINED, before, true);
        }
        else
        {
            if (methodExists(arg1))
                executeMethod(methods.at(indexOfMethod(arg1)));
            else
                error(METHOD_UNDEFINED, arg1, true);
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
            error(METHOD_DEFINED, arg1, false);
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
                                        error(IS_NULL, params.at(i), false);
                                }
                                else
                                    error(OBJ_VAR_UNDEFINED, after, false);
                            }
                            else
                                error(OBJ_METHOD_UNDEFINED, before, false);
                        }
                        else
                        {
                            if (isString(params.at(i)))
                                method.addMethodVariable(variables.at(indexOfVariable(params.at(i))).getString(), variables.at(indexOfVariable(params.at(i))).name());
                            else if (isNumber(params.at(i)))
                                method.addMethodVariable(variables.at(indexOfVariable(params.at(i))).getNumber(), variables.at(indexOfVariable(params.at(i))).name());
                            else
                                error(IS_NULL, params.at(i), false);
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
            error(METHOD_DEFINED, arg1, false);
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
                    error(OBJ_UNDEFINED, "", false);
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
                                        error(IS_NULL, params.at(i), false);
                                }
                                else
                                    error(OBJ_VAR_UNDEFINED, after, false);
                            }
                            else
                                error(OBJ_METHOD_UNDEFINED, before, false);
                        }
                        else
                        {
                            if (isString(params.at(i)))
                                method.addMethodVariable(variables.at(indexOfVariable(params.at(i))).getString(), variables.at(indexOfVariable(params.at(i))).name());
                            else if (isNumber(params.at(i)))
                                method.addMethodVariable(variables.at(indexOfVariable(params.at(i))).getNumber(), variables.at(indexOfVariable(params.at(i))).name());
                            else
                                error(IS_NULL, params.at(i), false);
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
            createFile(__SavedVars);
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
    else if (after == "noctis")
    {
        sValue = __Noctis;
    }
    else if (after == "os?")
    {
        sValue = getGuessedOS();
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
            if (objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getString() != __Null)
                text = (objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getString());
            else if (objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getNumber() != __NullNum)
                text = (dtos(objects.at(indexOfObject(beforeDot(arg1))).getVariable(afterDot(arg1)).getNumber()));
            else
            {
                error(IS_NULL, arg1, false);
                return;
            }
        }
        else
        {
            if (isString(arg1))
                text = (variables.at(indexOfVariable(arg1)).getString());
            else if (isNumber(arg1))
                text = (dtos(variables.at(indexOfVariable(arg1)).getNumber()));
            else
            {
                error(IS_NULL, arg1, false);
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
            cout << text << endl;
        }
        else
        {
            cout << text;
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
            saveVariable(arg1 + "&" + variables.at(indexOfVariable(arg1)).getString());
        else if (isNumber(arg1))
            saveVariable(arg1 + "&" + dtos(variables.at(indexOfVariable(arg1)).getNumber()));
        else
            error(IS_NULL, arg1, false);
    }
    else
        error(TARGET_UNDEFINED, arg1, false);
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
    c = start[0] == sc[0] ? sc[0] : (rand() % get_alpha_num(sc[0])) + start[0];
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
        error(READ_FAIL, path, false);

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

    __Null = "[null]";

    __ArgumentCount = 0,
    __NullNum = -DBL_MAX;

    if (contains(getEnvironmentVariable("HOMEPATH"), "Users"))
    {
        __GuessedOS = OS_WIN64;
        __SavedVarsPath = (getEnvironmentVariable("HOMEPATH") + "\\AppData") + "\\.__SavedVarsPath", __SavedVars = __SavedVarsPath + "\\.__SavedVars";
    }
    else if (contains(getEnvironmentVariable("HOMEPATH"), "Documents"))
    {
        __GuessedOS = OS_WIN32;
        __SavedVarsPath = getEnvironmentVariable("HOMEPATH") + "\\Application Data\\.__SavedVarsPath", __SavedVars = __SavedVarsPath + "\\.__SavedVars";
    }
    else if (startsWith(getEnvironmentVariable("HOME"), "/"))
    {
        __GuessedOS = OS_NIX;
        __SavedVarsPath = getEnvironmentVariable("HOME") + "/.__SavedVarsPath", __SavedVars = __SavedVarsPath + "/.__SavedVars";
    }
    else
    {
        __GuessedOS = OS_UNKNOWN;
        __SavedVarsPath = "\\.__SavedVarsPath", __SavedVars = __SavedVarsPath + "\\.__SavedVars";
    }
}

int main(int c, char ** v)
{
    string noctis = v[0];
    setup();
    __Noctis = noctis;
    __InitialDirectory = cwd();
    // __Logging = false;

#ifdef _WIN32
    SetConsoleTitle("noctis");
#endif

    srand((unsigned int)time(NULL));

    if (c == 1)
    {
        __CurrentScript = noctis;
        args.push_back(noctis);
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
            help(noctis);
        else if (is(opt, "u") || is(opt, "uninstall"))
            uninstall();
        else if (is(opt, "sl") || is(opt, "skipload"))
        {
            __CurrentScript = noctis;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loop(true);
        }
        else if (is(opt, "n") || is(opt, "negligence"))
        {
            __Negligence = true;
            __CurrentScript = noctis;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loop(true);
        }
        else if (is(opt, "v") || is(opt, "version"))
            displayVersion();
        else
        {
            __CurrentScript = noctis;
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
            __CurrentScript = noctis;

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
        else if (is(opt, "n") || is(opt, "negligence"))
        {
            __Negligence = true;
            args.push_back(opt);
            args.push_back(script);
            __ArgumentCount = (int)args.size();
            if (isScript(script))
            {
                __CurrentScript = script;
                loadScript(script);
            }
            else
            {
                __CurrentScript = noctis;
                loop(true);
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
                __CurrentScript = noctis;
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

            __CurrentScript = noctis;
            loop(false);
        }
    }
    else
        help(noctis);

    clearAll();

    return (0);
}
