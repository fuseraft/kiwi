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

Switch mainSwitchDef;

vector<List>        removeList(vector<List> v, string target);
vector<Variable>    removeVariable(vector<Variable> v, string target);
vector<Object>      removeObject(vector<Object> v, string target);
vector<Method>      removeMethod(vector<Method> v, string target);
vector<Constant>    removeConstant(vector<Constant> v, string target);

string  uslBinary,
        initDir,
        cleanString(string st),
        currentObject,
        currentMethodObject,
        currentScript,
        currentModule,
        promptStyle,
        errorVarName,
        goingTo,
        guessedOS,
        savedVarsPath,
        lastError,
        lastValue,
        logFile,
        null,
        parsedOutput,
        prevScript,
        savedVars,
        scriptName,
        currentLine,
        switchVarName,
        loopSymbol,
        shomp(string text),
        getParsedOutput(string cmd);

int argsLength,
    numOfBadMethods,
    numOfBadObjects,
    numOfBadVars,
    lineNum,
    numOfIF,
    numOfFOR,
    numOfWHILE,
    numOfParamVars;

double nullNum;

bool    captureParse,
        customPrompt,
        dontCollectMethodVars,
        failedIf,
        gotoMark,
        executedIF,
        inDefault,
        inIfDef,
        inForDef,
        inWhileDef,
        executedMethod,
        inModuleDef,
        inPrivateDef,
        inPublicDef,
        inScriptDef,
        inSwitchDef,
        executedTemplate,
        executedTry,
        leaving,
        logging,
        inMethodDef,
        negligent,
        failedNest,
        nesting,
        inObjectDef,
        inObjectMethodDef,
        inParamMethodDef,
        returning,
        skipCatch,
        skipDefault,
        skipToCatch,
        inMethodSwitchDef,
        inMethodWhileDef,
        inMethodForDef;

Method      getMethod(string s);
Object      getObject(string s);
Variable    getVariable(string s);
List        getDirectoryList(string before, bool filesOnly);

int methodAt(string s),
	objectAt(string s),
	variableAt(string s),
	listAt(string s),
    moduleAt(string s),
    constAt(string s),
    scriptAt(string s);

void    cd(string p),
        clearAll(),
        clearArgs(),
        clearFor(),
        clearIf(),
        clearMethods(),
        clearObjects(),
        clearVariables(),
        clearLists(),
        clearConstants(),
        clearWhile(),
        delay(int seconds),
        displayVersion(),
        error(string e, bool quit),
        failedIF(),
        failedFor(),
        failedWhile(),
        forLoop(Method m),
        help(string app),
        loadSavedVars(Crypt c, string &bs),
        loadScript(string script),
        loop(bool skip),
        executeMethod(Method m),
        executeNest(Container n),
        parse(string s),
        redefine(string target, string name),
        replaceElement(string before, string after, string replacement),
        saveVariable(string var),
        say(string st),
        setLastValue(string s),
        setVariable(string name, string value),
        setVariable(string name, double value),
        setup(),
        successfulIF(),
        successfulFor(),
        successfulFor(double a, double b, string op),
        successfullWhile(string v1, string op, string v2),
        successfulFor(List list),
        executeTemplate(Method m, vector<string> vs),
        uninstall(),
        whileLoop(Method m),
        __stdout(string st),
        __true(),
        __false();

void    zeroSpace(string argZero, string s, vector<string> command),
        oneSpace(string argZero, string argOne, string s, vector<string> command),
        twoSpace(string argZero, string argOne, string argTwo, string s, vector<string> command),
        threeSpace(string argZero, string argOne, string argTwo, string argThree, string s, vector<string> command);

void appendText(string argOne, string argTwo, bool newLine);
void __fwrite(string file, string contents);

bool is(string s, string si),
     isScript(string path),
     isString(string v_Name),
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
	 notStandardOneSpace(string argOne),
	 notStandardTwoSpace(string argOne);

double  getBytes(string path),
        getKBytes(string path),
        getMBytes(string path),
        getGBytes(string path),
        getTBytes(string path),
        getStack(string argTwo);

bool isStringStack(string argTwo);
bool stackReady(string argTwo);
string getStringStack(string argTwo);

void setLastValue(string s)
{
    lastValue = s;
}

void setList(string argOne, string argTwo, vector<string> params)
{
    if (methodExists(beforeParams(argTwo))) {
        executeTemplate(getMethod(beforeParams(argTwo)), params);

        if (containsParams(lastValue)) {
            vector<string> last_params = getParams(lastValue);

            for (int i = 0; i < (int)last_params.size(); i++)
                lists.at(listAt(argOne)).add(last_params.at(i));
        } else
            lists.at(listAt(argOne)).add(lastValue);
    } else if (objectExists(beforeDot(beforeParams(argTwo)))) {
        executeTemplate(objects.at(objectAt(beforeDot(beforeParams(argTwo)))).getMethod(afterDot(beforeParams(argTwo))), params);

        if (containsParams(lastValue)) {
            vector<string> last_params = getParams(lastValue);

            for (int i = 0; i < (int)last_params.size(); i++)
                lists.at(listAt(argOne)).add(last_params.at(i));
        } else
            lists.at(listAt(argOne)).add(lastValue);
    } else {
        for (int i = 0; i < (int)params.size(); i++) {
            if (variableExists(params.at(i))) {
                if (variables.at(variableAt(params.at(i))).getString() != null)
                    lists.at(listAt(argOne)).add(variables.at(variableAt(params.at(i))).getString());
                else if (variables.at(variableAt(params.at(i))).getNumber() != nullNum)
                    lists.at(listAt(argOne)).add(dtos(variables.at(variableAt(params.at(i))).getNumber()));
                else
                    error("is_null:" + params.at(i), false);
            } else
                lists.at(listAt(argOne)).add(params.at(i));
        }
    }
}

void setVariable(string name, string value)
{
    variables.at(variableAt(name)).setVariable(value);
    setLastValue(value);
}

void setVariable(string name, double value)
{
    if (variables.at(variableAt(name)).getString() != null)
        variables.at(variableAt(name)).setVariable(dtos(value));
    else if (variables.at(variableAt(name)).getNumber() != nullNum)
        variables.at(variableAt(name)).setVariable(value);
    else {
        if (variables.at(variableAt(name)).waiting())
            variables.at(variableAt(name)).stopWait();

        variables.at(variableAt(name)).setVariable(value);
    }

    setLastValue(dtos(value));
}

void createVariable(string name, string value) {
    Variable newVariable(name, value);

    if (executedTemplate || executedMethod || executedTry)
        newVariable.collect();
    else
        newVariable.dontCollect();

    variables.push_back(newVariable);
    setLastValue(value);
}

///	Creates a double type variable
void createVariable(string name, double value) {
	Variable newVariable(name, value);

    if (executedTemplate || executedMethod || executedTry)
        newVariable.collect();
    else
        newVariable.dontCollect();

    variables.push_back(newVariable);
    setLastValue(dtos(value));
}

void replaceElement(string before, string after, string replacement)
{
    vector<string> newList;

    for (int i = 0; i < (int)lists.at(listAt(before)).size(); i++) {
        if (i == stoi(after))
            newList.push_back(replacement);
        else
            newList.push_back(lists.at(listAt(before)).at(i));
    }

    lists.at(listAt(before)).clear();

    for (int i = 0; i < (int)newList.size(); i++)
        lists.at(listAt(before)).add(newList.at(i));

    newList.clear();
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;

    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }

    return elems;
}


std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;

    return split(s, delim, elems);
}

void appendText(string argOne, string argTwo, bool newLine)
{
	if (variableExists(argOne)) {
		if (variables.at(variableAt(argOne)).getString() != null) {
			if (fileExists(variables.at(variableAt(argOne)).getString())) {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getString() != null) {
						if (newLine)
							app(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argTwo)).getString() + "\r\n");
						else
							app(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argTwo)).getString());
					} else if (variables.at(variableAt(argTwo)).getNumber() != nullNum) {
						if (newLine)
							app(variables.at(variableAt(argOne)).getString(), dtos(variables.at(variableAt(argTwo)).getNumber()) + "\r\n");
						else
							app(variables.at(variableAt(argOne)).getString(), dtos(variables.at(variableAt(argTwo)).getNumber()));
					} else
						error("is_null:" + argTwo, false);
				} else {
					if (newLine)
						app(variables.at(variableAt(argOne)).getString(), argTwo + "\r\n");
					else
						app(variables.at(variableAt(argOne)).getString(), argTwo);
				}
			} else
				error("read_fail:" + variables.at(variableAt(argOne)).getString(), false);
		} else
			error("conversion_error:" + argOne, false);
	} else {
		if (variableExists(argTwo)) {
			if (variables.at(variableAt(argTwo)).getString() != null) {
				if (fileExists(argOne)) {
					if (newLine)
						app(argOne, variables.at(variableAt(argTwo)).getString() + "\r\n");
					else
						app(argOne, variables.at(variableAt(argTwo)).getString());
				} else
					error("read_fail:" + variables.at(variableAt(argTwo)).getString(), false);
			} else
				error("conversion_error:" + argTwo, false);
		} else {
			if (fileExists(argOne)) {
				if (newLine)
					app(argOne, argTwo + "\r\n");
				else
					app(argOne, argTwo);
			} else
				error("read_fail:" + argOne, false);
		}
	}
}

void __fwrite(string argOne, string argTwo)
{
	if (variableExists(argOne)) {
		if (variables.at(variableAt(argOne)).getString() != null) {
			if (fileExists(variables.at(variableAt(argOne)).getString())) {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getString() != null) {
						app(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argTwo)).getString() + "\r\n");
						lastValue = "0";
					} else if (variables.at(variableAt(argTwo)).getNumber() != nullNum) {
						app(variables.at(variableAt(argOne)).getString(), dtos(variables.at(variableAt(argTwo)).getNumber()) + "\r\n");
						lastValue = "0";
					} else {
						error("is_null:" + argTwo, false);
						lastValue = "-1";
					}
				} else {
					app(variables.at(variableAt(argOne)).getString(), argTwo + "\r\n");
					lastValue = "0";
				}
			} else {
			    touch(variables.at(variableAt(argOne)).getString());

                if (variables.at(variableAt(argTwo)).getString() != null) {
                    app(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argTwo)).getString() + "\r\n");
                    lastValue = "1";
                } else if (variables.at(variableAt(argTwo)).getNumber() != nullNum) {
                    app(variables.at(variableAt(argOne)).getString(), dtos(variables.at(variableAt(argTwo)).getNumber()) + "\r\n");
                    lastValue = "1";
                } else {
                    error("is_null:" + argTwo, false);
                    lastValue = "-1";
                }

			    lastValue = "1";
			}
		} else {
			error("conversion_error:" + argOne, false);
			lastValue = "-1";
		}
	} else {
		if (variableExists(argTwo)) {
			if (variables.at(variableAt(argTwo)).getString() != null) {
				if (fileExists(argOne)) {
					app(argOne, variables.at(variableAt(argTwo)).getString() + "\r\n");
					lastValue = "0";
				} else {
				    touch(variables.at(variableAt(argTwo)).getString());
					app(argOne, variables.at(variableAt(argTwo)).getString() + "\r\n");
				    lastValue = "1";
				}
			} else {
				error("conversion_error:" + argTwo, false);
				lastValue = "-1";
			}
		} else {
			if (fileExists(argOne)) {
				app(argOne, argTwo + "\r\n");
				lastValue = "0";
			} else {
			    touch(argOne);
			    app(argOne, argTwo + "\r\n");
			    lastValue = "1";
			}
		}
	}
}

string getPrompt()
{
    string new_style("");
    int length = promptStyle.length();
    char prevChar = 'a';

    for (int i = 0; i < length; i++) {
        switch (promptStyle[i]) {
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
                new_style.push_back(promptStyle[i]);
                break;
        }

        prevChar = promptStyle[i];
    }

    return (new_style);
}

void cd(string p)
{
	if (containsTilde(p)) {
		string cleaned("");
		int l = p.length();

		for (int i = 0; i < l; i++) {
			if (p[i] == '~') {
                if (guessedOS == "UNIXMacorLINUX")
				   cleaned.append(getEnvironmentVariable("HOME"));
                else if (guessedOS == "Win2000NTorXP")
                   cleaned.append(getEnvironmentVariable("HOMEPATH"));
                else if (guessedOS == "Win7orVista")
                   cleaned.append(getEnvironmentVariable("HOMEPATH"));
                else if (guessedOS == "UnknownWindowsOS")
                   cleaned.append(getEnvironmentVariable("HOME"));
                else
                   error("undefined_os", false);
			} else
				cleaned.push_back(p[i]);
		}

		if (directoryExists(cleaned))
			cd(cleaned);
		else
			error("read_fail:" + p, false);
	} else {
		if (p == "~") {
		    if (guessedOS == "UNIXMacorLINUX" || guessedOS == "UnknownWindowsOS")
                cd(getEnvironmentVariable("HOME"));
            else
                cd(getEnvironmentVariable("HOMEPATH"));
		} else if (p == "init_dir" || p == "initial_directory")
			cd(initDir);
		else {
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

	for (int i = 0; i < l; i++) {
	    if (buildSymbol) {
            if (st[i] == '}') {
	            builder = subtractChar(builder, "{");

	            if (variableExists(builder) && zeroDots(builder)) {
	                if (variables.at(variableAt(builder)).getString() != null)
                        cleaned.append(variables.at(variableAt(builder)).getString());
                    else if (variables.at(variableAt(builder)).getNumber() != nullNum)
                        cleaned.append(dtos(variables.at(variableAt(builder)).getNumber()));
                    else
                        cleaned.append("null");
	            } else if (methodExists(builder)) {
	                parse(builder);

	                cleaned.append(lastValue);
	            } else if (containsParams(builder)) {
	                if (stackReady(builder)) {
	                    if (isStringStack(builder))
	                        cleaned.append(getStringStack(builder));
	                    else
                            cleaned.append(dtos(getStack(builder)));
	                } else if (!zeroDots(builder)) {
	                    string before(beforeDot(builder)), after(afterDot(builder));

	                    if (objectExists(before)) {
	                        if (objects.at(objectAt(before)).methodExists(beforeParams(after))) {
	                            executeTemplate(objects.at(objectAt(before)).getMethod(beforeParams(after)), getParams(after));

	                            cleaned.append(lastValue);
	                        } else
                                error("invalid_operation:method_undefined:" + before + "." + beforeParams(after), false);
	                    } else
                            error("invalid_operation:object_undefined:" + before, false);
	                } else if (methodExists(beforeParams(builder))) {
	                    executeTemplate(methods.at(methodAt(beforeParams(builder))), getParams(builder));

	                    cleaned.append(lastValue);
	                } else
                        cleaned.append("null");
	            } else if (containsBrackets(builder)) {
	                string _beforeBrackets(beforeBrackets(builder)), afterBrackets(builder);
                    string rangeBegin(""), rangeEnd(""), _build("");

                    vector<string> listRange = getBracketRange(afterBrackets);

	                if (variableExists(_beforeBrackets)) {
	                    if (variables.at(variableAt(_beforeBrackets)).getString() != null) {
	                        string tempString(variables.at(variableAt(_beforeBrackets)).getString());

                            if (listRange.size() == 2) {
                                rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                                if (isNumeric(rangeBegin) && isNumeric(rangeEnd)) {
                                    if (stoi(rangeBegin) < stoi(rangeEnd)) {
                                        if ((int)tempString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                                            for (int z = stoi(rangeBegin); z <= stoi(rangeEnd); z++)
                                                _build.push_back(tempString[z]);

                                            cleaned.append(_build);
                                        } else
                                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                    } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
                                        if ((int)tempString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                                            for (int z = stoi(rangeBegin); z >= stoi(rangeEnd); z--)
                                                _build.push_back(tempString[z]);

                                            cleaned.append(_build);
                                        } else
                                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                    } else
                                        error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                                }
                            } else if (listRange.size() == 1) {
                                rangeBegin = listRange.at(0);

                                if (isNumeric(rangeBegin)) {
                                    if (stoi(rangeBegin) <= (int)tempString.length() - 1 && stoi(rangeBegin) >= 0) {
                                        string _cstr("");

                                        _cstr.push_back(tempString[stoi(rangeBegin)]);

                                        cleaned.append(_cstr);
                                    } else
                                        error("invalid_operation:index_out_of_bounds:" + afterBrackets, false);
                                } else
                                    error("invalid_operation:invalid_range:" + afterBrackets, false);
                            } else
                                error("invalid_operation:invalid_range:" + afterBrackets, false);
                        }
                    } else if (listExists(_beforeBrackets)) {
                        if (listRange.size() == 2) {
                            rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                            if (isNumeric(rangeBegin) && isNumeric(rangeEnd)) {
                                if (stoi(rangeBegin) < stoi(rangeEnd)) {
                                    if (lists.at(listAt(_beforeBrackets)).size() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                                        string bigString("(");

                                        for (int z = stoi(rangeBegin); z <= stoi(rangeEnd); z++) {
                                            bigString.append("\"" + lists.at(listAt(_beforeBrackets)).at(z) + "\"");

                                            if (z < stoi(rangeEnd))
                                                bigString.push_back(',');
                                        }

                                        bigString.push_back(')');

                                        cleaned.append(bigString);
                                    } else
                                        error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
                                    if (lists.at(listAt(_beforeBrackets)).size() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                                        string bigString("(");

                                        for (int z = stoi(rangeBegin); z >= stoi(rangeEnd); z--) {
                                            bigString.append("\"" + lists.at(listAt(_beforeBrackets)).at(z) + "\"");

                                            if (z > stoi(rangeEnd))
                                                bigString.push_back(',');
                                        }

                                        bigString.push_back(')');

                                        cleaned.append(bigString);
                                    } else
                                        error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                } else
                                    error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                            } else
                                error("invalid_operationg:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                        } else if (listRange.size() == 1) {
                            rangeBegin = listRange.at(0);

                            if (isNumeric(rangeBegin)) {
                                if (stoi(rangeBegin) <= (int)lists.at(listAt(_beforeBrackets)).size() - 1 && stoi(rangeBegin) >= 0)
                                    cleaned.append(lists.at(listAt(_beforeBrackets)).at(stoi(rangeBegin)));
                                else
                                    error("invalid_operation:index_out_of_bounds:" + afterBrackets, false);
                            } else
                                error("invalid_operation:invalid_range:" + afterBrackets, false);
                        } else
                            error("invalid_operation:invalid_range:" + afterBrackets, false);
                    } else
                        cleaned.append("null");
	            } else if (!zeroDots(builder)) {
                    string before(beforeDot(builder)), after(afterDot(builder));

                    if (objectExists(before)) {
                        if (objects.at(objectAt(before)).methodExists(after)) {
                            parse(before + "." + after);

                            cleaned.append(lastValue);
                        } else if (objects.at(objectAt(before)).variableExists(after)) {
                            if (objects.at(objectAt(before)).getVariable(after).getString() != null)
                                cleaned.append(objects.at(objectAt(before)).getVariable(after).getString());
                            else if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
                                cleaned.append(dtos(objects.at(objectAt(before)).getVariable(after).getNumber()));
                            else
                                cleaned.append("null");
                        } else
                            error("invalid_operation:variable_undefined:" + before + "." + after, false);
                    } else
                        error("invalid_operation:object_undefined:" + before, false);
                } else
                    cleaned.append(builder);

                builder.clear();

                buildSymbol = false;
            } else
                builder.push_back(st[i]);
        } else {
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
    if (captureParse)
        parsedOutput.append(cleanString(st));
    else
        cout << cleanString(st);

	if (logging)
        app(logFile, "[stdout]:" + st + "\r\n");
}

void say(string st)
{
	if (guessedOS == "UNIXMacorLINUX")
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
    if (executedTry) {
        skipToCatch = true;
        lastError = e + "(" + currentLine + ")";
    } else {
        if (captureParse)
            parsedOutput.append("#!=" + itos(lineNum) + ":" + e + "(" + currentLine + ")\r\n");
        else
            cerr << "#!=" << lineNum << ":" << e << "(" << currentLine << ")" << endl;
    }

	if (logging)
        app(logFile, "#!=" + itos(lineNum) + ":" + e + "(" + currentLine + ")\r\n");

	if (!negligent) {
		if (quit) {
			clearAll();
			exit(0);
		}
	}
}

string getParsedOutput(string cmd)
{
    captureParse = true;
    parse(cmd);
    string ret = parsedOutput;
    parsedOutput.clear();
    captureParse = false;

    return (ret);
}

List getDirectoryList(string before, bool filesOnly)
{
	List newList;
	int i = 1;

	DIR *pd;
	struct dirent *pe;

	string meat = variables.at(variableAt(before)).getString();

	if ((pd = opendir(meat.c_str())) == NULL)
		inForDef = false;
	else {
		while ((pe = readdir(pd)) != NULL) {
			if (string(pe->d_name) != "." && string(pe->d_name) != "..") {
				string tmp("");

				if (meat == "/")
					meat = "";

                if (guessedOS == "UNIXMacorLINUX")
                    tmp = meat + "/" + string(pe->d_name);
                else
                    tmp = meat + "\\" + string(pe->d_name);
					
				if (filesOnly) {
					if (fileExists(tmp)) {
						newList.add(tmp);
						i++;
					}
				} else {
					if (directoryExists(tmp)) {
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
	Method bad_meth("[bad_meth#" + itos(numOfBadMethods) + "]");

	if (methodExists(s))
		for (int i = 0; i < (int)methods.size(); i++)
			if (methods.at(i).name() == s)
				return (methods.at(i));

	numOfBadMethods++;
	return (bad_meth);
}

Object getObject(string s)
{
	Object bad_obj("[bad_obj#" + itos(numOfBadObjects) + "]");

	if (objectExists(s))
		for (int i = 0; i < (int)objects.size(); i++)
			if (objects.at(i).name() == s)
				return (objects.at(i));

	numOfBadObjects++;

	return (bad_obj);
}

Variable getVariable(string s)
{
	Variable bad_var("[bad_var#" + itos(numOfBadVars) + "]");

	if (variableExists(s))
		for (int i = 0; i < (int)variables.size(); i++)
			if (variables.at(i).name() == s)
				return (variables.at(i));

	numOfBadVars++;

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
    if (!zeroDots(s)) {
        if (objectExists(beforeDot(s))) {
            if (objects.at(objectAt(beforeDot(s))).methodExists(afterDot(s)))
                return (true);
            else
                return (false);
        }
    } else
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
    if (!zeroDots(s)) {
        string before(beforeDot(s)), after(afterDot(s));

        if (objectExists(before)) {
            if (objects.at(objectAt(before)).variableExists(after))
                return (true);
            else
                return (false);
        } else
            return (false);
    } else
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
    else {
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

	if (!fileExists(savedVars)) {
		if (!directoryExists(savedVarsPath))
			md(savedVarsPath);

		touch(savedVars);
		app(savedVars, c.e(variableName));
	} else {
		string line, bigStr("");
		ifstream file(savedVars.c_str());

		if (file.is_open()) {
			int i = 0;

			while (!file.eof()) {
				i++;
				getline(file, line);
				bigStr.append(line);
			}

			bigStr = c.d(bigStr);
			rm(savedVars);
			touch(savedVars);
			app(savedVars, c.e(bigStr + "#" + variableName));
			file.close();
		} else
			error("read_fail:" + savedVars, false);
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
		<< "\t" << app << " -n, --negligent\t// do not terminate on parse errors" << endl
		<< "\t" << app << " -sl, --skipload\t// start the shell, with fresh memory" << endl
		<< "\t" << app << " -l, --log {path}\t// create a shell log" << endl
		<< "\t" << app << " -u, --uninstall\t// remove $HOME/.savedVarsPath" << endl
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

int methodAt(string s)
{
	for (int i = 0; i < (int)methods.size(); i++) {
		if (methods.at(i).name() == s)
			return (i);
	}

	return (-1);
}

int objectAt(string s)
{
	for (int i = 0; i < (int)objects.size(); i++) {
		if (objects.at(i).name() == s)
			return (i);
	}

	return (-1);
}

int variableAt(string s)
{
	for (int i = 0; i < (int)variables.size(); i++) {
		if (variables.at(i).name() == s)
			return (i);
	}

	return (-1);
}

int listAt(string s)
{
	for (int i = 0; i < (int)lists.size(); i++) {
		if (lists.at(i).name() == s)
			return (i);
	}

	return (-1);
}

int moduleAt(string s)
{
	for (int i = 0; i < (int)modules.size(); i++) {
		if (modules.at(i).name() == s)
			return (i);
	}

	return (-1);
}

int scriptAt(string s)
{
	for (int i = 0; i < (int)scripts.size(); i++) {
		if (scripts.at(i).name() == s)
			return (i);
	}

	return (-1);
}

int constAt(string s)
{
	for (int i = 0; i < (int)constants.size(); i++) {
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
	ifstream file(savedVars.c_str());

	if (file.is_open()) {
		while (!file.eof()) {
			getline(file, line);
			bigStr.append(line);
		}

		file.close();

		bigStr = c.d(bigStr);

		int bigStrLength = bigStr.length();
		bool stop = false;
		vector<string> varNames;
		vector<string> varValues;

		string varName(""), vval("");
		varNames.push_back("");
		varValues.push_back("");

		for (int i = 0; i < bigStrLength; i++) {
			switch (bigStr[i]) {
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

		for (int i = 0; i < (int)varNames.size(); i++) {
			Variable newVariable(varNames.at(i), varValues.at(i));
			variables.push_back(newVariable);
		}

		varNames.clear();
		varValues.clear();
	}
	else
		error("read_fail:" + savedVars, false);
}

void runScript()
{
    for (int i = 0; i < scripts.at(scriptAt(currentScript)).size(); i++) {
        lineNum = i + 1;

        if (!gotoMark)
            parse(scripts.at(scriptAt(currentScript)).at(i));
        else {
            bool startParsing = false;
            inIfDef = false;
            inForDef = false;
            gotoMark = false;

            for (int z = 0; z < scripts.at(scriptAt(currentScript)).size(); z++) {
                if (endsWith(scripts.at(scriptAt(currentScript)).at(z), "::")) {
                    string s(scripts.at(scriptAt(currentScript)).at(z));
                    s = subtractString(s, "::");

                    if (s == goingTo)
                        startParsing = true;
                }

                if (startParsing)
                    parse(scripts.at(scriptAt(currentScript)).at(z));
            }
        }
    }

    currentScript = prevScript;
}

void loadScript(string script)
{
	string s("");
	ifstream f(script.c_str());
	currentScript = script;

	Script newScript(script);

	if (f.is_open()) {
		while (!f.eof()) {
			getline(f, s);

			if (s.length() > 0) {
				if (s[0] == '\r' || s[0] == '\n')
					doNothing();
				else if (s[0] == '\t') {
					s.erase(remove(s.begin(), s.end(), '\t'), s.end());
                    char * c = new char[s.size() + 1];
                    copy(s.begin(), s.end(), c);
                    c[s.size()] = '\0';
                    newScript.add(trim_leading_whitespace(c));
                    delete[] c;
				} else {
                    char * c = new char[s.size() + 1];
                    copy(s.begin(), s.end(), c);
                    c[s.size()] = '\0';
                    newScript.add(trim_leading_whitespace(c));
                    delete[] c;
                }
			} else
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

	if (!skip) {
		Crypt c;
		string bigStr("");

		if (fileExists(savedVars))
			loadSavedVars(c, bigStr);
	}

	while (active) {
		s.clear();

		if (customPrompt) {
            if (promptStyle == "bash")
                cout << getUser() << "@" << getMachine() << "(" << cwd() << ")" << "$ ";
            else if (promptStyle == "empty")
				doNothing();
            else
				cout << getPrompt();
        } else
            cout << "> ";

        getline(cin, s, '\n');

		if (s[0] == '\t')
			s.erase(remove(s.begin(), s.end(), '\t'), s.end());

        if (s == "exit") {
            if (!inObjectDef && !inMethodDef) {
                active = false;
                clearAll();
            } else
                parse(s);
        } else {
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
    for (int i = 0; i < m.size(); i++) {
        if (m.at(i) == "leave!")
            leaving = true;
        else
            parse(m.at(i));
    }
}

void forLoop(Method m)
{
    loopSymbol = "$";

    if (m.isListLoop()) {
	    int i = 0, stop = m.getList().size();

	    while (i < stop) {
			for (int z = 0; z < m.size(); z++) {
				string cleaned(""), builder("");
				int len = m.at(z).length();
                bool buildSymbol = false, almostBuild = false, ended = false;

				for (int a = 0; a < len; a++) {
				    if (almostBuild) {
				        if (m.at(z)[a] == '{')
                            buildSymbol = true;
				    }

				    if (buildSymbol) {
				        if (m.at(z)[a] == '}') {
				            almostBuild = false,
				            buildSymbol = false;
				            ended = true;

                            builder = subtractString(builder, "{");

				            if (builder == m.getSymbolString()) {
				                cleaned.append(m.getList().at(i));
							}

                            builder.clear();
				        } 
						else {
                            builder.push_back(m.at(z)[a]);
						}
				    }

					if (m.at(z)[a] == '$') {
                        almostBuild = true;
					}

                    if (!almostBuild && !buildSymbol) {
                        if (ended) {
                            ended = false;
                        } else {
                            cleaned.push_back(m.at(z)[a]);
						}
                    }
				}
				
                parse(cleaned);
			}

            i++;

			if (leaving == true) {
                leaving = false;
			    break;
            }
		}
	} else {
		if (m.isInfinite()) {
			if (negligent) {
				for (;;) {
					for (int z = 0; z < m.size(); z++)
                        parse(m.at(z));

    				if (leaving == true) {
                        leaving = false;
    				    break;
                    }
				}
			} else
				error("infinite_loop", true);
		} else if (m.start() < m.stop()) {
		    int start = m.start(), stop = m.stop();

		    while (start <= stop) {
				for (int z = 0; z < m.size(); z++) {
					string cleanString(""), builder(""), tmp(m.at(z));
					int l(tmp.length());
                    bool buildSymbol = false, almostBuild = false, ended = false;

					for (int a = 0; a < l; a++) {
                        if (almostBuild) {
                            if (tmp[a] == '{')
                                buildSymbol = true;
                        }

                        if (buildSymbol) {
                            if (tmp[a] == '}') {
                                almostBuild = false,
                                buildSymbol = false;
                                ended = true;

                                builder = subtractString(builder, "{");

                                if (builder == m.getSymbolString())
                                    cleanString.append(itos(start));

                                builder.clear();
                            } else
                                builder.push_back(tmp[a]);
                        }

                        if (tmp[a] == '$')
                            almostBuild = true;

                        if (!almostBuild && !buildSymbol) {
                            if (ended)
                                ended = false;
                            else
                                cleanString.push_back(tmp[a]);
                        }
					}

					parse(cleanString);
				}

                start++;

				if (leaving == true) {
                    leaving = false;
				    break;
                }
			}
		} else if (m.start() > m.stop()) {
		    int start = m.start(), stop = m.stop();

		    while (start >= stop) {
				for (int z = 0; z < m.size(); z++) {
					string cleaned(""), builder(""), tmp(m.at(z));
					int l(tmp.length());
                    bool buildSymbol = false, almostBuild = false, ended = false;

					for (int a = 0; a < l; a++) {
                        if (almostBuild) {
                            if (tmp[a] == '{')
                                buildSymbol = true;
                        }

                        if (buildSymbol) {
                            if (tmp[a] == '}') {
                                almostBuild = false,
                                buildSymbol = false;
                                ended = true;

                                builder = subtractString(builder, "{");

                                if (builder == m.getSymbolString())
                                    cleaned.append(itos(start));

                                builder.clear();
                            } else
                                builder.push_back(tmp[a]);
                        }

                        if (tmp[a] == '$')
                            almostBuild = true;

                        if (!almostBuild && !buildSymbol) {
                            if (ended)
                                ended = false;
                            else
                                cleaned.push_back(tmp[a]);
                        }
					}

                    parse(cleaned);
				}

				start--;

				if (leaving == true) {
                    leaving = false;
				    break;
                }
			}
		} else
			error("special_error(5)", false);
	}
}

void executeNest(Container n)
{
	nesting = false;
	inIfDef = false;

	for (int i = 0; i < n.size(); i++) {
        if (failedNest == false)
            parse(n.at(i));
        else
            break;
    }

    inIfDef = true;
}

void collectGarbage()
{
    vector<string> garbageVars;

    for (int i = 0; i < (int)variables.size(); i++)
        if (variables.at(i).garbage() && !executedIF)
            if (!dontCollectMethodVars)
                garbageVars.push_back(variables.at(i).name());

    for (int i = 0; i < (int)garbageVars.size(); i++)
        variables = removeVariable(variables, garbageVars.at(i));

    vector<string> garbageLists;

    for (int i = 0; i < (int)lists.size(); i++)
        if (lists.at(i).garbage() && !executedIF)
            garbageLists.push_back(lists.at(i).name());

    for (int i = 0; i < (int)garbageLists.size(); i++)
        lists = removeList(lists, garbageLists.at(i));

    vector<string> garbageObjects;

    for (int i = 0; i < (int)objects.size(); i++)
        if (objects.at(i).garbage() && !executedIF)
            garbageObjects.push_back(objects.at(i).name());

    for (int i = 0; i < (int)garbageObjects.size(); i++)
        objects = removeObject(objects, garbageObjects.at(i));
}

void executeTemplate(Method m, vector<string> strings)
{
    vector<string> methodLines;

    executedTemplate = true;
    dontCollectMethodVars = true;
    currentMethodObject = m.getObject();

    vector<Variable> methodVariables = m.getMethodVariables();

    for (int i = 0; i < (int)methodVariables.size(); i++) {
        if (variableExists(strings.at(i))) {
            if (variables.at(variableAt(strings.at(i))).getString() != null)
                createVariable(methodVariables.at(i).name(), variables.at(variableAt(strings.at(i))).getString());
            else if (variables.at(variableAt(strings.at(i))).getNumber() != nullNum)
                createVariable(methodVariables.at(i).name(), variables.at(variableAt(strings.at(i))).getNumber());
        } else if (methodExists(strings.at(i))) {
            parse(strings.at(i));

            if (isNumeric(lastValue))
                createVariable(methodVariables.at(i).name(), stod(lastValue));
            else
                createVariable(methodVariables.at(i).name(), lastValue);
        } else {
            if (isNumeric(strings.at(i)))
                createVariable(methodVariables.at(i).name(), stod(strings.at(i)));
            else
                createVariable(methodVariables.at(i).name(), strings.at(i));
        }
    }

    for (int i = 0; i < (int)m.size(); i++) {
        string line = m.at(i), word("");
        int len = line.length();
        vector<string> words;

        for (int x = 0; x < len; x++) {
            if (line[x] == ' ') {
                words.push_back(word);
                word.clear();
            } else
                word.push_back(line[x]);
        }

        words.push_back(word);

        vector<string> newWords;

        for (int x = 0; x < (int)words.size(); x++) {
            bool found = false;

            for (int a = 0; a < (int)strings.size(); a++) {
                string variableString("$");
                variableString.append(itos(a));

                if (words.at(x) == variableString) {
                    found = true;

                    newWords.push_back(strings.at(a));
                }
            }

            if (!found)
                newWords.push_back(words.at(x));
        }

        string freshLine("");

        for (int b = 0; b < (int)newWords.size(); b++) {
            freshLine.append(newWords.at(b));

            if (b != (int)newWords.size() - 1)
                freshLine.push_back(' ');
        }

        methodLines.push_back(freshLine);
    }

    for (int i = 0; i < (int)methodLines.size(); i++)
        parse(methodLines.at(i));

    executedTemplate = false, dontCollectMethodVars = false;

	collectGarbage(); // if (!dontCollectMethodVars)
}

void executeMethod(Method m)
{
    executedMethod = true;
    currentMethodObject = m.getObject();

    if (inParamMethodDef) {
        vector<string> methodLines;

        for (int i = 0; i < (int)m.size(); i++) {
            string line = m.at(i), word("");
            int len = line.length();
            vector<string> words;

            for (int x = 0; x < len; x++) {
                if (line[x] == ' ') {
                    words.push_back(word);
                    word.clear();
                } else
                    word.push_back(line[x]);
            }

            words.push_back(word);

            vector<string> newWords;

            for (int x = 0; x < (int)words.size(); x++) {
                bool found = false;

                for (int a = 0; a < (int)m.getMethodVariables().size(); a++) {
                    string variableString("$");
                    variableString.append(itos(a));

                    if (words.at(x) == m.getMethodVariables().at(a).name()) {
                        found = true;

                        if (m.getMethodVariables().at(a).getString() != null)
                            newWords.push_back(m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getNumber() != nullNum)
                            newWords.push_back(dtos(m.getMethodVariables().at(a).getNumber()));
                    } else if (words.at(x) == variableString) {
                        found = true;

                        if (m.getMethodVariables().at(a).getString() != null)
                            newWords.push_back(m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getNumber() != nullNum)
                            newWords.push_back(dtos(m.getMethodVariables().at(a).getNumber()));
                    }
                }

                if (!found)
                    newWords.push_back(words.at(x));
            }

            string freshLine("");

            for (int b = 0; b < (int)newWords.size(); b++) {
                freshLine.append(newWords.at(b));

                if (b != (int)newWords.size() - 1)
                    freshLine.push_back(' ');
            }

            methodLines.push_back(freshLine);
        }

        for (int i = 0; i < (int)methodLines.size(); i++)
            parse(methodLines.at(i));
    } else
    	for (int i = 0; i < m.size(); i++)
    		parse(m.at(i));

    executedMethod = false;

	collectGarbage();
}

bool suc_stat = false;

bool success()
{
    return (suc_stat);
}

void failedFor()
{
	Method forMethod("[for#" + itos(numOfFOR) + "]");
	forMethod.setFor(false);
	inForDef = true;
	forLoops.push_back(forMethod);
	loopSymbol = "$";
	suc_stat = false;
}

void failedWhile()
{
	Method whileMethod("[while#" + itos(numOfWHILE) + "]");
	whileMethod.setWhile(false);
	inWhileDef = true;
	whileLoops.push_back(whileMethod);
}

void successfullWhile(string v1, string op, string v2)
{
	Method whileMethod("[while#" + itos(numOfWHILE) + "]");
	whileMethod.setWhile(true);
	whileMethod.setWhileValues(v1, op, v2);
	inWhileDef = true;
	whileLoops.push_back(whileMethod);
	numOfWHILE++;
}

void successfulFor(List list)
{
	Method forMethod("[for#" + itos(numOfFOR) + "]");
	forMethod.setFor(true);
	forMethod.setForList(list);
	forMethod.setListLoop();
	forMethod.setSymbol(loopSymbol);
	inForDef = true;
	forLoops.push_back(forMethod);
	numOfFOR++;
	suc_stat = true;
}

void successfulFor(double a, double b, string op)
{
	Method forMethod("[for#" + itos(numOfFOR) + "]");
	forMethod.setFor(true);
	forMethod.setSymbol(loopSymbol);

	if (op == "<=")
        forMethod.setForValues((int)a, (int)b);
	else if (op == ">=")
        forMethod.setForValues((int)a, (int)b);
    else if (op == "<")
        forMethod.setForValues((int)a, (int)b - 1);
    else if (op == ">")
        forMethod.setForValues((int)a, (int)b + 1);

	inForDef = true;
	forLoops.push_back(forMethod);
	numOfFOR++;
	suc_stat = true;
}

void successfulFor()
{
	Method forMethod("[for#" + itos(numOfFOR) + "]");
	forMethod.setFor(true);
	forMethod.setInfinite();
	inForDef = true;
	forLoops.push_back(forMethod);
	numOfFOR++;
	suc_stat = true;
}

void failedIF()
{
    lastValue = "false";

	if (!nesting) {
		Method ifMethod("[failif]");
		ifMethod.setBool(false);
		inIfDef = true;
		ifStatements.push_back(ifMethod);
		failedIf = true;
		failedNest = true;
	}
	else
        failedNest = true;
}

void successfulIF()
{
    lastValue = "true";

	if (nesting) {
		ifStatements.at((int)ifStatements.size() - 1).buildNest();
		failedNest = false;
	} else {
		Method ifMethod("[if#" + itos(numOfIF) +"]");
		ifMethod.setBool(true);
		inIfDef = true;
		ifStatements.push_back(ifMethod);
		numOfIF++;
		failedIf = false;
		failedNest = false;
	}
}

bool stackReady(string argTwo)
{
    if (contains(argTwo, "+") || contains(argTwo, "-") || contains(argTwo, "*") || contains(argTwo, "/") || contains(argTwo, "%") || contains(argTwo, "^"))
        return (true);

    return (false);
}

bool isStringStack(string argTwo)
{
    string tempArgTwo = argTwo, temporaryBuild("");
    tempArgTwo = subtractChar(tempArgTwo, "(");
    tempArgTwo = subtractChar(tempArgTwo, ")");

    for (int i = 0; i < (int)tempArgTwo.length(); i++) {
        if (tempArgTwo[i] == ' ') {
            if (temporaryBuild.length() != 0) {
                if (variableExists(temporaryBuild)) {
                    if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum)
                        temporaryBuild.clear();
                    else if (variables.at(variableAt(temporaryBuild)).getString() != null)
                        return (true);
                } else if (methodExists(temporaryBuild)) {
                    parse(temporaryBuild);

                    if (isNumeric(lastValue))
                        temporaryBuild.clear();
                    else
                        return (true);
                } else
                    temporaryBuild.clear();
            }
        } else if (tempArgTwo[i] == '+') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum)
                    temporaryBuild.clear();
                else if (variables.at(variableAt(temporaryBuild)).getString() != null)
                    return (true);
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            } else if (!isNumeric(temporaryBuild))
                return (true);
            else
                temporaryBuild.clear();
        } else if (tempArgTwo[i] == '-') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum)
                    temporaryBuild.clear();
                else if (variables.at(variableAt(temporaryBuild)).getString() != null)
                    return (true);
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            } else if (!isNumeric(temporaryBuild))
                return (true);
            else
                temporaryBuild.clear();
        } else if (tempArgTwo[i] == '*') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum)
                    temporaryBuild.clear();
                else if (variables.at(variableAt(temporaryBuild)).getString() != null)
                    return (true);
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            } else if (!isNumeric(temporaryBuild))
                return (true);
            else
                temporaryBuild.clear();
        } else if (tempArgTwo[i] == '/') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum)
                    temporaryBuild.clear();
                else if (variables.at(variableAt(temporaryBuild)).getString() != null)
                    return (true);
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            } else if (!isNumeric(temporaryBuild))
                return (true);
            else
                temporaryBuild.clear();
        } else if (tempArgTwo[i] == '%') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum)
                    temporaryBuild.clear();
                else if (variables.at(variableAt(temporaryBuild)).getString() != null)
                    return (true);
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            } else if (!isNumeric(temporaryBuild))
                return (true);
            else
                temporaryBuild.clear();
        } else if (tempArgTwo[i] == '^') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum)
                    temporaryBuild.clear();
                else if (variables.at(variableAt(temporaryBuild)).getString() != null)
                    return (true);
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue))
                    temporaryBuild.clear();
                else
                    return (true);
            } else if (!isNumeric(temporaryBuild))
                return (true);
            else
                temporaryBuild.clear();
        } else
            temporaryBuild.push_back(tempArgTwo[i]);
    }

    return (false);
}

string getStringStack(string argTwo)
{
    string tempArgTwo = argTwo, temporaryBuild("");
    tempArgTwo = subtractChar(tempArgTwo, "(");
    tempArgTwo = subtractChar(tempArgTwo, ")");

    string stackValue("");

    vector<string> vars;
    vector<string> contents;

    for (int i = 0; i < (int)tempArgTwo.length(); i++) {
        if (tempArgTwo[i] == ' ') {
            if (temporaryBuild.length() != 0) {
                if (variableExists(temporaryBuild)) {
                    if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                        vars.push_back(temporaryBuild);
                        contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                        temporaryBuild.clear();
                    } else if (variables.at(variableAt(temporaryBuild)).getString() != null) {
                        vars.push_back(temporaryBuild);
                        contents.push_back(variables.at(variableAt(temporaryBuild)).getString());
                        temporaryBuild.clear();
                    }
                } else if (methodExists(temporaryBuild)) {
                    parse(temporaryBuild);

                    contents.push_back(lastValue);
                    temporaryBuild.clear();
                } else {
                    contents.push_back(temporaryBuild);
                    temporaryBuild.clear();
                }
            }
        } else if (tempArgTwo[i] == '+') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("+");
                } else if (variables.at(variableAt(temporaryBuild)).getString() != null) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(variables.at(variableAt(temporaryBuild)).getString());
                    temporaryBuild.clear();
                    contents.push_back("+");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                contents.push_back(lastValue);
                temporaryBuild.clear();

                contents.push_back("+");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("+");
            }
        } else if (tempArgTwo[i] == '-') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("-");
                } else if (variables.at(variableAt(temporaryBuild)).getString() != null) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(variables.at(variableAt(temporaryBuild)).getString());
                    temporaryBuild.clear();
                    contents.push_back("-");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                contents.push_back(lastValue);
                temporaryBuild.clear();

                contents.push_back("-");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("-");
            }
        } else if (tempArgTwo[i] == '*') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("*");
                } else if (variables.at(variableAt(temporaryBuild)).getString() != null) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(variables.at(variableAt(temporaryBuild)).getString());
                    temporaryBuild.clear();
                    contents.push_back("*");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                contents.push_back(lastValue);
                temporaryBuild.clear();

                contents.push_back("*");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("*");
            }
        } else
            temporaryBuild.push_back(tempArgTwo[i]);
    }

    if (variableExists(temporaryBuild)) {
        if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
            vars.push_back(temporaryBuild);
            contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
            temporaryBuild.clear();
        } else if (variables.at(variableAt(temporaryBuild)).getString() != null) {
            vars.push_back(temporaryBuild);
            contents.push_back(variables.at(variableAt(temporaryBuild)).getString());
            temporaryBuild.clear();
        }
    } else {
        contents.push_back(temporaryBuild);
        temporaryBuild.clear();
    }

    bool startOperating = false,
        addNext = false,
        subtractNext = false,
        multiplyNext = false;

    for (int i = 0; i < (int)contents.size(); i++) {
        if (startOperating) {
            if (addNext) {
                stackValue.append(contents.at(i));
                addNext = false;
            } else if (subtractNext) {
                stackValue = subtractString(stackValue, contents.at(i));
                subtractNext = false;
            } else if (multiplyNext) {
                if (isNumeric(contents.at(i))) {
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
        } else {
            startOperating = true;
            stackValue = contents.at(i);
        }
    }

    if (returning) {
        for (int i = 0; i < (int)vars.size(); i++)
            variables = removeVariable(variables, vars.at(i));

        returning = false;
    }

    return (stackValue);
}

double getStack(string argTwo)
{
    string tempArgTwo = argTwo, temporaryBuild("");
    tempArgTwo = subtractChar(tempArgTwo, "(");
    tempArgTwo = subtractChar(tempArgTwo, ")");

    double stackValue = (double)0.0;

    vector<string> contents;
    vector<string> vars;

    for (int i = 0; i < (int)tempArgTwo.length(); i++) {
        if (tempArgTwo[i] == ' ') {
            if (temporaryBuild.length() != 0) {
                if (variableExists(temporaryBuild)) {
                    if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                        vars.push_back(temporaryBuild);
                        contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                        temporaryBuild.clear();
                    }
                } else if (methodExists(temporaryBuild)) {
                    parse(temporaryBuild);

                    if (isNumeric(lastValue)) {
                        contents.push_back(lastValue);
                        temporaryBuild.clear();
                    }
                } else {
                    contents.push_back(temporaryBuild);
                    temporaryBuild.clear();
                }
            }
        } else if (tempArgTwo[i] == '+') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("+");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue)) {
                    contents.push_back(lastValue);
                    temporaryBuild.clear();
                }

                contents.push_back("+");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("+");
            }
        } else if (tempArgTwo[i] == '-') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("-");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue)) {
                    contents.push_back(lastValue);
                    temporaryBuild.clear();
                } contents.push_back("-");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("-");
            }
        } else if (tempArgTwo[i] == '*') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("*");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue)) {
                    contents.push_back(lastValue);
                    temporaryBuild.clear();
                }

                contents.push_back("*");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("*");
            }
        } else if (tempArgTwo[i] == '/') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("/");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue)) {
                    contents.push_back(lastValue);
                    temporaryBuild.clear();
                }

                contents.push_back("/");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("/");
            }
        } else if (tempArgTwo[i] == '%') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("%");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue)) {
                    contents.push_back(lastValue);
                    temporaryBuild.clear();
                }
                contents.push_back("%");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("%");
            }
        } else if (tempArgTwo[i] == '^') {
            if (variableExists(temporaryBuild)) {
                if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
                    vars.push_back(temporaryBuild);
                    contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
                    temporaryBuild.clear();
                    contents.push_back("^");
                }
            } else if (methodExists(temporaryBuild)) {
                parse(temporaryBuild);

                if (isNumeric(lastValue)) {
                    contents.push_back(lastValue);
                    temporaryBuild.clear();
                }
                contents.push_back("^");
            } else {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
                contents.push_back("^");
            }
        } else
            temporaryBuild.push_back(tempArgTwo[i]);
    }

    if (variableExists(temporaryBuild)) {
        if (variables.at(variableAt(temporaryBuild)).getNumber() != nullNum) {
            vars.push_back(temporaryBuild);
            contents.push_back(dtos(variables.at(variableAt(temporaryBuild)).getNumber()));
            temporaryBuild.clear();
        }
    } else {
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

    for (int i = 0; i < (int)contents.size(); i++) {
        if (startOperating) {
            if (addNext) {
                stackValue += stod(contents.at(i));
                addNext = false;
            } else if (subtractNext) {
                stackValue -= stod(contents.at(i));
                subtractNext = false;
            } else if (multiplyNext) {
                stackValue *= stod(contents.at(i));
                multiplyNext = false;
            } else if (divideNext) {
                stackValue /= stod(contents.at(i));
                divideNext = false;
            } else if (moduloNext) {
                stackValue = ((int)stackValue % (int)stod(contents.at(i)));
                moduloNext = false;
            } else if (powerNext) {
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
        } else {
            if (isNumeric(contents.at(i))) {
                startOperating = true;
                stackValue = stod(contents.at(i));
            }
        }
    }

    if (returning) {
        for (int i = 0; i < (int)vars.size(); i++)
            variables = removeVariable(variables, vars.at(i));

        returning = false;
    }

    return (stackValue);
}

bool commented, multiLineComment;

int sysExec(string s, vector<string> command)
{
//    string _cleaned;
//	_cleaned = cleanString(s);
    for (int i = 0; i < (int)methods.size(); i++) {
        if (command.at(0) == methods.at(i).name()) {
            if ((int)command.size() - 1 == (int)methods.at(i).getMethodVariables().size()) {
				// work
			}
		}
	}
	return system(cleanString(s).c_str());
}

/**
	The heart of it all. Parse a string and send for interpretation.
**/
void parse(string s) {
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

    currentLine = s; // store a copy of the current line
    if (logging) app(logFile, s + "\r\n"); // if logging a session, log the line

	command.push_back(""); // push back an empty string to begin.
	// iterate each char in the initial string
	for (int i = 0; i < length; i++) {
		switch (s[i]) {
			case ' ':
				/**
					we can push a space onto the string if:
						parsing a string literal AND not within parentheses AND not in comment mode
				**/
				if (quoted && !parenthesis) {
					if (!commented)
						command.at(count).push_back(' ');
				} else if (parenthesis && !quoted)
                    doNothing();
                else if (parenthesis && quoted) {
                    if (!commented)
                        command.at(count).push_back(' ');
                } else {
					if (!commented) {
					    if (prevChar != ' ') {
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
				if (quoted || parenthesis) {
					if (!commented)
						command.at(count).push_back('\\');
				}

				bigString.push_back('\\');
                break;

			case '\'':
				if (quoted || parenthesis) {
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
                else if (prevChar == '#' && multiLineComment == false) {
                    multiLineComment = true;
                    commented = true;
                    uncomment = false;
                } else if (prevChar == '#' && multiLineComment == true)
                    uncomment = true;
                else if (prevChar != '#' && multiLineComment == false) {
                    commented = true;
                    uncomment = true;
                }

				bigString.push_back('#');
                break;

			case '~':
				if (!commented) {
					if (prevChar == '\\')
						command.at(count).push_back('~');
					else {
					    if (guessedOS == "UNIXMacorLinux")
                            command.at(count).append(getEnvironmentVariable("HOME"));
                        else
                            command.at(count).append(getEnvironmentVariable("HOMEPATH"));
					}
				}
				bigString.push_back('~');
                break;

			case ';':
				if (!quoted) {
					if (!commented) {
						broken = true;
						stringContainer.add(bigString);
						bigString = "";
						count = 0;
						command.clear();
						command.push_back("");
					}
				} else {
					bigString.push_back(';');
					command.at(count).push_back(';');
				}
                break;

			default:
				if (!commented)
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
	
	if (!commented) {
    	if (!broken) {
    		for (int i = 0; i < size; i++) {
				// handle arguments
				// args[0], args[1], ..., args[n-1]
    			if (contains(command.at(i), "args") && command.at(i) != "args.size") {
    				vector<string> params = getBracketRange(command.at(i));

                    if (isNumeric(params.at(0))) {
                        if ((int)args.size() - 1 >= stoi(params.at(0)) && stoi(params.at(0)) >= 0) {
                            if (params.at(0) == "0")
                                command.at(i) = currentScript;
                            else
                                command.at(i) = args.at(stoi(params.at(0)));
                        } else
                            error("invalid_operation:index_out_of_bounds:" + command.at(i), false);
                    } else
                        error("invalid_operation:invalid_range:" + command.at(i), false);
    			}
    		}

            if (inSwitchDef) {
                if (s == "{")
                    doNothing();
                else if (startsWith(s, "case"))
                    mainSwitchDef.addCase(command.at(1));
                else if (s == "default")
                    inDefault = true;
                else if (s == "end" || s == "}") {
                    string switch_value("");

                    if (variables.at(variableAt(switchVarName)).getString() != null)
                        switch_value = variables.at(variableAt(switchVarName)).getString();
                    else if (variables.at(variableAt(switchVarName)).getNumber() != nullNum)
                        switch_value = dtos(variables.at(variableAt(switchVarName)).getNumber());
                    else
                        switch_value = "";

                    Container rightCase = mainSwitchDef.rightCase(switch_value);

                    inDefault = false;
                    inSwitchDef = false;

                    for (int i = 0; i < (int)rightCase.size(); i++)
                        parse(rightCase.at(i));

                    mainSwitchDef.clear();
                } else {
                    if (inDefault)
                        mainSwitchDef.addToDefault(s);
                    else
                        mainSwitchDef.addToCase(s);
                }
            } else if (inModuleDef) {
                if (s == ("[/" + currentModule + "]")) {
                    inModuleDef = false;
                    currentModule = "";
                } else
                    modules.at(moduleAt(currentModule)).add(s);
            } else if (inScriptDef) {
    			if (s == "__end__") {
    				scriptName = "";
    				inScriptDef = false;
    			} else
    				app(scriptName, s + "\n");
    		} else {
    			if (skipToCatch) {
    			    if (s == "catch")
                        skipToCatch = false;
    			} else if (executedTry && s == "catch")
    			    skipCatch = true;
    			else if (executedTry && skipCatch) {
    			    if (s == "caught") {
    			        skipCatch = false;
    			        parse("caught");
    			    }
    			} else if (inMethodDef) {
    			    if (contains(s, "while"))
                        inMethodWhileDef = true;

                    if (contains(s, "switch"))
                        inMethodSwitchDef = true;

                    if (inParamMethodDef) {
                        if (s == "{")
							doNothing();
                        else if (s == "end" || s == "}") {
                            if (inMethodWhileDef) {
                                inMethodWhileDef = false;

                                if (inObjectDef)
                                    objects.at(objectAt(currentObject)).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            } else if (inMethodSwitchDef) {
                                inMethodSwitchDef = false;

                                if (inObjectDef)
                                    objects.at(objectAt(currentObject)).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            } else {
                                inMethodDef = false;

                                if (inObjectDef) {
                                    inObjectMethodDef = false;
                                    objects.at(objects.size() - 1).setCurrentMethod("");
                                }
                            }
                        } else {
                            int _len = s.length();
                            vector<string> words;
                            string word("");

                            for (int z = 0; z < _len; z++) {
                                if (s[z] == ' ') {
                                    words.push_back(word);
                                    word.clear();
                                } else
                                    word.push_back(s[z]);
                            }

                            words.push_back(word);

                            string freshLine("");

                            for (int z = 0; z < (int)words.size(); z++) {
                                if (variableExists(words.at(z))) {
                                    if (variables.at(variableAt(words.at(z))).getString() != null)
                                        freshLine.append(variables.at(variableAt(words.at(z))).getString());
                                    else if (variables.at(variableAt(words.at(z))).getNumber() != nullNum)
                                        freshLine.append(dtos(variables.at(variableAt(words.at(z))).getNumber()));
                                } else
                                    freshLine.append(words.at(z));

                                if (z != (int)words.size() - 1)
                                    freshLine.push_back(' ');
                            }

                            if (inObjectDef) {
                                objects.at(objectAt(currentObject)).addToCurrentMethod(freshLine);

                                if (inPublicDef)
                                    objects.at(objectAt(currentObject)).setPublic();
                                else if (inPrivateDef)
                                    objects.at(objectAt(currentObject)).setPrivate();
                                else
                                    objects.at(objectAt(currentObject)).setPublic();
                            } else
                                methods.at(methods.size() - 1).add(freshLine);
                        }
                    } else {
                        if (s == "{")
                            doNothing();
        				else if (s == "end" || s == "}") {
        				    if (inMethodWhileDef) {
        				        inMethodWhileDef = false;

        				        if (inObjectDef)
                                    objects.at(objects.size() - 1).addToCurrentMethod(s);
        				        else
                                    methods.at(methods.size() - 1).add(s);
        				    } else if (inMethodSwitchDef) {
        				        inMethodSwitchDef = false;

        				        if (inObjectDef)
                                    objects.at(objects.size() - 1).addToCurrentMethod(s);
        				        else
                                    methods.at(methods.size() - 1).add(s);
        				    } else {
                                inMethodDef = false;

                                if (inObjectDef) {
                                    inObjectMethodDef = false;
                                    objects.at(objects.size() - 1).setCurrentMethod("");
                                }
        				    }
        				} else {
        					if (inObjectDef) {
        						objects.at(objects.size() - 1).addToCurrentMethod(s);

        						if (inPublicDef)
                                    objects.at(objects.size() - 1).setPublic();
                                else if (inPrivateDef)
                                    objects.at(objects.size() - 1).setPrivate();
                                else
                                    objects.at(objects.size() - 1).setPublic();
        					} else {
        						if (inObjectMethodDef) {
        							objects.at(objects.size() - 1).addToCurrentMethod(s);

            						if (inPublicDef)
                                        objects.at(objects.size() - 1).setPublic();
                                    else if (inPrivateDef)
                                        objects.at(objects.size() - 1).setPrivate();
                                    else
                                        objects.at(objects.size() - 1).setPublic();
        						} else
                                    methods.at(methods.size() - 1).add(s);
        					}
        				}
                    }
    			} else if (inIfDef) {
    				if (nesting) {
    					if (command.at(0) == "endif")
    						executeNest(ifStatements.at((int)ifStatements.size() - 1).getNest());
                        else
    						ifStatements.at((int)ifStatements.size() - 1).inNest(s);
    				} else {
    					if (command.at(0) == "if") {
    						nesting = true;

    						if (size == 4)
    							threeSpace("if", command.at(1), command.at(2), command.at(3), s, command);
    						else {
    							failedIF();
    							nesting = false;
    						}
    					} else if (command.at(0) == "endif") {
    						inIfDef = false;
    						executedIF = true;

    						for (int i = 0; i < (int)ifStatements.size(); i++) {
    							if (ifStatements.at(i).isIF()) {
    								executeMethod(ifStatements.at(i));

    								if (failedIf == false)
                                        break;
                                }
    						}

    						executedIF = false;

    						ifStatements.clear();

    						numOfIF = 0;
    						failedIf = false;
    					} else if (command.at(0) == "elsif" || command.at(0) == "elif") {
    						if (size == 4)
    							threeSpace("if", command.at(1), command.at(2), command.at(3), s, command);
                            else
    							failedIF();
    					} else if (s == "else")
                            threeSpace("if", "true", "is", "true", "if true is true", command);
    					else if (s == "failif") {
    						if (failedIf == true)
    							successfulIF();
    						else
    							failedIF();
    					} else
    						ifStatements.at((int)ifStatements.size() - 1).add(s);
    				}
    			} else {
    			    if (inWhileDef) {
    			        if (s == "{")
                            doNothing();
    			        else if (command.at(0) == "end" || command.at(0) == "}") {
    			            inWhileDef = false;

    			            string v1 = whileLoops.at(whileLoops.size() - 1).valueOne(),
                                v2 = whileLoops.at(whileLoops.size() - 1).valueTwo(),
                                op = whileLoops.at(whileLoops.size() - 1).logicOperator();

                            if (variableExists(v1) && variableExists(v2)) {
                                if (op == "==" || op == "is") {
                                    while (variables.at(variableAt(v1)).getNumber() == variables.at(variableAt(v2)).getNumber()) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == "<") {
                                    while (variables.at(variableAt(v1)).getNumber() < variables.at(variableAt(v2)).getNumber()) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == ">") {
                                    while (variables.at(variableAt(v1)).getNumber() > variables.at(variableAt(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == "<=") {
                                    while (variables.at(variableAt(v1)).getNumber() <= variables.at(variableAt(v2)).getNumber()) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == ">=") {
                                    while (variables.at(variableAt(v1)).getNumber() >= variables.at(variableAt(v2)).getNumber()) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == "!=" || op == "not") {
                                    while (variables.at(variableAt(v1)).getNumber() != variables.at(variableAt(v2)).getNumber()) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                }
                            } else if (variableExists(v1)) {
                                if (op == "==" || op == "is") {
                                    while (variables.at(variableAt(v1)).getNumber() == stoi(v2)) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == "<") {
                                    while (variables.at(variableAt(v1)).getNumber() < stoi(v2)) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == ">") {
                                    while (variables.at(variableAt(v1)).getNumber() > stoi(v2)) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == "<=") {
                                    while (variables.at(variableAt(v1)).getNumber() <= stoi(v2)) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == ">=") {
                                    while (variables.at(variableAt(v1)).getNumber() >= stoi(v2)) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                } else if (op == "!=" || op == "not") {
                                    while (variables.at(variableAt(v1)).getNumber() != stoi(v2)) {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (leaving)
                                            break;
                                    }

                                    whileLoops.clear();

                                    numOfWHILE = 0;
                                }
                            } else
                                error("special_error(8)", false);
    			        } else
                            whileLoops.at(whileLoops.size() - 1).add(s);
    			    } else if (inForDef) {
    					if (command.at(0) == "next" || command.at(0) == "endfor") {
                            inForDef = false;

                            for (int i = 0; i < (int)forLoops.size(); i++)
                                if (forLoops.at(i).isForLoop())
                                    forLoop(forLoops.at(i));

                            forLoops.clear();

                            numOfFOR = 0;
    					} else {
    					    if (s == "{")
                                doNothing();
                            else
                                forLoops.at(forLoops.size() - 1).add(s);
    					}
    				} else {
    					if (size == 1) {
    						if (notStandardZeroSpace(command.at(0))) {
    							string before(beforeDot(s)), after(afterDot(s));

    							if (before.length() != 0 && after.length() != 0) {
    								if (objectExists(before) && after.length() != 0) {
    								    if (containsParams(after)) {
                                            s = subtractChar(s, "\"");

                                            if (objects.at(objectAt(before)).methodExists(beforeParams(after)))
                                                executeTemplate(objects.at(objectAt(before)).getMethod(beforeParams(after)), getParams(after));
                                            else
                                                sysExec(s, command);
    								    } else if (objects.at(objectAt(before)).methodExists(after))
    										executeMethod(objects.at(objectAt(before)).getMethod(after));
    									else if (objects.at(objectAt(before)).variableExists(after)) {
    										if (objects.at(objectAt(before)).getVariable(after).getString() != null)
    											say(objects.at(objectAt(before)).getVariable(after).getString());
    										else if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
    											say(dtos(objects.at(objectAt(before)).getVariable(after).getNumber()));
    										else
    											error("is_null", false);
    									} else if (after == "clear")
                                            objects.at(objectAt(before)).clear();
    									else
    										error("undefined", false);
    								} else {
    								    // REFACTOR HERE
    									if (before == "env") {
    										if (after == "cwd")
    											say(cwd());
    										else if (after == "usl")
    											say(uslBinary);
    										else if (after == "os")
    											say(guessedOS);
    										else if (after == "user")
    										    say(getUser());
    										else if (after == "machine")
    										    say(getMachine());
                                            else if (after == "init_dir" || after == "initial_directory")
                                                say(initDir);
											else if (after == "now")
												say(timeNow());
    										else
    											say(getEnvironmentVariable(after));
    									} else if (variableExists(before)) {
    									    if (after == "clear")
                                                parse(before + " = null");
    									} else if (listExists(before)) {
    									    // REFACTOR HERE
    									    if (after == "clear")
                                                lists.at(listAt(before)).clear();
                                            else if (after == "sort")
                                                lists.at(listAt(before)).listSort();
                                            else if (after == "reverse")
                                                lists.at(listAt(before)).listReverse();
                                            else if (after == "revert")
                                                lists.at(listAt(before)).listRevert();
    									} else if (before == "self") {
                                            if (executedMethod)
                                                executeMethod(objects.at(objectAt(currentMethodObject)).getMethod(after));
                                        } else
    										sysExec(s, command);
    								}
    							} else if (endsWith(s, "::")) {
                                    if (currentScript != "") {
                                        string newMark(s);
                                        newMark = subtractString(s, "::");
                                        scripts.at(scriptAt(currentScript)).addMark(newMark);
                                    }
                                } else if (methodExists(s))
    							    executeMethod(getMethod(s));
                                else if (startsWith(s, "[") && endsWith(s, "]")) {
                                    string moduleName = s;
                                    moduleName = subtractString(moduleName, "[");
                                    moduleName = subtractString(moduleName, "]");

                                    Module newModule(moduleName);
                                    modules.push_back(newModule);
                                    inModuleDef = true;
                                    currentModule = moduleName;
                                } else {
                                    s = subtractChar(s, "\"");

                                    if (methodExists(beforeParams(s)))
                                        executeTemplate(getMethod(beforeParams(s)), getParams(s));
                                    else
    								    sysExec(s, command);
                                }
    						} else
    							zeroSpace(command.at(0), s, command);
    					} else if (size == 2) {
    						if (notStandardOneSpace(command.at(0)))
    							sysExec(s, command);
    						else {
    							oneSpace(command.at(0), command.at(1), s, command);
							}
    					} else if (size == 3) {
    						if (notStandardTwoSpace(command.at(1))) {
    							if (command.at(0) == "append")
    								appendText(command.at(1), command.at(2), false);
    							else if (command.at(0) == "appendl")
    								appendText(command.at(1), command.at(2), true);
                                else if ((command.at(0) == "fwrite"))
                                    __fwrite(command.at(1), command.at(2));
    							else if (command.at(0) == "redefine")
    								redefine(command.at(1), command.at(2));
                                else if (command.at(0) == "loop") {
                                    if (containsParams(command.at(2))) {
                                        loopSymbol = command.at(2);
                                        loopSymbol = subtractChar(loopSymbol, "(");
                                        loopSymbol = subtractChar(loopSymbol, ")");

                                        oneSpace(command.at(0), command.at(1), subtractString(s, command.at(2)), command);
                                        loopSymbol = "$";
                                    } else
                                        sysExec(s, command);
                                } else
    								sysExec(s, command);
    						} else
    							twoSpace(command.at(0), command.at(1), command.at(2), s, command);
    					} else if (size == 4)
    						threeSpace(command.at(0), command.at(1), command.at(2), command.at(3), s, command);
                        else if (size == 5) {
                            if (command.at(0) == "for") {
                                if (containsParams(command.at(4))) {
                                    loopSymbol = command.at(4);
                                    loopSymbol = subtractChar(loopSymbol, "(");
                                    loopSymbol = subtractChar(loopSymbol, ")");

                                    threeSpace(command.at(0), command.at(1), command.at(2), command.at(3), subtractString(s, command.at(4)), command);
                                    loopSymbol = "$";
                                } else
                                    sysExec(s, command);
                            } else
                                sysExec(s, command);
                        } else
							sysExec(s, command);
    				}
    			}
    		}
    	} else {
    		stringContainer.add(bigString);

    		for (int i = 0; i < (int)stringContainer.get().size(); i++)
    			parse(stringContainer.at(i));
    	}
    } else {
   	    if (multiLineComment) {
            if (uncomment) {
                commented = false;
                multiLineComment = false;
            }
   	    } else {
   	        if (uncomment) {
   	            commented = false;
   	            uncomment = false;

                if (!broken) {
                    string commentString("");

                    bool commentFound = false;

                    for (int i = 0; i < (int)bigString.length(); i++) {
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
                } else {
                    string commentString("");

                    bool commentFound = false;

                    for (int i = 0; i < (int)bigString.length(); i++) {
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
void redefine(string target, string name) {
	if (variableExists(target)) {
        if (fileExists(variables.at(variableAt(target)).getString()) || directoryExists(variables.at(variableAt(target)).getString())) {
            string old_name(variables.at(variableAt(target)).getString()), new_name("");

            if (variableExists(name)) {
                if (variables.at(variableAt(name)).getString() != null) {
                    new_name = variables.at(variableAt(name)).getString();

                    if (fileExists(old_name)) {
                        if (!fileExists(new_name)) {
                            if (fileExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error("invalid_operation:file_undefined:" + old_name, false);
                        } else
                            error("invalid_operation:file_defined:" + new_name, false);
                    } else if (directoryExists(old_name)) {
                        if (!directoryExists(new_name)) {
                            if (directoryExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error("invalid_operation:directory_undefined:" + old_name, false);
                        } else
                            error("invalid_operation:directory_defined:" + new_name, false);
                    } else
                        error("invalid_operation:target_undefined:" + old_name, false);
                } else
                    error("invalid_operation:null_string:" + name, false);
            } else {
                if (fileExists(old_name)) {
                    if (!fileExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error("invalid_operation:file_defined:" + name, false);
                } else if (directoryExists(old_name)) {
                    if (!directoryExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error("invalid_operation:directory_defined:" + name, false);
                } else
                    error("invalid_operation:target_undefined:" + old_name, false);
            }
        } else {
    		if (startsWith(name, "@")) {
    			if (!variableExists(name))
    				variables.at(variableAt(target)).setName(name);
    			else
    				error("invalid_operation:variable_defined:" + name, false);
    		} else
    			error("invalid_operation:invalid_variable_declaration:" + name, false);
        }
	} else if (listExists(target)) {
		if (!listExists(name))
			lists.at(listAt(target)).setName(name);
		else
			error("invalid_operation:list_undefined:" + name, false);
	} else if (objectExists(target)) {
		if (!objectExists(name))
			objects.at(objectAt(target)).setName(name);
		else
			error("invalid_operation:object_undefined:" + name, false);
	} else if (methodExists(target)) {
		if (!methodExists(name))
			methods.at(methodAt(target)).setName(name);
		else
			error("invalid_operation:method_undefined:" + name, false);
	} else if (fileExists(target) || directoryExists(target))
        rename(target.c_str(), name.c_str());
	else
		error("invalid_operation:target_undefined:" + target, false);
}

void setup() {
	numOfBadMethods = 0,
    numOfBadObjects = 0,
    numOfBadVars = 0,
    lineNum = 0,
    numOfIF = 0,
    numOfFOR = 0,
    numOfWHILE = 0,
    numOfParamVars = 0;
    captureParse = false,
	commented = false,
    customPrompt = false,
    dontCollectMethodVars = false,
    failedIf = false,
    gotoMark = false,
    executedIF = false,
    inDefault = false,
    executedMethod = false,
    inSwitchDef = false,
    inIfDef = false,
    inForDef = false,
    inWhileDef = false,
    inModuleDef = false,
    inPrivateDef = false,
    inPublicDef = false,
    inScriptDef = false,
    executedTemplate = false,
    executedTry = false,
	leaving = false,
    inMethodDef = false,
    multiLineComment = false,
    negligent = false,
    failedNest = false,
    nesting = false,
    inObjectDef = false,
    inObjectMethodDef = false,
    inParamMethodDef = false,
    returning = false,
    skipCatch = false,
    skipToCatch = false,
    inMethodSwitchDef = false,
    inMethodWhileDef = false,
    inMethodForDef = false;

    currentObject = "",
    currentMethodObject = "",
    currentModule = "",
    currentScript = "",
    errorVarName = "",
    goingTo = "",
    lastError = "",
    lastValue = "",
    parsedOutput = "",
    prevScript = "",
    scriptName = "",
    switchVarName = "",
    currentLine = "",
    loopSymbol = "$";

	null = "[null]";

	argsLength = 0,
	nullNum = -DBL_MAX;

	if (contains(getEnvironmentVariable("HOMEPATH"), "Users")) {
		guessedOS = "Win7orVista";
		savedVarsPath = (getEnvironmentVariable("HOMEPATH") + "\\AppData") + "\\.savedVarsPath", savedVars = savedVarsPath + "\\.savedVars";
	} else if (contains(getEnvironmentVariable("HOMEPATH"), "Documents")) {
		guessedOS = "Win2000NTorXP";
		savedVarsPath = getEnvironmentVariable("HOMEPATH") + "\\Application Data\\.savedVarsPath", savedVars = savedVarsPath + "\\.savedVars";
	} else if (startsWith(getEnvironmentVariable("HOME"), "/")) {
		guessedOS = "UNIXMacorLINUX";
		savedVarsPath = getEnvironmentVariable("HOME") + "/.savedVarsPath", savedVars = savedVarsPath + "/.savedVars";
	} else {
	    guessedOS = "UnknownWindowsOS";
		savedVarsPath = "\\.savedVarsPath", savedVars = savedVarsPath + "\\.savedVars";
	}
}

string getSubString(string argOne, string argTwo, string beforeBracket)
{
    string returnValue("");

    if (isString(beforeBracket)) {
        vector<string> listRange = getBracketRange(argTwo);

        string variableString = variables.at(variableAt(beforeBracket)).getString();

        if (listRange.size() == 2) {
            string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

            if (rangeBegin.length() != 0 && rangeEnd.length() != 0) {
                if (isNumeric(rangeBegin) && isNumeric(rangeEnd)) {
                    if (stoi(rangeBegin) < stoi(rangeEnd)) {
                        if ((int)variableString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                            string tempString("");

                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                tempString.push_back(variableString[i]);

                            returnValue = tempString;
                        } else
                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                    } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
                        if ((int)variableString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                            string tempString("");

                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                tempString.push_back(variableString[i]);

                            returnValue = tempString;
                        } else
                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                    } else
                        error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                } else
                    error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
            } else
                error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
        } else if (listRange.size() == 1) {
            string rangeBegin(listRange.at(0));

            if (rangeBegin.length() != 0) {
                if (isNumeric(rangeBegin)) {
                    if ((int)variableString.length() - 1 >= stoi(rangeBegin) && stoi(rangeBegin) >= 0) {
                        string tmp_("");
                        tmp_.push_back(variableString[stoi(rangeBegin)]);

                        returnValue = tmp_;
                    }
                }
            }
        } else
            error("invalid_operation:invalid_range:" + argTwo, false);
    } else
        error("invalid_operation:null_string:" + beforeBracket, false);

    return (returnValue);
}

void setSubString(string argOne, string argTwo, string beforeBracket)
{
    if (variables.at(variableAt(beforeBracket)).getString() != null) {
        vector<string> listRange = getBracketRange(argTwo);

        string variableString = variables.at(variableAt(beforeBracket)).getString();

        if (listRange.size() == 2) {
            string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

            if (rangeBegin.length() != 0 && rangeEnd.length() != 0) {
                if (isNumeric(rangeBegin) && isNumeric(rangeEnd)) {
                    if (stoi(rangeBegin) < stoi(rangeEnd)) {
                        if ((int)variableString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                            string tempString("");

                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                tempString.push_back(variableString[i]);

                            if (variableExists(argOne))
                                setVariable(argOne, tempString);
                            else
                                createVariable(argOne, tempString);
                        } else
                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                    } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
                        if ((int)variableString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                            string tempString("");

                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                tempString.push_back(variableString[i]);

                            if (variableExists(argOne))
                                setVariable(argOne, tempString);
                            else
                                createVariable(argOne, tempString);
                        } else
                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                    } else
                        error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                } else
                    error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
            } else
                error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
        } else if (listRange.size() == 1) {
            string rangeBegin(listRange.at(0));

            if (rangeBegin.length() != 0) {
                if (isNumeric(rangeBegin)) {
                    if ((int)variableString.length() - 1 >= stoi(rangeBegin) && stoi(rangeBegin) >= 0) {
                        string tmp_("");
                        tmp_.push_back(variableString[stoi(rangeBegin)]);

                        if (variableExists(argOne))
                            setVariable(argOne, tmp_);
                        else
                            createVariable(argOne, tmp_);
                    }
                }
            }
        } else
            error("invalid_operation:invalid_range:" + argTwo, false);
    } else
        error("invalid_operation:null_string:" + beforeBracket, false);
}

void zeroSpace(string argZero, string s, vector<string> command)
{
	if (argZero == "pass") {
		return;
	}
    else if (argZero == "caught") {
        string to_remove = "remove ";
        to_remove.append(errorVarName);

        parse(to_remove);

        executedTry = false,
        skipToCatch = false;
        lastError = "";
        errorVarName = "";
    } else if (argZero == "clear_methods!")
		clearMethods();
	else if (argZero == "clear_objects!")
		clearObjects();
	else if (argZero == "clear_variables!")
		clearVariables();
	else if (argZero == "clear_lists!")
		clearLists();
	else if (argZero == "clear_all!")
		clearAll();
    else if (argZero == "clear_constants!")
        clearConstants();
	else if (argZero == "help")
		printUSLHelp();
	else if (argZero == "exit") {
		clearAll();
		exit(0);
	} else if (argZero == "break" || argZero == "leave!")
	    leaving = true;
	else if (argZero == "no_methods?") {
		if (noMethods())
			__true();
		else
			__false();
	} else if (argZero == "no_objects?") {
		if (noObjects())
			__true();
		else
			__false();
	} else if (argZero == "no_variables?") {
		if (noVariables())
			__true();
		else
			__false();
	} else if (argZero == "no_lists?") {
		if (noLists())
			__true();
		else
			__false();
	} else if (argZero == "end" || argZero == "}") {
		inPrivateDef = false,
		inPublicDef = false;
		inObjectDef = false;
		inObjectMethodDef = false;
		currentObject = "";
    } else if (argZero == "parser")
		loop(false);
	else if (argZero == "private") {
	    inPrivateDef = true;
	    inPublicDef = false;
    } else if (argZero == "public") {
        inPrivateDef = false;
        inPublicDef = true;
    } else if (argZero == "try")
        executedTry = true;
	else if (argZero == "failif") {
		if (failedIf == true)
			successfulIF();
		else
			failedIF();
	} else
		sysExec(s, command);
}

string &replace(string &subj, string old, string neu)
{
    size_t uiui = subj.find(old);

    if (uiui != string::npos) {
       subj.erase(uiui, old.size());
       subj.insert(uiui, neu);
    }

    return subj;
}

void oneSpace(string argZero, string argOne, string s, vector<string> command)
{
	string before(beforeDot(argOne)), after(afterDot(argOne));

    if (contains(argOne, "self.")) {
        //cout << "replacing \"" << argOne << " with \"" << currentMethodObject << "..." << endl << endl;
        argOne = replace(argOne, "self", currentMethodObject);
    }

    if (argZero == "return") {
        returning = true;

        if (containsParams(argOne)) {
            string before(beforeParams(argOne));

            if (methodExists(before)) {
                executeTemplate(getMethod(before), getParams(argOne));

                parse("return " + lastValue);
            } else if (!zeroDots(argOne)) {
                if (objectExists(before)) {
                    if (objects.at(objectAt(before)).methodExists(beforeParams(after))) {
                        executeTemplate(objects.at(objectAt(before)).getMethod(beforeParams(after)), getParams(argOne));
                        parse("return " + lastValue);
                    } else
                        lastValue = argOne;
                } else
                    lastValue = argOne;
            } else {
                if (isStringStack(argOne))
                    lastValue = getStringStack(argOne);
                else if (stackReady(argOne))
                    lastValue = dtos(getStack(argOne));
                else {
                    argOne = subtractString(argOne, "(");
                    argOne = subtractString(argOne, ")");

                    oneSpace("return", argOne, "return " + argOne, command);
                }
            }
        } else if (variableExists(argOne)) {
            if (objectExists(beforeDot(argOne))) {
                if (objects.at(objectAt(beforeDot(argOne))).getVariable(afterDot(argOne)).getString() != null)
                    lastValue = objects.at(objectAt(beforeDot(argOne))).getVariable(afterDot(argOne)).getString();
                else if (objects.at(objectAt(beforeDot(argOne))).getVariable(afterDot(argOne)).getNumber() != nullNum)
                    lastValue = dtos(objects.at(objectAt(beforeDot(argOne))).getVariable(afterDot(argOne)).getNumber());
                else
                    lastValue = "null";
            } else {
                if (variables.at(variableAt(argOne)).getString() != null)
                    lastValue = variables.at(variableAt(argOne)).getString();
                else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
                    lastValue = dtos(variables.at(variableAt(argOne)).getNumber());
                else
                    lastValue = "null";

                if (variables.at(variableAt(argOne)).garbage())
                    variables = removeVariable(variables, argOne);
            }
        } else if (listExists(argOne)) {
            string bigString("(");

            for (int i = 0; i < (int)lists.at(listAt(argOne)).size(); i++) {
                bigString.append(lists.at(listAt(argOne)).at(i));

                if (i != (int)lists.at(listAt(argOne)).size() - 1)
                    bigString.push_back(',');
            }

            bigString.append(")");

            lastValue = bigString;

            if (lists.at(listAt(argOne)).garbage())
                lists = removeList(lists, argOne);
        } else
            lastValue = argOne;
    } else if (argZero == "switch") {
        if (variableExists(argOne)) {
            inSwitchDef = true;
            switchVarName = argOne;
        } else
            error("invalid_operation:variable_undefined:" + argOne, false);
    } else if (argZero == "goto") {
        if (currentScript != "") {
            if (scripts.at(scriptAt(currentScript)).markExists(argOne)) {
                goingTo = argOne;
                gotoMark = true;
            }
        }
    } else if (argZero == "help")
        comprehensiveHelp(argOne);
	else if (argZero == "prompt") {
        if (argOne == "bash") {
            customPrompt = true;
            promptStyle = "bash";
        } else if (argOne == "!") {
            if (customPrompt == true)
                customPrompt = false;
            else
                customPrompt = true;
        } else if (argOne == "empty") {
            customPrompt = true;
            promptStyle = "empty";
        } else {
            customPrompt = true;
            promptStyle = argOne;
        }
    } else if (argZero == "err" || argZero == "error") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null)
				cerr << variables.at(variableAt(argOne)).getString() << endl;
			else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
				cerr << variables.at(variableAt(argOne)).getNumber() << endl;
			else
				error("is_null:" + argOne, false);
		} else
			cerr << argOne << endl;
 	} else if (argZero == "delay") {
		if (isNumeric(argOne))
			delay(stoi(argOne));
		else
			error("conversion_error:" + argOne, false);
	} else if (argZero == "loop")
		threeSpace("for", "var", "in", argOne, "for var in " + argOne, command); // REFACTOR HERE
	else if (argZero == "for" && argOne == "infinity")
		successfulFor();
	else if (argZero == "remove") {
	    if (containsParams(argOne)) {
	        vector<string> params = getParams(argOne);

	        for (int i = 0; i < (int)params.size(); i++) {
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
	    } else if (variableExists(argOne))
			variables = removeVariable(variables, argOne);
		else if (listExists(argOne))
			lists = removeList(lists, argOne);
		else if (objectExists(argOne))
			objects = removeObject(objects, argOne);
		else if (methodExists(argOne))
			methods = removeMethod(methods, argOne);
		else
			error("invalid_operation:target_undefined:" + argOne, false);
	} else if (argZero == "see_string") {
		if (variableExists(argOne))
			__stdout(variables.at(variableAt(argOne)).getString());
		else
			error("invalid_operation:variable_undefined:" + argOne, false);
	} else if (argZero == "see_number") {
		if (variableExists(argOne))
			__stdout(dtos(variables.at(variableAt(argOne)).getNumber()));
		else
			error("invalid_operation:variable_undefined:" + argOne, false);
	} else if (argZero == "__begin__") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null) {
				if (!fileExists(variables.at(variableAt(argOne)).getString())) {
					touch(variables.at(variableAt(argOne)).getString());
					inScriptDef = true;
					scriptName = variables.at(variableAt(argOne)).getString();
				} else
					error("invalid_operation:file_defined:" + variables.at(variableAt(argOne)).getString(), false);
			}
		} else if (!fileExists(argOne)) {
			touch(argOne);
			inScriptDef = true;
			scriptName = argOne;
		} else
			error("invalid_operation:file_defined:" + argOne, false);
	} else if (argZero == "encrypt" || argZero == "decrypt") {
        Crypt c;

        if (argZero == "encrypt") {
            if (variableExists(argOne)) {
                if (getVariable(argOne).getString() != null)
                    __stdout(c.e(getVariable(argOne).getString()));
                else
                    __stdout(c.e(dtos(getVariable(argOne).getNumber())));
            } else
                __stdout(c.e(argOne));
        } else {
            if (variableExists(argOne)) {
                if (getVariable(argOne).getString() != null)
                    __stdout(c.d(getVariable(argOne).getString()));
                else
                    __stdout(c.d(dtos(getVariable(argOne).getNumber())));
            } else
                __stdout(c.d(argOne));
        }
	} else if (argZero == "globalize") {
        if (contains(argOne, ".") && methodExists(argOne) && !methodExists(afterDot(argOne))) {
            Method method(afterDot(argOne));

            vector<string> lines = getObject(beforeDot(argOne)).getMethod(afterDot(argOne)).getLines();

            for (int i = 0; i < (int)lines.size(); i++)
                method.add(lines[i]);

            methods.push_back(method);
        } else
            error("invalid_operation:object_method_undefined:" + argOne, false);
	} else if (argZero == "remember" || argZero == "save") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null)
				saveVariable(argOne + "&" + variables.at(variableAt(argOne)).getString());
			else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
				saveVariable(argOne + "&" + dtos(variables.at(variableAt(argOne)).getNumber()));
			else
				error("is_null:" + argOne, false);
		} else
			error("invalid_operation:target_undefined:" + argOne, false);
	} else if (argZero == "forget"  || argZero == "lose") {
		if (fileExists(savedVars)) {
			string line(""), bigStr("");
			ifstream file(savedVars.c_str());
// REFACTOR HERE
			Crypt c;

			if (file.is_open()) {
				while (!file.eof()) {
					getline(file, line);
					bigStr.append(line);
				}

				file.close();

				int bigStrLength = bigStr.length();
				bool stop = false;
				string varName(""), vval("");
				bigStr = c.d(bigStr);

				vector<string> varNames;
				vector<string> varValues;

				varNames.push_back("");
				varValues.push_back("");

				for (int i = 0; i < bigStrLength; i++) {
					switch (bigStr[i]) {
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

				for (int i = 0; i < (int)varNames.size(); i++) {
					if (varNames.at(i) != argOne) {
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

				rm(savedVars);
				touch(savedVars);
				app(savedVars, c.e(new_saved));
			}
		}
	} else if (argZero == "load") {
		if (fileExists(argOne)) {
			if (isScript(argOne)) {
			    prevScript = currentScript;
				loadScript(argOne);
            } else
				error("bad_load:" + argOne, true);
		} else if (moduleExists(argOne)) {
            vector<string> lines = modules.at(moduleAt(argOne)).get();

            for (int i = 0; i < (int)lines.size(); i++)
                parse(lines.at(i));
        } else
			error("bad_load:" + argOne, true);
	} else if (argZero == "say" || argZero == "stdout" || argZero == "out" || argZero == "print" || argZero == "println") {
		string text(argOne);
		bool is_say = (argZero == "say");
		bool is_print = (argZero == "print" || argZero == "println");
		// if parameter is variable, get it's value
		if (variableExists(argOne)) {
		    // set the value 
			if (!zeroDots(argOne)) {
		        if (objects.at(objectAt(beforeDot(argOne))).getVariable(afterDot(argOne)).getString() != null)
		            text = (objects.at(objectAt(beforeDot(argOne))).getVariable(afterDot(argOne)).getString());
		        else if (objects.at(objectAt(beforeDot(argOne))).getVariable(afterDot(argOne)).getNumber() != nullNum)
                    text = (dtos(objects.at(objectAt(beforeDot(argOne))).getVariable(afterDot(argOne)).getNumber()));
                else
                    text = ("is_null:" + argOne, false);
		    } else {
                if (variables.at(variableAt(argOne)).getString() != null)
                    text = (variables.at(variableAt(argOne)).getString());
                else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
                    text = (dtos(variables.at(variableAt(argOne)).getNumber()));
                else
                    text = ("is_null:" + argOne, false);
		    }
		}
		
		if (is_say) {
			say(text);
		}
		else if (is_print) {
			if (argZero == "println") {
				cout << text << endl;
			}
			else {
				cout << text;
			}
		}
		else {
			__stdout(text);
		}
	} else if (argZero == "cd" || argZero == "chdir") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null) {
				if (directoryExists(variables.at(variableAt(argOne)).getString()))
					cd(variables.at(variableAt(argOne)).getString());
				else
					error("read_fail:" + variables.at(variableAt(argOne)).getString(), false);
			} else
				error("invalid_operation:null_string:" + argOne, false);
		} else {
			if (argOne == "init_dir" || argOne == "initial_directory")
				cd(initDir);
			else if (directoryExists(argOne))
				cd(argOne);
			else
				cd(argOne);
		}
	} else if (argZero == "list") {
		if (listExists(argOne))
			lists.at(listAt(argOne)).clear();
		else {
			List newList(argOne);

			if (executedTemplate || executedMethod)
                newList.collect();
            else
                newList.dontCollect();

			lists.push_back(newList);
		}
	} else if (argZero == "!") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null)
				parse(variables.at(variableAt(argOne)).getString().c_str());
			else
				error("is_null:" + argOne, false);
		} else
			parse(argOne.c_str());
	} else if (argZero == "?") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null)
                 sysExec(variables.at(variableAt(argOne)).getString(), command);
			else
				error("is_null:" + argOne, false);
		} else
			sysExec(argOne, command);
	} else if (argZero == "init_dir" || argZero == "initial_directory") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null) {
				if (directoryExists(variables.at(variableAt(argOne)).getString())) {
					initDir = variables.at(variableAt(argOne)).getString();
					cd(initDir);
				} else
					error("read_fail:" + initDir, false);
			} else
				error("invalid_operation:null_string:" + argOne, false);
		} else {
			if (directoryExists(argOne)) {
			    if (argOne == ".")
                    initDir = cwd();
                else if (argOne == "..")
                    initDir = cwd() + "\\..";
                else
                    initDir = argOne;

				cd(initDir);
			} else
				error("read_fail:" + initDir, false);
		}
	} else if (argZero == "is_method?" || argZero == "method?") {
		if (before.length() != 0 && after.length() != 0) {
			if (objects.at(objectAt(before)).methodExists(after))
				__true();
			else
				__false();
		} else {
			if (methodExists(argOne))
				__true();
			else
				__false();
		}
	} else if (argZero == "is_object?" || argZero == "object?") {
		if (objectExists(argOne))
			__true();
		else
			__false();
	} else if (argZero == "is_variable?" || argZero == "var?" || argZero == "variable?") {
		if (before.length() != 0 && after.length() != 0) {
			if (objects.at(objectAt(before)).variableExists(after))
				__true();
			else
				__false();
		} else {
			if (variableExists(argOne))
				__true();
			else
				__false();
		}
	} else if (argZero == "is_list?" || argZero == "list?") {
		if (listExists(argOne))
			__true();
		else
			__false();
	} else if (argZero == "is_directory?" || argZero == "dir?" || argZero == "directory?") {
		if (before.length() != 0 && after.length() != 0) {
			if (objects.at(objectAt(before)).variableExists(after)) {
				if (directoryExists(objects.at(objectAt(before)).getVariable(after).getString()))
					__true();
				else
					__false();
			} else
				error("invalid_operation:target_undefined:" + argOne, false);
		} else {
			if (variableExists(argOne)) {
				if (variables.at(variableAt(argOne)).getString() != null) {
					if (directoryExists(variables.at(variableAt(argOne)).getString()))
						__true();
					else
						__false();
				} else
					error("invalid_operation:null_string:" + argOne, false);
			} else {
				if (directoryExists(argOne))
					__true();
				else
					__false();
			}
		}
	} else if (argZero == "is_file?" || argZero == "file?") {
		if (before.length() != 0 && after.length() != 0) {
			if (objects.at(objectAt(before)).variableExists(after)) {
				if (fileExists(objects.at(objectAt(before)).getVariable(after).getString()))
					__true();
				else
					__false();
			} else
				error("invalid_operation:target_undefined:" + argOne, false);
		} else {
			if (variableExists(argOne)) {
				if (variables.at(variableAt(argOne)).getString() != null) {
					if (fileExists(variables.at(variableAt(argOne)).getString()))
						__true();
					else
						__false();
				} else
					__false();
			} else {
				if (fileExists(argOne))
					__true();
				else
					__false();
			}
		}
	} else if (argZero == "collect?" || argZero == "garbage?") {
	    if (variableExists(argOne)) {
	        if (variables.at(variableAt(argOne)).garbage())
	            __true();
	        else
                __false();
	    } else
            cout << "under construction..." << endl;
	} else if (argZero == "is_number?" || argZero == "number?") {
		if (before.length() != 0 && after.length() != 0) {
			if (objects.at(objectAt(before)).variableExists(after)) {
				if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
					__true();
				else
					__false();
			} else
				error("invalid_operation:target_undefined:" + argOne, false);
		} else {
			if (variableExists(argOne)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum)
					__true();
				else
					__false();
			} else {
				if (isNumeric(argOne))
					__true();
				else
					__false();
			}
		}
	} else if (argZero == "is_string?" || argZero == "string?") {
		if (before.length() != 0 && after.length() != 0) {
			if (objects.at(objectAt(before)).variableExists(after)) {
				if (objects.at(objectAt(before)).getVariable(after).getString() != null)
					__true();
				else
					__false();
			} else
				error("invalid_operation:target_undefined:" + argOne, false);
		} else {
			if (variableExists(argOne)) {
				if (variables.at(variableAt(argOne)).getString() != null)
					__true();
				else
					__false();
			} else {
				if (isNumeric(argOne))
					__false();
				else
					__true();
			}
		}
	} else if (argZero == "is_uppercase?" || argZero == "upper?" || argZero == "uppercase?") {
		if (before.length() != 0 && after.length() != 0) {
			if (objects.at(objectAt(before)).variableExists(after)) {
				if (isUpper(objects.at(objectAt(before)).getVariable(after).getString()))
					__true();
				else
					__false();
			} else
				error("invalid_operation:target_undefined:" + argOne, false);
		} else {
			if (variableExists(argOne)) {
				if (variables.at(variableAt(argOne)).getString() != null) {
                    if (isUpper(variables.at(variableAt(argOne)).getString()))
    					__true();
				    else
				        __false();
				} else
					__false();
			} else {
				if (isNumeric(argOne))
					__false();
				else {
                    if (isUpper(argOne))
					    __true();
                    else
                        __false();
                }
			}
		}
	} else if (argZero == "is_lowercase?" || argZero == "lower?" || argZero == "lowercase?") {
		if (before.length() != 0 && after.length() != 0) {
			if (objects.at(objectAt(before)).variableExists(after)) {
				if (isLower(objects.at(objectAt(before)).getVariable(after).getString()))
					__true();
				else
					__false();
			} else
				error("invalid_operation:target_undefined:" + argOne, false);
		} else {
			if (variableExists(argOne)) {
				if (variables.at(variableAt(argOne)).getString() != null) {
                    if (isLower(variables.at(variableAt(argOne)).getString()))
    					__true();
				    else
				        __false();
				} else
					__false();
			} else {
				if (isNumeric(argOne))
					__false();
				else {
                    if (isLower(argOne))
					    __true();
                    else
                        __false();
                }
			}
		}
	} else if (argZero == "see") {
		if (before.length() != 0 && after.length() != 0) {
			if (objectExists(before)) {
				if (objects.at(objectAt(before)).methodExists(after)) {
					for (int i = 0; i < objects.at(objectAt(before)).getMethod(after).size(); i++)
						__stdout(objects.at(objectAt(before)).getMethod(after).at(i));
				} else if (objects.at(objectAt(before)).variableExists(after)) {
					if (objects.at(objectAt(before)).getVariable(after).getString() != null)
						__stdout(objects.at(objectAt(before)).getVariable(after).getString());
					else if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
						__stdout(dtos(objects.at(objectAt(before)).getVariable(after).getNumber()));
					else
						__stdout(null);
				} else
					error("invalid_operation:target_undefined:" + argOne, false);
			} else
				error("invalid_operation:object_undefined:" + before, false);
		} else {
			if (objectExists(argOne)) {
				for (int i = 0; i < objects.at(objectAt(argOne)).methodSize(); i++)
					__stdout(objects.at(objectAt(argOne)).getMethod(objects.at(objectAt(argOne)).getMethodName(i)).name());
				for (int i = 0; i < objects.at(objectAt(argOne)).variableSize(); i++)
					__stdout(objects.at(objectAt(argOne)).getVariable(objects.at(objectAt(argOne)).getVariableName(i)).name());
			} else if (constantExists(argOne)) {
			    if (constants.at(constAt(argOne)).isNumber())
                    __stdout(dtos(constants.at(constAt(argOne)).getNumber()));
                else if (constants.at(constAt(argOne)).isString())
                    __stdout(constants.at(constAt(argOne)).getString());
			} else if (methodExists(argOne)) {
				for (int i = 0; i < methods.at(methodAt(argOne)).size(); i++)
					__stdout(methods.at(methodAt(argOne)).at(i));
			} else if (variableExists(argOne)) {
				if (variables.at(variableAt(argOne)).getString() != null)
					__stdout(variables.at(variableAt(argOne)).getString());
				else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
					__stdout(dtos(variables.at(variableAt(argOne)).getNumber()));
			} else if (listExists(argOne)) {
				for (int i = 0; i < lists.at(listAt(argOne)).size(); i++)
					__stdout(lists.at(listAt(argOne)).at(i));
			} else if (argOne == "variables") {
				for (int i = 0; i < (int)variables.size(); i++) {
					if (variables.at(i).getString() != null)
						__stdout(variables.at(i).name() + ":\t" + variables.at(i).getString());
					else if (variables.at(i).getNumber() != nullNum)
						__stdout(variables.at(i).name() + ":\t" + dtos(variables.at(i).getNumber()));
					else
						__stdout(variables.at(i).name() + ":\tis_null");
				}
			} else if (argOne == "lists") {
				for (int i = 0; i < (int)lists.size(); i++)
					__stdout(lists.at(i).name());
			} else if (argOne == "methods") {
				for (int i = 0; i < (int)methods.size(); i++)
					__stdout(methods.at(i).name());
			} else if (argOne == "objects") {
				for (int i = 0; i < (int)objects.size(); i++)
					__stdout(objects.at(i).name());
			} else if (argOne == "constants") {
                for (int i = 0; i < (int)constants.size(); i++)
                    __stdout(constants.at(i).name());
            } else if (argOne == "os")
			    __stdout(guessedOS);
			else if (argOne == "last")
			    __stdout(lastValue);
			else
				error("invalid_operation:target_undefined:" + argOne, false);
		}
	} else if (argZero == "template") {
        if (methodExists(argOne))
            error("invalid_operation:method_defined:" + argOne, false);
        else {
            if (containsParams(argOne)) {
                vector<string> params = getParams(argOne);
                Method method(beforeParams(argOne), true);

                method.setTemplateSize((int)params.size());

                methods.push_back(method);

                inMethodDef = true;
            }
        }
    } else if (argZero == "lock") {
        if (variableExists(argOne))
            variables.at(variableAt(argOne)).setIndestructible();
        else if (methodExists(argOne))
            methods.at(methodAt(argOne)).setIndestructible();
    } else if (argZero == "unlock") {
        if (variableExists(argOne))
            variables.at(variableAt(argOne)).setDestructible();
        else if (methodExists(argOne))
            methods.at(methodAt(argOne)).setDestructible();
    } else if (argZero == "method" || argZero == "[method]") {
	    bool indestructable = false;

        if (argZero == "[method]")
            indestructable = true;

		if (inObjectDef) {
			if (objects.at(objectAt(currentObject)).methodExists(argOne))
				error("invalid_operation:method_defined:" + argOne, false);
			else {
			    if (containsParams(argOne)) {
                    vector<string> params = getParams(argOne);

                    Method method(beforeParams(argOne));

                    if (inPublicDef)
                        method.setPublic();
                    else if (inPrivateDef)
                        method.setPrivate();

                    method.setObject(currentObject);

                    for (int i = 0; i < (int)params.size(); i++) {
                        if (variableExists(params.at(i))) {
                            if (!zeroDots(params.at(i))) {
                                string before(beforeDot(params.at(i))), after(afterDot(params.at(i)));

                                if (objectExists(before)) {
                                    if (objects.at(objectAt(before)).variableExists(after)) {
                                        if (objects.at(objectAt(before)).getVariable(after).getString() != null)
                                            method.addMethodVariable(objects.at(objectAt(before)).getVariable(after).getString(), after);
                                        else if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
                                            method.addMethodVariable(objects.at(objectAt(before)).getVariable(after).getNumber(), after);
                                        else
                                            error("is_null:" + params.at(i), false);
                                    } else
                                        error("invalid_operation:object_variable_undefined:" + after, false);
                                } else
                                    error("invalid_operation:object_undefined:" + before, false);
                            } else {
                                if (variables.at(variableAt(params.at(i))).getString() != null)
                                    method.addMethodVariable(variables.at(variableAt(params.at(i))).getString(), variables.at(variableAt(params.at(i))).name());
                                else if (variables.at(variableAt(params.at(i))).getNumber() != nullNum)
                                    method.addMethodVariable(variables.at(variableAt(params.at(i))).getNumber(), variables.at(variableAt(params.at(i))).name());
                                else
                                    error("is_null:" + params.at(i), false);
                            }
                        } else {
                            if (isAlpha(params.at(i))) {
                                Variable newVariable("@[pm#" + itos(numOfParamVars) + "]", params.at(i));
                                method.addMethodVariable(newVariable);
                                numOfParamVars++;
                            } else {
                                Variable newVariable("@[pm#" + itos(numOfParamVars) + "]", stod(params.at(i)));
                                method.addMethodVariable(newVariable);
                                numOfParamVars++;
                            }
                        }
                    }

                    objects.at(objectAt(currentObject)).addMethod(method);
                    objects.at(objectAt(currentObject)).setCurrentMethod(beforeParams(argOne));
                    inMethodDef = true;
                    inParamMethodDef = true;
                    inObjectMethodDef = true;
                } else {
                    Method method(argOne);

                    if (inPublicDef)
                        method.setPublic();
                    else if (inPrivateDef)
                        method.setPrivate();

                    method.setObject(currentObject);
                    objects.at(objectAt(currentObject)).addMethod(method);
                    objects.at(objectAt(currentObject)).setCurrentMethod(argOne);
                    inMethodDef = true;
                    inObjectMethodDef = true;
                }
			}
		} else {
			if (methodExists(argOne))
				error("invalid_operation:method_defined:" + argOne, false);
			else {
                if (!zeroDots(argOne)) {
                    string before(beforeDot(argOne)), after(afterDot(argOne));

                    if (objectExists(before)) {
                        Method method(after);

                        if (inPublicDef)
                            method.setPublic();
                        else if (inPrivateDef)
                            method.setPrivate();

                        method.setObject(before);
                        objects.at(objectAt(before)).addMethod(method);
                        objects.at(objectAt(before)).setCurrentMethod(after);
                        inMethodDef = true;
                        inObjectMethodDef = true;
                    } else
                        error("invalid_operation:object_undefined", false);
                } else if (containsParams(argOne)) {
                    vector<string> params = getParams(argOne);

                    Method method(beforeParams(argOne));

                    if (indestructable)
                        method.setIndestructible();

                    for (int i = 0; i < (int)params.size(); i++) {
                        if (variableExists(params.at(i))) {
                            if (!zeroDots(params.at(i))) {
                                string before(beforeDot(params.at(i))), after(afterDot(params.at(i)));

                                if (objectExists(before)) {
                                    if (objects.at(objectAt(before)).variableExists(after)) {
                                        if (objects.at(objectAt(before)).getVariable(after).getString() != null)
                                            method.addMethodVariable(objects.at(objectAt(before)).getVariable(after).getString(), after);
                                        else if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
                                            method.addMethodVariable(objects.at(objectAt(before)).getVariable(after).getNumber(), after);
                                        else
                                            error("is_null:" + params.at(i), false);
                                    } else
                                        error("invalid_operation:object_variable_undefined:" + after, false);
                                } else
                                    error("invalid_operation:object_undefined:" + before, false);
                            } else {
                                if (variables.at(variableAt(params.at(i))).getString() != null)
                                    method.addMethodVariable(variables.at(variableAt(params.at(i))).getString(), variables.at(variableAt(params.at(i))).name());
                                else if (variables.at(variableAt(params.at(i))).getNumber() != nullNum)
                                    method.addMethodVariable(variables.at(variableAt(params.at(i))).getNumber(), variables.at(variableAt(params.at(i))).name());
                                else
                                    error("is_null:" + params.at(i), false);
                            }
                        } else {
                            if (isAlpha(params.at(i))) {
                                Variable newVariable("@" + params.at(i), "");
                                newVariable.setNull();
                                method.addMethodVariable(newVariable);
                                numOfParamVars++;
                            } else {
                                Variable newVariable("@" + params.at(i), 0);
                                newVariable.setNull();
                                method.addMethodVariable(newVariable);
                                numOfParamVars++;
                            }
                        }
                    }

                    methods.push_back(method);
                    inMethodDef = true;
                    inParamMethodDef = true;
                } else {
    				Method method(argOne);

                    if (indestructable)
                        method.setIndestructible();

    				methods.push_back(method);
    				inMethodDef = true;
                }
			}
		}
	} else if (argZero == "call_method") {
		if (inObjectDef) {
			if (objects.at(objectAt(currentObject)).methodExists(argOne))
				executeMethod(objects.at(objectAt(currentObject)).getMethod(argOne));
			else
				error("invalid_operation:method_undefined:" + argOne, false);
		} else {
			if (before.length() != 0 && after.length() != 0) {
				if (objectExists(before)) {
					if (objects.at(objectAt(before)).methodExists(after))
						executeMethod(objects.at(objectAt(before)).getMethod(after));
					else
						error("invalid_operation:method_undefined:" + argOne, false);
				} else
					error("invalid_operation:object_undefined:" + before, true);
			} else {
				if (methodExists(argOne))
					executeMethod(methods.at(methodAt(argOne)));
				else
					error("invalid_operation:method_undefined:" + argOne, true);
			}
		}
	} else if (argZero == "object") {
		if (objectExists(argOne)) {
			inObjectDef = true;
			currentObject = argOne;
		} else {
			Object object(argOne);
			currentObject = argOne;
			object.dontCollect();
			objects.push_back(object);
			inObjectDef = true;
		}
	} else if (argZero == "fpush") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null) {
				if (!fileExists(variables.at(variableAt(argOne)).getString()))
					touch(variables.at(variableAt(argOne)).getString());
				else
					error("invalid_operation:file_defined:" + variables.at(variableAt(argOne)).getString(), false);
			} else
				error("invalid_operation:null_string:" + argOne, false);
		} else {
			if (!fileExists(argOne))
				touch(argOne);
			else
				error("invalid_operation:file_defined:" + argOne, false);
		}
	} else if (argZero == "fpop") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null) {
				if (fileExists(variables.at(variableAt(argOne)).getString()))
					rm(variables.at(variableAt(argOne)).getString());
				else
					error("invalid_operation:file_undefined:" + variables.at(variableAt(argOne)).getString(), false);
			} else
				error("invalid_operation:null_string:" + argOne, false);
		} else {
			if (fileExists(argOne))
				rm(argOne);
			else
				error("invalid_operation:file_undefined:" + argOne, false);
		}
	} else if (argZero == "dpush") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null) {
				if (!directoryExists(variables.at(variableAt(argOne)).getString()))
					md(variables.at(variableAt(argOne)).getString());
				else
					error("invalid_operation:directory_defined:" + variables.at(variableAt(argOne)).getString(), false);
			} else
				error("invalid_operation:null_string:" + argOne, false);
		} else {
			if (!directoryExists(argOne))
				md(argOne);
			else
				error("invalid_operation:directory_defined:" + argOne, false);
		}
	} else if (argZero == "dpop") {
		if (variableExists(argOne)) {
			if (variables.at(variableAt(argOne)).getString() != null) {
				if (directoryExists(variables.at(variableAt(argOne)).getString()))
					rd(variables.at(variableAt(argOne)).getString());
				else
					error("invalid_operation:directory_undefined:" + variables.at(variableAt(argOne)).getString(), false);
			} else
				error("invalid_operation:null_string:" + argOne, false);
		} else {
			if (directoryExists(argOne))
				rd(argOne);
			else
				error("invalid_operation:directory_undefined:" + argOne, false);
		}
	} else
		sysExec(s, command);
}

bool isNumber(string v_Name)
{
     if (variables.at(variableAt(v_Name)).getNumber() != nullNum)
         return (true);

     return (false);
}

bool isString(string v_Name)
{
     if (variables.at(variableAt(v_Name)).getString() != null)
         return (true);

     return (false);
}

double getNumber(string v_Name)
{
      return (variables.at(variableAt(v_Name)).getNumber());
}

string getString(string v_Name)
{
      return (variables.at(variableAt(v_Name)).getString());
}

bool secondIsNumber(string s)
{
    if (variableExists(s)) {
        if (isNumber(s))
            return (true);
    } else if (stackReady(s)) {
        if (!isStringStack(s))
            return (true);
    } else {
        if (isNumeric(s))
            return (true);
    }

    return (false);
}

string getStringValue(string argOne, string op, string argTwo)
{
    string firstValue(""), lastValue(""), returnValue("");

    if (variableExists(argOne)) {
        if (isString(argOne))
            firstValue = getString(argOne);
    }

    if (variableExists(argTwo)) {
        if (isString(argTwo))
            lastValue = getString(argTwo);
        else if (isNumber(argTwo))
            lastValue = dtos(getNumber(argTwo));
    } else if (methodExists(argTwo)) {
        parse(argTwo);

        lastValue = lastValue;
    } else if (!zeroDots(argTwo)) {
        string _beforeDot(beforeDot(argTwo)), _afterDot(afterDot(argTwo));

        if (_beforeDot == "env") {
            if (_afterDot == "cwd")
                lastValue = cwd();
            else if (_afterDot == "init_dir" || _afterDot == "initial_directory")
                lastValue = initDir;
            else if (_afterDot == "usl")
                lastValue = uslBinary;
            else if (_afterDot == "os")
                lastValue = guessedOS;
            else if (_afterDot == "user")
                lastValue = getUser();
            else if (_afterDot == "machine")
                lastValue = getMachine();
            else if (_afterDot == "empty_string" || _afterDot == "string")
                lastValue = "";
            else if (_afterDot == "empty_number" || _afterDot == "number")
                lastValue = "0";
            else if (_afterDot == "last_error")
                lastValue = lastError;
            else if (_afterDot == "last_value")
                lastValue = lastValue;
            else if (_afterDot == "am_or_pm")
                lastValue = amOrPm();
            else if (_afterDot == "day_of_this_week")
                lastValue = dayOfTheWeek();
            else if (_afterDot == "day_of_this_month")
                lastValue = itos(dayOfTheMonth());
            else if (_afterDot == "day_of_this_year")
                lastValue = itos(dayOfTheYear());
            else if (_afterDot == "this_second")
                lastValue = itos(secondNow());
            else if (_afterDot == "this_minute")
                lastValue = itos(minuteNow());
            else if (_afterDot == "this_hour")
                lastValue = itos(hourNow());
            else if (_afterDot == "this_month")
                lastValue = itos(monthNow());
            else if (_afterDot == "this_year")
                lastValue = itos(yearNow());
            else if (_afterDot == "now")
                lastValue = timeNow();
            else
                lastValue = getEnvironmentVariable(_afterDot);
        } else if (_beforeDot == "args") {
            if (_afterDot == "size")
                lastValue = itos(args.size());
            else
                lastValue = "";
        } else if (objectExists(_beforeDot)) {
            executeTemplate(objects.at(objectAt(_beforeDot)).getMethod(_afterDot), getParams(_afterDot));

            lastValue = lastValue;
        } else
            lastValue = argTwo;
    } else if (containsBrackets(argTwo)) {
        string _beforeBrackets(beforeBrackets(argTwo)), _afterBrackets(afterBrackets(argTwo));

        if (_beforeBrackets == "args") {
            vector<string> params = getBracketRange(_afterBrackets);

            if (isNumeric(params.at(0))) {
                if ((int)args.size() - 1 >= stoi(params.at(0)) && stoi(params.at(0)) >= 0) {
                    if (params.at(0) == "0")
                        lastValue = currentScript;
                    else
                        lastValue = args.at(stoi(params.at(0)));
                } else
                    lastValue = "";
            } else
                lastValue = "";
        } else if (listExists(_beforeBrackets)) {
            _afterBrackets = subtractString(_afterBrackets, "]");

            if (lists.at(listAt(_beforeBrackets)).size() >= stoi(_afterBrackets)) {
                if (stoi(_afterBrackets) >= 0)
                    lastValue = lists.at(listAt(_beforeBrackets)).at(stoi(_afterBrackets));
                else
                    lastValue = "";
            } else
                lastValue = "";
        }
    } else if (containsParams(argTwo)) {
        if (beforeParams(argTwo).length() != 0) {
            executeTemplate(methods.at(methodAt(argTwo)), getParams(argTwo));

            lastValue = lastValue;
        } else {
            if (isStringStack(argTwo))
                lastValue = getStringStack(argTwo);
            else if (stackReady(argTwo))
                lastValue = dtos(getStack(argTwo));
        }
    } else
        lastValue = argTwo;

    if (op == "+=")
        returnValue = (firstValue + lastValue);
    else if (op == "-=")
        returnValue = subtractString(firstValue, lastValue);
    else if (op == "*=") {
        if (isNumeric(lastValue)) {
            string bigString("");

            for (int i = 0; i < (int)stod(lastValue); i++)
                bigString.append(firstValue);

            returnValue = bigString;
        }
    } else if (op == "/=")
        returnValue = subtractString(firstValue, lastValue);
    else if (op == "**=")
        returnValue = dtos(pow(stod(firstValue), stod(lastValue)));
    else if (op == "=")
        returnValue = lastValue;

    return (returnValue);
}

double getNumberValue(string argOne, string op, string argTwo)
{
    double firstValue = 0, _lastValue = 0, returnValue = 0;

    if (variableExists(argOne)) {
        if (isNumber(argOne))
            firstValue = getNumber(argOne);
    }

    if (variableExists(argTwo)) {
        if (isNumber(argTwo))
            _lastValue = getNumber(argTwo);
        else
            _lastValue = 0;
    } else if (methodExists(argTwo)) {
        parse(argTwo);

        if (isNumeric(lastValue))
            _lastValue = stod(lastValue);
        else
            _lastValue = 0;
    } else if (!zeroDots(argTwo)) {
        string _beforeDot(beforeDot(argTwo)), _afterDot(afterDot(argTwo));
        // REFACTOR HERE
        if (_beforeDot == "env") {
            if (_afterDot == "empty_number")
                _lastValue = 0;
            else if (_afterDot == "last_value")
                _lastValue = stod(lastValue);
            else if (_afterDot == "day_of_this_month")
                _lastValue = stod(itos(dayOfTheMonth()));
            else if (_afterDot == "day_of_this_year")
                _lastValue = stod(itos(dayOfTheYear()));
            else if (_afterDot == "this_second")
                _lastValue = stod(itos(secondNow()));
            else if (_afterDot == "this_minute")
                _lastValue = stod(itos(minuteNow()));
            else if (_afterDot == "this_hour")
                _lastValue = stod(itos(hourNow()));
            else if (_afterDot == "this_month")
                _lastValue = stod(itos(monthNow()));
            else if (_afterDot == "this_year")
                _lastValue = stod(itos(yearNow()));
            else
                _lastValue = 0;
        } else if (_beforeDot == "args") {
            if (_afterDot == "size")
                _lastValue = stod(itos(args.size()));
            else
                _lastValue = 0;
        } else if (objectExists(_beforeDot)) {
            executeTemplate(objects.at(objectAt(_beforeDot)).getMethod(_afterDot), getParams(_afterDot));

            if (isNumeric(lastValue))
                _lastValue = stod(lastValue);
            else
                _lastValue = 0;
        } else {
            if (isNumeric(lastValue))
                _lastValue = stod(argTwo);
            else
                _lastValue = 0;
        }
    } else if (containsBrackets(argTwo)) {
        string _beforeBrackets(beforeBrackets(argTwo)), _afterBrackets(afterBrackets(argTwo));

        if (listExists(_beforeBrackets)) {
            _afterBrackets = subtractString(_afterBrackets, "]");

            if (lists.at(listAt(_beforeBrackets)).size() >= stoi(_afterBrackets)) {
                if (stoi(_afterBrackets) >= 0) {
                    if (isNumeric(lists.at(listAt(_beforeBrackets)).at(stoi(_afterBrackets))))
                        _lastValue = stod(lists.at(listAt(_beforeBrackets)).at(stoi(_afterBrackets)));
                    else
                        _lastValue = 0;
                } else
                    _lastValue = 0;
            } else
                _lastValue = 0;
        }
    } else if (containsParams(argTwo)) {
        if (beforeParams(argTwo).length() != 0) {
            executeTemplate(methods.at(methodAt(argTwo)), getParams(argTwo));

            if (isNumeric(lastValue))
                _lastValue = stod(lastValue);
            else
                _lastValue = 0;
        } else {
            if (stackReady(argTwo))
                _lastValue = getStack(argTwo);
            else
                _lastValue = 0;
        }
    } else {
        if (isNumeric(argTwo))
            _lastValue = stod(argTwo);
        else
            _lastValue = 0;
    }


    if (op == "+=")
        returnValue = (firstValue + _lastValue);
    else if (op == "-=")
        returnValue = (firstValue - _lastValue);
    else if (op == "*=")
        returnValue = (firstValue * _lastValue);
    else if (op == "/=")
        returnValue = (firstValue / _lastValue);
    else if (op == "**=")
        returnValue = pow(firstValue, _lastValue);
    else if (op == "=")
        returnValue = _lastValue;

    return (returnValue);
}

/**

@s = "This is a string."
@n = 3.14 # and that's a number.

**/
void twoSpace(string argZero, string argOne, string argTwo, string s, vector<string> command)
{
    string last_val = "";

    if (contains(argTwo, "self."))
        argTwo = replace(argTwo, "self", currentMethodObject);

    if (contains(argZero, "self."))
        argZero = replace(argZero, "self", currentMethodObject);

	if (variableExists(argZero)) {
		if (objectExists(beforeDot(argZero)) || startsWith(argZero, "@")) {
            if (objectExists(beforeDot(argZero))) {
                if (objects.at(objectAt(beforeDot(argZero))).getVariable(afterDot(argZero)).getString() != null) {
                    string tempObjectVariableName("@____" + beforeDot(argZero) + "___" + afterDot(argZero) + "__string_var");

                    createVariable(tempObjectVariableName, objects.at(objectAt(beforeDot(argZero))).getVariable(afterDot(argZero)).getString());

                    twoSpace(tempObjectVariableName, argOne, argTwo, tempObjectVariableName + " " + argOne + " " + argTwo, command);

                    variables.at(variableAt(tempObjectVariableName)).setName(afterDot(argZero));

                    objects.at(objectAt(beforeDot(argZero))).removeVariable(afterDot(argZero));
                    objects.at(objectAt(beforeDot(argZero))).addVariable(variables.at(variableAt(afterDot(argZero))));
                    variables = removeVariable(variables, afterDot(argZero));
                } else if (objects.at(objectAt(beforeDot(argZero))).getVariable(afterDot(argZero)).getNumber() != nullNum) {
                    string tempObjectVariableName("@____" + beforeDot(argZero) + "___" + afterDot(argZero) + "__number_var");

                    createVariable(tempObjectVariableName, objects.at(objectAt(beforeDot(argZero))).getVariable(afterDot(argZero)).getNumber());

                    twoSpace(tempObjectVariableName, argOne, argTwo, tempObjectVariableName + " " + argOne + " " + argTwo, command);

                    variables.at(variableAt(tempObjectVariableName)).setName(afterDot(argZero));

                    objects.at(objectAt(beforeDot(argZero))).removeVariable(afterDot(argZero));
                    objects.at(objectAt(beforeDot(argZero))).addVariable(variables.at(variableAt(afterDot(argZero))));
                    variables = removeVariable(variables, afterDot(argZero));
                }
            } else if (argOne == "=") {
				string before(beforeDot(argTwo)), after(afterDot(argTwo));

                if (containsBrackets(argTwo) && (variableExists(beforeBrackets(argTwo)) || listExists(beforeBrackets(argTwo)))) {
                    string beforeBracket(beforeBrackets(argTwo)), afterBracket(afterBrackets(argTwo));

                    afterBracket = subtractString(afterBracket, "]");

                    if (listExists(beforeBracket)) {
                        if (lists.at(listAt(beforeBracket)).size() >= stoi(afterBracket)) {
                            if (lists.at(listAt(beforeBracket)).at(stoi(afterBracket)) == "#!=no_line")
                                error("invalid_operation:index_out_of_bounds:" + argTwo, false);
                            else {
                                string listValue(lists.at(listAt(beforeBracket)).at(stoi(afterBracket)));

                                if (isNumeric(listValue)) {
                                    if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                        setVariable(argZero, stod(listValue));
                                    else
                                        error("conversion_error:" + argZero, false);
                                } else {
                                    if (variables.at(variableAt(argZero)).getString() != null)
                                        setVariable(argZero, listValue);
                                    else
                                        error("conversion_error:" + argZero, false);
                                }
                            }
                        }
                    } else if (variables.at(variableAt(beforeBracket)).getString() != null)
                        setSubString(argZero, argTwo, beforeBracket);
                    else
                        error("invalid_operation:list_undefined:" + beforeBracket, false);
                } else if (before.length() != 0 && after.length() != 0) {
				    if (containsParams(argTwo)) {
                        if (beforeParams(argTwo) == "random") {
                            if (contains(argTwo, "..")) {
                                vector<string> range = getRange(argTwo);
                                string s0(range.at(0)), s2(range.at(1));

                                if (isNumeric(s0) && isNumeric(s2)) {
                                    if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                        double n0 = stod(s0), n2 = stod(s2);

                                        if (n0 < n2) {
                                            setVariable(argZero, (int)random(n0, n2));
                                            setLastValue(itos((int)random(n0, n2)));
                                        } else if (n0 > n2) {
                                            setVariable(argZero, (int)random(n2, n0));
                                            setLastValue(itos((int)random(n2, n0)));
                                        } else {
                                            setVariable(argZero, (int)random(n0, n2));
                                            setLastValue(itos((int)random(n0, n2)));
                                        }
                                    } else if (variables.at(variableAt(argZero)).getString() != null) {
                                        double n0 = stod(s0), n2 = stod(s2);

                                        if (n0 < n2) {
                                            setVariable(argZero, itos((int)random(n0, n2)));
                                            setLastValue(itos((int)random(n0, n2)));
                                        } else if (n0 > n2) {
                                            setVariable(argZero, itos((int)random(n2, n0)));
                                            setLastValue(itos((int)random(n2, n0)));
                                        } else {
                                            setVariable(argZero, itos((int)random(n0, n2)));
                                            setLastValue(itos((int)random(n0, n2)));
                                        }
                                    } else
                                        error("special_error(7)", false);
                                } else if (isAlpha(s0) && isAlpha(s2)) {
                                    if (variables.at(variableAt(argZero)).getString() != null) {
                                        if (get_alpha_num(s0[0]) < get_alpha_num(s2[0])) {
                                            setVariable(argZero, random(s0, s2));
                                            setLastValue(random(s0, s2));
                                        } else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0])) {
                                            setVariable(argZero, random(s2, s0));
                                            setLastValue(random(s2, s0));
                                        } else {
                                            setVariable(argZero, random(s2, s0));
                                            setLastValue(random(s2, s0));
                                        }
                                    } else
                                        error("invalid_operation:null_string:" + argZero, false);
                                } else if (variableExists(s0) || variableExists(s2)) {
                                    if (variableExists(s0)) {
                                        if (variables.at(variableAt(s0)).getNumber() != nullNum)
                                            s0 = dtos(variables.at(variableAt(s0)).getNumber());
                                        else if (variables.at(variableAt(s0)).getString() != null)
                                            s0 = variables.at(variableAt(s0)).getString();
                                    }

                                    if (variableExists(s2)) {
                                        if (variables.at(variableAt(s2)).getNumber() != nullNum)
                                            s2 = dtos(variables.at(variableAt(s2)).getNumber());
                                        else if (variables.at(variableAt(s2)).getString() != null)
                                            s2 = variables.at(variableAt(s2)).getString();
                                    }

                                    if (isNumeric(s0) && isNumeric(s2)) {
                                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                            double n0 = stod(s0), n2 = stod(s2);

                                            if (n0 < n2) {
                                                setVariable(argZero, (int)random(n0, n2));
                                                setLastValue(itos((int)random(n0, n2)));
                                            } else if (n0 > n2) {
                                                setVariable(argZero, (int)random(n2, n0));
                                                setLastValue(itos((int)random(n2, n0)));
                                            } else {
                                                setVariable(argZero, (int)random(n0, n2));
                                                setLastValue(itos((int)random(n0, n2)));
                                            }
                                        } else if (variables.at(variableAt(argZero)).getString() != null) {
                                            double n0 = stod(s0), n2 = stod(s2);

                                            if (n0 < n2) {
                                                setVariable(argZero, itos((int)random(n0, n2)));
                                                setLastValue(itos((int)random(n0, n2)));
                                            } else if (n0 > n2) {
                                                setVariable(argZero, itos((int)random(n2, n0)));
                                                setLastValue(itos((int)random(n2, n0)));
                                            } else {
                                                setVariable(argZero, itos((int)random(n0, n2)));
                                                setLastValue(itos((int)random(n0, n2)));
                                            }
                                        } else
                                            error("special_error(7)", false);
                                    } else if (isAlpha(s0) && isAlpha(s2)) {
                                        if (variables.at(variableAt(argZero)).getString() != null) {
                                            if (get_alpha_num(s0[0]) < get_alpha_num(s2[0])) {
                                                setVariable(argZero, random(s0, s2));
                                                setLastValue(random(s0, s2));
                                            } else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0])) {
                                                setVariable(argZero, random(s2, s0));
                                                setLastValue(random(s2, s0));
                                            } else {
                                                setVariable(argZero, random(s2, s0));
                                                setLastValue(random(s2, s0));
                                            }
                                        } else
                                            error("invalid_operation:null_string:" + argZero, false);
                                    }
                                } else
                                    error("invalid_operation:invalid_sequence:" + s0 + "_" + s2, false);
                            } else
                                error("invalid_operation:invalid_sequence_separator:" + argTwo, false);
                        }
				    } else if (listExists(before) && after == "size") {
				        if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                            setVariable(argZero, stod(itos(lists.at(listAt(before)).size())));
                        else if (variables.at(variableAt(argZero)).getString() != null)
                            setVariable(argZero, itos(lists.at(listAt(before)).size()));
                        else
                            error("is_null:" + argZero, false);
				    } else if (before == "self") {
                        if (objectExists(currentMethodObject))
                            twoSpace(argZero, argOne, (currentMethodObject + "." + after), (argZero + " " + argOne + " " + (currentMethodObject + "." + after)), command);
                        else
                            twoSpace(argZero, argOne, after, (argZero + " " + argOne + " " + after), command);
                    } else if (objectExists(before)) {
						if (objects.at(objectAt(before)).variableExists(after)) {
							if (objects.at(objectAt(before)).getVariable(after).getString() != null)
								setVariable(argZero, objects.at(objectAt(before)).getVariable(after).getString());
                            else if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
                                setVariable(argZero, objects.at(objectAt(before)).getVariable(after).getNumber());
                            else
								error("is_null:" + argTwo, false);
						} else if (objects.at(objectAt(before)).methodExists(after) && !containsParams(after)) {
                            parse(argTwo);

                            if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, lastValue);
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, stod(lastValue));
						} else if (containsParams(after)) {
                            if (objects.at(objectAt(before)).methodExists(beforeParams(after))) {
                                executeTemplate(objects.at(objectAt(before)).getMethod(beforeParams(after)), getParams(after));

                                if (isNumeric(lastValue)) {
                                    if (variables.at(variableAt(argZero)).getString() != null)
                                        setVariable(argZero, lastValue);
                                    else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                        setVariable(argZero, stod(lastValue));
                                    else
                                        error("is_null:" + argZero, false);
                                } else {
                                    if (variables.at(variableAt(argZero)).getString() != null)
                                        setVariable(argZero, lastValue);
                                    else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                        error("conversion_error:" + argZero, false);
                                    else
                                        error("is_null:" + argZero, false);
                                }
                            } else
                                sysExec(s, command);
						} else
							error("invalid_operation:variable_undefined:" + argTwo, false);
					} else if (before == "env") {
						if (after == "cwd")
							setVariable(argZero, cwd());
                        else if (after == "usl")
							setVariable(argZero, uslBinary);
						else if (after == "os")
                            setVariable(argZero, guessedOS);
						else if (after == "user")
                            setVariable(argZero, getUser());
						else if (after == "machine")
                            setVariable(argZero, getMachine());
                        else if (after == "init_dir" || after == "initial_directory")
                            setVariable(argZero, initDir);
                        else if (after == "am_or_pm")
                            setVariable(argZero, amOrPm());
                        else if (after == "now")
                            setVariable(argZero, timeNow());
                        else if (after == "day_of_this_week")
                            setVariable(argZero, dayOfTheWeek());
                        else if (after == "day_of_this_month")
                            setVariable(argZero, dayOfTheMonth());
                        else if (after == "day_of_this_year")
                            setVariable(argZero, dayOfTheYear());
                        else if (after == "month_of_this_year")
                            setVariable(argZero, monthOfTheYear());
                        else if (after == "this_second")
                            setVariable(argZero, secondNow());
                        else if (after == "this_minute")
                            setVariable(argZero, minuteNow());
                        else if (after == "this_hour")
                            setVariable(argZero, hourNow());
                        else if (after == "this_month")
                            setVariable(argZero, monthNow());
                        else if (after == "this_year")
                            setVariable(argZero, yearNow());
                        else if (after == "empty_string")
                            setVariable(argZero, "");
                        else if (after == "empty_number")
                            setVariable(argZero, 0);
                        else if (after == "last_error")
                            setVariable(argZero, lastError);
                        else if (after == "last_value")
                            setVariable(argZero, lastValue);
						else
                            setVariable(argZero, getEnvironmentVariable(after));
					} else if (after == "to_int") {
					    if (variableExists(before)) {
					        if (variables.at(variableAt(before)).getString() != null)
					            setVariable(argZero, (int)variables.at(variableAt(before)).getString()[0]);
					        else if (variables.at(variableAt(before)).getNumber() != nullNum)  {
                                int i = (int)variables.at(variableAt(before)).getNumber();
                                setVariable(argZero, (double)i);
                            } else
                                error("is_null:" + before, false);
					    } else
                            error("invalid_operation:variable_undefined:" + before, false);
					} else if (after == "to_double") {
					    if (variableExists(before)) {
					        if (variables.at(variableAt(before)).getString() != null)
					            setVariable(argZero, (double)variables.at(variableAt(before)).getString()[0]);
					        else if (variables.at(variableAt(before)).getNumber() != nullNum) {
                                double i = variables.at(variableAt(before)).getNumber();
                                setVariable(argZero, (double)i);
                            } else
                                error("is_null:" + before, false);
					    } else
                            error("invalid_operation:variable_undefined:" + before, false);
					} else if (after == "to_string") {
					    if (variableExists(before)) {
					        if (variables.at(variableAt(before)).getNumber() != nullNum)
                                setVariable(argZero, dtos(variables.at(variableAt(before)).getNumber()));
                            else
                                error("is_null:" + before, false);
					    } else
                            error("invalid_operation:variable_undefined:" + before, false);
					} else if (after == "to_number") {
					    if (variableExists(before)) {
					        if (variables.at(variableAt(before)).getString() != null)
                                setVariable(argZero, stod(variables.at(variableAt(before)).getString()));
                            else
                                error("is_null:" + before, false);
					    } else
                            error("invalid_operation:variable_undefined:" + before, false);
					} else if (before == "chomp") {
						if (variableExists(after)) {
							if (variables.at(variableAt(after)).getString() != null) {
								string chomper("");
								__stdout(cleanString(variables.at(variableAt(after)).getString()));
								getline(cin, chomper, '\n');

								if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
									if (isNumeric(chomper))
										setVariable(argZero, stod(chomper));
                                    else
										error("conversion_error:" + chomper, false);
								} else if (variables.at(variableAt(argZero)).getString() != null)
									setVariable(argZero, chomper);
                                else
									error("is_null:" + argZero, false);
							} else {
								string chomper("");
								cout << "chomp: ";
								getline(cin, chomper, '\n');

								if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
									if (isNumeric(chomper))
										setVariable(argZero, stod(chomper));
									else
										error("conversion_error:" + chomper, false);
								} else if (variables.at(variableAt(argZero)).getString() != null)
                                    setVariable(argZero, chomper);
								else
									error("is_null:" + argZero, false);
							}
						} else {
							string chomper("");
							cout << cleanString(after);
							getline(cin, chomper, '\n');

							if (isNumeric(chomper))
								setVariable(argZero, stod(chomper));
							else
							    setVariable(argZero, chomper);
						}
					} else if (before == "shomp") {
						if (variableExists(after)) {
							if (variables.at(variableAt(after)).getString() != null) {
								string chomper("");
								chomper = shomp(variables.at(variableAt(after)).getString());

								if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
									if (isNumeric(chomper))
                                        setVariable(argZero, stod(chomper));
                                    else
										error("conversion_error:" + chomper, false);
								} else if (variables.at(variableAt(argZero)).getString() != null)
                                    setVariable(argZero, chomper);
                                else
									error("is_null:" + argZero, false);

                                cout << endl;
							} else {
								string chomper("");
								chomper = shomp("shomp: ");

								if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
									if (isNumeric(chomper))
                                        setVariable(argZero, stod(chomper));
									else
										error("conversion_error:" + chomper, false);
								} else if (variables.at(variableAt(argZero)).getString() != null)
                                    setVariable(argZero, chomper);
								else
									error("is_null:" + argZero, false);

                                cout << endl;
							}
						} else {
							string chomper("");
							chomper = shomp(cleanString(after));

							if (isNumeric(chomper))
								setVariable(argZero, stod(chomper));
							else
								setVariable(argZero, chomper);

                            cout << endl;
						}
					} else if (after == "cos") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, cos(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(cos(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "acos") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, acos(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(acos(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "cosh") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, cosh(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(cosh(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "log") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, log(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(log(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "sqrt") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, sqrt(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(sqrt(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "abs") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, abs(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(abs(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "floor") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, floor(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(floor(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "ceil") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, ceil(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(ceil(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "exp") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, exp(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(exp(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "sin") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, sin(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(sin(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "sinh") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, sinh(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(sinh(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "asin") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, asin(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(asin(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "tan") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, tan(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(tan(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "tanh") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, tanh(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(tanh(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "atan") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, atan(variables.at(variableAt(before)).getNumber()));
                                else
                                    error("conversion_error:" + before, false);
                            } else if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getNumber() != nullNum)
                                    setVariable(argZero, dtos(atan(variables.at(variableAt(before)).getNumber())));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "to_lower") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getString() != null)
                                    setVariable(argZero, getLower(variables.at(variableAt(before)).getString()));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "read") {
                        if (variables.at(variableAt(argZero)).getString() != null) {
                            if (variableExists(before)) {
                                if (variables.at(variableAt(before)).getString() != null) {
                                    if (fileExists(variables.at(variableAt(before)).getString())) {
                                        ifstream file(variables.at(variableAt(before)).getString().c_str());
                                        string line(""), bigString("");

                                        if (file.is_open()) {
                                            while (!file.eof()) {
                                                getline(file, line);
                                                bigString.append(line + "\r\n");
                                            }

                                            file.close();

                                            setVariable(argZero, bigString);
                                        } else
                                            error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                                    } else
                                        error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                                } else
                                    error("invalid_operation:null_string:" + before, false);
                            } else {
                                if (fileExists(before)) {
                                    ifstream file(before.c_str());
									string line(""), bigString("");

									if (file.is_open()) {
										while (!file.eof()) {
											getline(file, line);
											bigString.append(line + "\r\n");
										}

										file.close();

                                        setVariable(argZero, bigString);
									} else
										error("read_fail:" + before, false);
                                } else
                                    error("read_fail:" + before, false);
                            }
                        } else
                            error("invalid_operation:null_string:" + argZero, false);
                    } else if (after == "to_upper") {
                        if (variableExists(before)) {
                            if (variables.at(variableAt(argZero)).getString() != null) {
                                if (variables.at(variableAt(before)).getString() != null)
                                    setVariable(argZero, getUpper(variables.at(variableAt(before)).getString()));
                                else
                                    error("conversion_error:" + before, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
                    } else if (after == "size") {
						if (variableExists(before)) {
							if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
								if (variables.at(variableAt(before)).getString() != null)
                                    setVariable(argZero, (double)variables.at(variableAt(before)).getString().length());
                                else
									error("conversion_error:" + before, false);
							} else
								error("conversion_error:" + argZero, false);
						} else {
							if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, (double)before.length());
                            else
								error("conversion_error:" + argZero, false);
						}
					} else if (after == "bytes") {
                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (variableExists(before)) {
                                 if (variables.at(variableAt(before)).getString() != null) {
                                     if (fileExists(variables.at(variableAt(before)).getString()))
                                         setVariable(argZero, getBytes(variables.at(variableAt(before)).getString()));
                                     else
                                         error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                                 } else
                                     error("conversion_error:" + before, false);
                            } else {
                                if (fileExists(before))
                                    setVariable(argZero, getBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        } else
                            error("conversion_error:" + argZero, false);
                    } else if (after == "kbytes") {
                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (variableExists(before)) {
                                 if (variables.at(variableAt(before)).getString() != null) {
                                     if (fileExists(variables.at(variableAt(before)).getString()))
                                         setVariable(argZero, getKBytes(variables.at(variableAt(before)).getString()));
                                     else
                                         error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                                 } else
                                     error("conversion_error:" + before, false);
                            } else {
                                if (fileExists(before))
                                    setVariable(argZero, getKBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        } else
                            error("conversion_error:" + argZero, false);
                    } else if (after == "mbytes") {
                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (variableExists(before)) {
                                 if (variables.at(variableAt(before)).getString() != null) {
                                     if (fileExists(variables.at(variableAt(before)).getString()))
                                         setVariable(argZero, getMBytes(variables.at(variableAt(before)).getString()));
                                     else
                                         error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                                 } else
                                     error("conversion_error:" + before, false);
                            } else {
                                if (fileExists(before))
                                    setVariable(argZero, getMBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        } else
                            error("conversion_error:" + argZero, false);
                    } else if (after == "gbytes") {
                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (variableExists(before)) {
                                 if (variables.at(variableAt(before)).getString() != null) {
                                     if (fileExists(variables.at(variableAt(before)).getString()))
                                         setVariable(argZero, getGBytes(variables.at(variableAt(before)).getString()));
                                     else
                                         error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                                 } else
                                     error("conversion_error:" + before, false);
                            } else {
                                if (fileExists(before))
                                    setVariable(argZero, getGBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        } else
                            error("conversion_error:" + argZero, false);
                    } else if (after == "tbytes") {
                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (variableExists(before)) {
                                 if (variables.at(variableAt(before)).getString() != null) {
                                     if (fileExists(variables.at(variableAt(before)).getString()))
                                         setVariable(argZero, getTBytes(variables.at(variableAt(before)).getString()));
                                     else
                                         error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                                 } else
                                     error("conversion_error:" + before, false);
                            } else {
                                if (fileExists(before))
                                    setVariable(argZero, getTBytes(before));
                                else
                                    error("read_fail:" + before, false);
                            }
                        } else
                            error("conversion_error:" + argZero, false);
                    } else {
						if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
							if (isNumeric(argTwo))
                                setVariable(argZero, stod(argTwo));
                            else
								error("conversion_error:" + argZero, false);
						} else if (variables.at(variableAt(argZero)).getString() != null)
                            setVariable(argZero, argTwo);
                        else if (variables.at(variableAt(argZero)).waiting()) {
                            if (isNumeric(argTwo))
                                setVariable(argZero, stod(before + "." + after));
                            else
                                setVariable(argZero, argTwo);
                        } else
							error("is_null:" + argZero, false);
					}
				} else {
                    if (variables.at(variableAt(argZero)).waiting()) {
                        if (isNumeric(argTwo))
                            setVariable(argZero, stod(argTwo));
                        else
                            setVariable(argZero, argTwo);
                    } else if (argTwo == "null") {
                        if (variables.at(variableAt(argZero)).getString() != null)
                            variables.at(variableAt(argZero)).setNull();
                        else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                            variables.at(variableAt(argZero)).setNull();
                        else
                            error("is_null:" + argZero, false);
                    } else if (constantExists(argTwo)) {
                        if (variables.at(variableAt(argZero)).getString() != null) {
                            if (constants.at(constAt(argTwo)).isNumber())
                                setVariable(argZero, dtos(constants.at(constAt(argTwo)).getNumber()));
                            else if (constants.at(constAt(argTwo)).isString())
                                setVariable(argZero, constants.at(constAt(argTwo)).getString());
                        } else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (constants.at(constAt(argTwo)).isNumber())
                                setVariable(argZero, constants.at(constAt(argTwo)).getNumber());
                            else
                                error("conversion_error:null_number:" + argTwo, false);
                        } else
                            error("is_null:" + argZero, false);
                    } else if (methodExists(argTwo)) {
                         parse(argTwo);

                         if (variables.at(variableAt(argZero)).getString() != null)
                             setVariable(argZero, lastValue);
                         else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                             setVariable(argZero, stod(lastValue));
                    } else if (variableExists(argTwo)) {
						if (variables.at(variableAt(argTwo)).getString() != null) {
							if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, variables.at(variableAt(argTwo)).getString());
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                error("conversion_error:" + argTwo, false);
                            else
								error("is_null:" + argZero, false);
						} else if (variables.at(variableAt(argTwo)).getNumber() != nullNum) {
							if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, dtos(variables.at(variableAt(argTwo)).getNumber()));
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, variables.at(variableAt(argTwo)).getNumber());
                            else
								error("is_null:" + argZero, false);
						} else
							error("is_null:" + argTwo, false);
					} else if (argTwo == "shomp" || argTwo == "chomp") {
						if (argTwo == "shomp") {
							string shomper("");
                            shomper = shomp("");

							if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
								if (isNumeric(shomper))
                                    setVariable(argZero, stod(shomper));
                                else
									error("conversion_error:" + shomper, false);
							} else if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, shomper);
                            else
                                setVariable(argZero, shomper);
						} else {
							string chomper("");
							cout << "chomp: ";
							getline(cin, chomper, '\n');

							if (isNumeric(chomper))
								createVariable(argZero, stod(chomper));
							else
                                createVariable(argZero, chomper);
						}
					} else if (containsParams(argTwo)) {
                        if (methodExists(beforeParams(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, lastValue);
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, stod(lastValue));
                        } else if (isStringStack(argTwo)) {
                            if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, getStringStack(argTwo));
                            else
                                error("conversion_error:" + argZero, false);
                        } else if (stackReady(argTwo)) {
                            if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, dtos(getStack(argTwo)));
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, getStack(argTwo));
                            else
                                error("is_null:" + argZero, false);
                        }
                    } else {
						if (isNumeric(argTwo)) {
							if (variables.at(variableAt(argZero)).getNumber() != nullNum)
								setVariable(argZero, stod(argTwo));
							else if (variables.at(variableAt(argZero)).getString() != null)
								setVariable(argZero, argTwo);
						} else {
							if (variables.at(variableAt(argZero)).getNumber() != nullNum)
								error("conversion_error:" + argZero, false);
							else if (variables.at(variableAt(argZero)).getString() != null)
								setVariable(argZero, cleanString(argTwo));
						}
					}
				}
			} else if (argOne == "+=") {
			    if (variableExists(argTwo)) {
					if (variables.at(variableAt(argZero)).getString() != null) {
						if (variables.at(variableAt(argTwo)).getString() != null)
							setVariable(argZero, variables.at(variableAt(argZero)).getString() + variables.at(variableAt(argTwo)).getString());
						else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
							setVariable(argZero, variables.at(variableAt(argZero)).getString() + dtos(variables.at(variableAt(argTwo)).getNumber()));
						else
							error("is_null:" + argTwo, false);
					} else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
						if (variables.at(variableAt(argTwo)).getString() != null)
							error("conversion_error:" + argTwo, false);
						else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
							setVariable(argZero, variables.at(variableAt(argZero)).getNumber() + variables.at(variableAt(argTwo)).getNumber());
						else
							error("is_null:" + argTwo, false);
					} else
						error("is_null:" + argZero, false);
				} else {
				    if (containsParams(argTwo)) {
				        if (isStringStack(argTwo)) {
				            if (variables.at(variableAt(argZero)).getString() != null)
				                setVariable(argZero, variables.at(variableAt(argZero)).getString() + getStringStack(argTwo));
                            else
                                error("conversion_error:" + argZero, false);
				        } else if (stackReady(argTwo)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, variables.at(variableAt(argZero)).getNumber() + getStack(argTwo));
                        } else if (methodExists(beforeParams(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, variables.at(variableAt(argZero)).getString() + lastValue);
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, variables.at(variableAt(argZero)).getNumber() + stod(lastValue));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("is_null:" + argZero, false);
                        } else if (objectExists(beforeDot(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, variables.at(variableAt(argZero)).getString() + lastValue);
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, variables.at(variableAt(argZero)).getNumber() + stod(lastValue));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
				    } else if (methodExists(argTwo)) {
				        parse(argTwo);

                        if (variables.at(variableAt(argZero)).getString() != null)
                            setVariable(argZero, variables.at(variableAt(argZero)).getString() + lastValue);
                        else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (isNumeric(lastValue))
                                setVariable(argZero, variables.at(variableAt(argZero)).getNumber() + stod(lastValue));
                            else
                                error("conversion_error:" + argZero, false);
                        } else
                            error("is_null:" + argZero, false);
				    } else if (isNumeric(argTwo)) {
						if (variables.at(variableAt(argZero)).getString() != null)
                            setVariable(argZero, variables.at(variableAt(argZero)).getString() + argTwo);
                        else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                            setVariable(argZero, variables.at(variableAt(argZero)).getNumber() + stod(argTwo));
                        else
							error("is_null:" + argZero, false);
					} else {
						if (variables.at(variableAt(argZero)).getString() != null)
                            setVariable(argZero, variables.at(variableAt(argZero)).getString() + cleanString(argTwo));
                        else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
							error("conversion_error:" + argZero, false);
						else
							error("is_null:" + argZero, false);
					}
				}
			} else if (argOne == "-=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argZero)).getString() != null) {
						if (variables.at(variableAt(argTwo)).getString() != null) {
							if (variables.at(variableAt(argTwo)).getString().length() == 1)
								setVariable(argZero, subtractChar(variables.at(variableAt(argZero)).getString(), variables.at(variableAt(argTwo)).getString()));
                            else
								setVariable(argZero, subtractString(variables.at(variableAt(argZero)).getString(), variables.at(variableAt(argTwo)).getString()));
						} else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
							setVariable(argZero, subtractString(variables.at(variableAt(argZero)).getString(), dtos(variables.at(variableAt(argTwo)).getNumber())));
						else
							error("is_null:" + argTwo, false);
					} else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
						if (variables.at(variableAt(argTwo)).getString() != null)
							error("conversion_error:" + argTwo, false);
						else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
							setVariable(argZero, variables.at(variableAt(argZero)).getNumber() - variables.at(variableAt(argTwo)).getNumber());
						else
							error("is_null:" + argTwo, false);
					} else
						error("is_null:" + argZero, false);
				} else {
				    if (containsParams(argTwo)) {
				        if (isStringStack(argTwo)) {
				            if (variables.at(variableAt(argZero)).getString() != null)
				                setVariable(argZero, subtractString(variables.at(variableAt(argZero)).getString(), getStringStack(argTwo)));
				            else
                                error("conversion_error:" + argZero, false);
				        } else if (stackReady(argTwo)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, variables.at(variableAt(argZero)).getNumber() - getStack(argTwo));
                        } else if (methodExists(beforeParams(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, subtractString(variables.at(variableAt(argZero)).getString(), lastValue));
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, variables.at(variableAt(argZero)).getNumber() - stod(lastValue));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("is_null:" + argZero, false);
                        } else if (objectExists(beforeDot(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getString() != null)
                                setVariable(argZero, subtractString(variables.at(variableAt(argZero)).getString(), lastValue));
                            else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, variables.at(variableAt(argZero)).getNumber() - stod(lastValue));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("is_null:" + argZero, false);
                        }
				    } else if (methodExists(argTwo)) {
                        parse(argTwo);

                        if (variables.at(variableAt(argZero)).getString() != null)
                            setVariable(argZero, subtractString(variables.at(variableAt(argZero)).getString(), lastValue));
                        else if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (isNumeric(lastValue))
                                setVariable(argZero, variables.at(variableAt(argZero)).getNumber() - stod(lastValue));
                            else
                                error("conversion_error:" + argZero, false);
                        } else
                            error("is_null:" + argZero, false);
				    } else if (isNumeric(argTwo)) {
						if (variables.at(variableAt(argZero)).getString() != null) {
							if (argTwo.length() == 1)
								setVariable(argZero, subtractChar(variables.at(variableAt(argZero)).getString(), argTwo));
                            else
                                setVariable(argZero, subtractString(variables.at(variableAt(argZero)).getString(), argTwo));
                        } else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                            setVariable(argZero, variables.at(variableAt(argZero)).getNumber() - stod(argTwo));
                        else
							error("is_null:" + argZero, false);
					} else {
						if (variables.at(variableAt(argZero)).getString() != null) {
							if (argTwo.length() == 1)
                                setVariable(argZero, subtractChar(variables.at(variableAt(argZero)).getString(), argTwo));
                            else
                                setVariable(argZero, subtractString(variables.at(variableAt(argZero)).getString(), cleanString(argTwo)));
                        } else if (variables.at(variableAt(argZero)).getNumber() != nullNum)
							error("conversion_error:" + argZero, false);
						else
							error("is_null:" + argZero, false);
					}
				}
			} else if (argOne == "*=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
                        setVariable(argZero, variables.at(variableAt(argZero)).getNumber() * variables.at(variableAt(argTwo)).getNumber());
                    else if (variables.at(variableAt(argTwo)).getString() != null)
						error("conversion_error:" + argTwo, false);
					else
						error("is_null:" + argTwo, false);
				} else {
				    if (containsParams(argTwo)) {
				        if (stackReady(argTwo)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, variables.at(variableAt(argZero)).getNumber() * getStack(argTwo));
                        } else if (methodExists(beforeParams(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, variables.at(variableAt(argZero)).getNumber() * stod(lastValue));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("invalid_operation:null_number:" + argZero, false);
                        } else if (objectExists(beforeDot(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, variables.at(variableAt(argZero)).getNumber() * stod(lastValue));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("invalid_operation:null_number:" + argZero, false);
                        }
				    } else if (methodExists(argTwo)) {
                        parse(argTwo);

                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (isNumeric(lastValue))
                                setVariable(argZero, variables.at(variableAt(argZero)).getNumber() * stod(lastValue));
                            else
                                error("conversion_error:" + argZero, false);
                        } else
                            error("invalid_operation:null_number:" + argZero, false);
				    } else if (isNumeric(argTwo)) {
						if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                            setVariable(argZero, variables.at(variableAt(argZero)).getNumber() * stod(argTwo));
                    } else
						setVariable(argZero, cleanString(argTwo));
				}
			} else if (argOne == "%=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
                        setVariable(argZero, (int)variables.at(variableAt(argZero)).getNumber() % (int)variables.at(variableAt(argTwo)).getNumber());
                    else if (variables.at(variableAt(argTwo)).getString() != null)
						error("conversion_error:" + argTwo, false);
					else
						error("is_null:" + argTwo, false);
				} else if (methodExists(argTwo)) {
                    parse(argTwo);

                    if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                        if (isNumeric(lastValue))
                            setVariable(argZero, (int)variables.at(variableAt(argZero)).getNumber() % (int)stod(lastValue));
                        else
                            error("conversion_error:" + argZero, false);
                    } else
                        error("invalid_operation:null_number:" + argZero, false);
				} else {
					if (isNumeric(argTwo)) {
						if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                            setVariable(argZero, (int)variables.at(variableAt(argZero)).getNumber() % (int)stod(argTwo));
                    } else
                        setVariable(argZero, cleanString(argTwo));
				}
			} else if (argOne == "**=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
                        setVariable(argZero, pow(variables.at(variableAt(argZero)).getNumber(), variables.at(variableAt(argTwo)).getNumber()));
                    else if (variables.at(variableAt(argTwo)).getString() != null)
						error("conversion_error:" + argTwo, false);
					else
						error("is_null:" + argTwo, false);
				} else {
				    if (containsParams(argTwo)) {
				        if (stackReady(argTwo)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, pow(variables.at(variableAt(argZero)).getNumber(), (int)getStack(argTwo)));
                        } else if (methodExists(beforeParams(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, pow(variables.at(variableAt(argZero)).getNumber(), (int)stod(lastValue)));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("invalid_operation:null_number:" + argZero, false);
                        } else if (objectExists(beforeDot(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, pow(variables.at(variableAt(argZero)).getNumber(), (int)stod(lastValue)));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("invalid_operation:null_number:" + argZero, false);
                        }
				    } else if (methodExists(argTwo)) {
                        parse(argTwo);

                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (isNumeric(lastValue))
                                setVariable(argZero, pow(variables.at(variableAt(argZero)).getNumber(), (int)stod(lastValue)));
                            else
                                error("conversion_error:" + argZero, false);
                        } else
                            error("invalid_operation:null_number:" + argZero, false);
				    } else if (isNumeric(argTwo)) {
						if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                            setVariable(argZero, pow(variables.at(variableAt(argZero)).getNumber(), stod(argTwo)));
                    } else
                        setVariable(argZero, cleanString(argTwo));
				}
			} else if (argOne == "/=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
                        setVariable(argZero, variables.at(variableAt(argZero)).getNumber() / variables.at(variableAt(argTwo)).getNumber());
                    else if (variables.at(variableAt(argTwo)).getString() != null)
						error("conversion_error:" + argTwo, false);
					else
						error("is_null:" + argTwo, false);
				} else {
				    if (containsParams(argTwo)) {
				        if (stackReady(argTwo)) {
                            if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                                setVariable(argZero, variables.at(variableAt(argZero)).getNumber() / getStack(argTwo));
                        } else if (methodExists(beforeParams(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, variables.at(variableAt(argZero)).getNumber() / stod(lastValue));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("invalid_operation:null_number:" + argZero, false);
                        } else if (objectExists(beforeDot(argTwo))) {
                            executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                            if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                                if (isNumeric(lastValue))
                                    setVariable(argZero, variables.at(variableAt(argZero)).getNumber() / stod(lastValue));
                                else
                                    error("conversion_error:" + argZero, false);
                            } else
                                error("invalid_operation:null_number:" + argZero, false);
                        }
				    } else if (methodExists(argTwo)) {
                        parse(argTwo);

                        if (variables.at(variableAt(argZero)).getNumber() != nullNum) {
                            if (isNumeric(lastValue))
                                setVariable(argZero, variables.at(variableAt(argZero)).getNumber() / stod(lastValue));
                            else
                                error("conversion_error:" + argZero, false);
                        } else
                            error("invalid_operation:null_number:" + argZero, false);
				    } else if (isNumeric(argTwo)) {
						if (variables.at(variableAt(argZero)).getNumber() != nullNum)
                            setVariable(argZero, variables.at(variableAt(argZero)).getNumber() / stod(argTwo));
					} else
					    setVariable(argZero, cleanString(argTwo));
                }
			} else if (argOne == "++=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getNumber() != nullNum) {
						if (variables.at(variableAt(argZero)).getString() != null) {
							int tempVarNumber((int)variables.at(variableAt(argTwo)).getNumber());
							string tempVarString(variables.at(variableAt(argZero)).getString());
							int len(tempVarString.length());
							string cleaned("");

							for (int i = 0; i < len; i++)
								cleaned.push_back((char)(((int)tempVarString[i]) + tempVarNumber));

							setVariable(argZero, cleaned);
						} else
							error("is_null:" + argZero, false);
					} else
						error("conversion_error:" + argTwo, false);
				} else {
					if (isNumeric(argTwo)) {
						int tempVarNumber(stoi(argTwo));
						string tempVarString(variables.at(variableAt(argZero)).getString());

						if (tempVarString != null) {
							int len(tempVarString.length());
							string cleaned("");

							for (int i = 0; i < len; i++)
								cleaned.push_back((char)(((int)tempVarString[i]) + tempVarNumber));

							setVariable(argZero, cleaned);
						} else
							error("is_null:" + tempVarString, false);
					} else
						error("conversion_error:" + argTwo, false);
				}
			} else if (argOne == "--=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getNumber() != nullNum) {
						if (variables.at(variableAt(argZero)).getString() != null) {
							int tempVarNumber((int)variables.at(variableAt(argTwo)).getNumber());
							string tempVarString(variables.at(variableAt(argZero)).getString());
							int len(tempVarString.length());
							string cleaned("");

							for (int i = 0; i < len; i++)
								cleaned.push_back((char)(((int)tempVarString[i]) - tempVarNumber));

							setVariable(argZero, cleaned);
						} else
							error("is_null:" + argZero, false);
					} else
						error("conversion_error:" + argTwo, false);
				} else {
					if (isNumeric(argTwo)) {
						int tempVarNumber(stoi(argTwo));
						string tempVarString(variables.at(variableAt(argZero)).getString());

						if (tempVarString != null) {
							int len(tempVarString.length());
							string cleaned("");

							for (int i = 0; i < len; i++)
								cleaned.push_back((char)(((int)tempVarString[i]) - tempVarNumber));

							setVariable(argZero, cleaned);
						} else
							error("is_null:" + tempVarString, false);
					} else
						error("conversion_error:" + argTwo, false);
				}
			} else if (argOne == "?") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getString() != null) {
						if (variables.at(variableAt(argZero)).getString() != null)
							setVariable(argZero, getStdout(variables.at(variableAt(argTwo)).getString().c_str()));
						else
							error("conversion_error:" + argZero, false);
					} else
						error("conversion_error:" + argTwo, false);
				} else {
					if (variables.at(variableAt(argZero)).getString() != null)
						setVariable(argZero, getStdout(cleanString(argTwo).c_str()));
					else
						error("conversion_error:" + argZero, false);
				}
			} else if (argOne == "!") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getString() != null) {
						if (variables.at(variableAt(argZero)).getString() != null)
							setVariable(argZero, getParsedOutput(variables.at(variableAt(argTwo)).getString().c_str()));
						else
							error("conversion_error:" + argZero, false);
					} else
						error("conversion_error:" + argTwo, false);
				} else {
					if (variables.at(variableAt(argZero)).getString() != null)
						setVariable(argZero, getParsedOutput(cleanString(argTwo).c_str()));
					else
						error("conversion_error:" + argZero, false);
				}
			} else
				error("invalid_operator:" + argOne, false);
		} else
			error("special_error(4)", false);
	} else if (listExists(argZero) || listExists(beforeBrackets(argZero))) {
		string _b(beforeDot(argTwo)), _a(afterDot(argTwo)),
            __b(beforeParams(argTwo));

        if (containsBrackets(argZero)) {
            string after(afterBrackets(argZero)), before(beforeBrackets(argZero));

            after = subtractString(after, "]");

            if (lists.at(listAt(before)).size() >= stoi(after)) {
                if (stoi(after) == 0) {
                    if (argOne == "=") {
                        if (variableExists(argTwo)) {
                            if (variables.at(variableAt(argTwo)).getString() != null)
                                replaceElement(before, after, variables.at(variableAt(argTwo)).getString());
                            else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
                                replaceElement(before, after, dtos(variables.at(variableAt(argTwo)).getNumber()));
                            else
                                error("is_null:" + argTwo, false);
                        } else
                            replaceElement(before, after, argTwo);
                    }
                } else if (lists.at(listAt(before)).at(stoi(after)) == "#!=no_line")
                    error("invalid_operation:index_out_of_bounds:" + argZero, false);
                else {
                    if (argOne == "=") {
                        if (variableExists(argTwo)) {
                            if (variables.at(variableAt(argTwo)).getString() != null)
                                replaceElement(before, after, variables.at(variableAt(argTwo)).getString());
                            else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
                                replaceElement(before, after, dtos(variables.at(variableAt(argTwo)).getNumber()));
                            else
                                error("is_null:" + argTwo, false);
                        } else
                            replaceElement(before, after, argTwo);
                    }
                }
            } else
                error("invalid_operation:index_out_of_bounds:" + argTwo, false);
        } else if (containsBrackets(argTwo)) {
            string listName(beforeBrackets(argTwo));

            if (listExists(listName)) {
                vector<string> listRange = getBracketRange(argTwo);

                if (listRange.size() == 2) {
                    string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

                    if (rangeBegin.length() != 0 && rangeEnd.length() != 0) {
                        if (isNumeric(rangeBegin) && isNumeric(rangeEnd)) {
                            if (stoi(rangeBegin) < stoi(rangeEnd)) {
                                if (lists.at(listAt(listName)).size() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                                    if (stoi(rangeBegin) >= 0) {
                                        if (argOne == "+=") {
                                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                                lists.at(listAt(argZero)).add(lists.at(listAt(listName)).at(i));
                                        } else if (argOne == "=") {
                                            lists.at(listAt(argZero)).clear();

                                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                                                lists.at(listAt(argZero)).add(lists.at(listAt(listName)).at(i));
                                        } else
                                            error("invalid_operator:" + argOne, false);
                                    } else
                                        error("invalid_operation:index_invalid:" + rangeBegin, false);
                                } else
                                    error("invalid_operation:index_out_of_bounds:" + rangeEnd, false);
                            } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
                                if (lists.at(listAt(listName)).size() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                                    if (stoi(rangeBegin) >= 0) {
                                        if (argOne == "+=") {
                                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                                lists.at(listAt(argZero)).add(lists.at(listAt(listName)).at(i));
                                        } else if (argOne == "=") {
                                            lists.at(listAt(argZero)).clear();

                                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                                                lists.at(listAt(argZero)).add(lists.at(listAt(listName)).at(i));
                                        } else
                                            error("invalid_operator:" + argOne, false);
                                    } else
                                        error("invalid_operation:index_invalid:" + rangeBegin, false);
                                } else
                                    error("invalid_operation:index_out_of_bounds:" + rangeEnd, false);
                            } else
                                error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                        } else
                            error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                    } else
                        error("invalid_operation:invalid_range_lengths:" + rangeBegin + ".." + rangeEnd, false);
                } else
                    error("invalid_operation:invalid_range:" + argTwo, false);
            } else
                error("invalid_operation:list_undefined:" + listName, false);
        } else if (variableExists(_b) && contains(_a, "split") && argOne == "=") {
			if (variables.at(variableAt(_b)).getString() != null) {
                vector<string> params = getParams(_a);
                vector<string> elements;

                if (params.at(0) == "")
                    elements = split(variables.at(variableAt(_b)).getString(), ' ');
                else {
                    if (params.at(0)[0] == ';')
                        elements = split(variables.at(variableAt(_b)).getString(), ';');
                    else
                        elements = split(variables.at(variableAt(_b)).getString(), params.at(0)[0]);
                }

                lists.at(listAt(argZero)).clear();

                for (int i = 0; i < (int)elements.size(); i++)
                    lists.at(listAt(argZero)).add(elements.at(i));
			} else
				error("invalid_operation:null_string:" + _b, false);
		} else if (containsParams(argTwo)) {
            vector<string> params = getParams(argTwo);

            if (argOne == "=") {
                lists.at(listAt(argZero)).clear();

                setList(argZero, argTwo, params);
            } else if (argOne == "+=")
                setList(argZero, argTwo, params);
            else if (argOne == "-=") {
                for (int i = 0; i < (int)params.size(); i++) {
                    if (variableExists(params.at(i))) {
                        if (variables.at(variableAt(params.at(i))).getString() != null)
                            lists.at(listAt(argZero)).remove(variables.at(variableAt(params.at(i))).getString());
                        else if (variables.at(variableAt(params.at(i))).getNumber() != nullNum)
                            lists.at(listAt(argZero)).remove(dtos(variables.at(variableAt(params.at(i))).getNumber()));
                        else
                            error("is_null:" + params.at(i), false);
                    } else
                        lists.at(listAt(argZero)).remove(params.at(i));
                }
            } else
                error("invalid_operator:" + argOne, false);
        } else if (variableExists(argTwo)) {
            if (argOne == "+=") {
    			if (variables.at(variableAt(argTwo)).getString() != null)
    				lists.at(listAt(argZero)).add(variables.at(variableAt(argTwo)).getString());
    			else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
    				lists.at(listAt(argZero)).add(dtos(variables.at(variableAt(argTwo)).getNumber()));
    			else
    				error("conversion_error:" + argTwo, false);
            } else if (argOne == "-=") {
    			if (variables.at(variableAt(argTwo)).getString() != null)
    				lists.at(listAt(argZero)).remove(variables.at(variableAt(argTwo)).getString());
    			else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
    				lists.at(listAt(argZero)).remove(dtos(variables.at(variableAt(argTwo)).getNumber()));
    			else
    				error("conversion_error:" + argTwo, false);
            } else
                error("invalid_operator:" + argOne, false);
		} else if (methodExists(argTwo)) {
		    parse(argTwo);

		    vector<string> _p = getParams(lastValue);

		    if (argOne == "=") {
                lists.at(listAt(argZero)).clear();

                for (int i = 0; i < (int)_p.size(); i++)
                    lists.at(listAt(argZero)).add(_p.at(i));
            } else if (argOne == "+=") {
                for (int i = 0; i < (int)_p.size(); i++)
                    lists.at(listAt(argZero)).add(_p.at(i));
            } else
                error("invalid_operator:" + argOne, false);
		} else {
            if (argOne == "+=") {
    			if (argTwo.length() != 0)
    				lists.at(listAt(argZero)).add(argTwo);
    			else
    				error("is_empty:" + argTwo, false);
            } else if (argOne == "-=") {
                if (argTwo.length() != 0)
                    lists.at(listAt(argZero)).remove(argTwo);
                else
                    error("is_empty:" + argTwo, false);
            }
		}
	} else {
		if (startsWith(argZero, "@") && zeroDots(argZero)) {
			if (argOne == "=") {
				string before(beforeDot(argTwo)), after(afterDot(argTwo));

                if (containsBrackets(argTwo) && (variableExists(beforeBrackets(argTwo)) || listExists(beforeBrackets(argTwo)))) {
                    string beforeBracket(beforeBrackets(argTwo)), afterBracket(afterBrackets(argTwo));

                    afterBracket = subtractString(afterBracket, "]");

                    if (listExists(beforeBracket)) {
                        if (lists.at(listAt(beforeBracket)).size() >= stoi(afterBracket)) {
                            if (lists.at(listAt(beforeBracket)).at(stoi(afterBracket)) == "#!=no_line")
                                error("invalid_operation:index_out_of_bounds:" + argTwo, false);
                            else {
                                string listValue(lists.at(listAt(beforeBracket)).at(stoi(afterBracket)));

                                if (isNumeric(listValue))
                                    createVariable(argZero, stod(listValue));
                                else
                                    createVariable(argZero, listValue);
                            }
                        } else
                            error("invalid_operation:index_out_of_bounds:" + argTwo, false);
                    } else if (variableExists(beforeBracket))
                        setSubString(argZero, argTwo, beforeBracket);
                    else
                        error("invalid_operation:list_undefined:" + beforeBracket, false);
                } else if (listExists(before) && after == "size")
                    createVariable(argZero, stod(itos(lists.at(listAt(before)).size())));
                else if (before == "self") {
                    if (objectExists(currentMethodObject))
                        twoSpace(argZero, argOne, (currentMethodObject + "." + after), (argZero + " " + argOne + " " + (currentMethodObject + "." + after)), command);
                    else
                        twoSpace(argZero, argOne, after, (argZero + " " + argOne + " " + after), command);
                } else if (after == "to_int" || after == "integer") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null)
                            createVariable(argZero, (int)variables.at(variableAt(before)).getString()[0]);
                        else if (variables.at(variableAt(before)).getNumber() != nullNum) {
                            int i = (int)variables.at(variableAt(before)).getNumber();
                            createVariable(argZero, (double)i);
                        } else
                            error("is_null:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "to_double" || after == "double") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null)
                            createVariable(argZero, (double)variables.at(variableAt(before)).getString()[0]);
                        else if (variables.at(variableAt(before)).getNumber() != nullNum) {
                            double i = variables.at(variableAt(before)).getNumber();
                            createVariable(argZero, (double)i);
                        } else
                            error("is_null:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "to_string") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                            createVariable(argZero, dtos(variables.at(variableAt(before)).getNumber()));
                        else
                            error("is_null:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "to_number") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null)
                            createVariable(argZero, stod(variables.at(variableAt(before)).getString()));
                        else
                            error("is_null:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (objectExists(before)) {
                    if (objects.at(objectAt(before)).methodExists(after) && !containsParams(after)) {
                        parse(argTwo);

                        if (isNumeric(lastValue))
                            createVariable(argZero, stod(lastValue));
                        else
                            createVariable(argZero, lastValue);
                    } else if (containsParams(after)) {
                        if (objects.at(objectAt(before)).methodExists(beforeParams(after))) {
                            executeTemplate(objects.at(objectAt(before)).getMethod(beforeParams(after)), getParams(after));

                            if (isNumeric(lastValue))
                                createVariable(argZero, stod(lastValue));
                            else
                                createVariable(argZero, lastValue);
                        } else
                            sysExec(s, command);
                    } else if (objects.at(objectAt(before)).variableExists(after)) {
                        if (objects.at(objectAt(before)).getVariable(after).getString() != null)
                            createVariable(argZero, objects.at(objectAt(before)).getVariable(after).getString());
                        else if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
                            createVariable(argZero, objects.at(objectAt(before)).getVariable(after).getNumber());
                        else
                            error("is_null:" + objects.at(objectAt(before)).getVariable(after).name(), false);
                    }
                } else if (variableExists(before) && after == "read") {
                    if (variables.at(variableAt(before)).getString() != null) {
                        if (fileExists(variables.at(variableAt(before)).getString())) {
                            ifstream file(variables.at(variableAt(before)).getString().c_str());
                            string line(""), bigString("");

                            if (file.is_open()) {
                                while (!file.eof()) {
                                    getline(file, line);
                                    bigString.append(line + "\r\n");
                                }

                                file.close();

                                createVariable(argZero, bigString);
                            } else
                                error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                        } else
                            error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                    } else
                        error("invalid_operation:null_string:" + before, false);
                } else if (inObjectDef) {
				    if (isNumeric(argTwo)) {
                        Variable newVariable(argZero, stod(argTwo));

                        if (inPrivateDef)
                            newVariable.setPrivate();
                        else if (inPublicDef)
                            newVariable.setPublic();

                        objects.at(objectAt(currentObject)).addVariable(newVariable);
                    } else {
                        Variable newVariable(argZero, argTwo);

                        if (inPrivateDef)
                            newVariable.setPrivate();
                        else if (inPublicDef)
                            newVariable.setPublic();

                        objects.at(objectAt(currentObject)).addVariable(newVariable);
                    }
                } else if (argTwo == "null")
                    createVariable(argZero, argTwo);
                else if (methodExists(argTwo)) {
                    parse(argTwo);

                    if (isNumeric(lastValue))
                        createVariable(argZero, stod(lastValue));
                    else
                        createVariable(argZero, lastValue);
                } else if (constantExists(argTwo)) {
                    if (constants.at(constAt(argTwo)).isNumber())
                        createVariable(argZero, constants.at(constAt(argTwo)).getNumber());
                    else if (constants.at(constAt(argTwo)).isString())
                        createVariable(argZero, constants.at(constAt(argTwo)).getString());
                    else
                        error("conversion_error:null_constant:" + argTwo, false);
                } else if (containsParams(argTwo)) {
                    if (isStringStack(argTwo))
                        createVariable(argZero, getStringStack(argTwo));
                    else if (stackReady(argTwo))
                        createVariable(argZero, getStack(argTwo));
                    else if (beforeParams(argTwo) == "random") {
                        if (contains(argTwo, "..")) {
                            vector<string> range = getRange(argTwo);
                            string s0(range.at(0)), s2(range.at(1));

                            if (isNumeric(s0) && isNumeric(s2)) {
                                double n0 = stod(s0), n2 = stod(s2);

                                if (n0 < n2) {
                                    createVariable(argZero, (int)random(n0, n2));
                                    setLastValue(itos((int)random(n0, n2)));
                                } else if (n0 > n2) {
                                    createVariable(argZero, (int)random(n2, n0));
                                    setLastValue(itos((int)random(n2, n0)));
                                } else {
                                    createVariable(argZero, (int)random(n0, n2));
                                    setLastValue(itos((int)random(n0, n2)));
                                }
                            } else if (isAlpha(s0) && isAlpha(s2)) {
                                if (get_alpha_num(s0[0]) < get_alpha_num(s2[0])) {
                                    createVariable(argZero, random(s0, s2));
                                    setLastValue(random(s0, s2));
                                } else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0])) {
                                    createVariable(argZero, random(s2, s0));
                                    setLastValue(random(s2, s0));
                                } else {
                                    createVariable(argZero, random(s2, s0));
                                    setLastValue(random(s2, s0));
                                }
                            } else if (variableExists(s0) || variableExists(s2)) {
                                if (variableExists(s0)) {
                                    if (variables.at(variableAt(s0)).getNumber() != nullNum)
                                        s0 = dtos(variables.at(variableAt(s0)).getNumber());
                                    else if (variables.at(variableAt(s0)).getString() != null)
                                        s0 = variables.at(variableAt(s0)).getString();
                                }

                                if (variableExists(s2)) {
                                    if (variables.at(variableAt(s2)).getNumber() != nullNum)
                                        s2 = dtos(variables.at(variableAt(s2)).getNumber());
                                    else if (variables.at(variableAt(s2)).getString() != null)
                                        s2 = variables.at(variableAt(s2)).getString();
                                }

                                if (isNumeric(s0) && isNumeric(s2)) {
                                    double n0 = stod(s0), n2 = stod(s2);

                                    if (n0 < n2) {
                                        createVariable(argZero, (int)random(n0, n2));
                                        setLastValue(itos((int)random(n0, n2)));
                                    } else if (n0 > n2) {
                                        createVariable(argZero, (int)random(n2, n0));
                                        setLastValue(itos((int)random(n2, n0)));
                                    } else {
                                        createVariable(argZero, (int)random(n0, n2));
                                        setLastValue(itos((int)random(n0, n2)));
                                    }
                                } else if (isAlpha(s0) && isAlpha(s2)) {
                                    if (get_alpha_num(s0[0]) < get_alpha_num(s2[0])) {
                                        createVariable(argZero, random(s0, s2));
                                        setLastValue(random(s0, s2));
                                    } else if (get_alpha_num(s0[0]) > get_alpha_num(s2[0])) {
                                        createVariable(argZero, random(s2, s0));
                                        setLastValue(random(s2, s0));
                                    } else {
                                        createVariable(argZero, random(s2, s0));
                                        setLastValue(random(s2, s0));
                                    }
                                }
                            } else
                                error("invalid_operation:invalid_range:" + s0 + ".." + s2, false);
                        } else
                            error("invalid_operation:invalid_range_separator:" + argTwo, false);
                    } else {
                        executeTemplate(getMethod(beforeParams(argTwo)), getParams(argTwo));

                        if (isNumeric(lastValue))
                            createVariable(argZero, stod(lastValue));
                        else
	                        createVariable(argZero, lastValue);
                    }
                } else if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
						createVariable(argZero, variables.at(variableAt(argTwo)).getNumber());
					else if (variables.at(variableAt(argTwo)).getString() != null)
						createVariable(argZero, variables.at(variableAt(argTwo)).getString());
					else
						createVariable(argZero, null);
				} else if (argTwo == "shomp" || argTwo == "chomp") {
					if (argTwo == "shomp") {
						string shomper("");
						shomper = shomp("");

						if (isNumeric(shomper))
							createVariable(argZero, stod(shomper));
						else
							createVariable(argZero, shomper);
					} else {
						string chomper("");
						cout << "chomp: ";
						getline(cin, chomper, '\n');

						if (isNumeric(chomper))
							createVariable(argZero, stod(chomper));
						else
							createVariable(argZero, chomper);
					}
				} else if (argTwo == "args.size")
					createVariable(argZero, (double)argsLength);
				else if (before == "chomp") {
					if (variableExists(after)) {
						if (variables.at(variableAt(after)).getString() != null) {
							string chomper("");
							cout << cleanString(variables.at(variableAt(after)).getString());
							getline(cin, chomper, '\n');

							if (isNumeric(chomper))
								createVariable(argZero, stod(chomper));
							else
								createVariable(argZero, chomper);
						} else {
							string chomper("");
							cout << "chomp: ";
							getline(cin, chomper, '\n');

							if (isNumeric(chomper))
								createVariable(argZero, stod(chomper));
							else
								createVariable(argZero, chomper);
						}
					} else {
						string chomper("");
						cout << cleanString(after);
						getline(cin, chomper, '\n');

						if (isNumeric(chomper))
							createVariable(argZero, stod(chomper));
						else
							createVariable(argZero, chomper);
					}
				} else if (before == "shomp") {
					if (variableExists(after)) {
						if (variables.at(variableAt(after)).getString() != null) {
							string chomper("");
							chomper = shomp(variables.at(variableAt(after)).getString());

							if (isNumeric(chomper))
								createVariable(argZero, stod(chomper));
							else
								createVariable(argZero, chomper);

                            cout << endl;
						} else {
							string chomper("");
                            chomper = shomp("shomp: ");

							if (isNumeric(chomper))
								createVariable(argZero, stod(chomper));
							else
								createVariable(argZero, chomper);

                            cout << endl;
						}
					} else {
						string chomper("");
						chomper = shomp(cleanString(after));

						if (isNumeric(chomper))
							createVariable(argZero, stod(chomper));
						else
							createVariable(argZero, chomper);

                        cout << endl;
					}
				} else if (after == "size") {
					if (variableExists(before)) {
						if (variables.at(variableAt(before)).getString() != null)
							createVariable(argZero, (double)variables.at(variableAt(before)).getString().length());
						else
							error("conversion_error:" + before, false);
					} else
						createVariable(argZero, (double)before.length());
				} else if (after == "sin") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, sin(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "sinh") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, sinh(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "asin") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, asin(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "tan") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, tan(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "tanh") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, tanh(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "atan") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, atan(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "cos") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, cos(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "acos") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, acos(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "cosh") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, cosh(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "log") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, log(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "sqrt") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, sqrt(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "abs") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, abs(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "floor") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, floor(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "ceil") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, ceil(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "exp") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getNumber() != nullNum)
                             createVariable(argZero, exp(variables.at(variableAt(before)).getNumber()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "to_upper") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null)
                             createVariable(argZero, getUpper(variables.at(variableAt(before)).getString()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "to_lower") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null)
                             createVariable(argZero, getLower(variables.at(variableAt(before)).getString()));
                        else
                            error("conversion_error:" + before, false);
                    } else
                        error("invalid_operation:variable_undefined:" + before, false);
                } else if (after == "bytes") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null) {
                            if (fileExists(variables.at(variableAt(before)).getString()))
                                createVariable(argZero, getBytes(variables.at(variableAt(before)).getString()));
                            else
                                error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                        } else
                            error("conversion_error:" + before, false);
                    } else {
                        if (fileExists(before))
                            createVariable(argZero, getBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                } else if (after == "kbytes") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null) {
                            if (fileExists(variables.at(variableAt(before)).getString()))
                                createVariable(argZero, getKBytes(variables.at(variableAt(before)).getString()));
                            else
                                error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                        } else
                            error("conversion_error:" + before, false);
                    } else {
                        if (fileExists(before))
                            createVariable(argZero, getKBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                } else if (after == "mbytes") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null) {
                            if (fileExists(variables.at(variableAt(before)).getString()))
                                createVariable(argZero, getMBytes(variables.at(variableAt(before)).getString()));
                            else
                                error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                        } else
                            error("conversion_error:" + before, false);
                    } else {
                        if (fileExists(before))
                            createVariable(argZero, getMBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                } else if (after == "gbytes") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null) {
                            if (fileExists(variables.at(variableAt(before)).getString()))
                                createVariable(argZero, getGBytes(variables.at(variableAt(before)).getString()));
                            else
                                error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                        } else
                            error("conversion_error:" + before, false);
                    } else {
                        if (fileExists(before))
                            createVariable(argZero, getGBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                } else if (after == "tbytes") {
                    if (variableExists(before)) {
                        if (variables.at(variableAt(before)).getString() != null) {
                            if (fileExists(variables.at(variableAt(before)).getString()))
                                createVariable(argZero, getTBytes(variables.at(variableAt(before)).getString()));
                            else
                                error("read_fail:" + variables.at(variableAt(before)).getString(), false);
                        } else
                            error("conversion_error:" + before, false);
                    } else {
                        if (fileExists(before))
                            createVariable(argZero, getTBytes(before));
                        else
                            error("read_fail:" + before, false);
                    }
                } else if (before == "env") {
                    // REFACTOR HERE
					if (after == "cwd")
						createVariable(argZero, cwd());
					else if (after == "usl")
						createVariable(argZero, uslBinary);
					else if (after == "os")
						createVariable(argZero, guessedOS);
					else if (after == "user")
                        createVariable(argZero, getUser());
					else if (after == "machine")
                        createVariable(argZero, getMachine());
                    else if (after == "init_dir" || after == "initial_directory")
                        createVariable(argZero, initDir);
                    else if (after == "this_second")
						createVariable(argZero, (double)secondNow());
					else if (after == "this_minute")
						createVariable(argZero, (double)minuteNow());
					else if (after == "this_hour")
						createVariable(argZero, (double)hourNow());
					else if (after == "this_month")
                        createVariable(argZero, (double)monthNow());
					else if (after == "this_year")
                        createVariable(argZero, (double)yearNow());
					else if (after == "day_of_this_month")
                        createVariable(argZero, (double)dayOfTheMonth());
					else if (after == "day_of_this_year")
                        createVariable(argZero, (double)dayOfTheYear());
					else if (after == "day_of_this_week")
                        createVariable(argZero, dayOfTheWeek());
					else if (after == "month_of_this_year")
                        createVariable(argZero, monthOfTheYear());
					else if (after == "am_or_pm")
                        createVariable(argZero, amOrPm());
                    else if (after == "last_error")
                        createVariable(argZero, lastError);
                    else if (after == "last_value")
                        createVariable(argZero, lastValue);
                    else if (after == "now")
                        createVariable(argZero, timeNow());
                    else if (after == "empty_string")
                        createVariable(argZero, "");
                    else if (after == "empty_number")
                        createVariable(argZero, 0);
					else
						createVariable(argZero, getEnvironmentVariable(after));
				} else {
					if (isNumeric(argTwo))
						createVariable(argZero, stod(argTwo));
					else
						createVariable(argZero, cleanString(argTwo));
				}
			} else if (argOne == "+=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getString() != null)
						createVariable(argZero, variables.at(variableAt(argTwo)).getString());
					else if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
						createVariable(argZero, variables.at(variableAt(argTwo)).getNumber());
					else
						createVariable(argZero, null);
				} else {
					if (isNumeric(argTwo))
						createVariable(argZero, stod(argTwo));
					else
						createVariable(argZero, cleanString(argTwo));
				}
			} else if (argOne == "-=") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getNumber() != nullNum)
						createVariable(argZero, 0 - variables.at(variableAt(argTwo)).getNumber());
					else
						createVariable(argZero, null);
				} else {
					if (is_num(argTwo))
						createVariable(argZero, stod(argTwo));
					else
						createVariable(argZero, cleanString(argTwo));
				}
			} else if (argOne == "?") {
				if (variableExists(argTwo)) {
					if (variables.at(variableAt(argTwo)).getString() != null)
						createVariable(argZero, getStdout(variables.at(variableAt(argTwo)).getString()));
					else
						error("conversion_error:" + argTwo, false);
				} else
					createVariable(argZero, getStdout(cleanString(argTwo)));
			} else if (argOne == "!") {
			    if (variableExists(argTwo)) {
			        if (variables.at(variableAt(argTwo)).getString() != null)
                        createVariable(argZero, getParsedOutput(variables.at(variableAt(argTwo)).getString()));
                    else
                        error("conversion_error:" + argTwo, false);
			    } else
                    createVariable(argZero, getParsedOutput(cleanString(argTwo)));
			} else
				error("invalid_operator:" + argTwo, false);
		} else if (startsWith(argZero, "@") && !zeroDots(argTwo)) {
            string before = beforeDot(argTwo),
                   after = afterDot(argTwo);

            if (objectExists(before)) {
                if (argOne == "=") {
                    if (objects.at(objectAt(before)).getVariable(after).getString() != null)
                        createVariable(argZero, objects.at(objectAt(before)).getVariable(after).getString());
                    else if (objects.at(objectAt(before)).getVariable(after).getNumber() != nullNum)
                        createVariable(argZero, objects.at(objectAt(before)).getVariable(after).getNumber());
                }
            }
        } else if (!objectExists(argZero) && objectExists(argTwo)) {
            if (argOne == "=") {
                vector<Method> objectMethods = objects.at(objectAt(argTwo)).getMethods();
                Object newObject(argZero);

                for (int i = 0; i < (int)objectMethods.size(); i++)
                    newObject.addMethod(objectMethods.at(i));


                vector<Variable> objectVariables = objects.at(objectAt(argTwo)).getVariables();

                for (int i = 0; i < (int)objectVariables.size(); i++)
                        newObject.addVariable(objectVariables.at(i));

                if (executedMethod)
                    newObject.collect();
                else
                    newObject.dontCollect();

                objects.push_back(newObject);
                currentObject = argOne;
                inObjectDef = false;

                newObject.clear();
                objectMethods.clear();
            } else
                error("invalid_operator:" + argOne, false);
        } else if (isUpperConstant(argZero)) {
            if (!constantExists(argZero)) {
                if (argOne == "=") {
                    if (isNumeric(argTwo)) {
                        Constant newConstant(argZero, stod(argTwo));
                        constants.push_back(newConstant);
                    } else {
                        Constant newConstant(argZero, argTwo);
                        constants.push_back(newConstant);
                    }
                } else
                    error("invalid_operator:" + argOne, false);
            } else
                error("invalid_operation:constant_defined:" + argZero, false);
        } else {
			if (isNumeric(argZero) && isNumeric(argTwo)) {
				if (argOne == "+") {
					say(dtos(stod(argZero) + stod(argTwo)));
					setLastValue(dtos(stod(argZero) + stod(argTwo)));
				} else if (argOne == "-") {
					say(dtos(stod(argZero) - stod(argTwo)));
					setLastValue(dtos(stod(argZero) - stod(argTwo)));
				} else if (argOne == "*") {
					say(dtos(stod(argZero) * stod(argTwo)));
					setLastValue(dtos(stod(argZero) * stod(argTwo)));
				} else if (argOne == "/") {
					say(dtos(stod(argZero) / stod(argTwo)));
					setLastValue(dtos(stod(argZero) / stod(argTwo)));
				} else if (argOne == "**") {
					say(dtos(pow(stod(argZero), stod(argTwo))));
					setLastValue(dtos(stod(argZero) * stod(argTwo)));
				} else if (argOne == "%") {
					if ((int)stod(argTwo) == 0)
						error("segfault:" + s, false);
					else {
                        say(dtos((int)stod(argZero) % (int)stod(argTwo)));
						setLastValue(itos((int)stod(argZero) % (int)stod(argTwo)));
                    }
				} else
					error("invalid_operator:" + argOne, false);
			} else {
				if (argOne == "+") {
					say(cleanString(argZero) + cleanString(argTwo));
					setLastValue(cleanString(argZero) + cleanString(argTwo));
				} else if (argOne == "-") {
					say(subtractString(cleanString(argZero), cleanString(argTwo)));
					setLastValue(subtractString(cleanString(argZero), cleanString(argTwo)));
				} else if (argOne == "*") {
					if (!zeroNumbers(argTwo)) {
                        string bigstr(cleanString(argZero));
						for (int i = 1; i <= stoi(argTwo); i++) {
                            bigstr.append(argZero);
							__stdout(argZero);
                        }

                        setLastValue(bigstr);
					} else
						error("invalid_operation:" + s, false);
				} else if (argOne == "/") {
					say(subtractString(cleanString(argZero), cleanString(argTwo)));
					setLastValue(subtractString(cleanString(argZero), cleanString(argTwo)));
				}
                else
					error("invalid_operator:" + argOne, false);
			}
		}
	}
}

void threeSpace(string argZero, string argOne, string argTwo, string argThree, string s, vector<string> command)
{
	if (argZero == "object") {
		if (objectExists(argOne)) {
			inObjectDef = true;
			currentObject = argOne;
		} else {
			if (objectExists(argThree)) {
				if (argTwo == "=") {
					vector<Method> objectMethods = objects.at(objectAt(argThree)).getMethods();
					Object newObject(argOne);

					for (int i = 0; i < (int)objectMethods.size(); i++) {
                        if (objectMethods.at(i).isPublic())
                            newObject.addMethod(objectMethods.at(i));
                    }

					objects.push_back(newObject);
					currentObject = argOne;
					inObjectDef = true;

					newObject.clear();
					objectMethods.clear();
				} else
					error("invalid_operator:" + argTwo, false);
			} else
				error("invalid_operation:object_undefined:" + argThree, false);
		}
	} else if (argZero == "unless") {
        if (listExists(argThree)) {
	        if (argTwo == "in") {
	            string testString("[none]");

                if (variableExists(argOne)) {
                    if (variables.at(variableAt(argOne)).getString() != null)
                        testString = variables.at(variableAt(argOne)).getString();
                    else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
                        testString = dtos(variables.at(variableAt(argOne)).getNumber());
                    else
                        error("is_null:" + argOne, false);
                } else
                    testString = argOne;

                if (testString != "[none]") {
                    bool elementFound = false;
                    for (int i = 0; i < (int)lists.at(listAt(argThree)).size(); i++) {
                        if (lists.at(listAt(argThree)).at(i) == testString) {
                            elementFound = true;
                            failedIF();
                            lastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        successfulIF();
                } else
                    successfulIF();
            }
	    } else if (variableExists(argOne) && variableExists(argThree)) {
			if (variables.at(variableAt(argOne)).getString() != null && variables.at(variableAt(argThree)).getString() != null) {
				if (argTwo == "==" || argTwo == "is") {
                    if (variables.at(variableAt(argOne)).getString() == variables.at(variableAt(argThree)).getString())
						failedIF();
                    else
						successfulIF();
				} else if (argTwo == "!=" || argTwo == "not") {
					if (variables.at(variableAt(argOne)).getString() != variables.at(variableAt(argThree)).getString())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == ">") {
					if (variables.at(variableAt(argOne)).getString().length() > variables.at(variableAt(argThree)).getString().length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "<") {
					if (variables.at(variableAt(argOne)).getString().length() < variables.at(variableAt(argThree)).getString().length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "<=") {
					if (variables.at(variableAt(argOne)).getString().length() <= variables.at(variableAt(argThree)).getString().length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == ">=") {
					if (variables.at(variableAt(argOne)).getString().length() >= variables.at(variableAt(argThree)).getString().length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "contains") {
					if (contains(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argThree)).getString()))
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "ends_with") {
					if (endsWith(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argThree)).getString()))
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "begins_with") {
					if (startsWith(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argThree)).getString()))
						failedIF();
					else
						successfulIF();
				} else {
					error("invalid_operator:" + argTwo, false);
					successfulIF();
				}
			} else if (variables.at(variableAt(argOne)).getNumber() != nullNum && variables.at(variableAt(argThree)).getNumber() != nullNum) {
				if (argTwo == "==" || argTwo == "is") {
					if (variables.at(variableAt(argOne)).getNumber() == variables.at(variableAt(argThree)).getNumber())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "!=" || argTwo == "not") {
					if (variables.at(variableAt(argOne)).getNumber() != variables.at(variableAt(argThree)).getNumber())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == ">") {
					if (variables.at(variableAt(argOne)).getNumber() > variables.at(variableAt(argThree)).getNumber())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == ">=") {
					if (variables.at(variableAt(argOne)).getNumber() >= variables.at(variableAt(argThree)).getNumber())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "<") {
					if (variables.at(variableAt(argOne)).getNumber() < variables.at(variableAt(argThree)).getNumber())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "<=") {
					if (variables.at(variableAt(argOne)).getNumber() <= variables.at(variableAt(argThree)).getNumber())
						failedIF();
					else
						successfulIF();
				} else {
					error("invalid_operator:" + argTwo, false);
					successfulIF();
				}
			} else {
				error("conversion_error:" + s, false);
				successfulIF();
			}
		} else if ((variableExists(argOne) && !variableExists(argThree)) && !methodExists(argThree) && notObjectMethod(argThree) && !containsParams(argThree)) {
			if (variables.at(variableAt(argOne)).getNumber()!= nullNum) {
			  	if (isNumeric(argThree)) {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argOne)).getNumber() == stod(argThree))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argOne)).getNumber() != stod(argThree))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argOne)).getNumber() > stod(argThree))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argOne)).getNumber() < stod(argThree))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argOne)).getNumber() >= stod(argThree))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argOne)).getNumber() <= stod(argThree))
							failedIF();
						else
							successfulIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						successfulIF();
					}
				} else if (argThree == "number?") {
				    if (argTwo == "==" || argTwo == "is")
				        failedIF();
				    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else
                        error("invalid_operator:" + argTwo, false);
				} else {
					error("conversion_error:" + s, false);
					successfulIF();
				}
			} else if (variables.at(variableAt(argOne)).getString() != null) {
				if (argThree == "string?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is")
							failedIF();
						else if (argTwo == "!=" || argTwo == "not")
							successfulIF();
						else {
							error("invalid_operator:" + argTwo, false);
							successfulIF();
						}
					} else {
						if (argTwo == "!")
							failedIF();
						else
							successfulIF();
					}
				} else if (argThree == "number?") {
					if (variables.at(variableAt(argOne)).getNumber() != nullNum) {
						if (argTwo == "==" || argTwo == "is")
							failedIF();
						else if (argTwo == "!=" || argTwo == "not")
							successfulIF();
						else {
							error("invalid_operator:" + argTwo, false);
							successfulIF();
						}
					} else {
						if (argTwo == "!=" || argTwo == "not")
							failedIF();
						else
							successfulIF();
					}
				} else if (argThree == "upper?" || argThree == "uppercase?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is") {
                            if (isUpper(variables.at(variableAt(argOne)).getString()))
							    failedIF();
		                    else
		                        successfulIF();
						} else if (argTwo == "!=" || argTwo == "not") {
                            if (isUpper(variables.at(variableAt(argOne)).getString()))
							    successfulIF();
		                    else
		                        failedIF();
						} else {
							error("invalid_operator:" + argTwo, false);
							successfulIF();
						}
					} else {
						if (argTwo == "!=" || argTwo == "not") {
                            if (isUpper(argTwo))
                                successfulIF();
                            else
							    failedIF();
						} else
							successfulIF();
					}
				} else if (argThree == "lower?" || argThree == "lowercase?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is") {
                            if (isLower(variables.at(variableAt(argOne)).getString()))
							    failedIF();
		                    else
		                        successfulIF();
						} else if (argTwo == "!=" || argTwo == "not") {
                            if (isLower(variables.at(variableAt(argOne)).getString()))
							    successfulIF();
		                    else
		                        failedIF();
						} else {
							error("invalid_operator:" + argTwo, false);
							successfulIF();
						}
					} else {
						if (argTwo == "!=" || argTwo == "not") {
                            if (isLower(argTwo))
                                successfulIF();
                            else
							    failedIF();
						} else
							successfulIF();
					}
				} else if (argThree == "file?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (fileExists(variables.at(variableAt(argOne)).getString())) {
							if (argTwo == "==" || argTwo == "is")
								failedIF();
							else if (argTwo == "!=" || argTwo == "not")
								successfulIF();
							else {
								error("invalid_operator:" + argTwo, false);
								successfulIF();
							}
						} else {
							if (argTwo == "!=" || argTwo == "not")
								failedIF();
							else
								successfulIF();
						}
					} else {
						error("is_null:" + argOne, false);
						successfulIF();
					}
				} else if (argThree == "dir?" || argThree == "directory?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (directoryExists(variables.at(variableAt(argOne)).getString())) {
							if (argTwo == "==" || argTwo == "is")
								failedIF();
							else if (argTwo == "!=" || argTwo == "not")
								successfulIF();
							else {
								error("invalid_operator:" + argTwo, false);
								successfulIF();
							}
						} else {
							if (argTwo == "!=" || argTwo == "not")
								failedIF();
							else
								successfulIF();
						}
					} else {
						error("is_null:" + argOne, false);
						successfulIF();
					}
				} else {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argOne)).getString() == argThree)
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argOne)).getString() != argThree)
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argOne)).getString().length() > argThree.length())
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argOne)).getString().length() < argThree.length())
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argOne)).getString().length() >= argThree.length())
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argOne)).getString().length() <= argThree.length())
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "contains") {
						if (contains(variables.at(variableAt(argOne)).getString(), argThree))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "ends_with") {
						if (endsWith(variables.at(variableAt(argOne)).getString(), argThree))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "begins_with") {
						if (startsWith(variables.at(variableAt(argOne)).getString(), argThree))
							failedIF();
						else
							successfulIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						successfulIF();
					}
				}
			} else {
                error("special_error(0)", false); // variable is neither string nor number
                successfulIF();
			}
		} else if ((variableExists(argOne) && !variableExists(argThree)) && !methodExists(argThree) && notObjectMethod(argThree) && containsParams(argThree)) {
		    string stackValue("");

		    if (isStringStack(argThree))
		        stackValue = getStringStack(argThree);
		    else if (stackReady(argThree))
                stackValue = dtos(getStack(argThree));
            else
                stackValue = argThree;

			if (variables.at(variableAt(argOne)).getNumber()!= nullNum) {
			  	if (isNumeric(stackValue)) {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argOne)).getNumber() == stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argOne)).getNumber() != stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argOne)).getNumber() > stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argOne)).getNumber() < stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argOne)).getNumber() >= stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argOne)).getNumber() <= stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						successfulIF();
					}
				} else if (stackValue == "is_number?" || stackValue == "number?") {
				    if (argTwo == "==" || argTwo == "is")
				        failedIF();
				    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else
                        error("invalid_operator:" + argTwo, false);
				} else {
					error("conversion_error:" + s, false);
					successfulIF();
				}
			} else if (variables.at(variableAt(argOne)).getString() != null) {
				if (stackValue == "is_string?" || stackValue == "string?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is")
							failedIF();
						else if (argTwo == "!=" || argTwo == "not")
							successfulIF();
						else {
							error("invalid_operator:" + argTwo, false);
							successfulIF();
						}
					} else {
						if (argTwo == "!=" || argTwo == "not")
							failedIF();
						else
							successfulIF();
					}
				} else if (stackValue == "is_number?" || stackValue == "number?") {
					if (variables.at(variableAt(argOne)).getNumber() != nullNum) {
						if (argTwo == "==" || argTwo == "is")
							failedIF();
						else if (argTwo == "!=" || argTwo == "not")
							successfulIF();
						else {
							error("invalid_operator:" + argTwo, false);
							successfulIF();
						}
					} else {
						if (argTwo == "!=" || argTwo == "not")
							failedIF();
						else
							successfulIF();
					}
				} else if (stackValue == "uppercase?" || stackValue == "upper?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is") {
                            if (isUpper(variables.at(variableAt(argOne)).getString()))
							    failedIF();
		                    else
		                        successfulIF();
						} else if (argTwo == "!=" || argTwo == "not") {
                            if (isUpper(variables.at(variableAt(argOne)).getString()))
							    successfulIF();
		                    else
		                        failedIF();
						} else {
							error("invalid_operator:" + argTwo, false);
							successfulIF();
						}
					} else {
						if (argTwo == "!=" || argTwo == "not") {
                            if (isUpper(argTwo))
                                successfulIF();
                            else
							    failedIF();
						} else
							successfulIF();
					}
				} else if (stackValue == "lowercase?" || stackValue == "lower?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is") {
                            if (isLower(variables.at(variableAt(argOne)).getString()))
							    failedIF();
		                    else
		                        successfulIF();
						} else if (argTwo == "!=" || argTwo == "not") {
                            if (isLower(variables.at(variableAt(argOne)).getString()))
							    successfulIF();
		                    else
		                        failedIF();
						} else {
							error("invalid_operator:" + argTwo, false);
							successfulIF();
						}
					} else {
						if (argTwo == "!=" || argTwo == "not") {
                            if (isLower(argTwo))
                                successfulIF();
                            else
							    failedIF();
						} else
							successfulIF();
					}
				} else if (stackValue == "file?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (fileExists(variables.at(variableAt(argOne)).getString())) {
							if (argTwo == "==" || argTwo == "is")
								failedIF();
							else if (argTwo == "!=" || argTwo == "not")
								successfulIF();
							else {
								error("invalid_operator:" + argTwo, false);
								successfulIF();
							}
						} else {
							if (argTwo == "!=" || argTwo == "not")
								failedIF();
							else
								successfulIF();
						}
					} else {
						error("is_null:" + argOne, false);
						successfulIF();
					}
				} else if (stackValue == "dir?" || stackValue == "directory?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (directoryExists(variables.at(variableAt(argOne)).getString())) {
							if (argTwo == "==" || argTwo == "is")
								failedIF();
							else if (argTwo == "!=" || argTwo == "not")
								successfulIF();
							else {
								error("invalid_operator:" + argTwo, false);
								successfulIF();
							}
						} else {
							if (argTwo == "!=" || argTwo == "not")
								failedIF();
							else
								successfulIF();
						}
					} else {
						error("is_null:" + argOne, false);
						successfulIF();
					}
				} else {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argOne)).getString() == stackValue)
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argOne)).getString() != stackValue)
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argOne)).getString().length() > stackValue.length())
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argOne)).getString().length() < stackValue.length())
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argOne)).getString().length() >= stackValue.length())
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argOne)).getString().length() <= stackValue.length())
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "contains") {
						if (contains(variables.at(variableAt(argOne)).getString(), stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "ends_with") {
						if (endsWith(variables.at(variableAt(argOne)).getString(), stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "begins_with") {
						if (startsWith(variables.at(variableAt(argOne)).getString(), stackValue))
							failedIF();
						else
							successfulIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						successfulIF();
					}
				}
			} else {
				error("special_error(0)", false); // variable is neither string nor number
				successfulIF();
			}
        } else if ((!variableExists(argOne) && variableExists(argThree)) && !methodExists(argOne) && notObjectMethod(argOne) && !containsParams(argOne)) {
			if (variables.at(variableAt(argThree)).getNumber() != nullNum) {
				if (isNumeric(argOne)) {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argThree)).getNumber() == stod(argOne))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argThree)).getNumber() != stod(argOne))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argThree)).getNumber() > stod(argOne))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argThree)).getNumber() < stod(argOne))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argThree)).getNumber() >= stod(argOne))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argThree)).getNumber() <= stod(argOne))
							failedIF();
						else
							successfulIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						successfulIF();
					}
				} else {
					error("conversion_error:" + s, false);
					successfulIF();
				}
			} else if (variables.at(variableAt(argThree)).getString() != null) {
				if (argTwo == "==" || argTwo == "is") {
					if (variables.at(variableAt(argThree)).getString() == argOne)
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "!=" || argTwo == "not") {
					if (variables.at(variableAt(argThree)).getString() != argOne)
						failedIF();
					else
						successfulIF();
				} else if (argTwo == ">") {
					if (variables.at(variableAt(argThree)).getString().length() > argOne.length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "<") {
					if (variables.at(variableAt(argThree)).getString().length() < argOne.length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == ">=") {
					if (variables.at(variableAt(argThree)).getString().length() >= argOne.length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "<=") {
					if (variables.at(variableAt(argThree)).getString().length() <= argOne.length())
						failedIF();
					else
						successfulIF();
				} else {
					error("invalid_operator:" + argTwo, false);
					successfulIF();
				}
			} else {
				error("special_error(1)", false); // variable is neither string nor number
				successfulIF();
			}
		} else if ((!variableExists(argOne) && variableExists(argThree)) && !methodExists(argOne) && notObjectMethod(argOne) && containsParams(argOne)) {
		    string stackValue("");

		    if (isStringStack(argOne))
                stackValue = getStringStack(argOne);
            else if (stackReady(argOne))
                stackValue = dtos(getStack(argOne));
            else
                stackValue = argOne;

			if (variables.at(variableAt(argThree)).getNumber() != nullNum) {
				if (isNumeric(stackValue)) {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argThree)).getNumber() == stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argThree)).getNumber() != stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argThree)).getNumber() > stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argThree)).getNumber() < stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argThree)).getNumber() >= stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argThree)).getNumber() <= stod(stackValue))
							failedIF();
						else
							successfulIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						successfulIF();
					}
				} else {
					error("conversion_error:" + s, false);
					successfulIF();
				}
			} else if (variables.at(variableAt(argThree)).getString() != null) {
				if (argTwo == "==" || argTwo == "is") {
					if (variables.at(variableAt(argThree)).getString() == stackValue)
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "!=" || argTwo == "not") {
					if (variables.at(variableAt(argThree)).getString() != stackValue)
						failedIF();
					else
						successfulIF();
				} else if (argTwo == ">") {
					if (variables.at(variableAt(argThree)).getString().length() > stackValue.length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "<") {
					if (variables.at(variableAt(argThree)).getString().length() < stackValue.length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == ">=") {
					if (variables.at(variableAt(argThree)).getString().length() >= stackValue.length())
						failedIF();
					else
						successfulIF();
				} else if (argTwo == "<=") {
					if (variables.at(variableAt(argThree)).getString().length() <= stackValue.length())
						failedIF();
					else
						successfulIF();
				} else {
					error("invalid_operator:" + argTwo, false);
					successfulIF();
				}
			} else {
				error("special_error(1)", false); // variable is neither string nor number
				successfulIF();
			}
		} else if (containsParams(argOne) || containsParams(argThree)) {
		    if (containsParams(argOne) && containsParams(argThree)) {
		        if (!zeroDots(argOne) && !zeroDots(argThree)) {
                    string argOnebefore(beforeDot(argOne)), argOneafter(afterDot(argOne)),
                        argThreebefore(beforeDot(argThree)), argThreeafter(afterDot(argThree));

                    string argOneResult(""), argThreeResult("");

                    if (objectExists(argOnebefore) && objectExists(argThreebefore)) {
                        if (objects.at(objectAt(argOnebefore)).methodExists(beforeParams(argOneafter)))
                            executeTemplate(objects.at(objectAt(argOnebefore)).getMethod(beforeParams(argOneafter)), getParams(argOneafter));

                        argOneResult = lastValue;

                        if (objects.at(objectAt(argThreebefore)).methodExists(beforeParams(argThreeafter)))
                            executeTemplate(objects.at(objectAt(argThreebefore)).getMethod(beforeParams(argThreeafter)), getParams(argThreeafter));

                        argThreeResult = lastValue;

                        if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(argOneResult) == stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(argOneResult) != stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "<") {
                                if (stod(argOneResult) < stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == ">") {
                                if (stod(argOneResult) > stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "<=") {
                                if (stod(argOneResult) <= stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == ">=") {
                                if (stod(argOneResult) >= stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                successfulIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (argOneResult == argThreeResult)
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (argOneResult != argThreeResult)
                                    failedIF();
                                else
                                    successfulIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                successfulIF();
                            }
                        }
                    } else {
                        if (!objectExists(argOnebefore))
                            error("invalid_operation:object_undefined:" + argOnebefore, false);

                        if (!objectExists(argThreebefore))
                            error("invalid_operation:object_undefined:" + argThreebefore, false);

                        successfulIF();
                    }
		        } else if (!zeroDots(argOne) && zeroDots(argThree)) {
		            string argOnebefore(beforeDot(argOne)), argOneafter(afterDot(argOne));

                    string argOneResult(""), argThreeResult("");

                    if (objectExists(argOnebefore)) {
                        if (objects.at(objectAt(argOnebefore)).methodExists(beforeParams(argOneafter)))
                            executeTemplate(objects.at(objectAt(argOnebefore)).getMethod(beforeParams(argOneafter)), getParams(argOneafter));

                        argOneResult = lastValue;

                        if (methodExists(beforeParams(argThree)))
                            executeTemplate(methods.at(methodAt(beforeParams(argThree))), getParams(argThree));

                        argThreeResult = lastValue;

                        if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(argOneResult) == stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(argOneResult) != stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "<") {
                                if (stod(argOneResult) < stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == ">") {
                                if (stod(argOneResult) > stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "<=") {
                                if (stod(argOneResult) <= stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == ">=") {
                                if (stod(argOneResult) >= stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                successfulIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (argOneResult == argThreeResult)
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (argOneResult != argThreeResult)
                                    failedIF();
                                else
                                    successfulIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                successfulIF();
                            }
                        }
                    } else {
                        error("invalid_operation:object_undefined:" + argOnebefore, false);
                        successfulIF();
                    }
		        } else if (zeroDots(argOne) && !zeroDots(argThree)) {
                    string argThreebefore(beforeDot(argThree)), argThreeafter(afterDot(argThree));

                    string argOneResult(""), argThreeResult("");

                    if (objectExists(argThreebefore)) {
                        if (objects.at(objectAt(argThreebefore)).methodExists(beforeParams(argThreeafter)))
                            executeTemplate(objects.at(objectAt(argThreebefore)).getMethod(beforeParams(argThreeafter)), getParams(argThreeafter));

                        argThreeResult = lastValue;

                        if (methodExists(beforeParams(argOne)))
                            executeTemplate(methods.at(methodAt(beforeParams(argOne))), getParams(argOne));

                        argOneResult = lastValue;

                        if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(argOneResult) == stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(argOneResult) != stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "<") {
                                if (stod(argOneResult) < stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == ">") {
                                if (stod(argOneResult) > stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "<=") {
                                if (stod(argOneResult) <= stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == ">=") {
                                if (stod(argOneResult) >= stod(argThreeResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                successfulIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (argOneResult == argThreeResult)
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (argOneResult != argThreeResult)
                                    failedIF();
                                else
                                    successfulIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                successfulIF();
                            }
                        }
                    } else {
                        error("invalid_operation:object_undefined:" + argThreebefore, false);
                        successfulIF();
                    }
		        } else if (zeroDots(argOne) && zeroDots(argThree)) {
		            string argOneResult(""), argThreeResult("");

                    if (methodExists(beforeParams(argOne)))
                        executeTemplate(methods.at(methodAt(beforeParams(argOne))), getParams(argOne));

                    argOneResult = lastValue;

                    if (methodExists(beforeParams(argThree)))
                        executeTemplate(methods.at(methodAt(beforeParams(argThree))), getParams(argThree));

                    argThreeResult = lastValue;

                    if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                        if (argTwo == "==" || argTwo == "is") {
                            if (stod(argOneResult) == stod(argThreeResult))
                                failedIF();
                            else
                                successfulIF();
                        } else if (argTwo == "!=" || argTwo == "not") {
                            if (stod(argOneResult) != stod(argThreeResult))
                                failedIF();
                            else
                                successfulIF();
                        } else if (argTwo == "<") {
                            if (stod(argOneResult) < stod(argThreeResult))
                                failedIF();
                            else
                                successfulIF();
                        } else if (argTwo == ">") {
                            if (stod(argOneResult) > stod(argThreeResult))
                                failedIF();
                            else
                                successfulIF();
                        } else if (argTwo == "<=") {
                            if (stod(argOneResult) <= stod(argThreeResult))
                                failedIF();
                            else
                                successfulIF();
                        } else if (argTwo == ">=") {
                            if (stod(argOneResult) >= stod(argThreeResult))
                                failedIF();
                            else
                                successfulIF();
                        } else {
                            error("invalid_operator:" + argTwo, false);
                            successfulIF();
                        }
                    } else {
                        if (argTwo == "==" || argTwo == "is") {
                            if (argOneResult == argThreeResult)
                                failedIF();
                            else
                                successfulIF();
                        } else if (argTwo == "!=" || argTwo == "not") {
                            if (argOneResult != argThreeResult)
                                failedIF();
                            else
                                successfulIF();
                        } else {
                            error("invalid_operator:" + argTwo, false);
                            successfulIF();
                        }
                    }
		        } else {
		            error("special_error(9)", false);
                    successfulIF();
		        }
		    } else if (containsParams(argOne) && !containsParams(argThree)) {
		        string argOneResult(""), argThreeResult("");

		        bool pass = true;

		        if (zeroDots(argOne)) {
                    if (methodExists(beforeParams(argOne))) {
                        executeTemplate(methods.at(methodAt(beforeParams(argOne))), getParams(argOne));

                        argOneResult = lastValue;

                        if (methodExists(argThree)) {
                            parse(argThree);
                            argThreeResult = lastValue;
                        } else if (variableExists(argThree)) {
                            if (variables.at(variableAt(argThree)).getString() != null)
                                argThreeResult = variables.at(variableAt(argThree)).getString();
                            else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                                argThreeResult = dtos(variables.at(variableAt(argThree)).getNumber());
                            else {
                                pass = false;
                                error("is_null:" + argThree, false);
                                successfulIF();
                            }
                        } else
                            argThreeResult = argThree;

                        if (pass) {
                            if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (stod(argOneResult) == stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (stod(argOneResult) != stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "<") {
                                    if (stod(argOneResult) < stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == ">") {
                                    if (stod(argOneResult) > stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "<=") {
                                    if (stod(argOneResult) <= stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == ">=") {
                                    if (stod(argOneResult) >= stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    successfulIF();
                                }
                            } else {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (argOneResult == argThreeResult)
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (argOneResult != argThreeResult)
                                        failedIF();
                                    else
                                        successfulIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    successfulIF();
                                }
                            }
                        } else
                            successfulIF();
                    } else {
                        error("invalid_operation:method_undefined:" + beforeParams(argOne), false);
                        successfulIF();
                    }
		        } else {
		            string argOnebefore(beforeDot(argOne)), argOneafter(afterDot(argOne));

		            if (objectExists(argOnebefore)) {
                        if (objects.at(objectAt(argOnebefore)).methodExists(beforeParams(argOneafter)))
                            executeTemplate(objects.at(objectAt(argOnebefore)).getMethod(beforeParams(argOneafter)), getParams(argOneafter));

                        argOneResult = lastValue;

                        if (variableExists(argThree)) {
                            if (variables.at(variableAt(argThree)).getString() != null)
                                argThreeResult = variables.at(variableAt(argThree)).getString();
                            else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                                argThreeResult = dtos(variables.at(variableAt(argThree)).getNumber());
                            else {
                                pass = false;
                                error("is_null:" + argThree, false);
                                successfulIF();
                            }
                        } else if (methodExists(argThree)) {
                            parse(argThree);

                            argThreeResult = lastValue;
                        } else
                            argThreeResult = argThree;

                        if (pass) {
                            if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (stod(argOneResult) == stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (stod(argOneResult) != stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "<") {
                                    if (stod(argOneResult) < stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == ">") {
                                    if (stod(argOneResult) > stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "<=") {
                                    if (stod(argOneResult) <= stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == ">=") {
                                    if (stod(argOneResult) >= stod(argThreeResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    successfulIF();
                                }
                            } else {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (argOneResult == argThreeResult)
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (argOneResult != argThreeResult)
                                        failedIF();
                                    else
                                        successfulIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    successfulIF();
                                }
                            }
                        }
		            } else {
		                error("invalid_operation:object_undefined:" + argOnebefore, false);
		                successfulIF();
		            }
		        }
		    } else if (!containsParams(argOne) && containsParams(argThree)) {
		        string argOneResult(""), argThreeResult("");

		        bool pass = true;

		        if (zeroDots(argThree)) {
                    if (methodExists(beforeParams(argThree))) {
                        executeTemplate(methods.at(methodAt(beforeParams(argThree))), getParams(argThree));

                        argThreeResult = lastValue;

                        if (methodExists(argOne)) {
                            parse(argOne);
                            argOneResult = lastValue;
                        } else if (variableExists(argOne)) {
                            if (variables.at(variableAt(argOne)).getString() != null)
                                argOneResult = variables.at(variableAt(argOne)).getString();
                            else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
                                argOneResult = dtos(variables.at(variableAt(argOne)).getNumber());
                            else {
                                pass = false;
                                error("is_null:" + argOne, false);
                                successfulIF();
                            }
                        } else
                            argOneResult = argOne;

                        if (pass) {
                            if (isNumeric(argThreeResult) && isNumeric(argOneResult)) {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (stod(argThreeResult) == stod(argOneResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (stod(argThreeResult) != stod(argOneResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "<") {
                                    if (stod(argThreeResult) < stod(argOneResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == ">") {
                                    if (stod(argThreeResult) > stod(argOneResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "<=") {
                                    if (stod(argThreeResult) <= stod(argOneResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == ">=") {
                                    if (stod(argThreeResult) >= stod(argOneResult))
                                        failedIF();
                                    else
                                        successfulIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    successfulIF();
                                }
                            } else {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (argThreeResult == argOneResult)
                                        failedIF();
                                    else
                                        successfulIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (argThreeResult != argOneResult)
                                        failedIF();
                                    else
                                        successfulIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    successfulIF();
                                }
                            }
                        }
                    } else {
                        error("invalid_operation:method_undefined:" + beforeParams(argThree), false);
                        successfulIF();
                    }
		        } else {
		            string argThreebefore(beforeDot(argThree)), argThreeafter(afterDot(argThree));

		            if (objectExists(argThreebefore)) {
                        if (objects.at(objectAt(argThreebefore)).methodExists(beforeParams(argThreeafter)))
                            executeTemplate(objects.at(objectAt(argThreebefore)).getMethod(beforeParams(argThreeafter)), getParams(argThreeafter));

                        argThreeResult = lastValue;

                        if (variableExists(argOne)) {
                            if (variables.at(variableAt(argOne)).getString() != null)
                                argOneResult = variables.at(variableAt(argOne)).getString();
                            else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                                argOneResult = dtos(variables.at(variableAt(argOne)).getNumber());
                            else {
                                error("is_null:" + argOne, false);
                                successfulIF();
                            }
                        } else if (methodExists(argOne)) {
                            parse(argOne);

                            argOneResult = lastValue;
                        } else
                            argOneResult = argOne;

                        if (isNumeric(argThreeResult) && isNumeric(argOneResult)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(argThreeResult) == stod(argOneResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(argThreeResult) != stod(argOneResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "<") {
                                if (stod(argThreeResult) < stod(argOneResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == ">") {
                                if (stod(argThreeResult) > stod(argOneResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "<=") {
                                if (stod(argThreeResult) <= stod(argOneResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == ">=") {
                                if (stod(argThreeResult) >= stod(argOneResult))
                                    failedIF();
                                else
                                    successfulIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                successfulIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (argThreeResult == argOneResult)
                                    failedIF();
                                else
                                    successfulIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (argThreeResult != argOneResult)
                                    failedIF();
                                else
                                    successfulIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                successfulIF();
                            }
                        }
		            } else {
		                error("invalid_operation:object_undefined:" + argThreebefore, false);
		                successfulIF();
		            }
		        }
		    }
		} else if ((methodExists(argOne) && argThree != "method?")|| methodExists(argThree)) {
		    string argOneResult(""), argThreeResult("");

		    if (methodExists(argOne)) {
		        parse(argOne);
		        argOneResult = lastValue;
		    } else if (variableExists(argOne)) {
		        if (variables.at(variableAt(argOne)).getString() != null)
                    argOneResult = variables.at(variableAt(argOne)).getString();
                else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
                    argOneResult = dtos(variables.at(variableAt(argOne)).getNumber());
                else {
                    error("is_null:" + argOne, false);
                    successfulIF();
                }
		    } else
                argOneResult = argOne;

		    if (methodExists(argThree)) {
		        parse(argThree);
		        argThreeResult = lastValue;
		    } else if (variableExists(argThree)) {
		        if (variables.at(variableAt(argThree)).getString() != null)
                    argThreeResult = variables.at(variableAt(argThree)).getString();
                else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                    argThreeResult = dtos(variables.at(variableAt(argThree)).getNumber());
                else {
                    error("is_null:" + argThree, false);
                    successfulIF();
                }
		    } else
                argThreeResult = argThree;

		    if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
		        if (argTwo == "==" || argTwo == "is") {
		            if (stod(argOneResult) == stod(argThreeResult))
                        failedIF();
                    else
                        successfulIF();
		        } else if (argTwo == "!=" || argTwo == "not") {
		            if (stod(argOneResult) != stod(argThreeResult))
                        failedIF();
                    else
                        successfulIF();
		        } else if (argTwo == "<") {
		            if (stod(argOneResult) < stod(argThreeResult))
                        failedIF();
                    else
                        successfulIF();
		        } else if (argTwo == ">") {
		            if (stod(argOneResult) > stod(argThreeResult))
                        failedIF();
                    else
                        successfulIF();
		        } else if (argTwo == "<=") {
		            if (stod(argOneResult) <= stod(argThreeResult))
                        failedIF();
                    else
                        successfulIF();
		        } else if (argTwo == ">=") {
		            if (stod(argOneResult) >= stod(argThreeResult))
                        failedIF();
                    else
                        successfulIF();
		        } else {
		            error("invalid_operator:" + argTwo, false);
		            successfulIF();
		        }
		    } else {
		        if (argTwo == "==" || argTwo == "is") {
		            if (argOneResult == argThreeResult)
                        failedIF();
                    else
                        successfulIF();
		        } else if (argTwo == "!=" || argTwo == "not") {
		            if (argOneResult != argThreeResult)
                        failedIF();
                    else
                        successfulIF();
		        } else {
		            error("invalid_operator:" + argTwo, false);
		            successfulIF();
		        }
		    }
		} else {
            if (argThree == "object?") {
                if (objectExists(argOne)) {
                    if (argTwo == "==" || argTwo == "is")
                        failedIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        successfulIF();
                    }
                } else {
                    if (argTwo == "==" || argTwo == "is")
                        successfulIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        successfulIF();
                    }
                }
            } else if (argThree == "var?" || argThree == "variable?") {
                if (variableExists(argOne)) {
                    if (argTwo == "==" || argTwo == "is")
                        failedIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        successfulIF();
                    }
                } else {
                    if (argTwo == "=")
                        successfulIF();
                    else if (argTwo == "!")
                        failedIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        successfulIF();
                    }
                }
            } else if (argThree == "method?") {
                if (methodExists(argOne)) {
                    if (argTwo == "==" || argTwo == "is")
                        failedIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        successfulIF();
                    }
                } else {
                    if (argTwo == "==" || argTwo == "is")
                        successfulIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        successfulIF();
                    }
                }
            } else if (argThree == "list?") {
                if (listExists(argOne)) {
                    if (argTwo == "==" || argTwo == "is")
                        failedIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        successfulIF();
                    }
                } else {
                    if (argTwo == "==" || argTwo == "is")
                        successfulIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        successfulIF();
                    }
                }
            } else if (argTwo == "==" || argTwo == "is") {
				if (argOne == argThree)
					failedIF();
				else
					successfulIF();
			} else if (argTwo == "!=" || argTwo == "not") {
				if (argOne != argThree)
					failedIF();
				else
					successfulIF();
			} else if (argTwo == ">") {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) > stod(argThree))
						failedIF();
					else
						successfulIF();
				} else {
					if (argOne.length() > argThree.length())
						failedIF();
					else
						successfulIF();
				}
			} else if (argTwo == "<") {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) < stod(argThree))
						failedIF();
					else
						successfulIF();
				} else {
					if (argOne.length() < argThree.length())
						failedIF();
					else
						successfulIF();
				}
			} else if (argTwo == ">=") {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) >= stod(argThree))
						failedIF();
					else
						successfulIF();
				} else {
				    error("invalid_operator:>=", false);
				    successfulIF();
				}
			} else if (argTwo == "<=") {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) <= stod(argThree))
						failedIF();
					else
						successfulIF();
				} else {
					error("invalid_operator:<=", false);
                    failedIF();
                }
			} else if (argTwo == "begins_with") {
				if (startsWith(argOne, argThree))
					failedIF();
				else
					successfulIF();
			} else if (argTwo == "ends_with") {
				if (endsWith(argOne, argThree))
					failedIF();
				else
					successfulIF();
			} else if (argTwo == "contains") {
				if (contains(argOne, argThree))
					failedIF();
				else
					successfulIF();
			} else {
				error("invalid_operator:" + argTwo, false);
				successfulIF();
			}
		}
	} else if (argZero == "if") {
	    if (listExists(argThree)) {
	        if (argTwo == "in") {
	            string testString("[none]");

                if (variableExists(argOne)) {
                    if (variables.at(variableAt(argOne)).getString() != null)
                        testString = variables.at(variableAt(argOne)).getString();
                    else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
                        testString = dtos(variables.at(variableAt(argOne)).getNumber());
                    else
                        error("is_null:" + argOne, false);
                } else
                    testString = argOne;

                if (testString != "[none]") {
                    bool elementFound = false;
                    for (int i = 0; i < (int)lists.at(listAt(argThree)).size(); i++) {
                        if (lists.at(listAt(argThree)).at(i) == testString) {
                            elementFound = true;
                            successfulIF();
                            lastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        failedIF();
                } else
                    failedIF();
            }
	    } else if (listExists(argOne) && argThree != "list?") {
            if (argTwo == "contains") {
	            string testString("[none]");

                if (variableExists(argThree)) {
                    if (variables.at(variableAt(argThree)).getString() != null)
                        testString = variables.at(variableAt(argThree)).getString();
                    else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                        testString = dtos(variables.at(variableAt(argThree)).getNumber());
                    else
                        error("is_null:" + argThree, false);
                } else
                    testString = argThree;

                if (testString != "[none]") {
                    bool elementFound = false;
                    for (int i = 0; i < (int)lists.at(listAt(argOne)).size(); i++) {
                        if (lists.at(listAt(argOne)).at(i) == testString) {
                            elementFound = true;
                            successfulIF();
                            lastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        failedIF();
                } else
                    failedIF();
            }
	    } else if (variableExists(argOne) && variableExists(argThree)) {
			if (variables.at(variableAt(argOne)).getString() != null && variables.at(variableAt(argThree)).getString() != null) {
				if (argTwo == "==" || argTwo == "is") {
                    if (variables.at(variableAt(argOne)).getString() == variables.at(variableAt(argThree)).getString())
						successfulIF();
                    else
						failedIF();
				} else if (argTwo == "!=" || argTwo == "not") {
					if (variables.at(variableAt(argOne)).getString() != variables.at(variableAt(argThree)).getString())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == ">") {
					if (variables.at(variableAt(argOne)).getString().length() > variables.at(variableAt(argThree)).getString().length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "<") {
					if (variables.at(variableAt(argOne)).getString().length() < variables.at(variableAt(argThree)).getString().length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "<=") {
					if (variables.at(variableAt(argOne)).getString().length() <= variables.at(variableAt(argThree)).getString().length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == ">=") {
					if (variables.at(variableAt(argOne)).getString().length() >= variables.at(variableAt(argThree)).getString().length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "contains") {
					if (contains(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argThree)).getString()))
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "ends_with") {
					if (endsWith(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argThree)).getString()))
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "begins_with") {
					if (startsWith(variables.at(variableAt(argOne)).getString(), variables.at(variableAt(argThree)).getString()))
						successfulIF();
					else
						failedIF();
				} else {
					error("invalid_operator:" + argTwo, false);
					failedIF();
				}
			} else if (variables.at(variableAt(argOne)).getNumber() != nullNum && variables.at(variableAt(argThree)).getNumber() != nullNum) {
				if (argTwo == "==" || argTwo == "is") {
					if (variables.at(variableAt(argOne)).getNumber() == variables.at(variableAt(argThree)).getNumber())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "!=" || argTwo == "not") {
					if (variables.at(variableAt(argOne)).getNumber() != variables.at(variableAt(argThree)).getNumber())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == ">") {
					if (variables.at(variableAt(argOne)).getNumber() > variables.at(variableAt(argThree)).getNumber())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == ">=") {
					if (variables.at(variableAt(argOne)).getNumber() >= variables.at(variableAt(argThree)).getNumber())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "<") {
					if (variables.at(variableAt(argOne)).getNumber() < variables.at(variableAt(argThree)).getNumber())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "<=") {
					if (variables.at(variableAt(argOne)).getNumber() <= variables.at(variableAt(argThree)).getNumber())
						successfulIF();
					else
						failedIF();
				} else {
					error("invalid_operator:" + argTwo, false);
					failedIF();
				}
			} else {
				error("conversion_error:" + s, false);
				failedIF();
			}
		} else if ((variableExists(argOne) && !variableExists(argThree)) && !methodExists(argThree) && notObjectMethod(argThree) && !containsParams(argThree)) {
			if (variables.at(variableAt(argOne)).getNumber()!= nullNum) {
			  	if (isNumeric(argThree)) {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argOne)).getNumber() == stod(argThree))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argOne)).getNumber() != stod(argThree))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argOne)).getNumber() > stod(argThree))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argOne)).getNumber() < stod(argThree))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argOne)).getNumber() >= stod(argThree))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argOne)).getNumber() <= stod(argThree))
							successfulIF();
						else
							failedIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						failedIF();
					}
				} else if (argThree == "number?") {
				    if (argTwo == "==" || argTwo == "is")
				        successfulIF();
				    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else
                        error("invalid_operator:" + argTwo, false);
				} else {
					error("conversion_error:" + s, false);
					failedIF();
				}
			} else if (variables.at(variableAt(argOne)).getString() != null) {
				if (argThree == "string?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is")
							successfulIF();
						else if (argTwo == "!=" || argTwo == "not")
							failedIF();
						else {
							error("invalid_operator:" + argTwo, false);
							failedIF();
						}
					} else {
						if (argTwo == "!=")
							successfulIF();
						else
							failedIF();
					}
				} else if (argThree == "number?") {
					if (variables.at(variableAt(argOne)).getNumber() != nullNum) {
						if (argTwo == "==" || argTwo == "is")
							successfulIF();
						else if (argTwo == "!=" || argTwo == "not")
							failedIF();
						else {
							error("invalid_operator:" + argTwo, false);
							failedIF();
						}
					} else {
						if (argTwo == "!=")
							successfulIF();
						else
							failedIF();
					}
				} else if (argThree == "upper?" || argThree == "uppercase?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is") {
                            if (isUpper(variables.at(variableAt(argOne)).getString()))
							    successfulIF();
		                    else
		                        failedIF();
						} else if (argTwo == "!=" || argTwo == "not") {
                            if (isUpper(variables.at(variableAt(argOne)).getString()))
							    failedIF();
		                    else
		                        successfulIF();
						} else {
							error("invalid_operator:" + argTwo, false);
							failedIF();
						}
					} else {
						if (argTwo == "!=") {
                            if (isUpper(argTwo))
                                failedIF();
                            else
							    successfulIF();
						} else
							failedIF();
					}
				} else if (argThree == "lower?" || argThree == "lowercase?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is") {
                            if (isLower(variables.at(variableAt(argOne)).getString()))
							    successfulIF();
		                    else
		                        failedIF();
						} else if (argTwo == "!=" || argTwo == "not") {
                            if (isLower(variables.at(variableAt(argOne)).getString()))
							    failedIF();
		                    else
		                        successfulIF();
						} else {
							error("invalid_operator:" + argTwo, false);
							failedIF();
						}
					} else {
						if (argTwo == "!=") {
                            if (isLower(argTwo))
                                failedIF();
                            else
							    successfulIF();
						} else
							failedIF();
					}
				} else if (argThree == "file?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (fileExists(variables.at(variableAt(argOne)).getString())) {
							if (argTwo == "==" || argTwo == "is")
								successfulIF();
							else if (argTwo == "!=" || argTwo == "not")
								failedIF();
							else {
								error("invalid_operator:" + argTwo, false);
								failedIF();
							}
						} else {
							if (argTwo == "!=")
								successfulIF();
							else
								failedIF();
						}
					} else {
						error("is_null:" + argOne, false);
						failedIF();
					}
				} else if (argThree == "dir?" || argThree == "directory?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (directoryExists(variables.at(variableAt(argOne)).getString())) {
							if (argTwo == "==" || argTwo == "is")
								successfulIF();
							else if (argTwo == "!=" || argTwo == "not")
								failedIF();
							else {
								error("invalid_operator:" + argTwo, false);
								failedIF();
							}
						} else {
							if (argTwo == "!=")
								successfulIF();
							else
								failedIF();
						}
					} else {
						error("is_null:" + argOne, false);
						failedIF();
					}
				} else {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argOne)).getString() == argThree)
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argOne)).getString() != argThree)
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argOne)).getString().length() > argThree.length())
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argOne)).getString().length() < argThree.length())
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argOne)).getString().length() >= argThree.length())
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argOne)).getString().length() <= argThree.length())
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "contains") {
						if (contains(variables.at(variableAt(argOne)).getString(), argThree))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "ends_with") {
						if (endsWith(variables.at(variableAt(argOne)).getString(), argThree))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "begins_with") {
						if (startsWith(variables.at(variableAt(argOne)).getString(), argThree))
							successfulIF();
						else
							failedIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						failedIF();
					}
				}
			} else {
                error("special_error(0)", false); // variable is neither string nor number
                failedIF();
			}
		} else if ((variableExists(argOne) && !variableExists(argThree)) && !methodExists(argThree) && notObjectMethod(argThree) && containsParams(argThree)) {
		    string stackValue("");

		    if (isStringStack(argThree))
		        stackValue = getStringStack(argThree);
		    else if (stackReady(argThree))
                stackValue = dtos(getStack(argThree));
            else
                stackValue = argThree;

			if (variables.at(variableAt(argOne)).getNumber()!= nullNum) {
			  	if (isNumeric(stackValue)) {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argOne)).getNumber() == stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argOne)).getNumber() != stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argOne)).getNumber() > stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argOne)).getNumber() < stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argOne)).getNumber() >= stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argOne)).getNumber() <= stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						failedIF();
					}
				} else if (stackValue == "number?") {
				    if (argTwo == "==" || argTwo == "is")
				        successfulIF();
				    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else
                        error("invalid_operator:" + argTwo, false);
				} else {
					error("conversion_error:" + s, false);
					failedIF();
				}
			} else if (variables.at(variableAt(argOne)).getString() != null) {
				if (stackValue == "string?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is")
							successfulIF();
						else if (argTwo == "!=" || argTwo == "not")
							failedIF();
						else {
							error("invalid_operator:" + argTwo, false);
							failedIF();
						}
					} else {
						if (argTwo == "!=")
							successfulIF();
						else
							failedIF();
					}
				} else if (stackValue == "number?") {
					if (variables.at(variableAt(argOne)).getNumber() != nullNum) {
						if (argTwo == "==" || argTwo == "is")
							successfulIF();
						else if (argTwo == "!=" || argTwo == "not")
							failedIF();
						else {
							error("invalid_operator:" + argTwo, false);
							failedIF();
						}
					} else {
						if (argTwo == "!=")
							successfulIF();
						else
							failedIF();
					}
				} else if (stackValue == "upper?" || stackValue == "uppercase?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is") {
                            if (isUpper(variables.at(variableAt(argOne)).getString()))
							    successfulIF();
		                    else
		                        failedIF();
						} else if (argTwo == "!=" || argTwo == "not") {
                            if (isUpper(variables.at(variableAt(argOne)).getString()))
							    failedIF();
		                    else
		                        successfulIF();
						} else {
							error("invalid_operator:" + argTwo, false);
							failedIF();
						}
					} else {
						if (argTwo == "!=") {
                            if (isUpper(argTwo))
                                failedIF();
                            else
							    successfulIF();
						} else
							failedIF();
					}
				} else if (stackValue == "lower?" || stackValue == "lowercase?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (argTwo == "==" || argTwo == "is") {
                            if (isLower(variables.at(variableAt(argOne)).getString()))
							    successfulIF();
		                    else
		                        failedIF();
						} else if (argTwo == "!=" || argTwo == "not") {
                            if (isLower(variables.at(variableAt(argOne)).getString()))
							    failedIF();
		                    else
		                        successfulIF();
						} else {
							error("invalid_operator:" + argTwo, false);
							failedIF();
						}
					} else {
						if (argTwo == "!=") {
                            if (isLower(argTwo))
                                failedIF();
                            else
							    successfulIF();
						} else
							failedIF();
					}
				} else if (stackValue == "file?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (fileExists(variables.at(variableAt(argOne)).getString())) {
							if (argTwo == "==" || argTwo == "is")
								successfulIF();
							else if (argTwo == "!=" || argTwo == "not")
								failedIF();
							else {
								error("invalid_operator:" + argTwo, false);
								failedIF();
							}
						} else {
							if (argTwo == "!=")
								successfulIF();
							else
								failedIF();
						}
					} else {
						error("is_null:" + argOne, false);
						failedIF();
					}
				} else if (stackValue == "dir?" || stackValue == "directory?") {
					if (variables.at(variableAt(argOne)).getString() != null) {
						if (directoryExists(variables.at(variableAt(argOne)).getString())) {
							if (argTwo == "==" || argTwo == "is")
								successfulIF();
							else if (argTwo == "!=" || argTwo == "not")
								failedIF();
							else {
								error("invalid_operator:" + argTwo, false);
								failedIF();
							}
						} else {
							if (argTwo == "!=")
								successfulIF();
							else
								failedIF();
						}
					} else {
						error("is_null:" + argOne, false);
						failedIF();
					}
				} else {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argOne)).getString() == stackValue)
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argOne)).getString() != stackValue)
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argOne)).getString().length() > stackValue.length())
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argOne)).getString().length() < stackValue.length())
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argOne)).getString().length() >= stackValue.length())
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argOne)).getString().length() <= stackValue.length())
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "contains") {
						if (contains(variables.at(variableAt(argOne)).getString(), stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "ends_with") {
						if (endsWith(variables.at(variableAt(argOne)).getString(), stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "begins_with") {
						if (startsWith(variables.at(variableAt(argOne)).getString(), stackValue))
							successfulIF();
						else
							failedIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						failedIF();
					}
				}
			} else {
				error("special_error(0)", false); // variable is neither string nor number
				failedIF();
			}
        } else if ((!variableExists(argOne) && variableExists(argThree)) && !methodExists(argOne) && notObjectMethod(argOne) && !containsParams(argOne)) {
			if (variables.at(variableAt(argThree)).getNumber() != nullNum) {
				if (isNumeric(argOne)) {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argThree)).getNumber() == stod(argOne))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argThree)).getNumber() != stod(argOne))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argThree)).getNumber() > stod(argOne))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argThree)).getNumber() < stod(argOne))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argThree)).getNumber() >= stod(argOne))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argThree)).getNumber() <= stod(argOne))
							successfulIF();
						else
							failedIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						failedIF();
					}
				} else {
					error("conversion_error:" + s, false);
					failedIF();
				}
			} else if (variables.at(variableAt(argThree)).getString() != null) {
				if (argTwo == "==" || argTwo == "is") {
					if (variables.at(variableAt(argThree)).getString() == argOne)
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "!=" || argTwo == "not") {
					if (variables.at(variableAt(argThree)).getString() != argOne)
						successfulIF();
					else
						failedIF();
				} else if (argTwo == ">") {
					if (variables.at(variableAt(argThree)).getString().length() > argOne.length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "<") {
					if (variables.at(variableAt(argThree)).getString().length() < argOne.length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == ">=") {
					if (variables.at(variableAt(argThree)).getString().length() >= argOne.length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "<=") {
					if (variables.at(variableAt(argThree)).getString().length() <= argOne.length())
						successfulIF();
					else
						failedIF();
				} else {
					error("invalid_operator:" + argTwo, false);
					failedIF();
				}
			} else {
				error("special_error(1)", false); // variable is neither string nor number
				failedIF();
			}
		} else if ((!variableExists(argOne) && variableExists(argThree)) && !methodExists(argOne) && notObjectMethod(argOne) && containsParams(argOne)) {
		    string stackValue("");

		    if (isStringStack(argOne))
                stackValue = getStringStack(argOne);
            else if (stackReady(argOne))
                stackValue = dtos(getStack(argOne));
            else
                stackValue = argOne;

			if (variables.at(variableAt(argThree)).getNumber() != nullNum) {
				if (isNumeric(stackValue)) {
					if (argTwo == "==" || argTwo == "is") {
						if (variables.at(variableAt(argThree)).getNumber() == stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "!=" || argTwo == "not") {
						if (variables.at(variableAt(argThree)).getNumber() != stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">") {
						if (variables.at(variableAt(argThree)).getNumber() > stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<") {
						if (variables.at(variableAt(argThree)).getNumber() < stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == ">=") {
						if (variables.at(variableAt(argThree)).getNumber() >= stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else if (argTwo == "<=") {
						if (variables.at(variableAt(argThree)).getNumber() <= stod(stackValue))
							successfulIF();
						else
							failedIF();
					} else {
						error("invalid_operator:" + argTwo, false);
						failedIF();
					}
				} else {
					error("conversion_error:" + s, false);
					failedIF();
				}
			} else if (variables.at(variableAt(argThree)).getString() != null) {
				if (argTwo == "==" || argTwo == "is") {
					if (variables.at(variableAt(argThree)).getString() == stackValue)
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "!=" || argTwo == "not") {
					if (variables.at(variableAt(argThree)).getString() != stackValue)
						successfulIF();
					else
						failedIF();
				} else if (argTwo == ">") {
					if (variables.at(variableAt(argThree)).getString().length() > stackValue.length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "<") {
					if (variables.at(variableAt(argThree)).getString().length() < stackValue.length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == ">=") {
					if (variables.at(variableAt(argThree)).getString().length() >= stackValue.length())
						successfulIF();
					else
						failedIF();
				} else if (argTwo == "<=") {
					if (variables.at(variableAt(argThree)).getString().length() <= stackValue.length())
						successfulIF();
					else
						failedIF();
				} else {
					error("invalid_operator:" + argTwo, false);
					failedIF();
				}
			} else {
				error("special_error(1)", false); // variable is neither string nor number
				failedIF();
			}
		}
		else if (containsParams(argOne) || containsParams(argThree)) {
		    if (containsParams(argOne) && containsParams(argThree)) {
		        if (!zeroDots(argOne) && !zeroDots(argThree)) {
                    string argOnebefore(beforeDot(argOne)), argOneafter(afterDot(argOne)),
                        argThreebefore(beforeDot(argThree)), argThreeafter(afterDot(argThree));

                    string argOneResult(""), argThreeResult("");

                    if (objectExists(argOnebefore) && objectExists(argThreebefore)) {
                        if (objects.at(objectAt(argOnebefore)).methodExists(beforeParams(argOneafter)))
                            executeTemplate(objects.at(objectAt(argOnebefore)).getMethod(beforeParams(argOneafter)), getParams(argOneafter));

                        argOneResult = lastValue;

                        if (objects.at(objectAt(argThreebefore)).methodExists(beforeParams(argThreeafter)))
                            executeTemplate(objects.at(objectAt(argThreebefore)).getMethod(beforeParams(argThreeafter)), getParams(argThreeafter));

                        argThreeResult = lastValue;

                        if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(argOneResult) == stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(argOneResult) != stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<") {
                                if (stod(argOneResult) < stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">") {
                                if (stod(argOneResult) > stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<=") {
                                if (stod(argOneResult) <= stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">=") {
                                if (stod(argOneResult) >= stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (argOneResult == argThreeResult)
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (argOneResult != argThreeResult)
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        }
                    } else {
                        if (!objectExists(argOnebefore))
                            error("invalid_operation:object_undefined:" + argOnebefore, false);

                        if (!objectExists(argThreebefore))
                            error("invalid_operation:object_undefined:" + argThreebefore, false);

                        failedIF();
                    }
		        } else if (!zeroDots(argOne) && zeroDots(argThree)) {
		            string argOnebefore(beforeDot(argOne)), argOneafter(afterDot(argOne));

                    string argOneResult(""), argThreeResult("");

                    if (objectExists(argOnebefore)) {
                        if (objects.at(objectAt(argOnebefore)).methodExists(beforeParams(argOneafter)))
                            executeTemplate(objects.at(objectAt(argOnebefore)).getMethod(beforeParams(argOneafter)), getParams(argOneafter));

                        argOneResult = lastValue;

                        if (methodExists(beforeParams(argThree)))
                            executeTemplate(methods.at(methodAt(beforeParams(argThree))), getParams(argThree));

                        argThreeResult = lastValue;

                        if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(argOneResult) == stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(argOneResult) != stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<") {
                                if (stod(argOneResult) < stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">") {
                                if (stod(argOneResult) > stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<=") {
                                if (stod(argOneResult) <= stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">=") {
                                if (stod(argOneResult) >= stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (argOneResult == argThreeResult)
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (argOneResult != argThreeResult)
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        }
                    } else {
                        error("invalid_operation:object_undefined:" + argOnebefore, false);
                        failedIF();
                    }
		        } else if (zeroDots(argOne) && !zeroDots(argThree)) {
                    string argThreebefore(beforeDot(argThree)), argThreeafter(afterDot(argThree));

                    string argOneResult(""), argThreeResult("");

                    if (objectExists(argThreebefore)) {
                        if (objects.at(objectAt(argThreebefore)).methodExists(beforeParams(argThreeafter)))
                            executeTemplate(objects.at(objectAt(argThreebefore)).getMethod(beforeParams(argThreeafter)), getParams(argThreeafter));

                        argThreeResult = lastValue;

                        if (methodExists(beforeParams(argOne)))
                            executeTemplate(methods.at(methodAt(beforeParams(argOne))), getParams(argOne));

                        argOneResult = lastValue;

                        if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(argOneResult) == stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(argOneResult) != stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<") {
                                if (stod(argOneResult) < stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">") {
                                if (stod(argOneResult) > stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<=") {
                                if (stod(argOneResult) <= stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">=") {
                                if (stod(argOneResult) >= stod(argThreeResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (argOneResult == argThreeResult)
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (argOneResult != argThreeResult)
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        }
                    } else {
                        error("invalid_operation:object_undefined:" + argThreebefore, false);
                        failedIF();
                    }
		        } else if (zeroDots(argOne) && zeroDots(argThree)) {
		            string argOneResult(""), argThreeResult("");

                    if (methodExists(beforeParams(argOne)))
                        executeTemplate(methods.at(methodAt(beforeParams(argOne))), getParams(argOne));

                    argOneResult = lastValue;

                    if (methodExists(beforeParams(argThree)))
                        executeTemplate(methods.at(methodAt(beforeParams(argThree))), getParams(argThree));

                    argThreeResult = lastValue;

                    if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                        if (argTwo == "==" || argTwo == "is") {
                            if (stod(argOneResult) == stod(argThreeResult))
                                successfulIF();
                            else
                                failedIF();
                        } else if (argTwo == "!=" || argTwo == "not") {
                            if (stod(argOneResult) != stod(argThreeResult))
                                successfulIF();
                            else
                                failedIF();
                        } else if (argTwo == "<") {
                            if (stod(argOneResult) < stod(argThreeResult))
                                successfulIF();
                            else
                                failedIF();
                        } else if (argTwo == ">") {
                            if (stod(argOneResult) > stod(argThreeResult))
                                successfulIF();
                            else
                                failedIF();
                        } else if (argTwo == "<=") {
                            if (stod(argOneResult) <= stod(argThreeResult))
                                successfulIF();
                            else
                                failedIF();
                        } else if (argTwo == ">=") {
                            if (stod(argOneResult) >= stod(argThreeResult))
                                successfulIF();
                            else
                                failedIF();
                        } else {
                            error("invalid_operator:" + argTwo, false);
                            failedIF();
                        }
                    } else {
                        if (argTwo == "==" || argTwo == "is") {
                            if (argOneResult == argThreeResult)
                                successfulIF();
                            else
                                failedIF();
                        } else if (argTwo == "!=" || argTwo == "not") {
                            if (argOneResult != argThreeResult)
                                successfulIF();
                            else
                                failedIF();
                        } else {
                            error("invalid_operator:" + argTwo, false);
                            failedIF();
                        }
                    }
		        } else {
		            error("special_error(9)", false);
                    failedIF();
		        }
		    } else if (containsParams(argOne) && !containsParams(argThree)) {
		        string argOneResult(""), argThreeResult("");

		        bool pass = true;

		        if (zeroDots(argOne)) {
                    if (methodExists(beforeParams(argOne))) {
                        executeTemplate(methods.at(methodAt(beforeParams(argOne))), getParams(argOne));

                        argOneResult = lastValue;

                        if (methodExists(argThree)) {
                            parse(argThree);
                            argThreeResult = lastValue;
                        } else if (variableExists(argThree)) {
                            if (variables.at(variableAt(argThree)).getString() != null)
                                argThreeResult = variables.at(variableAt(argThree)).getString();
                            else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                                argThreeResult = dtos(variables.at(variableAt(argThree)).getNumber());
                            else {
                                pass = false;
                                error("is_null:" + argThree, false);
                                failedIF();
                            }
                        } else
                            argThreeResult = argThree;

                        if (pass) {
                            if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (stod(argOneResult) == stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (stod(argOneResult) != stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "<") {
                                    if (stod(argOneResult) < stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == ">") {
                                    if (stod(argOneResult) > stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "<=") {
                                    if (stod(argOneResult) <= stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == ">=") {
                                    if (stod(argOneResult) >= stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    failedIF();
                                }
                            } else {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (argOneResult == argThreeResult)
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (argOneResult != argThreeResult)
                                        successfulIF();
                                    else
                                        failedIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    failedIF();
                                }
                            }
                        } else
                            failedIF();
                    } else if (stackReady(argOne)) {
                        string stackValue("");

                        if (isStringStack(argOne))
                            stackValue = getStringStack(argOne);
                        else
                            stackValue = dtos(getStack(argOne));

                        string comp("");

                        if (variableExists(argThree)) {
                            if (variables.at(variableAt(argThree)).getString() != null)
                                comp = variables.at(variableAt(argThree)).getString();
                            else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                                comp = dtos(variables.at(variableAt(argThree)).getNumber());
                        } else if (methodExists(argThree)) {
                            parse(argThree);

                            comp = lastValue;
                        } else if (containsParams(argThree)) {
                            executeTemplate(getMethod(beforeParams(argThree)), getParams(argThree));

                            comp = lastValue;
                        } else
                            comp = argThree;

                        if (isNumeric(stackValue) && isNumeric(comp)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(stackValue) == stod(comp))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(stackValue) != stod(comp))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<") {
                                if (stod(stackValue) < stod(comp))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">") {
                                if (stod(stackValue) > stod(comp))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<=") {
                                if (stod(stackValue) <= stod(comp))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">=") {
                                if (stod(stackValue) >= stod(comp))
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stackValue == comp)
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stackValue != comp)
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        }
                    } else {
                        error("invalid_operation:method_undefined:" + beforeParams(argOne), false);
                        failedIF();
                    }
		        } else {
		            string argOnebefore(beforeDot(argOne)), argOneafter(afterDot(argOne));

		            if (objectExists(argOnebefore)) {
                        if (objects.at(objectAt(argOnebefore)).methodExists(beforeParams(argOneafter)))
                            executeTemplate(objects.at(objectAt(argOnebefore)).getMethod(beforeParams(argOneafter)), getParams(argOneafter));

                        argOneResult = lastValue;

                        if (variableExists(argThree)) {
                            if (variables.at(variableAt(argThree)).getString() != null)
                                argThreeResult = variables.at(variableAt(argThree)).getString();
                            else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                                argThreeResult = dtos(variables.at(variableAt(argThree)).getNumber());
                            else {
                                pass = false;
                                error("is_null:" + argThree, false);
                                failedIF();
                            }
                        } else if (methodExists(argThree)) {
                            parse(argThree);

                            argThreeResult = lastValue;
                        } else
                            argThreeResult = argThree;

                        if (pass) {
                            if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (stod(argOneResult) == stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (stod(argOneResult) != stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "<") {
                                    if (stod(argOneResult) < stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == ">") {
                                    if (stod(argOneResult) > stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "<=") {
                                    if (stod(argOneResult) <= stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == ">=") {
                                    if (stod(argOneResult) >= stod(argThreeResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    failedIF();
                                }
                            } else {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (argOneResult == argThreeResult)
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (argOneResult != argThreeResult)
                                        successfulIF();
                                    else
                                        failedIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    failedIF();
                                }
                            }
                        }
		            } else {
		                error("invalid_operation:object_undefined:" + argOnebefore, false);
		                failedIF();
		            }
		        }
		    } else if (!containsParams(argOne) && containsParams(argThree)) {
		        string argOneResult(""), argThreeResult("");

		        bool pass = true;

		        if (zeroDots(argThree)) {
                    if (methodExists(beforeParams(argThree))) {
                        executeTemplate(methods.at(methodAt(beforeParams(argThree))), getParams(argThree));

                        argThreeResult = lastValue;

                        if (methodExists(argOne)) {
                            parse(argOne);
                            argOneResult = lastValue;
                        } else if (variableExists(argOne)) {
                            if (variables.at(variableAt(argOne)).getString() != null)
                                argOneResult = variables.at(variableAt(argOne)).getString();
                            else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
                                argOneResult = dtos(variables.at(variableAt(argOne)).getNumber());
                            else {
                                pass = false;
                                error("is_null:" + argOne, false);
                                failedIF();
                            }
                        } else
                            argOneResult = argOne;

                        if (pass) {
                            if (isNumeric(argThreeResult) && isNumeric(argOneResult)) {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (stod(argThreeResult) == stod(argOneResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (stod(argThreeResult) != stod(argOneResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "<") {
                                    if (stod(argThreeResult) < stod(argOneResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == ">") {
                                    if (stod(argThreeResult) > stod(argOneResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "<=") {
                                    if (stod(argThreeResult) <= stod(argOneResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == ">=") {
                                    if (stod(argThreeResult) >= stod(argOneResult))
                                        successfulIF();
                                    else
                                        failedIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    failedIF();
                                }
                            } else {
                                if (argTwo == "==" || argTwo == "is") {
                                    if (argThreeResult == argOneResult)
                                        successfulIF();
                                    else
                                        failedIF();
                                } else if (argTwo == "!=" || argTwo == "not") {
                                    if (argThreeResult != argOneResult)
                                        successfulIF();
                                    else
                                        failedIF();
                                } else {
                                    error("invalid_operator:" + argTwo, false);
                                    failedIF();
                                }
                            }
                        }
                    } else {
                        error("invalid_operation:method_undefined:" + beforeParams(argThree), false);
                        failedIF();
                    }
		        } else {
		            string argThreebefore(beforeDot(argThree)), argThreeafter(afterDot(argThree));

		            if (objectExists(argThreebefore)) {
                        if (objects.at(objectAt(argThreebefore)).methodExists(beforeParams(argThreeafter)))
                            executeTemplate(objects.at(objectAt(argThreebefore)).getMethod(beforeParams(argThreeafter)), getParams(argThreeafter));

                        argThreeResult = lastValue;

                        if (variableExists(argOne)) {
                            if (variables.at(variableAt(argOne)).getString() != null)
                                argOneResult = variables.at(variableAt(argOne)).getString();
                            else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                                argOneResult = dtos(variables.at(variableAt(argOne)).getNumber());
                            else {
                                error("is_null:" + argOne, false);
                                failedIF();
                            }
                        } else if (methodExists(argOne)) {
                            parse(argOne);

                            argOneResult = lastValue;
                        } else
                            argOneResult = argOne;

                        if (isNumeric(argThreeResult) && isNumeric(argOneResult)) {
                            if (argTwo == "==" || argTwo == "is") {
                                if (stod(argThreeResult) == stod(argOneResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (stod(argThreeResult) != stod(argOneResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<") {
                                if (stod(argThreeResult) < stod(argOneResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">") {
                                if (stod(argThreeResult) > stod(argOneResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "<=") {
                                if (stod(argThreeResult) <= stod(argOneResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == ">=") {
                                if (stod(argThreeResult) >= stod(argOneResult))
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        } else {
                            if (argTwo == "==" || argTwo == "is") {
                                if (argThreeResult == argOneResult)
                                    successfulIF();
                                else
                                    failedIF();
                            } else if (argTwo == "!=" || argTwo == "not") {
                                if (argThreeResult != argOneResult)
                                    successfulIF();
                                else
                                    failedIF();
                            } else {
                                error("invalid_operator:" + argTwo, false);
                                failedIF();
                            }
                        }
		            } else {
		                error("invalid_operation:object_undefined:" + argThreebefore, false);
		                failedIF();
		            }
		        }
		    }
		} else if ((methodExists(argOne) && argThree != "method?") || methodExists(argThree)) {
		    string argOneResult(""), argThreeResult("");

		    if (methodExists(argOne)) {
		        parse(argOne);
		        argOneResult = lastValue;
		    } else if (variableExists(argOne)) {
		        if (variables.at(variableAt(argOne)).getString() != null)
                    argOneResult = variables.at(variableAt(argOne)).getString();
                else if (variables.at(variableAt(argOne)).getNumber() != nullNum)
                    argOneResult = dtos(variables.at(variableAt(argOne)).getNumber());
                else {
                    error("is_null:" + argOne, false);
                    failedIF();
                }
		    } else
                argOneResult = argOne;

		    if (methodExists(argThree)) {
		        parse(argThree);
		        argThreeResult = lastValue;
		    } else if (variableExists(argThree)) {
		        if (variables.at(variableAt(argThree)).getString() != null)
                    argThreeResult = variables.at(variableAt(argThree)).getString();
                else if (variables.at(variableAt(argThree)).getNumber() != nullNum)
                    argThreeResult = dtos(variables.at(variableAt(argThree)).getNumber());
                else {
                    error("is_null:" + argThree, false);
                    failedIF();
                }
		    } else
                argThreeResult = argThree;

		    if (isNumeric(argOneResult) && isNumeric(argThreeResult)) {
		        if (argTwo == "==" || argTwo == "is") {
		            if (stod(argOneResult) == stod(argThreeResult))
                        successfulIF();
                    else
                        failedIF();
		        } else if (argTwo == "!=" || argTwo == "not") {
		            if (stod(argOneResult) != stod(argThreeResult))
                        successfulIF();
                    else
                        failedIF();
		        } else if (argTwo == "<") {
		            if (stod(argOneResult) < stod(argThreeResult))
                        successfulIF();
                    else
                        failedIF();
		        } else if (argTwo == ">") {
		            if (stod(argOneResult) > stod(argThreeResult))
                        successfulIF();
                    else
                        failedIF();
		        } else if (argTwo == "<=") {
		            if (stod(argOneResult) <= stod(argThreeResult))
                        successfulIF();
                    else
                        failedIF();
		        } else if (argTwo == ">=") {
		            if (stod(argOneResult) >= stod(argThreeResult))
                        successfulIF();
                    else
                        failedIF();
		        } else {
		            error("invalid_operator:" + argTwo, false);
		            failedIF();
		        }
		    } else {
		        if (argTwo == "==" || argTwo == "is") {
		            if (argOneResult == argThreeResult)
                        successfulIF();
                    else
                        failedIF();
		        } else if (argTwo == "!=" || argTwo == "not") {
		            if (argOneResult != argThreeResult)
                        successfulIF();
                    else
                        failedIF();
		        } else {
		            error("invalid_operator:" + argTwo, false);
		            failedIF();
		        }
		    }
		} else {
            if (argThree == "object?") {
                if (objectExists(argOne)) {
                    if (argTwo == "==" || argTwo == "is")
                        successfulIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        failedIF();
                    }
                } else {
                    if (argTwo == "==" || argTwo == "is")
                        failedIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        failedIF();
                    }
                }
            }
            else if (argThree == "var?" || argThree == "variable?") {
                if (variableExists(argOne)) {
                    if (argTwo == "==" || argTwo == "is")
                        successfulIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        failedIF();
                    }
                } else {
                    if (argTwo == "==" || argTwo == "is")
                        failedIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        failedIF();
                    }
                }
            } else if (argThree == "method?") {
                if (methodExists(argOne)) {
                    if (argTwo == "==" || argTwo == "is")
                        successfulIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        failedIF();
                    }
                } else {
                    if (argTwo == "==" || argTwo == "is")
                        failedIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        failedIF();
                    }
                }
            } else if (argThree == "list?") {
                if (listExists(argOne)) {
                    if (argTwo == "==" || argTwo == "is")
                        successfulIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        failedIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        failedIF();
                    }
                } else {
                    if (argTwo == "==" || argTwo == "is")
                        failedIF();
                    else if (argTwo == "!=" || argTwo == "not")
                        successfulIF();
                    else {
                        error("invalid_operator:" + argTwo, false);
                        failedIF();
                    }
                }
            } else if (argTwo == "==" || argTwo == "is") {
				if (argOne == argThree)
					successfulIF();
				else
					failedIF();
			} else if (argTwo == "!=" || argTwo == "not") {
				if (argOne != argThree)
					successfulIF();
				else
					failedIF();
			} else if (argTwo == ">") {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) > stod(argThree))
						successfulIF();
					else
						failedIF();
				} else {
					if (argOne.length() > argThree.length())
						successfulIF();
					else
						failedIF();
				}
			} else if (argTwo == "<") {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) < stod(argThree))
						successfulIF();
					else
						failedIF();
				} else {
					if (argOne.length() < argThree.length())
						successfulIF();
					else
						failedIF();
				}
			} else if (argTwo == ">=") {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) >= stod(argThree))
						successfulIF();
					else
						failedIF();
				} else {
				    error("invalid_operator:>=", false);
				    failedIF();
				}
			} else if (argTwo == "<=") {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) <= stod(argThree))
						successfulIF();
					else
						failedIF();
				} else {
					error("invalid_operator:<=", false);
                    failedIF();
				}
			} else if (argTwo == "begins_with") {
				if (startsWith(argOne, argThree))
					successfulIF();
				else
					failedIF();
			} else if (argTwo == "ends_with") {
				if (endsWith(argOne, argThree))
					successfulIF();
				else
					failedIF();
			} else if (argTwo == "contains") {
				if (contains(argOne, argThree))
					successfulIF();
				else
					failedIF();
			} else {
				error("invalid_operator:" + argTwo, false);
				failedIF();
			}
		}
	} else if (argZero == "for") {
		if (argTwo == "<") {
			if (variableExists(argOne) && variableExists(argThree)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum && variables.at(variableAt(argThree)).getNumber() != nullNum) {
					if (variables.at(variableAt(argOne)).getNumber() < variables.at(variableAt(argThree)).getNumber())
						successfulFor(variables.at(variableAt(argOne)).getNumber(), variables.at(variableAt(argThree)).getNumber(), "<");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (variableExists(argOne) && !variableExists(argThree)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum && isNumeric(argThree)) {
					if (variables.at(variableAt(argOne)).getNumber() < stod(argThree))
						successfulFor(variables.at(variableAt(argOne)).getNumber(), stod(argThree), "<");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (!variableExists(argOne) && variableExists(argThree)) {
				if (isNumeric(argOne) && variables.at(variableAt(argThree)).getNumber() != nullNum) {
					if (stod(argOne) < variables.at(variableAt(argThree)).getNumber())
						successfulFor(stod(argOne), variables.at(variableAt(argThree)).getNumber(), "<");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (!variableExists(argOne) && !variableExists(argThree)) {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) < stod(argThree))
						successfulFor(stod(argOne), stod(argThree), "<");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else {
				error("special_error(2)", false); // impossible operation
				failedFor();
			}
		} else if (argTwo == ">") {
			if (variableExists(argOne) && variableExists(argThree)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum && variables.at(variableAt(argThree)).getNumber() != nullNum) {
					if (variables.at(variableAt(argOne)).getNumber() > variables.at(variableAt(argThree)).getNumber())
						successfulFor(variables.at(variableAt(argOne)).getNumber(), variables.at(variableAt(argThree)).getNumber(), ">");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (variableExists(argOne) && !variableExists(argThree)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum && isNumeric(argThree)) {
					if (variables.at(variableAt(argOne)).getNumber() > stod(argThree))
						successfulFor(variables.at(variableAt(argOne)).getNumber(), stod(argThree), ">");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (!variableExists(argOne) && variableExists(argThree)) {
				if (isNumeric(argOne) && variables.at(variableAt(argThree)).getNumber() != nullNum) {
					if (stod(argOne) > variables.at(variableAt(argThree)).getNumber())
						successfulFor(stod(argOne), variables.at(variableAt(argThree)).getNumber(), ">");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (!variableExists(argOne) && !variableExists(argThree)) {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) > stod(argThree))
						successfulFor(stod(argOne), stod(argThree), ">");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else {
				error("special_error(3)", false); // impossible operation
				failedFor();
			}
		} else if (argTwo == "<=") {
			if (variableExists(argOne) && variableExists(argThree)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum && variables.at(variableAt(argThree)).getNumber() != nullNum) {
					if (variables.at(variableAt(argOne)).getNumber() <= variables.at(variableAt(argThree)).getNumber())
						successfulFor(variables.at(variableAt(argOne)).getNumber(), variables.at(variableAt(argThree)).getNumber(), "<=");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (variableExists(argOne) && !variableExists(argThree)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum && isNumeric(argThree)) {
					if (variables.at(variableAt(argOne)).getNumber() <= stod(argThree))
						successfulFor(variables.at(variableAt(argOne)).getNumber(), stod(argThree), "<=");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (!variableExists(argOne) && variableExists(argThree)) {
				if (isNumeric(argOne) && variables.at(variableAt(argThree)).getNumber() != nullNum) {
					if (stod(argOne) <= variables.at(variableAt(argThree)).getNumber())
						successfulFor(stod(argOne), variables.at(variableAt(argThree)).getNumber(), "<=");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (!variableExists(argOne) && !variableExists(argThree)) {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) <= stod(argThree))
						successfulFor(stod(argOne), stod(argThree), "<=");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else {
				error("special_error(2)", false); // impossible operation
				failedFor();
			}
		} else if (argTwo == ">=") {
			if (variableExists(argOne) && variableExists(argThree)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum && variables.at(variableAt(argThree)).getNumber() != nullNum) {
					if (variables.at(variableAt(argOne)).getNumber() >= variables.at(variableAt(argThree)).getNumber())
						successfulFor(variables.at(variableAt(argOne)).getNumber(), variables.at(variableAt(argThree)).getNumber(), ">=");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (variableExists(argOne) && !variableExists(argThree)) {
				if (variables.at(variableAt(argOne)).getNumber() != nullNum && isNumeric(argThree)) {
					if (variables.at(variableAt(argOne)).getNumber() >= stod(argThree))
						successfulFor(variables.at(variableAt(argOne)).getNumber(), stod(argThree), ">=");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (!variableExists(argOne) && variableExists(argThree)) {
				if (isNumeric(argOne) && variables.at(variableAt(argThree)).getNumber() != nullNum) {
					if (stod(argOne) >= variables.at(variableAt(argThree)).getNumber())
						successfulFor(stod(argOne), variables.at(variableAt(argThree)).getNumber(), ">=");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else if (!variableExists(argOne) && !variableExists(argThree)) {
				if (isNumeric(argOne) && isNumeric(argThree)) {
					if (stod(argOne) >= stod(argThree))
						successfulFor(stod(argOne), stod(argThree), ">=");
					else
						failedFor();
				} else {
					error("conversion_error:" + s, false);
					failedFor();
				}
			} else {
				error("special_error(3)", false); // impossible operation
				failedFor();
			}
		} else if (argTwo == "in") {
			if (argOne == "var") {
				string before(beforeDot(argThree)), after(afterDot(argThree));

				if (before == "args" && after == "size") {
					List newList;

					for (int i = 0; i < (int)args.size(); i++)
						newList.add(args.at(i));

					successfulFor(newList);
				} else if (objectExists(before) && (after == "get_methods" || after == "methods")) {
                    List newList;

                    vector<Method> objMethods = objects.at(objectAt(before)).getMethods();

                    for (int i = 0; i < (int)objMethods.size(); i++)
                        newList.add(objMethods.at(i).name());

                    successfulFor(newList);
				} else if (objectExists(before) && (after == "get_variables" || after == "variables")) {
                    List newList;

                    vector<Variable> objVars = objects.at(objectAt(before)).getVariables();

                    for (int i = 0; i < (int)objVars.size(); i++)
                        newList.add(objVars.at(i).name());

                    successfulFor(newList);
				} else if (variableExists(before) && (after == "size" || after == "length")) {
					if (variables.at(variableAt(before)).getString() != null) {
						List newList;
						string tempVarStr = variables.at(variableAt(before)).getString();
						int len = tempVarStr.length();

						for (int i = 0; i < len; i++) {
							string tempStr("");
							tempStr.push_back(tempVarStr[i]);
							newList.add(tempStr);
						}

						successfulFor(newList);
					}
				} else {
					if (before.length() != 0 && after.length() != 0) {
						if (variableExists(before)) {
							if (after == "read_dirs" || after == "directories") {
								if (directoryExists(variables.at(variableAt(before)).getString()))
									successfulFor(getDirectoryList(before, false));
								else {
									error("read_fail:" + variables.at(variableAt(before)).getString(), false);
									failedFor();
								}
							} else if (after == "read_files" || after == "files") {
								if (directoryExists(variables.at(variableAt(before)).getString()))
									successfulFor(getDirectoryList(before, true));
								else {
									error("read_fail:" + variables.at(variableAt(before)).getString(), false);
									failedFor();
								}
							} else if (after == "read") {
								if (fileExists(variables.at(variableAt(before)).getString())) {
									List newList;

									ifstream file(variables.at(variableAt(before)).getString().c_str());
									string line("");

									if (file.is_open()) {
										while (!file.eof()) {
											getline(file, line);
											newList.add(line);
										}

										file.close();

										successfulFor(newList);
									} else {
										error("read_fail:" + variables.at(variableAt(before)).getString(), false);
										failedFor();
									}
								}
							} else {
								error("invalid_operation:method_undefined:" + after, false);
								failedFor();
							}
						} else {
							error("invalid_operation:variable_undefined:" + before, false);
							failedFor();
						}
					} else {
						if (listExists(argThree))
							successfulFor(lists.at(listAt(argThree)));
						else {
							error("invalid_operation:list_undefined:" + argThree, false);
							failedFor();
						}
					}
				}
			} else if (containsParams(argThree)) {
			    vector<string> rangeSpecifiers;

			    rangeSpecifiers = getRange(argThree);

			    if (rangeSpecifiers.size() == 2) {
			        string firstRangeSpecifier(rangeSpecifiers.at(0)), lastRangeSpecifier(rangeSpecifiers.at(1));

                    if (variableExists(firstRangeSpecifier)) {
                        if (variables.at(variableAt(firstRangeSpecifier)).getNumber() != nullNum)
                            firstRangeSpecifier = dtos(variables.at(variableAt(firstRangeSpecifier)).getNumber());
                        else
                            failedFor();
                    }

                    if (variableExists(lastRangeSpecifier)) {
                        if (variables.at(variableAt(lastRangeSpecifier)).getNumber() != nullNum)
                            lastRangeSpecifier = dtos(variables.at(variableAt(lastRangeSpecifier)).getNumber());
                        else
                            failedFor();
                    }

			        if (isNumeric(firstRangeSpecifier) && isNumeric(lastRangeSpecifier)) {
			            loopSymbol = argOne;

			            int ifrs = stoi(firstRangeSpecifier), ilrs(stoi(lastRangeSpecifier));

			            if (ifrs < ilrs)
			                successfulFor(stod(firstRangeSpecifier), stod(lastRangeSpecifier), "<=");
			            else if (ifrs > ilrs)
                            successfulFor(stod(firstRangeSpecifier), stod(lastRangeSpecifier), ">=");
                        else
                            failedFor();
			        } else
                        failedFor();
			    }
			} else if (containsBrackets(argThree)) {
			    string before(beforeBrackets(argThree));

			    if (variableExists(before)) {
			        if (variables.at(variableAt(before)).getString() != null) {
			            string tempVarString(variables.at(variableAt(before)).getString());

			            vector<string> range = getBracketRange(argThree);

                        if (range.size() == 2) {
                            string rangeBegin(range.at(0)), rangeEnd(range.at(1));

                            if (rangeBegin.length() != 0 && rangeEnd.length() != 0) {
                                if (isNumeric(rangeBegin) && isNumeric(rangeEnd)) {
                                    if (stoi(rangeBegin) < stoi(rangeEnd)) {
                                        if ((int)tempVarString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                                            List newList("&l&i&s&t&");

                                            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++) {
                                                string tempString("");
                                                tempString.push_back(tempVarString[i]);
                                                newList.add(tempString);
                                            }

                                            loopSymbol = argOne;

                                            successfulFor(newList);

                                            lists = removeList(lists, "&l&i&s&t&");
                                        } else
                                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                    } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
                                        if ((int)tempVarString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) {
                                            List newList("&l&i&s&t&");

                                            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--) {
                                                string tempString("");
                                                tempString.push_back(tempVarString[i]);
                                                newList.add(tempString);
                                            }

                                            loopSymbol = argOne;

                                            successfulFor(newList);

                                            lists = removeList(lists, "&l&i&s&t&");
                                        } else
                                            error("invalid_operation:index_out_of_bounds:" + rangeBegin + ".." + rangeEnd, false);
                                    } else
                                        error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                                } else
                                    error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                            } else
                                error("invalid_operation:invalid_range:" + rangeBegin + ".." + rangeEnd, false);
                        } else
                            error("invalid_operation:invalid_range:" + argThree, false);
			        } else {
                        error("invalid_operation:null_string:" + before, false);
                        failedFor();
			        }
			    }
			} else if (listExists(argThree)) {
			    loopSymbol = argOne;

				successfulFor(lists.at(listAt(argThree)));
			} else if (!zeroDots(argThree)) {
			    string _b(beforeDot(argThree)), _a(afterDot(argThree));

				if (_b == "args" && _a == "size") {
					List newList;

                    loopSymbol = argOne;

					for (int i = 0; i < (int)args.size(); i++)
						newList.add(args.at(i));

					successfulFor(newList);
				} else if (_b == "env" && (_a == "get_members" || _a == "members")) {
				    List newList;

				    newList.add("cwd");
				    newList.add("usl");
				    newList.add("os");
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

                    loopSymbol = argOne;
				    successfulFor(newList);
				} else if (objectExists(_b) && (_a == "get_methods" || _a == "methods")) {
                    List newList;

                    vector<Method> objMethods = objects.at(objectAt(_b)).getMethods();

                    for (int i = 0; i < (int)objMethods.size(); i++)
                        newList.add(objMethods.at(i).name());

                    loopSymbol = argOne;
                    successfulFor(newList);
				} else if (objectExists(_b) && (_a == "get_variables" || _a == "variables")) {
                    List newList;

                    vector<Variable> objVars = objects.at(objectAt(_b)).getVariables();

                    for (int i = 0; i < (int)objVars.size(); i++)
                        newList.add(objVars.at(i).name());

                    loopSymbol = argOne;
                    successfulFor(newList);
				} else if (variableExists(_b) && (_a == "size" || _a == "length")) {
					if (variables.at(variableAt(_b)).getString() != null) {
                        loopSymbol = argOne;
						List newList;
						string _t = variables.at(variableAt(_b)).getString();
						int _l = _t.length();

						for (int i = 0; i < _l; i++) {
							string tmpStr("");
							tmpStr.push_back(_t[i]);
							newList.add(tmpStr);
						}

						successfulFor(newList);
					}
				} else {
					if (_b.length() != 0 && _a.length() != 0) {
						if (variableExists(_b)) {
							if (_a == "read_dirs" || _a == "directories") {
								if (directoryExists(variables.at(variableAt(_b)).getString())) {
                                    loopSymbol = argOne;
									successfulFor(getDirectoryList(_b, false));
								} else {
									error("read_fail:" + variables.at(variableAt(_b)).getString(), false);
									failedFor();
								}
							} else if (_a == "read_files" || _a == "files") {
								if (directoryExists(variables.at(variableAt(_b)).getString())) {
                                    loopSymbol = argOne;
									successfulFor(getDirectoryList(_b, true));
								} else {
									error("read_fail:" + variables.at(variableAt(_b)).getString(), false);
									failedFor();
								}
							} else if (_a == "read") {
								if (fileExists(variables.at(variableAt(_b)).getString())) {
									List newList;

									ifstream file(variables.at(variableAt(_b)).getString().c_str());
									string line("");

									if (file.is_open()) {
										while (!file.eof()) {
											getline(file, line);
											newList.add(line);
										}

										file.close();

                                        loopSymbol = argOne;
										successfulFor(newList);
									} else {
										error("read_fail:" + variables.at(variableAt(_b)).getString(), false);
										failedFor();
									}
								}
							} else {
								error("invalid_operation:method_undefined:" + _a, false);
								failedFor();
							}
						} else {
							error("invalid_operation:variable_undefined:" + _b, false);
							failedFor();
						}
					}
				}
			} else {
				error("invalid_operation:" + s, false);
				failedFor();
			}
		} else {
			error("invalid_operation:" + s, false);
			failedFor();
		}
	} else if (argZero == "while") {
	    if (variableExists(argOne) && variableExists(argThree)) {
	        if (variables.at(variableAt(argOne)).getNumber() != nullNum && variables.at(variableAt(argThree)).getNumber() != nullNum) {
	            if (argTwo == "<" || argTwo == "<=" || argTwo == ">=" || argTwo == ">" || argTwo == "==" || argTwo == "!=")
                    successfullWhile(argOne, argTwo, argThree);
                else {
                    error("invalid_operation:" + s, false);
                    failedWhile();
                }
	        } else {
	            error("conversion_error:null_number:" + argOne + argTwo + argThree, false);
	            failedWhile();
	        }
	    } else if (isNumeric(argThree) && variableExists(argOne)) {
	        if (variables.at(variableAt(argOne)).getNumber() != nullNum) {
	            if (argTwo == "<" || argTwo == "<=" || argTwo == ">=" || argTwo == ">" || argTwo == "==" || argTwo == "!=")
                    successfullWhile(argOne, argTwo, argThree);
                else {
                    error("invalid_operation:" + s, false);
                    failedWhile();
                }
	        } else {
	            error("conversion_error:null_number:" + argOne + argTwo + argThree, false);
	            failedWhile();
	        }
	    } else if (isNumeric(argOne) && isNumeric(argThree)) {
	        if (argTwo == "<" || argTwo == "<=" || argTwo == ">=" || argTwo == ">" || argTwo == "==" || argTwo == "!=")
                successfullWhile(argOne, argTwo, argThree);
            else {
                error("invalid_operation:" + s, false);
                failedWhile();
            }
	    } else {
	        error("invalid_operation:" + s, false);
	        failedWhile();
	    }
	} else
		sysExec(s, command);
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

	if (t->tm_hour <= 11) {
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

	switch (t->tm_wday) {
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

	switch (t->tm_mon) {
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
	if (directoryExists(savedVarsPath)) {
		if (fileExists(savedVars))
			rm(savedVars);
		else
			cerr << "...no remembered variables" << endl;

		rd(savedVarsPath);

		if (!directoryExists(savedVarsPath) && !fileExists(savedVars))
			cout << "...removed successfully" << endl;
		else
			cerr << "...failed to remove" << endl;
	} else
		cerr << "...found nothing to remove" << endl;
}

double getBytes(string path)
{
    int bytes;

    ifstream file(path.c_str());

    if(!file.is_open()) {
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

string shomp(string text)
{
    char * s = getpass(cleanString(text).c_str());

    return (s);
}

#elif defined _WIN32 || defined _WIN64

string shomp(string text)
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
	uslBinary = app;
	initDir = cwd();
	logging = false;

	#ifdef _WIN32
	SetConsoleTitle("USL");
	#endif

    srand((unsigned int)time(NULL));

	if (c == 1) {
		currentScript = app;
		args.push_back(app);
		argsLength = (int)args.size();
		loop(false);
	} else if (c == 2) {
		string opt = v[1];

		if (isScript(opt)) {
		    currentScript = opt;
			args.push_back(opt);
			argsLength = (int)args.size();
			loadScript(opt);
		} else if (is(opt, "h") || is(opt, "help"))
			help(app);
		else if (is(opt, "u") || is(opt, "uninstall"))
			uninstall();
		else if (is(opt, "sl") || is(opt, "skipload")) {
			currentScript = app;
			args.push_back(opt);
			argsLength = (int)args.size();
			loop(true);
		} else if (is(opt, "n") || is(opt, "negligent")) {
			negligent = true;
			currentScript = app;
			args.push_back(opt);
			argsLength = (int)args.size();
			loop(true);
		} else if (is(opt, "v") || is(opt, "version"))
			displayVersion();
		else {
			currentScript = app;
			args.push_back(opt);
			argsLength = (int)args.size();
			loop(false);
		}
	} else if (c == 3) {
		string opt = v[1], script = v[2];

		if (is(opt, "sl") || is(opt, "skipload")) {
			currentScript = app;

			if (isScript(script)) {
				currentScript = script;
				args.push_back(opt);
				args.push_back(script);
				argsLength = (int)args.size();
				loadScript(script);
			} else {
				args.push_back(opt);
				args.push_back(script);
				argsLength = (int)args.size();
				loop(true);
			}
		} else if (is(opt, "n") || is(opt, "negligent")) {
			if (isScript(script)) {
				negligent = true;
				currentScript = script;
				args.push_back(opt);
				args.push_back(script);
				argsLength = (int)args.size();
				loadScript(script);
			} else {
				negligent = true;
				currentScript = app;
				args.push_back(opt);
				args.push_back(script);
				argsLength = (int)args.size();
				loop(true);
			}
		} else if (is(opt, "l") || is(opt, "log")) {
            logFile = script;

            if (fileExists(logFile)) {
                logging = true;
                loop(false);
            } else {
                touch(logFile);
                logging = true;
                loop(false);
            }
        } else if (is(opt, "p") || is(opt, "parse")) {
            string stringBuilder("");

            for (int i = 0; i < (int)script.length(); i++) {
                if (script[i] == '\'')
                    stringBuilder.push_back('\"');
                else
                    stringBuilder.push_back(script[i]);
            }

            parse(stringBuilder);
        } else {
			if (isScript(opt)) {
				currentScript = opt;
				args.push_back(opt);
				args.push_back(script);
				argsLength = (int)args.size();
				loadScript(opt);
			} else {
				currentScript = app;
				args.push_back(opt);
				args.push_back(script);
				argsLength = (int)args.size();
				loop(false);
			}
		}
	} else if (c > 3) {
		string opt = v[1];

		if (isScript(opt)) {
			for (int i = 2; i < c; i++) {
				string tmpStr = v[i];
				args.push_back(tmpStr);
			}

			argsLength = (int)args.size();

			loadScript(opt);
		} else {
			for (int i = 1; i < c; i++) {
				string tmpStr = v[i];
				args.push_back(tmpStr);
			}

			argsLength = (int)args.size();

			currentScript = app;
			loop(false);
		}
	} else
		help(app);

	clearAll();

	return (0);
}
