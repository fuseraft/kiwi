/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#ifndef PARSER_H
#define PARSER_H

string getParsedOutput(string cmd);
void parse(string s);
void zeroSpace(string arg0, string s, vector<string> command);
void oneSpace(string arg0, string arg1, string s, vector<string> command);
void twoSpace(string arg0, string arg1, string arg2, string s, vector<string> command);
void threeSpace(string arg0, string arg1, string arg2, string arg3, string s, vector<string> command);

string getParsedOutput(string cmd)
{
    State.CaptureParse = true;
    parse(cmd);
    string ret = State.ParsedOutput;
    State.ParsedOutput.clear();
    State.CaptureParse = false;
	
    return ret.length() == 0 ? State.LastValue : ret;
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

    State.CurrentLine = s; // store a copy of the current line
    // if (__Logging) app(State.LogFile, s + "\r\n"); // if __Logging a session, log the line

    command.push_back(""); // push back an empty string to begin.
    // iterate each char in the initial string
    for (int i = 0; i < length; i++)
    {
        switch (s[i])
        {
        case ' ':
            if (!State.IsCommented)
            {
                if ((!parenthesis && quoted) || (parenthesis && quoted))
                {
                    command.at(count).push_back(' ');
                }
                else if (parenthesis && !quoted)
                {
                    doNothing();
                }
                else
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
            quoted = !quoted;
            if (parenthesis)
            {
                command.at(count).push_back('\"');
            }
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
                if (!State.IsCommented)
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
            else if (prevChar == '#' && State.IsMultilineComment == false)
            {
                State.IsMultilineComment = true;
                State.IsCommented = true;
                uncomment = false;
            }
            else if (prevChar == '#' && State.IsMultilineComment == true)
                uncomment = true;
            else if (prevChar != '#' && State.IsMultilineComment == false)
            {
                State.IsCommented = true;
                uncomment = true;
            }

            bigString.push_back('#');
            break;

        case '~':
            if (!State.IsCommented)
            {
                if (prevChar == '\\')
                    command.at(count).push_back('~');
                else
                {
                    if (NoctisEnv.GuessedOS == OS_NIX)
                        command.at(count).append(Env::getEnvironmentVariable("HOME"));
                    else
                        command.at(count).append(Env::getEnvironmentVariable("HOMEPATH"));
                }
            }
            bigString.push_back('~');
            break;

        case ';':
            if (!quoted)
            {
                if (!State.IsCommented)
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
            if (!State.IsCommented)
                command.at(count).push_back(s[i]);
            bigString.push_back(s[i]);
            break;
        }

        prevChar = s[i];
    }

    // for (unsigned int x = 0; x < command.size(); x++) {
    // cout << x << ":\t__ " << command.at(x) << " __" << endl;
    // }

    size = (int)command.size();

    if (command.at(size - 1) == "{" && size != 1)
        command.pop_back();

    size = (int)command.size();

    if (!State.IsCommented)
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
                                command.at(i) = State.CurrentScript;
                            else
                                command.at(i) = args.at(stoi(params.at(0)));
                        }
                        else
                            error(ErrorMessage::OUT_OF_BOUNDS, command.at(i), false);
                    }
                    else
                        error(ErrorMessage::OUT_OF_BOUNDS, command.at(i), false);
                }
            }

            if (State.DefiningSwitchBlock)
            {
                if (s == "{")
                    doNothing();
                else if (startsWith(s, "case"))
                    mainSwitch.addCase(command.at(1));
                else if (s == "default")
                    State.InDefaultCase = true;
                else if (s == "end" || s == "}")
                {
                    string switch_value("");

                    if (isString(State.SwitchVarName))
                        switch_value = variables.at(indexOfVariable(State.SwitchVarName)).getString();
                    else if (isNumber(State.SwitchVarName))
                        switch_value = dtos(variables.at(indexOfVariable(State.SwitchVarName)).getNumber());
                    else
                        switch_value = "";

                    Container rightCase = mainSwitch.rightCase(switch_value);

                    State.InDefaultCase = false;
                    State.DefiningSwitchBlock = false;

                    for (int i = 0; i < (int)rightCase.size(); i++)
                        parse(rightCase.at(i));

                    mainSwitch.clear();
                }
                else
                {
                    if (State.InDefaultCase)
                        mainSwitch.addToDefault(s);
                    else
                        mainSwitch.addToCase(s);
                }
            }
            else if (State.DefiningModule)
            {
                if (s == ("[/" + State.CurrentModule + "]"))
                {
                    State.DefiningModule = false;
                    State.CurrentModule = "";
                }
                else
                    modules.at(indexOfModule(State.CurrentModule)).add(s);
            }
            else if (State.DefiningScript)
            {
                if (s == "__end__")
                {
                    State.CurrentScriptName = "";
                    State.DefiningScript = false;
                }
                else
                    Env::app(State.CurrentScriptName, s + "\n");
            }
            else
            {
                if (State.RaiseCatchBlock)
                {
                    if (s == "catch")
                        State.RaiseCatchBlock = false;
                }
                else if (State.ExecutedTryBlock && s == "catch")
                    State.SkipCatchBlock = true;
                else if (State.ExecutedTryBlock && State.SkipCatchBlock)
                {
                    if (s == "caught")
                    {
                        State.SkipCatchBlock = false;
                        parse("caught");
                    }
                }
                else if (State.DefiningMethod)
                {
                    if (contains(s, "while"))
                        State.DefiningLocalWhileLoop = true;

                    if (contains(s, "switch"))
                        State.DefiningLocalSwitchBlock = true;

                    if (State.DefiningParameterizedMethod)
                    {
                        if (s == "{")
                            doNothing();
                        else if (s == "end" || s == "}")
                        {
                            if (State.DefiningLocalWhileLoop)
                            {
                                State.DefiningLocalWhileLoop = false;

                                if (State.DefiningObject)
                                    objects.at(indexOfObject(State.CurrentObject)).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                            else if (State.DefiningLocalSwitchBlock)
                            {
                                State.DefiningLocalSwitchBlock = false;

                                if (State.DefiningObject)
                                    objects.at(indexOfObject(State.CurrentObject)).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                            else
                            {
                                State.DefiningMethod = false;

                                if (State.DefiningObject)
                                {
                                    State.DefiningObjectMethod = false;
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

                            if (State.DefiningObject)
                            {
                                objects.at(indexOfObject(State.CurrentObject)).addToCurrentMethod(freshLine);

                                if (State.DefiningPublicCode)
                                    objects.at(indexOfObject(State.CurrentObject)).setPublic();
                                else if (State.DefiningPrivateCode)
                                    objects.at(indexOfObject(State.CurrentObject)).setPrivate();
                                else
                                    objects.at(indexOfObject(State.CurrentObject)).setPublic();
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
                            if (State.DefiningLocalWhileLoop)
                            {
                                State.DefiningLocalWhileLoop = false;

                                if (State.DefiningObject)
                                    objects.at(objects.size() - 1).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                            else if (State.DefiningLocalSwitchBlock)
                            {
                                State.DefiningLocalSwitchBlock = false;

                                if (State.DefiningObject)
                                    objects.at(objects.size() - 1).addToCurrentMethod(s);
                                else
                                    methods.at(methods.size() - 1).add(s);
                            }
                            else
                            {
                                State.DefiningMethod = false;

                                if (State.DefiningObject)
                                {
                                    State.DefiningObjectMethod = false;
                                    objects.at(objects.size() - 1).setCurrentMethod("");
                                }
                            }
                        }
                        else
                        {
                            if (State.DefiningObject)
                            {
                                objects.at(objects.size() - 1).addToCurrentMethod(s);

                                if (State.DefiningPublicCode)
                                    objects.at(objects.size() - 1).setPublic();
                                else if (State.DefiningPrivateCode)
                                    objects.at(objects.size() - 1).setPrivate();
                                else
                                    objects.at(objects.size() - 1).setPublic();
                            }
                            else
                            {
                                if (State.DefiningObjectMethod)
                                {
                                    objects.at(objects.size() - 1).addToCurrentMethod(s);

                                    if (State.DefiningPublicCode)
                                        objects.at(objects.size() - 1).setPublic();
                                    else if (State.DefiningPrivateCode)
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
                else if (State.DefiningIfStatement)
                {
                    if (State.DefiningNest)
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
                            State.DefiningNest = true;

							if (size == 4)
                                threeSpace("if", command.at(1), command.at(2), command.at(3), s, command);
                            else
                            {
                                setFalseIf();
                                State.DefiningNest = false;
                            }
                        }
                        else if (command.at(0) == "endif")
                        {
                            State.DefiningIfStatement = false;
                            State.ExecutedIfStatement = true;

                            for (int i = 0; i < (int)ifStatements.size(); i++)
                            {
                                if (ifStatements.at(i).isIF())
                                {
                                    executeMethod(ifStatements.at(i));

                                    if (State.FailedIfStatement == false)
                                        break;
                                }
                            }

                            State.ExecutedIfStatement = false;

                            ifStatements.clear();

                            State.IfStatementCount = 0;
                            State.FailedIfStatement = false;
                        }
                        else if (command.at(0) == "elsif" || command.at(0) == "elif")
                        {
                            if (size == 4)
                                threeSpace("if", command.at(1), command.at(2), command.at(3), s, command);
                            else
                                setFalseIf();
                        }
                        else if (s == "else")
                            threeSpace("if", "true", "==", "true", "if true == true", command);
                        else if (s == "failif")
                        {
                            if (State.FailedIfStatement == true)
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                            ifStatements.at((int)ifStatements.size() - 1).add(s);
                    }
                }
                else
                {
                    if (State.DefiningWhileLoop)
                    {
                        if (s == "{")
                            doNothing();
                        else if (command.at(0) == "end" || command.at(0) == "}")
                        {
                            State.DefiningWhileLoop = false;

                            string v1 = whileLoops.at(whileLoops.size() - 1).valueOne(),
                                   v2 = whileLoops.at(whileLoops.size() - 1).valueTwo(),
                                   op = whileLoops.at(whileLoops.size() - 1).logicOperator();

                            if (variableExists(v1) && variableExists(v2))
                            {
                                if (op == "==")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() == variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "<")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() < variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == ">")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() > variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "<=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() <= variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == ">=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() >= variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "!=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() != variables.at(indexOfVariable(v2)).getNumber())
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                            }
                            else if (variableExists(v1))
                            {
                                if (op == "==")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() == stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "<")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() < stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == ">")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() > stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "<=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() <= stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == ">=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() >= stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "!=")
                                {
                                    while (variables.at(indexOfVariable(v1)).getNumber() != stoi(v2))
                                    {
                                        whileLoop(whileLoops.at(whileLoops.size() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    whileLoops.clear();

                                    State.WhileLoopCount = 0;
                                }
                            }
                        }
                        else
                            whileLoops.at(whileLoops.size() - 1).add(s);
                    }
                    else if (State.DefiningForLoop)
                    {
                        if (command.at(0) == "next" || command.at(0) == "endfor")
                        {
                            State.DefiningForLoop = false;

                            for (int i = 0; i < (int)forLoops.size(); i++)
                                if (forLoops.at(i).isForLoop())
                                    forLoop(forLoops.at(i));

                            forLoops.clear();

                            State.ForLoopCount = 0;
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
                                                Env::sysExec(s, command);
                                        }
                                        else if (objects.at(indexOfObject(before)).methodExists(after))
                                            executeMethod(objects.at(indexOfObject(before)).getMethod(after));
                                        else if (objects.at(indexOfObject(before)).variableExists(after))
                                        {
                                            if (objects.at(indexOfObject(before)).getVariable(after).getString() != State.Null)
                                                writeline(objects.at(indexOfObject(before)).getVariable(after).getString());
                                            else if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != State.NullNum)
                                                writeline(dtos(objects.at(indexOfObject(before)).getVariable(after).getNumber()));
                                            else
                                                error(ErrorMessage::IS_NULL, "", false);
                                        }
                                        else if (after == "clear")
                                            objects.at(indexOfObject(before)).clear();
                                        else
                                            error(ErrorMessage::UNDEFINED, "", false);
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
                                                parse(before + " = State.Null");
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
                                            if (State.ExecutedMethod)
                                                executeMethod(objects.at(indexOfObject(State.CurrentMethodObject)).getMethod(after));
                                        }
                                        else
                                            Env::sysExec(s, command);
                                    }
                                }
                                else if (endsWith(s, "::"))
                                {
                                    if (State.CurrentScript != "")
                                    {
                                        string newMark(s);
                                        newMark = subtractString(s, "::");
                                        scripts.at(indexOfScript(State.CurrentScript)).addMark(newMark);
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
                                        Env::sysExec(s, command);
                                }
                            }
                            else
                                zeroSpace(command.at(0), s, command);
                        }
                        else if (size == 2)
                        {
                            if (notStandardOneSpace(command.at(0)))
                                Env::sysExec(s, command);
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
                                        State.DefaultLoopSymbol = command.at(2);
                                        State.DefaultLoopSymbol = subtractChar(State.DefaultLoopSymbol, "(");
                                        State.DefaultLoopSymbol = subtractChar(State.DefaultLoopSymbol, ")");

                                        oneSpace(command.at(0), command.at(1), subtractString(s, command.at(2)), command);
                                        State.DefaultLoopSymbol = "$";
                                    }
                                    else
                                        Env::sysExec(s, command);
                                }
                                else
                                    Env::sysExec(s, command);
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
                                    State.DefaultLoopSymbol = command.at(4);
                                    State.DefaultLoopSymbol = subtractChar(State.DefaultLoopSymbol, "(");
                                    State.DefaultLoopSymbol = subtractChar(State.DefaultLoopSymbol, ")");

                                    threeSpace(command.at(0), command.at(1), command.at(2), command.at(3), subtractString(s, command.at(4)), command);
                                    State.DefaultLoopSymbol = "$";
                                }
                                else
                                    Env::sysExec(s, command);
                            }
                            else
                                Env::sysExec(s, command);
                        }
                        else
                            Env::sysExec(s, command);
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
        if (State.IsMultilineComment)
        {
            if (uncomment)
            {
                State.IsCommented = false;
                State.IsMultilineComment = false;
            }
        }
        else
        {
            if (uncomment)
            {
                State.IsCommented = false;
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
                    parse(trimLeadingWhitespace(c));
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

                    stringContainer.add(trimLeadingWhitespace(c));

                    delete[] c;

                    for (int i = 0; i < (int)stringContainer.get().size(); i++)
                        parse(stringContainer.at(i));
                }
            }
        }
    }
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
        to_remove.append(State.ErrorVarName);

        parse(to_remove);

        State.ExecutedTryBlock = false,
        State.RaiseCatchBlock = false;
        State.LastError = "";
        State.ErrorVarName = "";
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
    else if (arg0 == "exit")
    {
        clearAll();
        exit(0);
    }
    else if (arg0 == "break" || arg0 == "leave!")
        State.Breaking = true;
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
        State.DefiningPrivateCode = false,
        State.DefiningPublicCode = false;
        State.DefiningObject = false;
        State.DefiningObjectMethod = false;
        State.CurrentObject = "";
    }
    else if (arg0 == "parser")
        loop(false);
    else if (arg0 == "private")
    {
        State.DefiningPrivateCode = true;
        State.DefiningPublicCode = false;
    }
    else if (arg0 == "public")
    {
        State.DefiningPrivateCode = false;
        State.DefiningPublicCode = true;
    }
    else if (arg0 == "try")
        State.ExecutedTryBlock = true;
    else if (arg0 == "failif")
    {
        if (State.FailedIfStatement == true)
            setTrueIf();
        else
            setFalseIf();
    }
    else
        Env::sysExec(s, command);
}

void oneSpace(string arg0, string arg1, string s, vector<string> command)
{
    string before(beforeDot(arg1)), after(afterDot(arg1));

    if (contains(arg1, "self."))
    {
        arg1 = replace(arg1, "self", State.CurrentMethodObject);
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
            State.DefiningSwitchBlock = true;
            State.SwitchVarName = arg1;
        }
        else
            error(ErrorMessage::VAR_UNDEFINED, arg1, false);
    }
    else if (arg0 == "goto")
    {
        if (State.CurrentScript != "")
        {
            if (scripts.at(indexOfScript(State.CurrentScript)).markExists(arg1))
            {
                State.GoTo = arg1;
                State.GoToLabel = true;
            }
        }
    }
	else if (arg0 == "if") {
		string tmpValue("");
		// if arg1 is a variable
		if (variableExists(arg1)) {
			// can we can assume that arg1 belongs to an object?
			if (!zeroDots(arg1)) {
				string objName(beforeDot(arg1)), varName(afterDot(arg1));
				Variable tmpVar = getObject(objName).getVariable(varName);
				
				if (isString(tmpVar)) {
					tmpValue = tmpVar.getString();
				} else if (isNumber(tmpVar)) {
					tmpValue = dtos(tmpVar.getNumber());
				} else {
					// error(ErrorMessage::IS_NULL, arg1, true);
				}
			} else {
				if (isString(arg1)) {
					tmpValue = getVariable(arg1).getString();
				} else if (isNumber(arg1)) {
					tmpValue = getVariable(arg1).getNumber();
				} else {
					// error(ErrorMessage::IS_NULL, arg1, true);
				}
			}
		} else {
			if (isNumeric(arg1) || isTrue(arg1) || isFalse(arg1)) {
				tmpValue = arg1;
			} else {
				string tmpCode("");
				
				if (startsWith(arg1, "(\"") && endsWith(arg1, "\")")) {
					tmpCode = getInner(arg1, 2, arg1.length() - 3);
				} else {
					tmpCode = arg1;
				}
				tmpValue = getParsedOutput(tmpCode);
			}
		}
		
		if (isTrue(tmpValue)) {
			setTrueIf();
		} else if (isFalse(tmpValue)) {
			setFalseIf();
		} else {
			// error(ErrorMessage::INVALID_OP, arg1, true);
		}
	}
    else if (arg0 == "prompt")
    {
        if (arg1 == "bash")
        {
            State.UseCustomPrompt = true;
            State.PromptStyle = "bash";
        }
        else if (arg1 == "!")
        {
            if (State.UseCustomPrompt == true)
                State.UseCustomPrompt = false;
            else
                State.UseCustomPrompt = true;
        }
        else if (arg1 == "empty")
        {
            State.UseCustomPrompt = true;
            State.PromptStyle = "empty";
        }
        else
        {
            State.UseCustomPrompt = true;
            State.PromptStyle = arg1;
        }
    }
    else if (arg0 == "err" || arg0 == "error")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
                IO::printerrln(variables.at(indexOfVariable(arg1)).getString());
            else if (isNumber(arg1))
                IO::printerrln(dtos(variables.at(indexOfVariable(arg1)).getNumber()));
            else
                error(ErrorMessage::IS_NULL, arg1, false);
        }
        else
            IO::printerrln(arg1);
    }
    else if (arg0 == "delay")
    {
        if (isNumeric(arg1))
            delay(stoi(arg1));
        else
            error(ErrorMessage::CONV_ERR, arg1, false);
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
                    error(ErrorMessage::TARGET_UNDEFINED, params.at(i), false);
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
            error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
    }
    else if (arg0 == "see_string")
    {
        if (variableExists(arg1))
            write(variables.at(indexOfVariable(arg1)).getString());
        else
            error(ErrorMessage::VAR_UNDEFINED, arg1, false);
    }
    else if (arg0 == "see_number")
    {
        if (variableExists(arg1))
            write(dtos(variables.at(indexOfVariable(arg1)).getNumber()));
        else
            error(ErrorMessage::VAR_UNDEFINED, arg1, false);
    }
    else if (arg0 == "__begin__")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (!Env::fileExists(variables.at(indexOfVariable(arg1)).getString()))
                {
                    Env::createFile(variables.at(indexOfVariable(arg1)).getString());
                    State.DefiningScript = true;
                    State.CurrentScriptName = variables.at(indexOfVariable(arg1)).getString();
                }
                else
                    error(ErrorMessage::FILE_EXISTS, variables.at(indexOfVariable(arg1)).getString(), false);
            }
        }
        else if (!Env::fileExists(arg1))
        {
            Env::createFile(arg1);
            State.DefiningScript = true;
            State.CurrentScriptName = arg1;
        }
        else
            error(ErrorMessage::FILE_EXISTS, arg1, false);
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
        if (Env::fileExists(arg1))
        {
            if (isScript(arg1))
            {
                State.PreviousScript = State.CurrentScript;
                loadScript(arg1);
            }
            else
                error(ErrorMessage::BAD_LOAD, arg1, true);
        }
        else if (moduleExists(arg1))
        {
            vector<string> lines = modules.at(indexOfModule(arg1)).get();

            for (int i = 0; i < (int)lines.size(); i++)
                parse(lines.at(i));
        }
        else
            error(ErrorMessage::BAD_LOAD, arg1, true);
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
                if (Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                    Env::cd(variables.at(indexOfVariable(arg1)).getString());
                else
                    error(ErrorMessage::READ_FAIL, variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error(ErrorMessage::NULL_STRING, arg1, false);
        }
        else
        {
            if (arg1 == "init_dir" || arg1 == "initial_directory")
                Env::cd(NoctisEnv.InitialDirectory);
            else if (Env::directoryExists(arg1))
                Env::cd(arg1);
            else
                Env::cd(arg1);
        }
    }
    else if (arg0 == "list")
    {
        if (listExists(arg1))
            lists.at(indexOfList(arg1)).clear();
        else
        {
            List newList(arg1);

            if (State.ExecutedTemplate || State.ExecutedMethod)
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
                error(ErrorMessage::IS_NULL, arg1, false);
        }
        else
            parse(arg1.c_str());
    }
    else if (arg0 == "?")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
                Env::sysExec(variables.at(indexOfVariable(arg1)).getString(), command);
            else
                error(ErrorMessage::IS_NULL, arg1, false);
        }
        else
            Env::sysExec(arg1, command);
    }
    else if (arg0 == "init_dir" || arg0 == "initial_directory")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                {
                    NoctisEnv.InitialDirectory = variables.at(indexOfVariable(arg1)).getString();
                    Env::cd(NoctisEnv.InitialDirectory);
                }
                else
                    error(ErrorMessage::READ_FAIL, NoctisEnv.InitialDirectory, false);
            }
            else
                error(ErrorMessage::NULL_STRING, arg1, false);
        }
        else
        {
            if (Env::directoryExists(arg1))
            {
                if (arg1 == ".")
                    NoctisEnv.InitialDirectory = Env::cwd();
                else if (arg1 == "..")
                    NoctisEnv.InitialDirectory = Env::cwd() + "\\..";
                else
                    NoctisEnv.InitialDirectory = arg1;

                Env::cd(NoctisEnv.InitialDirectory);
            }
            else
                error(ErrorMessage::READ_FAIL, NoctisEnv.InitialDirectory, false);
        }
    }
    else if (arg0 == "method?")
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
    else if (arg0 == "object?")
    {
        if (objectExists(arg1))
            __true();
        else
            __false();
    }
    else if (arg0 == "variable?")
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
    else if (arg0 == "list?")
    {
        if (listExists(arg1))
            __true();
        else
            __false();
    }
    else if (arg0 == "directory?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (Env::directoryExists(objects.at(indexOfObject(before)).getVariable(after).getString()))
                    __true();
                else
                    __false();
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
        }
        else
        {
            if (variableExists(arg1))
            {
                if (isString(arg1))
                {
                    if (Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        __true();
                    else
                        __false();
                }
                else
                    error(ErrorMessage::NULL_STRING, arg1, false);
            }
            else
            {
                if (Env::directoryExists(arg1))
                    __true();
                else
                    __false();
            }
        }
    }
    else if (arg0 == "file?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (Env::fileExists(objects.at(indexOfObject(before)).getVariable(after).getString()))
                    __true();
                else
                    __false();
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
        }
        else
        {
            if (variableExists(arg1))
            {
                if (isString(arg1))
                {
                    if (Env::fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        __true();
                    else
                        __false();
                }
                else
                    __false();
            }
            else
            {
                if (Env::fileExists(arg1))
                    __true();
                else
                    __false();
            }
        }
    }
    else if (arg0 == "collect?")
    {
        if (variableExists(arg1))
        {
            if (variables.at(indexOfVariable(arg1)).garbage())
                __true();
            else
                __false();
        }
        else
            IO::println("under construction...");
    }
    else if (arg0 == "number?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (objects.at(indexOfObject(before)).getVariable(after).getNumber() != State.NullNum)
                    __true();
                else
                    __false();
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
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
    else if (arg0 == "string?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (objects.at(indexOfObject(before)).variableExists(after))
            {
                if (objects.at(indexOfObject(before)).getVariable(after).getString() != State.Null)
                    __true();
                else
                    __false();
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
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
    else if (arg0 == "uppercase?")
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
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
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
    else if (arg0 == "lowercase?")
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
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
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
            error(ErrorMessage::METHOD_DEFINED, arg1, false);
        else
        {
            if (containsParams(arg1))
            {
                vector<string> params = getParams(arg1);
                Method method(beforeParams(arg1), true);

                method.setTemplateSize((int)params.size());

                methods.push_back(method);

                State.DefiningMethod = true;
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
                if (!Env::fileExists(variables.at(indexOfVariable(arg1)).getString()))
                    Env::createFile(variables.at(indexOfVariable(arg1)).getString());
                else
                    error(ErrorMessage::FILE_EXISTS, variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error(ErrorMessage::NULL_STRING, arg1, false);
        }
        else
        {
            if (!Env::fileExists(arg1))
                Env::createFile(arg1);
            else
                error(ErrorMessage::FILE_EXISTS, arg1, false);
        }
    }
    else if (arg0 == "fpop")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (Env::fileExists(variables.at(indexOfVariable(arg1)).getString()))
                    Env::rm(variables.at(indexOfVariable(arg1)).getString());
                else
                    error(ErrorMessage::FILE_NOT_FOUND, variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error(ErrorMessage::NULL_STRING, arg1, false);
        }
        else
        {
            if (Env::fileExists(arg1))
                Env::rm(arg1);
            else
                error(ErrorMessage::FILE_NOT_FOUND, arg1, false);
        }
    }
    else if (arg0 == "dpush")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (!Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                    Env::md(variables.at(indexOfVariable(arg1)).getString());
                else
                    error(ErrorMessage::DIR_EXISTS, variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error(ErrorMessage::NULL_STRING, arg1, false);
        }
        else
        {
            if (!Env::directoryExists(arg1))
                Env::md(arg1);
            else
                error(ErrorMessage::DIR_EXISTS, arg1, false);
        }
    }
    else if (arg0 == "dpop")
    {
        if (variableExists(arg1))
        {
            if (isString(arg1))
            {
                if (Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                    Env::rd(variables.at(indexOfVariable(arg1)).getString());
                else
                    error(ErrorMessage::DIR_NOT_FOUND, variables.at(indexOfVariable(arg1)).getString(), false);
            }
            else
                error(ErrorMessage::NULL_STRING, arg1, false);
        }
        else
        {
            if (Env::directoryExists(arg1))
                Env::rd(arg1);
            else
                error(ErrorMessage::DIR_NOT_FOUND, arg1, false);
        }
    }
    else
        Env::sysExec(s, command);
}

void twoSpace(string arg0, string arg1, string arg2, string s, vector<string> command)
{
    string last_val = "";

    if (contains(arg2, "self."))
        arg2 = replace(arg2, "self", State.CurrentMethodObject);

    if (contains(arg0, "self."))
        arg0 = replace(arg0, "self", State.CurrentMethodObject);

    if (variableExists(arg0))
    {
        initializeVariable(arg0, arg1, arg2, s, command);
    }
    else if (listExists(arg0) || listExists(beforeBrackets(arg0)))
    {
        initializeListValues(arg0, arg1, arg2, s, command);
    }
    else
    {
        if (startsWith(arg0, "@") && zeroDots(arg0)) 
        {
            createGlobalVariable(arg0, arg1, arg2, s, command);
        }
        else if (startsWith(arg0, "@") && !zeroDots(arg2)) 
        {
            createObjectVariable(arg0, arg1, arg2, s, command);
        }
        else if (!objectExists(arg0) && objectExists(arg2))
        {
            copyObject(arg0, arg1, arg2, s, command);
        }
        else if (isUpperConstant(arg0))
        {
            createConstant(arg0, arg1, arg2, s, command);
        }
        else
        {
            executeSimpleStatement(arg0, arg1, arg2, s, command);
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
            State.DefiningObject = true;
            State.CurrentObject = arg1;
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
                    State.CurrentObject = arg1;
                    State.DefiningObject = true;

                    newObject.clear();
                    objectMethods.clear();
                }
                else
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
            }
            else
                error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg3, false);
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
                        error(ErrorMessage::IS_NULL, arg1, false);
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
                            setFalseIf();
                            State.LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        setTrueIf();
                }
                else
                    setTrueIf();
            }
        }
        else if (variableExists(arg1) && variableExists(arg3))
        {
            if (isString(arg1) && isString(arg3))
            {
                if (arg2 == "==")
                {
                    if (variables.at(indexOfVariable(arg1)).getString() == variables.at(indexOfVariable(arg3)).getString())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "!=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString() != variables.at(indexOfVariable(arg3)).getString())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() > variables.at(indexOfVariable(arg3)).getString().length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() < variables.at(indexOfVariable(arg3)).getString().length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() <= variables.at(indexOfVariable(arg3)).getString().length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() >= variables.at(indexOfVariable(arg3)).getString().length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "contains")
                {
                    if (contains(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "ends_with")
                {
                    if (endsWith(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "begins_with")
                {
                    if (startsWith(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setTrueIf();
                }
            }
            else if (isNumber(arg1) && isNumber(arg3))
            {
                if (arg2 == "==")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() == variables.at(indexOfVariable(arg3)).getNumber())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "!=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() != variables.at(indexOfVariable(arg3)).getNumber())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() > variables.at(indexOfVariable(arg3)).getNumber())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() >= variables.at(indexOfVariable(arg3)).getNumber())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() < variables.at(indexOfVariable(arg3)).getNumber())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() <= variables.at(indexOfVariable(arg3)).getNumber())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setTrueIf();
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, s, false);
                setTrueIf();
            }
        }
        else if ((variableExists(arg1) && !variableExists(arg3)) && !methodExists(arg3) && notObjectMethod(arg3) && !containsParams(arg3))
        {
            if (isNumber(arg1))
            {
                if (isNumeric(arg3))
                {
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() == stod(arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() != stod(arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() > stod(arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() < stod(arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
                else if (arg3 == "number?")
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    setTrueIf();
                }
            }
            else
            {
                if (arg3 == "string?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                            setFalseIf();
                        else if (arg2 == "!=")
                            setTrueIf();
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!")
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                }
                else if (arg3 == "number?")
                {
                    if (isNumber(arg1))
                    {
                        if (arg2 == "==")
                            setFalseIf();
                        else if (arg2 == "!=")
                            setTrueIf();
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                }
                else if (arg3 == "uppercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                            setTrueIf();
                    }
                }
                else if (arg3 == "lowercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                            setTrueIf();
                    }
                }
                else if (arg3 == "file?")
                {
                    if (isString(arg1))
                    {
                        if (Env::fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==")
                                setFalseIf();
                            else if (arg2 == "!=")
                                setTrueIf();
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        setTrueIf();
                    }
                }
                else if (arg3 == "directory?")
                {
                    if (isString(arg1))
                    {
                        if (Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==")
                                setFalseIf();
                            else if (arg2 == "!=")
                                setTrueIf();
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        setTrueIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() == arg3)
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() != arg3)
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() > arg3.length())
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() < arg3.length())
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() >= arg3.length())
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() <= arg3.length())
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
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
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() == stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() != stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() > stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() < stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
                else if (stackValue == "number?")
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    setTrueIf();
                }
            }
            else
            {
                if (stackValue == "string?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                            setFalseIf();
                        else if (arg2 == "!=")
                            setTrueIf();
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                }
                else if (stackValue == "number?")
                {
                    if (isNumber(arg1))
                    {
                        if (arg2 == "==")
                            setFalseIf();
                        else if (arg2 == "!=")
                            setTrueIf();
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                }
                else if (stackValue == "uppercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                            setTrueIf();
                    }
                }
                else if (stackValue == "lowercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                            setTrueIf();
                    }
                }
                else if (stackValue == "file?")
                {
                    if (isString(arg1))
                    {
                        if (Env::fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==")
                                setFalseIf();
                            else if (arg2 == "!=")
                                setTrueIf();
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        setTrueIf();
                    }
                }
                else if (stackValue == "directory?")
                {
                    if (isString(arg1))
                    {
                        if (Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==")
                                setFalseIf();
                            else if (arg2 == "!=")
                                setTrueIf();
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        setTrueIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() == stackValue)
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() != stackValue)
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() > stackValue.length())
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() < stackValue.length())
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() >= stackValue.length())
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() <= stackValue.length())
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
            }
        }
        else if ((!variableExists(arg1) && variableExists(arg3)) && !methodExists(arg1) && notObjectMethod(arg1) && !containsParams(arg1))
        {
            if (isNumber(arg3))
            {
                if (isNumeric(arg1))
                {
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() == stod(arg1))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() != stod(arg1))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() > stod(arg1))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() < stod(arg1))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() >= stod(arg1))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() <= stod(arg1))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    setTrueIf();
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() == arg1)
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "!=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() != arg1)
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() > arg1.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() < arg1.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() >= arg1.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() <= arg1.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setTrueIf();
                }
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
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() == stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() != stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() > stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() < stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() >= stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() <= stod(stackValue))
                            setFalseIf();
                        else
                            setTrueIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    setTrueIf();
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() == stackValue)
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "!=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() != stackValue)
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() > stackValue.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() < stackValue.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() >= stackValue.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() <= stackValue.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setTrueIf();
                }
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

                        arg1Result = State.LastValue;

                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                    }
                    else
                    {
                        if (!objectExists(arg1before))
                            error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg1before, false);

                        if (!objectExists(arg3before))
                            error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg3before, false);

                        setTrueIf();
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

                        arg1Result = State.LastValue;

                        if (methodExists(beforeParams(arg3)))
                            executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                        arg3Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg1before, false);
                        setTrueIf();
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

                        arg3Result = State.LastValue;

                        if (methodExists(beforeParams(arg1)))
                            executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                        arg1Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg3before, false);
                        setTrueIf();
                    }
                }
                else
                {
                    string arg1Result(""), arg3Result("");

                    if (methodExists(beforeParams(arg1)))
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                    arg1Result = State.LastValue;

                    if (methodExists(beforeParams(arg3)))
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                    arg3Result = State.LastValue;

                    if (isNumeric(arg1Result) && isNumeric(arg3Result))
                    {
                        if (arg2 == "==")
                        {
                            if (stod(arg1Result) == stod(arg3Result))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (stod(arg1Result) != stod(arg3Result))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == "<")
                        {
                            if (stod(arg1Result) < stod(arg3Result))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == ">")
                        {
                            if (stod(arg1Result) > stod(arg3Result))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == "<=")
                        {
                            if (stod(arg1Result) <= stod(arg3Result))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == ">=")
                        {
                            if (stod(arg1Result) >= stod(arg3Result))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "==")
                        {
                            if (arg1Result == arg3Result)
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (arg1Result != arg3Result)
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setTrueIf();
                        }
                    }
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

                        arg1Result = State.LastValue;

                        if (methodExists(arg3))
                        {
                            parse(arg3);
                            arg3Result = State.LastValue;
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
                                error(ErrorMessage::IS_NULL, arg3, false);
                                setTrueIf();
                            }
                        }
                        else
                            arg3Result = arg3;

                        if (pass)
                        {
                            if (isNumeric(arg1Result) && isNumeric(arg3Result))
                            {
                                if (arg2 == "==")
                                {
                                    if (stod(arg1Result) == stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setTrueIf();
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg1Result == arg3Result)
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg1Result != arg3Result)
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setTrueIf();
                                }
                            }
                        }
                        else
                            setTrueIf();
                    }
                    else
                    {
                        error(ErrorMessage::METHOD_UNDEFINED, beforeParams(arg1), false);
                        setTrueIf();
                    }
                }
                else
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    if (objectExists(arg1before))
                    {
                        if (objects.at(indexOfObject(arg1before)).methodExists(beforeParams(arg1after)))
                            executeTemplate(objects.at(indexOfObject(arg1before)).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = State.LastValue;

                        if (variableExists(arg3))
                        {
                            if (isString(arg3))
                                arg3Result = variables.at(indexOfVariable(arg3)).getString();
                            else if (isNumber(arg3))
                                arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                            else
                            {
                                pass = false;
                                error(ErrorMessage::IS_NULL, arg3, false);
                                setTrueIf();
                            }
                        }
                        else if (methodExists(arg3))
                        {
                            parse(arg3);

                            arg3Result = State.LastValue;
                        }
                        else
                            arg3Result = arg3;

                        if (pass)
                        {
                            if (isNumeric(arg1Result) && isNumeric(arg3Result))
                            {
                                if (arg2 == "==")
                                {
                                    if (stod(arg1Result) == stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setTrueIf();
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg1Result == arg3Result)
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg1Result != arg3Result)
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setTrueIf();
                                }
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg1before, false);
                        setTrueIf();
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

                        arg3Result = State.LastValue;

                        if (methodExists(arg1))
                        {
                            parse(arg1);
                            arg1Result = State.LastValue;
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
                                error(ErrorMessage::IS_NULL, arg1, false);
                                setTrueIf();
                            }
                        }
                        else
                            arg1Result = arg1;

                        if (pass)
                        {
                            if (isNumeric(arg3Result) && isNumeric(arg1Result))
                            {
                                if (arg2 == "==")
                                {
                                    if (stod(arg3Result) == stod(arg1Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg3Result) != stod(arg1Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg3Result) < stod(arg1Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg3Result) > stod(arg1Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg3Result) <= stod(arg1Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg3Result) >= stod(arg1Result))
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setTrueIf();
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg3Result == arg1Result)
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg3Result != arg1Result)
                                        setFalseIf();
                                    else
                                        setTrueIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setTrueIf();
                                }
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::METHOD_UNDEFINED, beforeParams(arg3), false);
                        setTrueIf();
                    }
                }
                else
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    if (objectExists(arg3before))
                    {
                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (variableExists(arg1))
                        {
                            if (isString(arg1))
                                arg1Result = variables.at(indexOfVariable(arg1)).getString();
                            else if (isNumber(arg3))
                                arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                            else
                            {
                                error(ErrorMessage::IS_NULL, arg1, false);
                                setTrueIf();
                            }
                        }
                        else if (methodExists(arg1))
                        {
                            parse(arg1);

                            arg1Result = State.LastValue;
                        }
                        else
                            arg1Result = arg1;

                        if (isNumeric(arg3Result) && isNumeric(arg1Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg3Result) == stod(arg1Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg3Result) != stod(arg1Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg3Result) < stod(arg1Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg3Result) > stod(arg1Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg3Result) <= stod(arg1Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg3Result) >= stod(arg1Result))
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg3Result == arg1Result)
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg3Result != arg1Result)
                                    setFalseIf();
                                else
                                    setTrueIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setTrueIf();
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg3before, false);
                        setTrueIf();
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
                arg1Result = State.LastValue;
            }
            else if (variableExists(arg1))
            {
                if (isString(arg1))
                    arg1Result = variables.at(indexOfVariable(arg1)).getString();
                else if (isNumber(arg1))
                    arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                else
                {
                    error(ErrorMessage::IS_NULL, arg1, false);
                    setTrueIf();
                }
            }
            else
                arg1Result = arg1;

            if (methodExists(arg3))
            {
                parse(arg3);
                arg3Result = State.LastValue;
            }
            else if (variableExists(arg3))
            {
                if (isString(arg3))
                    arg3Result = variables.at(indexOfVariable(arg3)).getString();
                else if (isNumber(arg3))
                    arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                else
                {
                    error(ErrorMessage::IS_NULL, arg3, false);
                    setTrueIf();
                }
            }
            else
                arg3Result = arg3;

            if (isNumeric(arg1Result) && isNumeric(arg3Result))
            {
                if (arg2 == "==")
                {
                    if (stod(arg1Result) == stod(arg3Result))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "!=")
                {
                    if (stod(arg1Result) != stod(arg3Result))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<")
                {
                    if (stod(arg1Result) < stod(arg3Result))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">")
                {
                    if (stod(arg1Result) > stod(arg3Result))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "<=")
                {
                    if (stod(arg1Result) <= stod(arg3Result))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == ">=")
                {
                    if (stod(arg1Result) >= stod(arg3Result))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setTrueIf();
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (arg1Result == arg3Result)
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else if (arg2 == "!=")
                {
                    if (arg1Result != arg3Result)
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setTrueIf();
                }
            }
        }
        else
        {
            if (arg3 == "object?")
            {
                if (objectExists(arg1))
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
            }
            else if (arg3 == "variable?")
            {
                if (variableExists(arg1))
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
                else
                {
                    if (arg2 == "=")
                        setTrueIf();
                    else if (arg2 == "!")
                        setFalseIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
            }
            else if (arg3 == "method?")
            {
                if (methodExists(arg1))
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
            }
            else if (arg3 == "list?")
            {
                if (listExists(arg1))
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setTrueIf();
                    }
                }
            }
            else if (arg2 == "==")
            {
                if (arg1 == arg3)
                    setFalseIf();
                else
                    setTrueIf();
            }
            else if (arg2 == "!=")
            {
                if (arg1 != arg3)
                    setFalseIf();
                else
                    setTrueIf();
            }
            else if (arg2 == ">")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) > stod(arg3))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    if (arg1.length() > arg3.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
            }
            else if (arg2 == "<")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) < stod(arg3))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    if (arg1.length() < arg3.length())
                        setFalseIf();
                    else
                        setTrueIf();
                }
            }
            else if (arg2 == ">=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) >= stod(arg3))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setTrueIf();
                }
            }
            else if (arg2 == "<=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) <= stod(arg3))
                        setFalseIf();
                    else
                        setTrueIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
            }
            else if (arg2 == "begins_with")
            {
                if (startsWith(arg1, arg3))
                    setFalseIf();
                else
                    setTrueIf();
            }
            else if (arg2 == "ends_with")
            {
                if (endsWith(arg1, arg3))
                    setFalseIf();
                else
                    setTrueIf();
            }
            else if (arg2 == "contains")
            {
                if (contains(arg1, arg3))
                    setFalseIf();
                else
                    setTrueIf();
            }
            else
            {
                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                setTrueIf();
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
                        error(ErrorMessage::IS_NULL, arg1, false);
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
                            setTrueIf();
                            State.LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        setFalseIf();
                }
                else
                    setFalseIf();
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
                        error(ErrorMessage::IS_NULL, arg3, false);
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
                            setTrueIf();
                            State.LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        setFalseIf();
                }
                else
                    setFalseIf();
            }
        }
        else if (variableExists(arg1) && variableExists(arg3))
        {
            if (isString(arg1) && isString(arg3))
            {
                if (arg2 == "==")
                {
                    if (variables.at(indexOfVariable(arg1)).getString() == variables.at(indexOfVariable(arg3)).getString())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "!=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString() != variables.at(indexOfVariable(arg3)).getString())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() > variables.at(indexOfVariable(arg3)).getString().length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() < variables.at(indexOfVariable(arg3)).getString().length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() <= variables.at(indexOfVariable(arg3)).getString().length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg1)).getString().length() >= variables.at(indexOfVariable(arg3)).getString().length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "contains")
                {
                    if (contains(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "ends_with")
                {
                    if (endsWith(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "begins_with")
                {
                    if (startsWith(variables.at(indexOfVariable(arg1)).getString(), variables.at(indexOfVariable(arg3)).getString()))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
            }
            else if (isNumber(arg1) && isNumber(arg3))
            {
                if (arg2 == "==")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() == variables.at(indexOfVariable(arg3)).getNumber())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "!=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() != variables.at(indexOfVariable(arg3)).getNumber())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() > variables.at(indexOfVariable(arg3)).getNumber())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() >= variables.at(indexOfVariable(arg3)).getNumber())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() < variables.at(indexOfVariable(arg3)).getNumber())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg1)).getNumber() <= variables.at(indexOfVariable(arg3)).getNumber())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, s, false);
                setFalseIf();
            }
        }
        else if ((variableExists(arg1) && !variableExists(arg3)) && !methodExists(arg3) && notObjectMethod(arg3) && !containsParams(arg3))
        {
            if (isNumber(arg1))
            {
                if (isNumeric(arg3))
                {
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() == stod(arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() != stod(arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() > stod(arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() < stod(arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
                else if (arg3 == "number?")
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    setFalseIf();
                }
            }
            else
            {
                if (arg3 == "string?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                            setTrueIf();
                        else if (arg2 == "!=")
                            setFalseIf();
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                }
                else if (arg3 == "number?")
                {
                    if (isNumber(arg1))
                    {
                        if (arg2 == "==")
                            setTrueIf();
                        else if (arg2 == "!=")
                            setFalseIf();
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                }
                else if (arg3 == "uppercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                            setFalseIf();
                    }
                }
                else if (arg3 == "lowercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                            setFalseIf();
                    }
                }
                else if (arg3 == "file?")
                {
                    if (isString(arg1))
                    {
                        if (Env::fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==")
                                setTrueIf();
                            else if (arg2 == "!=")
                                setFalseIf();
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        setFalseIf();
                    }
                }
                else if (arg3 == "dir?" || arg3 == "directory?")
                {
                    if (isString(arg1))
                    {
                        if (Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==")
                                setTrueIf();
                            else if (arg2 == "!=")
                                setFalseIf();
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        setFalseIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() == arg3)
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() != arg3)
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() > arg3.length())
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() < arg3.length())
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() >= arg3.length())
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() <= arg3.length())
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(variables.at(indexOfVariable(arg1)).getString(), arg3))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
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
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() == stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() != stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() > stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() < stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() >= stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getNumber() <= stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
                else if (stackValue == "number?")
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    setFalseIf();
                }
            }
            else
            {
                if (stackValue == "string?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                            setTrueIf();
                        else if (arg2 == "!=")
                            setFalseIf();
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                }
                else if (stackValue == "number?")
                {
                    if (isNumber(arg1))
                    {
                        if (arg2 == "==")
                            setTrueIf();
                        else if (arg2 == "!=")
                            setFalseIf();
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                }
                else if (stackValue == "uppercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (isUpper(variables.at(indexOfVariable(arg1)).getString()))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                            setFalseIf();
                    }
                }
                else if (stackValue == "lower?" || stackValue == "lowercase?")
                {
                    if (isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (isLower(variables.at(indexOfVariable(arg1)).getString()))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                setFalseIf();
                            else
                                setTrueIf();
                        }
                        else
                            setFalseIf();
                    }
                }
                else if (stackValue == "file?")
                {
                    if (isString(arg1))
                    {
                        if (Env::fileExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==")
                                setTrueIf();
                            else if (arg2 == "!=")
                                setFalseIf();
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        setFalseIf();
                    }
                }
                else if (stackValue == "directory?")
                {
                    if (isString(arg1))
                    {
                        if (Env::directoryExists(variables.at(indexOfVariable(arg1)).getString()))
                        {
                            if (arg2 == "==")
                                setTrueIf();
                            else if (arg2 == "!=")
                                setFalseIf();
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        setFalseIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() == stackValue)
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString() != stackValue)
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() > stackValue.length())
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() < stackValue.length())
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() >= stackValue.length())
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg1)).getString().length() <= stackValue.length())
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(variables.at(indexOfVariable(arg1)).getString(), stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
            }
        }
        else if ((!variableExists(arg1) && variableExists(arg3)) && !methodExists(arg1) && notObjectMethod(arg1) && !containsParams(arg1))
        {
            if (isNumber(arg3))
            {
                if (isNumeric(arg1))
                {
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() == stod(arg1))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() != stod(arg1))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() > stod(arg1))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() < stod(arg1))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() >= stod(arg1))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() <= stod(arg1))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    setFalseIf();
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() == arg1)
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "!=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() != arg1)
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() > arg1.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() < arg1.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() >= arg1.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() <= arg1.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
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
                    if (arg2 == "==")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() == stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "!=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() != stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() > stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() < stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == ">=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() >= stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else if (arg2 == "<=")
                    {
                        if (variables.at(indexOfVariable(arg3)).getNumber() <= stod(stackValue))
                            setTrueIf();
                        else
                            setFalseIf();
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    setFalseIf();
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() == stackValue)
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "!=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString() != stackValue)
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() > stackValue.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() < stackValue.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() >= stackValue.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<=")
                {
                    if (variables.at(indexOfVariable(arg3)).getString().length() <= stackValue.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
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

                        arg1Result = State.LastValue;

                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                    }
                    else
                    {
                        if (!objectExists(arg1before))
                            error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg1before, false);

                        if (!objectExists(arg3before))
                            error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg3before, false);

                        setFalseIf();
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

                        arg1Result = State.LastValue;

                        if (methodExists(beforeParams(arg3)))
                            executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                        arg3Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg1before, false);
                        setFalseIf();
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

                        arg3Result = State.LastValue;

                        if (methodExists(beforeParams(arg1)))
                            executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                        arg1Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg3before, false);
                        setFalseIf();
                    }
                }
                else
                {
                    string arg1Result(""), arg3Result("");

                    if (methodExists(beforeParams(arg1)))
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg1))), getParams(arg1));

                    arg1Result = State.LastValue;

                    if (methodExists(beforeParams(arg3)))
                        executeTemplate(methods.at(indexOfMethod(beforeParams(arg3))), getParams(arg3));

                    arg3Result = State.LastValue;

                    if (isNumeric(arg1Result) && isNumeric(arg3Result))
                    {
                        if (arg2 == "==")
                        {
                            if (stod(arg1Result) == stod(arg3Result))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (stod(arg1Result) != stod(arg3Result))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == "<")
                        {
                            if (stod(arg1Result) < stod(arg3Result))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == ">")
                        {
                            if (stod(arg1Result) > stod(arg3Result))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == "<=")
                        {
                            if (stod(arg1Result) <= stod(arg3Result))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == ">=")
                        {
                            if (stod(arg1Result) >= stod(arg3Result))
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
                    else
                    {
                        if (arg2 == "==")
                        {
                            if (arg1Result == arg3Result)
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else if (arg2 == "!=")
                        {
                            if (arg1Result != arg3Result)
                                setTrueIf();
                            else
                                setFalseIf();
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            setFalseIf();
                        }
                    }
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

                        arg1Result = State.LastValue;

                        if (methodExists(arg3))
                        {
                            parse(arg3);
                            arg3Result = State.LastValue;
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
                                error(ErrorMessage::IS_NULL, arg3, false);
                                setFalseIf();
                            }
                        }
                        else
                            arg3Result = arg3;

                        if (pass)
                        {
                            if (isNumeric(arg1Result) && isNumeric(arg3Result))
                            {
                                if (arg2 == "==")
                                {
                                    if (stod(arg1Result) == stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setFalseIf();
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg1Result == arg3Result)
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg1Result != arg3Result)
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setFalseIf();
                                }
                            }
                        }
                        else
                            setFalseIf();
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

                            comp = State.LastValue;
                        }
                        else if (containsParams(arg3))
                        {
                            executeTemplate(getMethod(beforeParams(arg3)), getParams(arg3));

                            comp = State.LastValue;
                        }
                        else
                            comp = arg3;

                        if (isNumeric(stackValue) && isNumeric(comp))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(stackValue) == stod(comp))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(stackValue) != stod(comp))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(stackValue) < stod(comp))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(stackValue) > stod(comp))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(stackValue) <= stod(comp))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(stackValue) >= stod(comp))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (stackValue == comp)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stackValue != comp)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::METHOD_UNDEFINED, beforeParams(arg1), false);
                        setFalseIf();
                    }
                }
                else
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    if (objectExists(arg1before))
                    {
                        if (objects.at(indexOfObject(arg1before)).methodExists(beforeParams(arg1after)))
                            executeTemplate(objects.at(indexOfObject(arg1before)).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = State.LastValue;

                        if (variableExists(arg3))
                        {
                            if (isString(arg3))
                                arg3Result = variables.at(indexOfVariable(arg3)).getString();
                            else if (isNumber(arg3))
                                arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                            else
                            {
                                pass = false;
                                error(ErrorMessage::IS_NULL, arg3, false);
                                setFalseIf();
                            }
                        }
                        else if (methodExists(arg3))
                        {
                            parse(arg3);

                            arg3Result = State.LastValue;
                        }
                        else
                            arg3Result = arg3;

                        if (pass)
                        {
                            if (isNumeric(arg1Result) && isNumeric(arg3Result))
                            {
                                if (arg2 == "==")
                                {
                                    if (stod(arg1Result) == stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setFalseIf();
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg1Result == arg3Result)
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg1Result != arg3Result)
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setFalseIf();
                                }
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg1before, false);
                        setFalseIf();
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

                        arg3Result = State.LastValue;

                        if (methodExists(arg1))
                        {
                            parse(arg1);
                            arg1Result = State.LastValue;
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
                                error(ErrorMessage::IS_NULL, arg1, false);
                                setFalseIf();
                            }
                        }
                        else
                            arg1Result = arg1;

                        if (pass)
                        {
                            if (isNumeric(arg3Result) && isNumeric(arg1Result))
                            {
                                if (arg2 == "==")
                                {
                                    if (stod(arg3Result) == stod(arg1Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg3Result) != stod(arg1Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg3Result) < stod(arg1Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg3Result) > stod(arg1Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg3Result) <= stod(arg1Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg3Result) >= stod(arg1Result))
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setFalseIf();
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg3Result == arg1Result)
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg3Result != arg1Result)
                                        setTrueIf();
                                    else
                                        setFalseIf();
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    setFalseIf();
                                }
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::METHOD_UNDEFINED, beforeParams(arg3), false);
                        setFalseIf();
                    }
                }
                else
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    if (objectExists(arg3before))
                    {
                        if (objects.at(indexOfObject(arg3before)).methodExists(beforeParams(arg3after)))
                            executeTemplate(objects.at(indexOfObject(arg3before)).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (variableExists(arg1))
                        {
                            if (isString(arg1))
                                arg1Result = variables.at(indexOfVariable(arg1)).getString();
                            else if (isNumber(arg3))
                                arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                            else
                            {
                                error(ErrorMessage::IS_NULL, arg1, false);
                                setFalseIf();
                            }
                        }
                        else if (methodExists(arg1))
                        {
                            parse(arg1);

                            arg1Result = State.LastValue;
                        }
                        else
                            arg1Result = arg1;

                        if (isNumeric(arg3Result) && isNumeric(arg1Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg3Result) == stod(arg1Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg3Result) != stod(arg1Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg3Result) < stod(arg1Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg3Result) > stod(arg1Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg3Result) <= stod(arg1Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg3Result) >= stod(arg1Result))
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg3Result == arg1Result)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg3Result != arg1Result)
                                    setTrueIf();
                                else
                                    setFalseIf();
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                setFalseIf();
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::OBJ_METHOD_UNDEFINED, arg3before, false);
                        setFalseIf();
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
                arg1Result = State.LastValue;
            }
            else if (variableExists(arg1))
            {
                if (isString(arg1))
                    arg1Result = variables.at(indexOfVariable(arg1)).getString();
                else if (isNumber(arg1))
                    arg1Result = dtos(variables.at(indexOfVariable(arg1)).getNumber());
                else
                {
                    error(ErrorMessage::IS_NULL, arg1, false);
                    setFalseIf();
                }
            }
            else
                arg1Result = arg1;

            if (methodExists(arg3))
            {
                parse(arg3);
                arg3Result = State.LastValue;
            }
            else if (variableExists(arg3))
            {
                if (isString(arg3))
                    arg3Result = variables.at(indexOfVariable(arg3)).getString();
                else if (isNumber(arg3))
                    arg3Result = dtos(variables.at(indexOfVariable(arg3)).getNumber());
                else
                {
                    error(ErrorMessage::IS_NULL, arg3, false);
                    setFalseIf();
                }
            }
            else
                arg3Result = arg3;

            if (isNumeric(arg1Result) && isNumeric(arg3Result))
            {
                if (arg2 == "==")
                {
                    if (stod(arg1Result) == stod(arg3Result))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "!=")
                {
                    if (stod(arg1Result) != stod(arg3Result))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<")
                {
                    if (stod(arg1Result) < stod(arg3Result))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">")
                {
                    if (stod(arg1Result) > stod(arg3Result))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "<=")
                {
                    if (stod(arg1Result) <= stod(arg3Result))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == ">=")
                {
                    if (stod(arg1Result) >= stod(arg3Result))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (arg1Result == arg3Result)
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else if (arg2 == "!=")
                {
                    if (arg1Result != arg3Result)
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
            }
        }
        else
        {
            if (arg3 == "object?")
            {
                if (objectExists(arg1))
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
            }
            else if (arg3 == "variable?")
            {
                if (variableExists(arg1))
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
            }
            else if (arg3 == "method?")
            {
                if (methodExists(arg1))
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
            }
            else if (arg3 == "list?")
            {
                if (listExists(arg1))
                {
                    if (arg2 == "==")
                        setTrueIf();
                    else if (arg2 == "!=")
                        setFalseIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
                else
                {
                    if (arg2 == "==")
                        setFalseIf();
                    else if (arg2 == "!=")
                        setTrueIf();
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        setFalseIf();
                    }
                }
            }
            else if (arg2 == "==")
            {
                if (arg1 == arg3)
                    setTrueIf();
                else
                    setFalseIf();
            }
            else if (arg2 == "!=")
            {
                if (arg1 != arg3)
                    setTrueIf();
                else
                    setFalseIf();
            }
            else if (arg2 == ">")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) > stod(arg3))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    if (arg1.length() > arg3.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
            }
            else if (arg2 == "<")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) < stod(arg3))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    if (arg1.length() < arg3.length())
                        setTrueIf();
                    else
                        setFalseIf();
                }
            }
            else if (arg2 == ">=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) >= stod(arg3))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
            }
            else if (arg2 == "<=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) <= stod(arg3))
                        setTrueIf();
                    else
                        setFalseIf();
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    setFalseIf();
                }
            }
            else if (arg2 == "begins_with")
            {
                if (startsWith(arg1, arg3))
                    setTrueIf();
                else
                    setFalseIf();
            }
            else if (arg2 == "ends_with")
            {
                if (endsWith(arg1, arg3))
                    setTrueIf();
                else
                    setFalseIf();
            }
            else if (arg2 == "contains")
            {
                if (contains(arg1, arg3))
                    setTrueIf();
                else
                    setFalseIf();
            }
            else
            {
                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                setFalseIf();
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
                    error(ErrorMessage::CONV_ERR, s, false);
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
                    error(ErrorMessage::CONV_ERR, s, false);
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
                    error(ErrorMessage::CONV_ERR, s, false);
                    failedFor();
                }
            }
            else
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
                    error(ErrorMessage::CONV_ERR, s, false);
                    failedFor();
                }
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
                    error(ErrorMessage::CONV_ERR, s, false);
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
                    error(ErrorMessage::CONV_ERR, s, false);
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
                    error(ErrorMessage::CONV_ERR, s, false);
                    failedFor();
                }
            }
            else
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
                    error(ErrorMessage::CONV_ERR, s, false);
                    failedFor();
                }
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
                    error(ErrorMessage::CONV_ERR, s, false);
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
                    error(ErrorMessage::CONV_ERR, s, false);
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
                    error(ErrorMessage::CONV_ERR, s, false);
                    failedFor();
                }
            }
            else
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
                    error(ErrorMessage::CONV_ERR, s, false);
                    failedFor();
                }
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
                    error(ErrorMessage::CONV_ERR, s, false);
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
                    error(ErrorMessage::CONV_ERR, s, false);
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
                    error(ErrorMessage::CONV_ERR, s, false);
                    failedFor();
                }
            }
            else
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
                    error(ErrorMessage::CONV_ERR, s, false);
                    failedFor();
                }
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
                else if (objectExists(before) && after == "get_methods")
                {
                    List newList;

                    vector<Method> objMethods = objects.at(indexOfObject(before)).getMethods();

                    for (int i = 0; i < (int)objMethods.size(); i++)
                        newList.add(objMethods.at(i).name());

                    successfulFor(newList);
                }
                else if (objectExists(before) && after == "get_variables")
                {
                    List newList;

                    vector<Variable> objVars = objects.at(indexOfObject(before)).getVariables();

                    for (int i = 0; i < (int)objVars.size(); i++)
                        newList.add(objVars.at(i).name());

                    successfulFor(newList);
                }
                else if (variableExists(before) && after == "length")
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
                            if (after == "get_dirs")
                            {
                                if (Env::directoryExists(variables.at(indexOfVariable(before)).getString()))
                                    successfulFor(getDirectoryList(before, false));
                                else
                                {
                                    error(ErrorMessage::READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
                                    failedFor();
                                }
                            }
                            else if (after == "get_files")
                            {
                                if (Env::directoryExists(variables.at(indexOfVariable(before)).getString()))
                                    successfulFor(getDirectoryList(before, true));
                                else
                                {
                                    error(ErrorMessage::READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
                                    failedFor();
                                }
                            }
                            else if (after == "read")
                            {
                                if (Env::fileExists(variables.at(indexOfVariable(before)).getString()))
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
                                        error(ErrorMessage::READ_FAIL, variables.at(indexOfVariable(before)).getString(), false);
                                        failedFor();
                                    }
                                }
                            }
                            else
                            {
                                error(ErrorMessage::METHOD_UNDEFINED, after, false);
                                failedFor();
                            }
                        }
                        else
                        {
                            error(ErrorMessage::VAR_UNDEFINED, before, false);
                            failedFor();
                        }
                    }
                    else
                    {
                        if (listExists(arg3))
                            successfulFor(lists.at(indexOfList(arg3)));
                        else
                        {
                            error(ErrorMessage::LIST_UNDEFINED, arg3, false);
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
                        State.DefaultLoopSymbol = arg1;

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

                                            State.DefaultLoopSymbol = arg1;

                                            successfulFor(newList);

                                            lists = removeList(lists, "&l&i&s&t&");
                                        }
                                        else
                                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + ".." + rangeEnd, false);
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

                                            State.DefaultLoopSymbol = arg1;

                                            successfulFor(newList);

                                            lists = removeList(lists, "&l&i&s&t&");
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
                        else
                            error(ErrorMessage::OUT_OF_BOUNDS, arg3, false);
                    }
                    else
                    {
                        error(ErrorMessage::NULL_STRING, before, false);
                        failedFor();
                    }
                }
            }
            else if (listExists(arg3))
            {
                State.DefaultLoopSymbol = arg1;

                successfulFor(lists.at(indexOfList(arg3)));
            }
            else if (!zeroDots(arg3))
            {
                string _b(beforeDot(arg3)), _a(afterDot(arg3));

                if (_b == "args" && _a == "size")
                {
                    List newList;

                    State.DefaultLoopSymbol = arg1;

                    for (int i = 0; i < (int)args.size(); i++)
                        newList.add(args.at(i));

                    successfulFor(newList);
                }
                else if (_b == "env" && _a == "get_variables")
                {
                    List newList;

                    newList.add("cwd");
                    newList.add("noctis");
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

                    State.DefaultLoopSymbol = arg1;
                    successfulFor(newList);
                }
                else if (objectExists(_b) && _a == "get_methods")
                {
                    List newList;

                    vector<Method> objMethods = objects.at(indexOfObject(_b)).getMethods();

                    for (int i = 0; i < (int)objMethods.size(); i++)
                        newList.add(objMethods.at(i).name());

                    State.DefaultLoopSymbol = arg1;
                    successfulFor(newList);
                }
                else if (objectExists(_b) && _a == "get_variables")
                {
                    List newList;

                    vector<Variable> objVars = objects.at(indexOfObject(_b)).getVariables();

                    for (int i = 0; i < (int)objVars.size(); i++)
                        newList.add(objVars.at(i).name());

                    State.DefaultLoopSymbol = arg1;
                    successfulFor(newList);
                }
                else if (variableExists(_b) && _a == "length")
                {
                    if (isString(_b))
                    {
                        State.DefaultLoopSymbol = arg1;
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
                            if (_a == "get_dirs")
                            {
                                if (Env::directoryExists(variables.at(indexOfVariable(_b)).getString()))
                                {
                                    State.DefaultLoopSymbol = arg1;
                                    successfulFor(getDirectoryList(_b, false));
                                }
                                else
                                {
                                    error(ErrorMessage::READ_FAIL, variables.at(indexOfVariable(_b)).getString(), false);
                                    failedFor();
                                }
                            }
                            else if (_a == "get_files")
                            {
                                if (Env::directoryExists(variables.at(indexOfVariable(_b)).getString()))
                                {
                                    State.DefaultLoopSymbol = arg1;
                                    successfulFor(getDirectoryList(_b, true));
                                }
                                else
                                {
                                    error(ErrorMessage::READ_FAIL, variables.at(indexOfVariable(_b)).getString(), false);
                                    failedFor();
                                }
                            }
                            else if (_a == "read")
                            {
                                if (Env::fileExists(variables.at(indexOfVariable(_b)).getString()))
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

                                        State.DefaultLoopSymbol = arg1;
                                        successfulFor(newList);
                                    }
                                    else
                                    {
                                        error(ErrorMessage::READ_FAIL, variables.at(indexOfVariable(_b)).getString(), false);
                                        failedFor();
                                    }
                                }
                            }
                            else
                            {
                                error(ErrorMessage::METHOD_UNDEFINED, _a, false);
                                failedFor();
                            }
                        }
                        else
                        {
                            error(ErrorMessage::VAR_UNDEFINED, _b, false);
                            failedFor();
                        }
                    }
                }
            }
            else
            {
                error(ErrorMessage::INVALID_OP, s, false);
                failedFor();
            }
        }
        else
        {
            error(ErrorMessage::INVALID_OP, s, false);
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
                    successfulWhile(arg1, arg2, arg3);
                else
                {
                    error(ErrorMessage::INVALID_OP, s, false);
                    failedWhile();
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, arg1 + arg2 + arg3, false);
                failedWhile();
            }
        }
        else if (isNumeric(arg3) && variableExists(arg1))
        {
            if (isNumber(arg1))
            {
                if (arg2 == "<" || arg2 == "<=" || arg2 == ">=" || arg2 == ">" || arg2 == "==" || arg2 == "!=")
                    successfulWhile(arg1, arg2, arg3);
                else
                {
                    error(ErrorMessage::INVALID_OP, s, false);
                    failedWhile();
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, arg1 + arg2 + arg3, false);
                failedWhile();
            }
        }
        else if (isNumeric(arg1) && isNumeric(arg3))
        {
            if (arg2 == "<" || arg2 == "<=" || arg2 == ">=" || arg2 == ">" || arg2 == "==" || arg2 == "!=")
                successfulWhile(arg1, arg2, arg3);
            else
            {
                error(ErrorMessage::INVALID_OP, s, false);
                failedWhile();
            }
        }
        else
        {
            error(ErrorMessage::INVALID_OP, s, false);
            failedWhile();
        }
    }
    else
        Env::sysExec(s, command);
}

#endif
