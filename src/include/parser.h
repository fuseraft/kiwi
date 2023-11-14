#ifndef PARSER_H
#define PARSER_H

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
    vector<string> command;  // a tokenized command container
    int length = s.length(), //	length of the line
        count = 0,           // command token counter
        size = 0;            // final size of tokenized command container
    bool quoted = false,     // flag: parsing string literals
        broken = false,      // flag: end of a command
        uncomment = false,   // flag: end a command
        parenthesis = false; // flag: parsing contents within parentheses
    char prevChar = 'a';     // previous character in string

    StringContainer stringContainer; // contains separate commands
    string bigString("");            // a string to build upon

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
                        if (mem.getArgCount() - 1 >= stoi(params.at(0)) && stoi(params.at(0)) >= 0)
                        {
                            if (params.at(0) == "0")
                                command.at(i) = State.CurrentScript;
                            else
                                command.at(i) = mem.getArg(stoi(params.at(0)));
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
                if (startsWith(s, "case"))
                    mem.getMainSwitch().addCase(command.at(1));
                else if (s == "default")
                    State.InDefaultCase = true;
                else if (s == "end")
                {
                    string switch_value("");

                    if (mem.isString(State.SwitchVarName))
                        switch_value = mem.varString(State.SwitchVarName);
                    else if (mem.isNumber(State.SwitchVarName))
                        switch_value = mem.varNumberString(State.SwitchVarName);
                    else
                        switch_value = "";

                    Container rightCase = mem.getMainSwitch().rightCase(switch_value);

                    State.InDefaultCase = false;
                    State.DefiningSwitchBlock = false;

                    for (int i = 0; i < (int)rightCase.size(); i++)
                        parse(rightCase.at(i));

                    mem.getMainSwitch().clear();
                }
                else
                {
                    if (State.InDefaultCase)
                        mem.getMainSwitch().addToDefault(s);
                    else
                        mem.getMainSwitch().addToCase(s);
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
                    mem.getModule(State.CurrentModule).add(s);
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
                        if (s == "end")
                        {
                            if (State.DefiningLocalWhileLoop)
                            {
                                State.DefiningLocalWhileLoop = false;

                                if (State.DefiningClass)
                                    mem.getClass(State.CurrentClass).addToCurrentMethod(s);
                                else
                                    mem.getMethod(mem.getMethodCount() - 1).add(s);
                            }
                            else if (State.DefiningLocalSwitchBlock)
                            {
                                State.DefiningLocalSwitchBlock = false;

                                if (State.DefiningClass)
                                    mem.getClass(State.CurrentClass).addToCurrentMethod(s);
                                else
                                    mem.getMethod(mem.getMethodCount() - 1).add(s);
                            }
                            else
                            {
                                State.DefiningMethod = false;

                                if (State.DefiningClass)
                                {
                                    State.DefiningClassMethod = false;
                                    mem.getClass(mem.getClassCount() - 1).setCurrentMethod("");
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
                                if (mem.variableExists(words.at(z)))
                                {
                                    if (mem.isString(words.at(z)))
                                        freshLine.append(mem.varString(words.at(z)));
                                    else if (mem.isNumber(words.at(z)))
                                        freshLine.append(mem.varNumberString(words.at(z)));
                                }
                                else
                                    freshLine.append(words.at(z));

                                if (z != (int)words.size() - 1)
                                    freshLine.push_back(' ');
                            }

                            if (State.DefiningClass)
                            {
                                mem.getClass(State.CurrentClass).addToCurrentMethod(freshLine);

                                if (State.DefiningPublicCode)
                                    mem.getClass(State.CurrentClass).setPublic();
                                else if (State.DefiningPrivateCode)
                                    mem.getClass(State.CurrentClass).setPrivate();
                                else
                                    mem.getClass(State.CurrentClass).setPublic();
                            }
                            else
                                mem.getMethod(mem.getMethodCount() - 1).add(freshLine);
                        }
                    }
                    else
                    {
                        if (s == "end")
                        {
                            if (State.DefiningLocalWhileLoop)
                            {
                                State.DefiningLocalWhileLoop = false;

                                if (State.DefiningClass)
                                    mem.getClass(mem.getClassCount() - 1).addToCurrentMethod(s);
                                else
                                    mem.getMethod(mem.getMethodCount() - 1).add(s);
                            }
                            else if (State.DefiningLocalSwitchBlock)
                            {
                                State.DefiningLocalSwitchBlock = false;

                                if (State.DefiningClass)
                                    mem.getClass(mem.getClassCount() - 1).addToCurrentMethod(s);
                                else
                                    mem.getMethod(mem.getMethodCount() - 1).add(s);
                            }
                            else
                            {
                                State.DefiningMethod = false;

                                if (State.DefiningClass)
                                {
                                    State.DefiningClassMethod = false;
                                    mem.getClass(mem.getClassCount() - 1).setCurrentMethod("");
                                }
                            }
                        }
                        else
                        {
                            if (State.DefiningClass)
                            {
                                mem.getClass(mem.getClassCount() - 1).addToCurrentMethod(s);

                                if (State.DefiningPublicCode)
                                    mem.getClass(mem.getClassCount() - 1).setPublic();
                                else if (State.DefiningPrivateCode)
                                    mem.getClass(mem.getClassCount() - 1).setPrivate();
                                else
                                    mem.getClass(mem.getClassCount() - 1).setPublic();
                            }
                            else
                            {
                                if (State.DefiningClassMethod)
                                {
                                    mem.getClass(mem.getClassCount() - 1).addToCurrentMethod(s);

                                    if (State.DefiningPublicCode)
                                        mem.getClass(mem.getClassCount() - 1).setPublic();
                                    else if (State.DefiningPrivateCode)
                                        mem.getClass(mem.getClassCount() - 1).setPrivate();
                                    else
                                        mem.getClass(mem.getClassCount() - 1).setPublic();
                                }
                                else
                                    mem.getMethod(mem.getMethodCount() - 1).add(s);
                            }
                        }
                    }
                }
                else if (State.DefiningIfStatement)
                {
                    if (State.DefiningNest)
                    {
                        if (command.at(0) == "endif")
                            exec.executeNest(mem.getIfStatement(mem.getIfStatementCount() - 1).getNest());
                        else
                            mem.getIfStatement(mem.getIfStatementCount() - 1).inNest(s);
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
                                mem.createIfStatement(false);
                                State.DefiningNest = false;
                            }
                        }
                        else if (command.at(0) == "endif")
                        {
                            State.DefiningIfStatement = false;
                            State.ExecutedIfStatement = true;

                            for (int i = 0; i < mem.getIfStatementCount(); i++)
                            {
                                if (mem.getIfStatement(i).isIF())
                                {
                                    exec.executeMethod(mem.getIfStatement(i));

                                    if (State.FailedIfStatement == false)
                                        break;
                                }
                            }

                            mem.clearIf();

                            State.ExecutedIfStatement = false;
                            State.FailedIfStatement = false;
                            State.IfStatementCount = 0;
                        }
                        else if (command.at(0) == "elsif" || command.at(0) == "elif")
                        {
                            if (size == 4)
                                threeSpace("if", command.at(1), command.at(2), command.at(3), s, command);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (s == "else")
                            threeSpace("if", "true", "==", "true", "if true == true", command);
                        else if (s == "failif")
                        {
                            if (State.FailedIfStatement == true)
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                            mem.getIfStatement(mem.getIfStatementCount() - 1).add(s);
                    }
                }
                else
                {
                    if (State.DefiningWhileLoop)
                    {
                        if (command.at(0) == "end")
                        {
                            State.DefiningWhileLoop = false;

                            string v1 = mem.getWhileLoop(mem.getWhileLoopCount() - 1).valueOne(),
                                   v2 = mem.getWhileLoop(mem.getWhileLoopCount() - 1).valueTwo(),
                                   op = mem.getWhileLoop(mem.getWhileLoopCount() - 1).logicOperator();

                            if (mem.variableExists(v1) && mem.variableExists(v2))
                            {
                                if (op == "==")
                                {
                                    while (mem.varNumber(v1) == mem.varNumber(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "<")
                                {
                                    while (mem.varNumber(v1) < mem.varNumber(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == ">")
                                {
                                    while (mem.varNumber(v1) > mem.varNumber(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "<=")
                                {
                                    while (mem.varNumber(v1) <= mem.varNumber(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == ">=")
                                {
                                    while (mem.varNumber(v1) >= mem.varNumber(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "!=")
                                {
                                    while (mem.varNumber(v1) != mem.varNumber(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                            }
                            else if (mem.variableExists(v1))
                            {
                                if (op == "==")
                                {
                                    while (mem.varNumber(v1) == stoi(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "<")
                                {
                                    while (mem.varNumber(v1) < stoi(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == ">")
                                {
                                    while (mem.varNumber(v1) > stoi(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "<=")
                                {
                                    while (mem.varNumber(v1) <= stoi(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == ">=")
                                {
                                    while (mem.varNumber(v1) >= stoi(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                                else if (op == "!=")
                                {
                                    while (mem.varNumber(v1) != stoi(v2))
                                    {
                                        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

                                        if (State.Breaking)
                                            break;
                                    }

                                    mem.clearWhile();

                                    State.WhileLoopCount = 0;
                                }
                            }
                        }
                        else
                            mem.getWhileLoop(mem.getWhileLoopCount() - 1).add(s);
                    }
                    else if (State.DefiningForLoop)
                    {
                        if (command.at(0) == "next" || command.at(0) == "endfor")
                        {
                            State.DefiningForLoop = false;

                            for (int i = 0; i < mem.getForLoopCount(); i++)
                                if (mem.getForLoop(i).isForLoop())
                                    exec.executeForLoop(mem.getForLoop(i));

                            mem.clearFor();

                            State.ForLoopCount = 0;
                        }
                        else
                        {
                            mem.getForLoop(mem.getForLoopCount() - 1).add(s);
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
                                    if (mem.classExists(before) && after.length() != 0)
                                    {
                                        if (containsParams(after))
                                        {
                                            s = subtractChar(s, "\"");

                                            if (mem.getClass(before).hasMethod(beforeParams(after)))
                                                exec.executeTemplate(mem.getClass(before).getMethod(beforeParams(after)), getParams(after));
                                            else
                                                Env::sysExec(s, command);
                                        }
                                        else if (mem.getClass(before).hasMethod(after))
                                            exec.executeMethod(mem.getClass(before).getMethod(after));
                                        else if (mem.getClass(before).hasVariable(after))
                                        {
                                            if (mem.getClass(before).getVariable(after).getString() != State.Null)
                                                writeline(mem.getClass(before).getVariable(after).getString());
                                            else if (mem.getClass(before).getVariable(after).getNumber() != State.NullNum)
                                                writeline(dtos(mem.getClass(before).getVariable(after).getNumber()));
                                            else
                                                error(ErrorMessage::IS_NULL, "", false);
                                        }
                                        else if (after == "clear")
                                            mem.getClass(before).clear();
                                        else
                                            error(ErrorMessage::UNDEFINED, "", false);
                                    }
                                    else
                                    {
                                        if (before == "env")
                                        {
                                            InternalGetEnv("", after, 3);
                                        }
                                        else if (mem.variableExists(before))
                                        {
                                            if (after == "clear")
                                                parse(before + " = State.Null");
                                        }
                                        else if (mem.listExists(before))
                                        {
                                            // REFACTOR HERE
                                            if (after == "clear")
                                                mem.getList(before).clear();
                                            else if (after == "sort")
                                                mem.getList(before).listSort();
                                            else if (after == "reverse")
                                                mem.getList(before).listReverse();
                                            else if (after == "revert")
                                                mem.getList(before).listRevert();
                                        }
                                        else if (before == "self")
                                        {
                                            if (State.ExecutedMethod)
                                                exec.executeMethod(mem.getClass(State.CurrentMethodClass).getMethod(after));
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
                                        mem.getScript().addMark(newMark);
                                    }
                                }
                                else if (mem.methodExists(s))
                                    exec.executeMethod(mem.getMethod(s));
                                else if (startsWith(s, "[") && endsWith(s, "]"))
                                {
                                    mem.createModule(s);
                                }
                                else
                                {
                                    s = subtractChar(s, "\"");

                                    if (mem.methodExists(beforeParams(s)))
                                        exec.executeTemplate(mem.getMethod(beforeParams(s)), getParams(s));
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
                                    FileIO::appendText(command.at(1), command.at(2), false);
                                else if (command.at(0) == "appendl")
                                    FileIO::appendText(command.at(1), command.at(2), true);
                                else if ((command.at(0) == "fwrite"))
                                    FileIO::writeText(command.at(1), command.at(2));
                                else if (command.at(0) == "redefine")
                                    mem.redefine(command.at(1), command.at(2));
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

                    parse(trimLeadingWhitespace(commentString));
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

                    stringContainer.add(trimLeadingWhitespace(commentString));

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
        mem.clearMethods();
    else if (arg0 == "clear_classes!")
        mem.clearClasses();
    else if (arg0 == "clear_variables!")
        mem.clearVariables();
    else if (arg0 == "clear_lists!")
        mem.clearLists();
    else if (arg0 == "clear_all!")
        mem.clearAll();
    else if (arg0 == "clear_constants!")
        mem.clearConstants();
    else if (arg0 == "exit")
    {
        mem.clearAll();
        exit(0);
    }
    else if (arg0 == "break" || arg0 == "leave!")
        State.Breaking = true;
    else if (arg0 == "no_methods?")
    {
        if (mem.noMethods())
            State.LastValue = "true";
        else
            State.LastValue = "false";
    }
    else if (arg0 == "no_classes?")
    {
        if (mem.noClasses())
            State.LastValue = "true";
        else
            State.LastValue = "false";
    }
    else if (arg0 == "no_variables?")
    {
        if (mem.noVariables())
            State.LastValue = "true";
        else
            State.LastValue = "false";
    }
    else if (arg0 == "no_lists?")
    {
        if (mem.noLists())
            State.LastValue = "true";
        else
            State.LastValue = "false";
    }
    else if (arg0 == "end" || arg0 == "}")
    {
        State.DefiningPrivateCode = false,
        State.DefiningPublicCode = false;
        State.DefiningClass = false;
        State.DefiningClassMethod = false;
        State.CurrentClass = "";
    }
    else if (arg0 == "parser")
        startREPL();
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
            mem.createIfStatement(true);
        else
            mem.createIfStatement(false);
    }
    else
        Env::sysExec(s, command);
}

void oneSpace(string arg0, string arg1, string s, vector<string> command)
{
    string before(beforeDot(arg1)), after(afterDot(arg1));

    if (contains(arg1, "self."))
    {
        arg1 = replace(arg1, "self", State.CurrentMethodClass);
    }

    if (arg0 == "return")
    {
        if (!InternalReturn(arg0, arg1, before, after))
            oneSpace("return", arg1, "return " + arg1, command);
    }
    else if (arg0 == "switch")
    {
        if (mem.variableExists(arg1))
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
            if (mem.getScript().markExists(arg1))
            {
                State.GoTo = arg1;
                State.GoToLabel = true;
            }
        }
    }
    else if (arg0 == "if")
    {
        string tmpValue("");
        // if arg1 is a variable
        if (mem.variableExists(arg1))
        {
            // can we can assume that arg1 belongs to an object?
            if (!zeroDots(arg1))
            {
                string objName(beforeDot(arg1)), varName(afterDot(arg1));
                Variable tmpVar = mem.getClass(objName).getVariable(varName);

                if (mem.isString(tmpVar))
                {
                    tmpValue = tmpVar.getString();
                }
                else if (mem.isNumber(tmpVar))
                {
                    tmpValue = dtos(tmpVar.getNumber());
                }
                else
                {
                    // error(ErrorMessage::IS_NULL, arg1, true);
                }
            }
            else
            {
                if (mem.isString(arg1))
                {
                    tmpValue = mem.varString(arg1);
                }
                else if (mem.isNumber(arg1))
                {
                    tmpValue = mem.varNumber(arg1);
                }
                else
                {
                    // error(ErrorMessage::IS_NULL, arg1, true);
                }
            }
        }
        else
        {
            if (isNumeric(arg1) || isTrue(arg1) || isFalse(arg1))
            {
                tmpValue = arg1;
            }
            else
            {
                string tmpCode("");

                if (startsWith(arg1, "(\"") && endsWith(arg1, "\")"))
                {
                    tmpCode = getInner(arg1, 2, arg1.length() - 3);
                }
                else
                {
                    tmpCode = arg1;
                }
                tmpValue = getParsedOutput(tmpCode);
            }
        }

        if (isTrue(tmpValue))
        {
            mem.createIfStatement(true);
        }
        else if (isFalse(tmpValue))
        {
            mem.createIfStatement(false);
        }
        else
        {
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
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
                IO::printerrln(mem.varString(arg1));
            else if (mem.isNumber(arg1))
                IO::printerrln(dtos(mem.varNumber(arg1)));
            else
                error(ErrorMessage::IS_NULL, arg1, false);
        }
        else
            IO::printerrln(arg1);
    }
    else if (arg0 == "delay")
    {
        if (isNumeric(arg1))
            DT::delay(stoi(arg1));
        else
            error(ErrorMessage::CONV_ERR, arg1, false);
    }
    else if (arg0 == "loop")
        threeSpace("for", "var", "in", arg1, "for var in " + arg1, command); // REFACTOR HERE
    else if (arg0 == "for" && arg1 == "inf")
        mem.createForLoop();
    else if (arg0 == "remove")
    {
        if (containsParams(arg1))
        {
            vector<string> params = getParams(arg1);

            for (int i = 0; i < (int)params.size(); i++)
            {
                if (mem.variableExists(params.at(i)))
                    mem.removeVariable(params.at(i));
                else if (mem.listExists(params.at(i)))
                    mem.removeList(params.at(i));
                else if (mem.classExists(params.at(i)))
                    mem.removeClass(params.at(i));
                else if (mem.methodExists(params.at(i)))
                    mem.removeMethod(params.at(i));
                else
                    error(ErrorMessage::TARGET_UNDEFINED, params.at(i), false);
            }
        }
        else if (mem.variableExists(arg1))
            mem.removeVariable(arg1);
        else if (mem.listExists(arg1))
            mem.removeList(arg1);
        else if (mem.classExists(arg1))
            mem.removeClass(arg1);
        else if (mem.methodExists(arg1))
            mem.removeMethod(arg1);
        else
            error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
    }
    else if (arg0 == "see_string")
    {
        if (mem.variableExists(arg1))
            write(mem.varString(arg1));
        else
            error(ErrorMessage::VAR_UNDEFINED, arg1, false);
    }
    else if (arg0 == "see_number")
    {
        if (mem.variableExists(arg1))
            write(dtos(mem.varNumber(arg1)));
        else
            error(ErrorMessage::VAR_UNDEFINED, arg1, false);
    }
    else if (arg0 == "__begin__")
    {
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
            {
                if (!Env::fileExists(mem.varString(arg1)))
                {
                    Env::createFile(mem.varString(arg1));
                    State.DefiningScript = true;
                    State.CurrentScriptName = mem.varString(arg1);
                }
                else
                    error(ErrorMessage::FILE_EXISTS, mem.varString(arg1), false);
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
        mem.globalize(arg1);
    }
    else if (arg0 == "load")
    {
        if (Env::fileExists(arg1))
        {
            if (isScript(arg1))
            {
                State.PreviousScript = State.CurrentScript;
                mem.loadScript(arg1);
                exec.executeScript();
            }
            else
                error(ErrorMessage::BAD_LOAD, arg1, true);
        }
        else if (mem.moduleExists(arg1))
        {
            vector<string> lines = mem.getModule(arg1).get();

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
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
            {
                if (Env::directoryExists(mem.varString(arg1)))
                    Env::cd(mem.varString(arg1));
                else
                    error(ErrorMessage::READ_FAIL, mem.varString(arg1), false);
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
        if (mem.listExists(arg1))
            mem.getList(arg1).clear();
        else
        {
            List newList(arg1);

            newList.setCollectable(State.ExecutedTemplate || State.ExecutedMethod);

            mem.addList(newList);
        }
    }
    else if (arg0 == "!")
    {
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
                parse(mem.varString(arg1).c_str());
            else
                error(ErrorMessage::IS_NULL, arg1, false);
        }
        else
            parse(arg1.c_str());
    }
    else if (arg0 == "?")
    {
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
                Env::sysExec(mem.varString(arg1), command);
            else
                error(ErrorMessage::IS_NULL, arg1, false);
        }
        else
            Env::sysExec(arg1, command);
    }
    else if (arg0 == "init_dir" || arg0 == "initial_directory")
    {
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
            {
                if (Env::directoryExists(mem.varString(arg1)))
                {
                    NoctisEnv.InitialDirectory = mem.varString(arg1);
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
            if (mem.getClass(before).hasMethod(after))
                State.LastValue = "true";
            else
                State.LastValue = "false";
        }
        else
        {
            if (mem.methodExists(arg1))
                State.LastValue = "true";
            else
                State.LastValue = "false";
        }
    }
    else if (arg0 == "class?")
    {
        if (mem.classExists(arg1))
            State.LastValue = "true";
        else
            State.LastValue = "false";
    }
    else if (arg0 == "variable?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (mem.getClass(before).hasVariable(after))
                State.LastValue = "true";
            else
                State.LastValue = "false";
        }
        else
        {
            if (mem.variableExists(arg1))
                State.LastValue = "true";
            else
                State.LastValue = "false";
        }
    }
    else if (arg0 == "list?")
    {
        if (mem.listExists(arg1))
            State.LastValue = "true";
        else
            State.LastValue = "false";
    }
    else if (arg0 == "directory?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (mem.getClass(before).hasVariable(after))
            {
                if (Env::directoryExists(mem.getClass(before).getVariable(after).getString()))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
        }
        else
        {
            if (mem.variableExists(arg1))
            {
                if (mem.isString(arg1))
                {
                    if (Env::directoryExists(mem.varString(arg1)))
                        State.LastValue = "true";
                    else
                        State.LastValue = "false";
                }
                else
                    error(ErrorMessage::NULL_STRING, arg1, false);
            }
            else
            {
                if (Env::directoryExists(arg1))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
        }
    }
    else if (arg0 == "file?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (mem.getClass(before).hasVariable(after))
            {
                if (Env::fileExists(mem.getClass(before).getVariable(after).getString()))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
        }
        else
        {
            if (mem.variableExists(arg1))
            {
                if (mem.isString(arg1))
                {
                    if (Env::fileExists(mem.varString(arg1)))
                        State.LastValue = "true";
                    else
                        State.LastValue = "false";
                }
                else
                    State.LastValue = "false";
            }
            else
            {
                if (Env::fileExists(arg1))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
        }
    }
    else if (arg0 == "collect?")
    {
        if (mem.variableExists(arg1))
        {
            if (mem.getVar(arg1).isCollectable())
                State.LastValue = "true";
            else
                State.LastValue = "false";
        }
        else
            IO::println("under construction...");
    }
    else if (arg0 == "number?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (mem.getClass(before).hasVariable(after))
            {
                if (mem.getClass(before).getVariable(after).getNumber() != State.NullNum)
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
        }
        else
        {
            if (mem.variableExists(arg1))
            {
                if (mem.isNumber(arg1))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
            else
            {
                if (isNumeric(arg1))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
        }
    }
    else if (arg0 == "string?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (mem.getClass(before).hasVariable(after))
            {
                if (mem.getClass(before).getVariable(after).getString() != State.Null)
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
        }
        else
        {
            if (mem.variableExists(arg1))
            {
                if (mem.isString(arg1))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
            else
            {
                if (isNumeric(arg1))
                    State.LastValue = "false";
                else
                    State.LastValue = "true";
            }
        }
    }
    else if (arg0 == "uppercase?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (mem.getClass(before).hasVariable(after))
            {
                if (isUpper(mem.getClass(before).getVariable(after).getString()))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
        }
        else
        {
            if (mem.variableExists(arg1))
            {
                if (mem.isString(arg1))
                {
                    if (isUpper(mem.varString(arg1)))
                        State.LastValue = "true";
                    else
                        State.LastValue = "false";
                }
                else
                    State.LastValue = "false";
            }
            else
            {
                if (isNumeric(arg1))
                    State.LastValue = "false";
                else
                {
                    if (isUpper(arg1))
                        State.LastValue = "true";
                    else
                        State.LastValue = "false";
                }
            }
        }
    }
    else if (arg0 == "lowercase?")
    {
        if (before.length() != 0 && after.length() != 0)
        {
            if (mem.getClass(before).hasVariable(after))
            {
                if (isLower(mem.getClass(before).getVariable(after).getString()))
                    State.LastValue = "true";
                else
                    State.LastValue = "false";
            }
            else
                error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
        }
        else
        {
            if (mem.variableExists(arg1))
            {
                if (mem.isString(arg1))
                {
                    if (isLower(mem.varString(arg1)))
                        State.LastValue = "true";
                    else
                        State.LastValue = "false";
                }
                else
                    State.LastValue = "false";
            }
            else
            {
                if (isNumeric(arg1))
                    State.LastValue = "false";
                else
                {
                    if (isLower(arg1))
                        State.LastValue = "true";
                    else
                        State.LastValue = "false";
                }
            }
        }
    }
    else if (arg0 == "see")
    {
        exec.executeInspection(arg0, arg1, before, after);
    }
    else if (arg0 == "template")
    {
        if (mem.methodExists(arg1))
            error(ErrorMessage::METHOD_DEFINED, arg1, false);
        else
        {
            if (containsParams(arg1))
            {
                vector<string> params = getParams(arg1);
                Method method(beforeParams(arg1), true);
                method.setTemplateSize((int)params.size());
                mem.addMethod(method);
                State.DefiningMethod = true;
            }
        }
    }
    else if (arg0 == "lock")
    {
        if (mem.variableExists(arg1))
            mem.getVar(arg1).setIndestructible(true);
        else if (mem.methodExists(arg1))
            mem.getMethod(arg1).setIndestructible(true);
    }
    else if (arg0 == "unlock")
    {
        if (mem.variableExists(arg1))
            mem.getVar(arg1).setIndestructible(false);
        else if (mem.methodExists(arg1))
            mem.getMethod(arg1).setIndestructible(false);
    }
    else if (arg0 == "method" || arg0 == "[method]")
    {
        mem.createMethod(arg0, arg1);
    }
    else if (arg0 == "call_method")
    {
        exec.executeMethod(arg1, before, after);
    }
    else if (arg0 == "class")
    {
        mem.createClass(arg1);
    }
    else if (arg0 == "fpush")
    {
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
            {
                if (!Env::fileExists(mem.varString(arg1)))
                    Env::createFile(mem.varString(arg1));
                else
                    error(ErrorMessage::FILE_EXISTS, mem.varString(arg1), false);
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
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
            {
                if (Env::fileExists(mem.varString(arg1)))
                    Env::rm(mem.varString(arg1));
                else
                    error(ErrorMessage::FILE_NOT_FOUND, mem.varString(arg1), false);
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
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
            {
                if (!Env::directoryExists(mem.varString(arg1)))
                    Env::md(mem.varString(arg1));
                else
                    error(ErrorMessage::DIR_EXISTS, mem.varString(arg1), false);
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
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
            {
                if (Env::directoryExists(mem.varString(arg1)))
                    Env::rd(mem.varString(arg1));
                else
                    error(ErrorMessage::DIR_NOT_FOUND, mem.varString(arg1), false);
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
        arg2 = replace(arg2, "self", State.CurrentMethodClass);

    if (contains(arg0, "self."))
        arg0 = replace(arg0, "self", State.CurrentMethodClass);

    if (mem.variableExists(arg0))
    {
        initializeVariable(arg0, arg1, arg2, s, command);
    }
    else if (mem.listExists(arg0) || mem.listExists(beforeBrackets(arg0)))
    {
        initializeListValues(arg0, arg1, arg2, s, command);
    }
    else
    {
        if (startsWith(arg0, "@") && zeroDots(arg0))
        {
            initializeGlobalVariable(arg0, arg1, arg2, s, command);
        }
        else if (startsWith(arg0, "@") && !zeroDots(arg2))
        {
            initializeClassVariable(arg0, arg1, arg2, s, command);
        }
        else if (!mem.classExists(arg0) && mem.classExists(arg2))
        {
            copyClass(arg0, arg1, arg2, s, command);
        }
        else if (isUpperConstant(arg0))
        {
            initializeConstant(arg0, arg1, arg2, s);
        }
        else
        {
            exec.executeSimpleStatement(arg0, arg1, arg2, s);
        }
    }
}

void threeSpace(string arg0, string arg1, string arg2, string arg3, string s, vector<string> command)
{
    // isNumber(arg3)
    // isString(arg3)

    if (arg0 == "class")
    {
        if (mem.classExists(arg1))
        {
            State.DefiningClass = true;
            State.CurrentClass = arg1;
        }
        else
        {
            if (mem.classExists(arg3))
            {
                if (arg2 == "=")
                {
                    vector<Method> classMethods = mem.getClass(arg3).getMethods();
                    Class newClass(arg1);

                    for (int i = 0; i < (int)classMethods.size(); i++)
                    {
                        if (classMethods.at(i).isPublic())
                            newClass.addMethod(classMethods.at(i));
                    }

                    mem.addClass(newClass);
                    State.CurrentClass = arg1;
                    State.DefiningClass = true;

                    newClass.clear();
                    classMethods.clear();
                }
                else
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
            }
            else
                error(ErrorMessage::CLS_METHOD_UNDEFINED, arg3, false);
        }
    }
    else if (arg0 == "unless")
    {
        if (mem.listExists(arg3))
        {
            if (arg2 == "in")
            {
                string testString("[none]");

                if (mem.variableExists(arg1))
                {
                    if (mem.isString(arg1))
                        testString = mem.varString(arg1);
                    else if (mem.isNumber(arg1))
                        testString = dtos(mem.varNumber(arg1));
                    else
                        error(ErrorMessage::IS_NULL, arg1, false);
                }
                else
                    testString = arg1;

                if (testString != "[none]")
                {
                    bool elementFound = false;
                    for (int i = 0; i < (int)mem.getList(arg3).size(); i++)
                    {
                        if (mem.getList(arg3).at(i) == testString)
                        {
                            elementFound = true;
                            mem.createIfStatement(false);
                            State.LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        mem.createIfStatement(true);
                }
                else
                    mem.createIfStatement(true);
            }
        }
        else if (mem.variableExists(arg1) && mem.variableExists(arg3))
        {
            if (mem.isString(arg1) && mem.isString(arg3))
            {
                if (arg2 == "==")
                {
                    if (mem.varString(arg1) == mem.varString(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "!=")
                {
                    if (mem.varString(arg1) != mem.varString(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">")
                {
                    if (mem.varString(arg1).length() > mem.varString(arg3).length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<")
                {
                    if (mem.varString(arg1).length() < mem.varString(arg3).length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<=")
                {
                    if (mem.varString(arg1).length() <= mem.varString(arg3).length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">=")
                {
                    if (mem.varString(arg1).length() >= mem.varString(arg3).length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "contains")
                {
                    if (contains(mem.varString(arg1), mem.varString(arg3)))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "ends_with")
                {
                    if (endsWith(mem.varString(arg1), mem.varString(arg3)))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "begins_with")
                {
                    if (startsWith(mem.varString(arg1), mem.varString(arg3)))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(true);
                }
            }
            else if (mem.isNumber(arg1) && mem.isNumber(arg3))
            {
                if (arg2 == "==")
                {
                    if (mem.varNumber(arg1) == mem.varNumber(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "!=")
                {
                    if (mem.varNumber(arg1) != mem.varNumber(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">")
                {
                    if (mem.varNumber(arg1) > mem.varNumber(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">=")
                {
                    if (mem.varNumber(arg1) >= mem.varNumber(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<")
                {
                    if (mem.varNumber(arg1) < mem.varNumber(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<=")
                {
                    if (mem.varNumber(arg1) <= mem.varNumber(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(true);
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, s, false);
                mem.createIfStatement(true);
            }
        }
        else if ((mem.variableExists(arg1) && !mem.variableExists(arg3)) && !mem.methodExists(arg3) && mem.notClassMethod(arg3) && !containsParams(arg3))
        {
            if (mem.isNumber(arg1))
            {
                if (isNumeric(arg3))
                {
                    if (arg2 == "==")
                    {
                        if (mem.varNumber(arg1) == stod(arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varNumber(arg1) != stod(arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varNumber(arg1) > stod(arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varNumber(arg1) < stod(arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varNumber(arg1) >= stod(arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varNumber(arg1) <= stod(arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
                else if (arg3 == "number?")
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createIfStatement(true);
                }
            }
            else
            {
                if (arg3 == "string?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                            mem.createIfStatement(false);
                        else if (arg2 == "!=")
                            mem.createIfStatement(true);
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "!")
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                }
                else if (arg3 == "number?")
                {
                    if (mem.isNumber(arg1))
                    {
                        if (arg2 == "==")
                            mem.createIfStatement(false);
                        else if (arg2 == "!=")
                            mem.createIfStatement(true);
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                }
                else if (arg3 == "uppercase?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isUpper(mem.varString(arg1)))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == "!=")
                        {
                            if (isUpper(mem.varString(arg1)))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                            mem.createIfStatement(true);
                    }
                }
                else if (arg3 == "lowercase?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isLower(mem.varString(arg1)))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == "!=")
                        {
                            if (isLower(mem.varString(arg1)))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                            mem.createIfStatement(true);
                    }
                }
                else if (arg3 == "file?")
                {
                    if (mem.isString(arg1))
                    {
                        if (Env::fileExists(mem.varString(arg1)))
                        {
                            if (arg2 == "==")
                                mem.createIfStatement(false);
                            else if (arg2 == "!=")
                                mem.createIfStatement(true);
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        mem.createIfStatement(true);
                    }
                }
                else if (arg3 == "directory?")
                {
                    if (mem.isString(arg1))
                    {
                        if (Env::directoryExists(mem.varString(arg1)))
                        {
                            if (arg2 == "==")
                                mem.createIfStatement(false);
                            else if (arg2 == "!=")
                                mem.createIfStatement(true);
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    if (arg2 == "==")
                    {
                        if (mem.varString(arg1) == arg3)
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varString(arg1) != arg3)
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varString(arg1).length() > arg3.length())
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varString(arg1).length() < arg3.length())
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varString(arg1).length() >= arg3.length())
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varString(arg1).length() <= arg3.length())
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(mem.varString(arg1), arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(mem.varString(arg1), arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(mem.varString(arg1), arg3))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
            }
        }
        else if ((mem.variableExists(arg1) && !mem.variableExists(arg3)) && !mem.methodExists(arg3) && mem.notClassMethod(arg3) && containsParams(arg3))
        {
            string stackValue("");

            if (isStringStack(arg3))
                stackValue = getStringStack(arg3);
            else if (stackReady(arg3))
                stackValue = dtos(getStack(arg3));
            else
                stackValue = arg3;

            if (mem.isNumber(arg1))
            {
                if (isNumeric(stackValue))
                {
                    if (arg2 == "==")
                    {
                        if (mem.varNumber(arg1) == stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varNumber(arg1) != stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varNumber(arg1) > stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varNumber(arg1) < stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varNumber(arg1) >= stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varNumber(arg1) <= stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
                else if (stackValue == "number?")
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createIfStatement(true);
                }
            }
            else
            {
                if (stackValue == "string?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                            mem.createIfStatement(false);
                        else if (arg2 == "!=")
                            mem.createIfStatement(true);
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                }
                else if (stackValue == "number?")
                {
                    if (mem.isNumber(arg1))
                    {
                        if (arg2 == "==")
                            mem.createIfStatement(false);
                        else if (arg2 == "!=")
                            mem.createIfStatement(true);
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                }
                else if (stackValue == "uppercase?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isUpper(mem.varString(arg1)))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == "!=")
                        {
                            if (isUpper(mem.varString(arg1)))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                            mem.createIfStatement(true);
                    }
                }
                else if (stackValue == "lowercase?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isLower(mem.varString(arg1)))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == "!=")
                        {
                            if (isLower(mem.varString(arg1)))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                            mem.createIfStatement(true);
                    }
                }
                else if (stackValue == "file?")
                {
                    if (mem.isString(arg1))
                    {
                        if (Env::fileExists(mem.varString(arg1)))
                        {
                            if (arg2 == "==")
                                mem.createIfStatement(false);
                            else if (arg2 == "!=")
                                mem.createIfStatement(true);
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        mem.createIfStatement(true);
                    }
                }
                else if (stackValue == "directory?")
                {
                    if (mem.isString(arg1))
                    {
                        if (Env::directoryExists(mem.varString(arg1)))
                        {
                            if (arg2 == "==")
                                mem.createIfStatement(false);
                            else if (arg2 == "!=")
                                mem.createIfStatement(true);
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    if (arg2 == "==")
                    {
                        if (mem.varString(arg1) == stackValue)
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varString(arg1) != stackValue)
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varString(arg1).length() > stackValue.length())
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varString(arg1).length() < stackValue.length())
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varString(arg1).length() >= stackValue.length())
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varString(arg1).length() <= stackValue.length())
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(mem.varString(arg1), stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(mem.varString(arg1), stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(mem.varString(arg1), stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
            }
        }
        else if ((!mem.variableExists(arg1) && mem.variableExists(arg3)) && !mem.methodExists(arg1) && mem.notClassMethod(arg1) && !containsParams(arg1))
        {
            if (mem.isNumber(arg3))
            {
                if (isNumeric(arg1))
                {
                    if (arg2 == "==")
                    {
                        if (mem.varNumber(arg3) == stod(arg1))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varNumber(arg3) != stod(arg1))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varNumber(arg3) > stod(arg1))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varNumber(arg3) < stod(arg1))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varNumber(arg3) >= stod(arg1))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varNumber(arg3) <= stod(arg1))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createIfStatement(true);
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (mem.varString(arg3) == arg1)
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "!=")
                {
                    if (mem.varString(arg3) != arg1)
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">")
                {
                    if (mem.varString(arg3).length() > arg1.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<")
                {
                    if (mem.varString(arg3).length() < arg1.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">=")
                {
                    if (mem.varString(arg3).length() >= arg1.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<=")
                {
                    if (mem.varString(arg3).length() <= arg1.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(true);
                }
            }
        }
        else if ((!mem.variableExists(arg1) && mem.variableExists(arg3)) && !mem.methodExists(arg1) && mem.notClassMethod(arg1) && containsParams(arg1))
        {
            string stackValue("");

            if (isStringStack(arg1))
                stackValue = getStringStack(arg1);
            else if (stackReady(arg1))
                stackValue = dtos(getStack(arg1));
            else
                stackValue = arg1;

            if (mem.isNumber(arg3))
            {
                if (isNumeric(stackValue))
                {
                    if (arg2 == "==")
                    {
                        if (mem.varNumber(arg3) == stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varNumber(arg3) != stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varNumber(arg3) > stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varNumber(arg3) < stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varNumber(arg3) >= stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varNumber(arg3) <= stod(stackValue))
                            mem.createIfStatement(false);
                        else
                            mem.createIfStatement(true);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createIfStatement(true);
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (mem.varString(arg3) == stackValue)
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "!=")
                {
                    if (mem.varString(arg3) != stackValue)
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">")
                {
                    if (mem.varString(arg3).length() > stackValue.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<")
                {
                    if (mem.varString(arg3).length() < stackValue.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">=")
                {
                    if (mem.varString(arg3).length() >= stackValue.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<=")
                {
                    if (mem.varString(arg3).length() <= stackValue.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(true);
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

                    if (mem.classExists(arg1before) && mem.classExists(arg3before))
                    {
                        if (mem.getClass(arg1before).hasMethod(beforeParams(arg1after)))
                            exec.executeTemplate(mem.getClass(arg1before).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = State.LastValue;

                        if (mem.getClass(arg3before).hasMethod(beforeParams(arg3after)))
                            exec.executeTemplate(mem.getClass(arg3before).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                    }
                    else
                    {
                        if (!mem.classExists(arg1before))
                            error(ErrorMessage::CLS_METHOD_UNDEFINED, arg1before, false);

                        if (!mem.classExists(arg3before))
                            error(ErrorMessage::CLS_METHOD_UNDEFINED, arg3before, false);

                        mem.createIfStatement(true);
                    }
                }
                else if (!zeroDots(arg1) && zeroDots(arg3))
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    string arg1Result(""), arg3Result("");

                    if (mem.classExists(arg1before))
                    {
                        if (mem.getClass(arg1before).hasMethod(beforeParams(arg1after)))
                            exec.executeTemplate(mem.getClass(arg1before).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = State.LastValue;

                        if (mem.methodExists(beforeParams(arg3)))
                            exec.executeTemplate(mem.getMethod(beforeParams(arg3)), getParams(arg3));

                        arg3Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg1before, false);
                        mem.createIfStatement(true);
                    }
                }
                else if (zeroDots(arg1) && !zeroDots(arg3))
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    string arg1Result(""), arg3Result("");

                    if (mem.classExists(arg3before))
                    {
                        if (mem.getClass(arg3before).hasMethod(beforeParams(arg3after)))
                            exec.executeTemplate(mem.getClass(arg3before).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (mem.methodExists(beforeParams(arg1)))
                            exec.executeTemplate(mem.getMethod(beforeParams(arg1)), getParams(arg1));

                        arg1Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg3before, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    string arg1Result(""), arg3Result("");

                    if (mem.methodExists(beforeParams(arg1)))
                        exec.executeTemplate(mem.getMethod(beforeParams(arg1)), getParams(arg1));

                    arg1Result = State.LastValue;

                    if (mem.methodExists(beforeParams(arg3)))
                        exec.executeTemplate(mem.getMethod(beforeParams(arg3)), getParams(arg3));

                    arg3Result = State.LastValue;

                    if (isNumeric(arg1Result) && isNumeric(arg3Result))
                    {
                        if (arg2 == "==")
                        {
                            if (stod(arg1Result) == stod(arg3Result))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == "!=")
                        {
                            if (stod(arg1Result) != stod(arg3Result))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == "<")
                        {
                            if (stod(arg1Result) < stod(arg3Result))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == ">")
                        {
                            if (stod(arg1Result) > stod(arg3Result))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == "<=")
                        {
                            if (stod(arg1Result) <= stod(arg3Result))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == ">=")
                        {
                            if (stod(arg1Result) >= stod(arg3Result))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
                        }
                    }
                    else
                    {
                        if (arg2 == "==")
                        {
                            if (arg1Result == arg3Result)
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else if (arg2 == "!=")
                        {
                            if (arg1Result != arg3Result)
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(true);
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
                    if (mem.methodExists(beforeParams(arg1)))
                    {
                        exec.executeTemplate(mem.getMethod(beforeParams(arg1)), getParams(arg1));

                        arg1Result = State.LastValue;

                        if (mem.methodExists(arg3))
                        {
                            parse(arg3);
                            arg3Result = State.LastValue;
                        }
                        else if (mem.variableExists(arg3))
                        {
                            if (mem.isString(arg3))
                                arg3Result = mem.varString(arg3);
                            else if (mem.isNumber(arg3))
                                arg3Result = dtos(mem.varNumber(arg3));
                            else
                            {
                                pass = false;
                                error(ErrorMessage::IS_NULL, arg3, false);
                                mem.createIfStatement(true);
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
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(true);
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg1Result == arg3Result)
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg1Result != arg3Result)
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(true);
                                }
                            }
                        }
                        else
                            mem.createIfStatement(true);
                    }
                    else
                    {
                        error(ErrorMessage::METHOD_UNDEFINED, beforeParams(arg1), false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    if (mem.classExists(arg1before))
                    {
                        if (mem.getClass(arg1before).hasMethod(beforeParams(arg1after)))
                            exec.executeTemplate(mem.getClass(arg1before).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = State.LastValue;

                        if (mem.variableExists(arg3))
                        {
                            if (mem.isString(arg3))
                                arg3Result = mem.varString(arg3);
                            else if (mem.isNumber(arg3))
                                arg3Result = dtos(mem.varNumber(arg3));
                            else
                            {
                                pass = false;
                                error(ErrorMessage::IS_NULL, arg3, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else if (mem.methodExists(arg3))
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
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(true);
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg1Result == arg3Result)
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg1Result != arg3Result)
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(true);
                                }
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg1before, false);
                        mem.createIfStatement(true);
                    }
                }
            }
            else if (!containsParams(arg1) && containsParams(arg3))
            {
                string arg1Result(""), arg3Result("");

                bool pass = true;

                if (zeroDots(arg3))
                {
                    if (mem.methodExists(beforeParams(arg3)))
                    {
                        exec.executeTemplate(mem.getMethod(beforeParams(arg3)), getParams(arg3));

                        arg3Result = State.LastValue;

                        if (mem.methodExists(arg1))
                        {
                            parse(arg1);
                            arg1Result = State.LastValue;
                        }
                        else if (mem.variableExists(arg1))
                        {
                            if (mem.isString(arg1))
                                arg1Result = mem.varString(arg1);
                            else if (mem.isNumber(arg1))
                                arg1Result = dtos(mem.varNumber(arg1));
                            else
                            {
                                pass = false;
                                error(ErrorMessage::IS_NULL, arg1, false);
                                mem.createIfStatement(true);
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
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg3Result) != stod(arg1Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg3Result) < stod(arg1Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg3Result) > stod(arg1Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg3Result) <= stod(arg1Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg3Result) >= stod(arg1Result))
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(true);
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg3Result == arg1Result)
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg3Result != arg1Result)
                                        mem.createIfStatement(false);
                                    else
                                        mem.createIfStatement(true);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(true);
                                }
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::METHOD_UNDEFINED, beforeParams(arg3), false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    if (mem.classExists(arg3before))
                    {
                        if (mem.getClass(arg3before).hasMethod(beforeParams(arg3after)))
                            exec.executeTemplate(mem.getClass(arg3before).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (mem.variableExists(arg1))
                        {
                            if (mem.isString(arg1))
                                arg1Result = mem.varString(arg1);
                            else if (mem.isNumber(arg3))
                                arg1Result = dtos(mem.varNumber(arg1));
                            else
                            {
                                error(ErrorMessage::IS_NULL, arg1, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else if (mem.methodExists(arg1))
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
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg3Result) != stod(arg1Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg3Result) < stod(arg1Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg3Result) > stod(arg1Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg3Result) <= stod(arg1Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg3Result) >= stod(arg1Result))
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg3Result == arg1Result)
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg3Result != arg1Result)
                                    mem.createIfStatement(false);
                                else
                                    mem.createIfStatement(true);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(true);
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg3before, false);
                        mem.createIfStatement(true);
                    }
                }
            }
        }
        else if ((mem.methodExists(arg1) && arg3 != "method?") || mem.methodExists(arg3))
        {
            string arg1Result(""), arg3Result("");

            if (mem.methodExists(arg1))
            {
                parse(arg1);
                arg1Result = State.LastValue;
            }
            else if (mem.variableExists(arg1))
            {
                if (mem.isString(arg1))
                    arg1Result = mem.varString(arg1);
                else if (mem.isNumber(arg1))
                    arg1Result = dtos(mem.varNumber(arg1));
                else
                {
                    error(ErrorMessage::IS_NULL, arg1, false);
                    mem.createIfStatement(true);
                }
            }
            else
                arg1Result = arg1;

            if (mem.methodExists(arg3))
            {
                parse(arg3);
                arg3Result = State.LastValue;
            }
            else if (mem.variableExists(arg3))
            {
                if (mem.isString(arg3))
                    arg3Result = mem.varString(arg3);
                else if (mem.isNumber(arg3))
                    arg3Result = dtos(mem.varNumber(arg3));
                else
                {
                    error(ErrorMessage::IS_NULL, arg3, false);
                    mem.createIfStatement(true);
                }
            }
            else
                arg3Result = arg3;

            if (isNumeric(arg1Result) && isNumeric(arg3Result))
            {
                if (arg2 == "==")
                {
                    if (stod(arg1Result) == stod(arg3Result))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "!=")
                {
                    if (stod(arg1Result) != stod(arg3Result))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<")
                {
                    if (stod(arg1Result) < stod(arg3Result))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">")
                {
                    if (stod(arg1Result) > stod(arg3Result))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "<=")
                {
                    if (stod(arg1Result) <= stod(arg3Result))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == ">=")
                {
                    if (stod(arg1Result) >= stod(arg3Result))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(true);
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (arg1Result == arg3Result)
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else if (arg2 == "!=")
                {
                    if (arg1Result != arg3Result)
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(true);
                }
            }
        }
        else
        {
            if (arg3 == "class?")
            {
                if (mem.classExists(arg1))
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
            }
            else if (arg3 == "variable?")
            {
                if (mem.variableExists(arg1))
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    if (arg2 == "=")
                        mem.createIfStatement(true);
                    else if (arg2 == "!")
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
            }
            else if (arg3 == "method?")
            {
                if (mem.methodExists(arg1))
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
            }
            else if (arg3 == "list?")
            {
                if (mem.listExists(arg1))
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
                else
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(true);
                    }
                }
            }
            else if (arg2 == "==")
            {
                if (arg1 == arg3)
                    mem.createIfStatement(false);
                else
                    mem.createIfStatement(true);
            }
            else if (arg2 == "!=")
            {
                if (arg1 != arg3)
                    mem.createIfStatement(false);
                else
                    mem.createIfStatement(true);
            }
            else if (arg2 == ">")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) > stod(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    if (arg1.length() > arg3.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
            }
            else if (arg2 == "<")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) < stod(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    if (arg1.length() < arg3.length())
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
            }
            else if (arg2 == ">=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) >= stod(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(true);
                }
            }
            else if (arg2 == "<=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) <= stod(arg3))
                        mem.createIfStatement(false);
                    else
                        mem.createIfStatement(true);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
            else if (arg2 == "begins_with")
            {
                if (startsWith(arg1, arg3))
                    mem.createIfStatement(false);
                else
                    mem.createIfStatement(true);
            }
            else if (arg2 == "ends_with")
            {
                if (endsWith(arg1, arg3))
                    mem.createIfStatement(false);
                else
                    mem.createIfStatement(true);
            }
            else if (arg2 == "contains")
            {
                if (contains(arg1, arg3))
                    mem.createIfStatement(false);
                else
                    mem.createIfStatement(true);
            }
            else
            {
                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                mem.createIfStatement(true);
            }
        }
    }
    else if (arg0 == "if")
    {
        if (mem.listExists(arg3))
        {
            if (arg2 == "in")
            {
                string testString("[none]");

                if (mem.variableExists(arg1))
                {
                    if (mem.isString(arg1))
                        testString = mem.varString(arg1);
                    else if (mem.isNumber(arg1))
                        testString = dtos(mem.varNumber(arg1));
                    else
                        error(ErrorMessage::IS_NULL, arg1, false);
                }
                else
                    testString = arg1;

                if (testString != "[none]")
                {
                    bool elementFound = false;
                    for (int i = 0; i < (int)mem.getList(arg3).size(); i++)
                    {
                        if (mem.getList(arg3).at(i) == testString)
                        {
                            elementFound = true;
                            mem.createIfStatement(true);
                            State.LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        mem.createIfStatement(false);
                }
                else
                    mem.createIfStatement(false);
            }
        }
        else if (mem.listExists(arg1) && arg3 != "list?")
        {
            if (arg2 == "contains")
            {
                string testString("[none]");

                if (mem.variableExists(arg3))
                {
                    if (mem.isString(arg3))
                        testString = mem.varString(arg3);
                    else if (mem.isNumber(arg3))
                        testString = dtos(mem.varNumber(arg3));
                    else
                        error(ErrorMessage::IS_NULL, arg3, false);
                }
                else
                    testString = arg3;

                if (testString != "[none]")
                {
                    bool elementFound = false;
                    for (int i = 0; i < (int)mem.getList(arg1).size(); i++)
                    {
                        if (mem.getList(arg1).at(i) == testString)
                        {
                            elementFound = true;
                            mem.createIfStatement(true);
                            State.LastValue = itos(i);
                            break;
                        }
                    }

                    if (!elementFound)
                        mem.createIfStatement(false);
                }
                else
                    mem.createIfStatement(false);
            }
        }
        else if (mem.variableExists(arg1) && mem.variableExists(arg3))
        {
            if (mem.isString(arg1) && mem.isString(arg3))
            {
                if (arg2 == "==")
                {
                    if (mem.varString(arg1) == mem.varString(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "!=")
                {
                    if (mem.varString(arg1) != mem.varString(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">")
                {
                    if (mem.varString(arg1).length() > mem.varString(arg3).length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<")
                {
                    if (mem.varString(arg1).length() < mem.varString(arg3).length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<=")
                {
                    if (mem.varString(arg1).length() <= mem.varString(arg3).length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">=")
                {
                    if (mem.varString(arg1).length() >= mem.varString(arg3).length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "contains")
                {
                    if (contains(mem.varString(arg1), mem.varString(arg3)))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "ends_with")
                {
                    if (endsWith(mem.varString(arg1), mem.varString(arg3)))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "begins_with")
                {
                    if (startsWith(mem.varString(arg1), mem.varString(arg3)))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
            else if (mem.isNumber(arg1) && mem.isNumber(arg3))
            {
                if (arg2 == "==")
                {
                    if (mem.varNumber(arg1) == mem.varNumber(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "!=")
                {
                    if (mem.varNumber(arg1) != mem.varNumber(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">")
                {
                    if (mem.varNumber(arg1) > mem.varNumber(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">=")
                {
                    if (mem.varNumber(arg1) >= mem.varNumber(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<")
                {
                    if (mem.varNumber(arg1) < mem.varNumber(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<=")
                {
                    if (mem.varNumber(arg1) <= mem.varNumber(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, s, false);
                mem.createIfStatement(false);
            }
        }
        else if ((mem.variableExists(arg1) && !mem.variableExists(arg3)) && !mem.methodExists(arg3) && mem.notClassMethod(arg3) && !containsParams(arg3))
        {
            if (mem.isNumber(arg1))
            {
                if (isNumeric(arg3))
                {
                    if (arg2 == "==")
                    {
                        if (mem.varNumber(arg1) == stod(arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varNumber(arg1) != stod(arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varNumber(arg1) > stod(arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varNumber(arg1) < stod(arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varNumber(arg1) >= stod(arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varNumber(arg1) <= stod(arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else if (arg3 == "number?")
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createIfStatement(false);
                }
            }
            else
            {
                if (arg3 == "string?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                            mem.createIfStatement(true);
                        else if (arg2 == "!=")
                            mem.createIfStatement(false);
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                }
                else if (arg3 == "number?")
                {
                    if (mem.isNumber(arg1))
                    {
                        if (arg2 == "==")
                            mem.createIfStatement(true);
                        else if (arg2 == "!=")
                            mem.createIfStatement(false);
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                }
                else if (arg3 == "uppercase?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isUpper(mem.varString(arg1)))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == "!=")
                        {
                            if (isUpper(mem.varString(arg1)))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                            mem.createIfStatement(false);
                    }
                }
                else if (arg3 == "lowercase?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isLower(mem.varString(arg1)))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == "!=")
                        {
                            if (isLower(mem.varString(arg1)))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                            mem.createIfStatement(false);
                    }
                }
                else if (arg3 == "file?")
                {
                    if (mem.isString(arg1))
                    {
                        if (Env::fileExists(mem.varString(arg1)))
                        {
                            if (arg2 == "==")
                                mem.createIfStatement(true);
                            else if (arg2 == "!=")
                                mem.createIfStatement(false);
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        mem.createIfStatement(false);
                    }
                }
                else if (arg3 == "dir?" || arg3 == "directory?")
                {
                    if (mem.isString(arg1))
                    {
                        if (Env::directoryExists(mem.varString(arg1)))
                        {
                            if (arg2 == "==")
                                mem.createIfStatement(true);
                            else if (arg2 == "!=")
                                mem.createIfStatement(false);
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    if (arg2 == "==")
                    {
                        if (mem.varString(arg1) == arg3)
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varString(arg1) != arg3)
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varString(arg1).length() > arg3.length())
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varString(arg1).length() < arg3.length())
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varString(arg1).length() >= arg3.length())
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varString(arg1).length() <= arg3.length())
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(mem.varString(arg1), arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(mem.varString(arg1), arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(mem.varString(arg1), arg3))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
            }
        }
        else if ((mem.variableExists(arg1) && !mem.variableExists(arg3)) && !mem.methodExists(arg3) && mem.notClassMethod(arg3) && containsParams(arg3))
        {
            string stackValue("");

            if (isStringStack(arg3))
                stackValue = getStringStack(arg3);
            else if (stackReady(arg3))
                stackValue = dtos(getStack(arg3));
            else
                stackValue = arg3;

            if (mem.isNumber(arg1))
            {
                if (isNumeric(stackValue))
                {
                    if (arg2 == "==")
                    {
                        if (mem.varNumber(arg1) == stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varNumber(arg1) != stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varNumber(arg1) > stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varNumber(arg1) < stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varNumber(arg1) >= stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varNumber(arg1) <= stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else if (stackValue == "number?")
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createIfStatement(false);
                }
            }
            else
            {
                if (stackValue == "string?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                            mem.createIfStatement(true);
                        else if (arg2 == "!=")
                            mem.createIfStatement(false);
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                }
                else if (stackValue == "number?")
                {
                    if (mem.isNumber(arg1))
                    {
                        if (arg2 == "==")
                            mem.createIfStatement(true);
                        else if (arg2 == "!=")
                            mem.createIfStatement(false);
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                }
                else if (stackValue == "uppercase?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isUpper(mem.varString(arg1)))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == "!=")
                        {
                            if (isUpper(mem.varString(arg1)))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isUpper(arg2))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                            mem.createIfStatement(false);
                    }
                }
                else if (stackValue == "lower?" || stackValue == "lowercase?")
                {
                    if (mem.isString(arg1))
                    {
                        if (arg2 == "==")
                        {
                            if (isLower(mem.varString(arg1)))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == "!=")
                        {
                            if (isLower(mem.varString(arg1)))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "!=")
                        {
                            if (isLower(arg2))
                                mem.createIfStatement(false);
                            else
                                mem.createIfStatement(true);
                        }
                        else
                            mem.createIfStatement(false);
                    }
                }
                else if (stackValue == "file?")
                {
                    if (mem.isString(arg1))
                    {
                        if (Env::fileExists(mem.varString(arg1)))
                        {
                            if (arg2 == "==")
                                mem.createIfStatement(true);
                            else if (arg2 == "!=")
                                mem.createIfStatement(false);
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        mem.createIfStatement(false);
                    }
                }
                else if (stackValue == "directory?")
                {
                    if (mem.isString(arg1))
                    {
                        if (Env::directoryExists(mem.varString(arg1)))
                        {
                            if (arg2 == "==")
                                mem.createIfStatement(true);
                            else if (arg2 == "!=")
                                mem.createIfStatement(false);
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "!=")
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        error(ErrorMessage::IS_NULL, arg1, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    if (arg2 == "==")
                    {
                        if (mem.varString(arg1) == stackValue)
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varString(arg1) != stackValue)
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varString(arg1).length() > stackValue.length())
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varString(arg1).length() < stackValue.length())
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varString(arg1).length() >= stackValue.length())
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varString(arg1).length() <= stackValue.length())
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "contains")
                    {
                        if (contains(mem.varString(arg1), stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "ends_with")
                    {
                        if (endsWith(mem.varString(arg1), stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "begins_with")
                    {
                        if (startsWith(mem.varString(arg1), stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
            }
        }
        else if ((!mem.variableExists(arg1) && mem.variableExists(arg3)) && !mem.methodExists(arg1) && mem.notClassMethod(arg1) && !containsParams(arg1))
        {
            if (mem.isNumber(arg3))
            {
                if (isNumeric(arg1))
                {
                    if (arg2 == "==")
                    {
                        if (mem.varNumber(arg3) == stod(arg1))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varNumber(arg3) != stod(arg1))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varNumber(arg3) > stod(arg1))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varNumber(arg3) < stod(arg1))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varNumber(arg3) >= stod(arg1))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varNumber(arg3) <= stod(arg1))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createIfStatement(false);
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (mem.varString(arg3) == arg1)
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "!=")
                {
                    if (mem.varString(arg3) != arg1)
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">")
                {
                    if (mem.varString(arg3).length() > arg1.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<")
                {
                    if (mem.varString(arg3).length() < arg1.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">=")
                {
                    if (mem.varString(arg3).length() >= arg1.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<=")
                {
                    if (mem.varString(arg3).length() <= arg1.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
        }
        else if ((!mem.variableExists(arg1) && mem.variableExists(arg3)) && !mem.methodExists(arg1) && mem.notClassMethod(arg1) && containsParams(arg1))
        {
            string stackValue("");

            if (isStringStack(arg1))
                stackValue = getStringStack(arg1);
            else if (stackReady(arg1))
                stackValue = dtos(getStack(arg1));
            else
                stackValue = arg1;

            if (mem.isNumber(arg3))
            {
                if (isNumeric(stackValue))
                {
                    if (arg2 == "==")
                    {
                        if (mem.varNumber(arg3) == stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "!=")
                    {
                        if (mem.varNumber(arg3) != stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">")
                    {
                        if (mem.varNumber(arg3) > stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<")
                    {
                        if (mem.varNumber(arg3) < stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == ">=")
                    {
                        if (mem.varNumber(arg3) >= stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else if (arg2 == "<=")
                    {
                        if (mem.varNumber(arg3) <= stod(stackValue))
                            mem.createIfStatement(true);
                        else
                            mem.createIfStatement(false);
                    }
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createIfStatement(false);
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (mem.varString(arg3) == stackValue)
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "!=")
                {
                    if (mem.varString(arg3) != stackValue)
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">")
                {
                    if (mem.varString(arg3).length() > stackValue.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<")
                {
                    if (mem.varString(arg3).length() < stackValue.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">=")
                {
                    if (mem.varString(arg3).length() >= stackValue.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<=")
                {
                    if (mem.varString(arg3).length() <= stackValue.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
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

                    if (mem.classExists(arg1before) && mem.classExists(arg3before))
                    {
                        if (mem.getClass(arg1before).hasMethod(beforeParams(arg1after)))
                            exec.executeTemplate(mem.getClass(arg1before).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = State.LastValue;

                        if (mem.getClass(arg3before).hasMethod(beforeParams(arg3after)))
                            exec.executeTemplate(mem.getClass(arg3before).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                    }
                    else
                    {
                        if (!mem.classExists(arg1before))
                            error(ErrorMessage::CLS_METHOD_UNDEFINED, arg1before, false);

                        if (!mem.classExists(arg3before))
                            error(ErrorMessage::CLS_METHOD_UNDEFINED, arg3before, false);

                        mem.createIfStatement(false);
                    }
                }
                else if (!zeroDots(arg1) && zeroDots(arg3))
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    string arg1Result(""), arg3Result("");

                    if (mem.classExists(arg1before))
                    {
                        if (mem.getClass(arg1before).hasMethod(beforeParams(arg1after)))
                            exec.executeTemplate(mem.getClass(arg1before).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = State.LastValue;

                        if (mem.methodExists(beforeParams(arg3)))
                            exec.executeTemplate(mem.getMethod(beforeParams(arg3)), getParams(arg3));

                        arg3Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg1before, false);
                        mem.createIfStatement(false);
                    }
                }
                else if (zeroDots(arg1) && !zeroDots(arg3))
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    string arg1Result(""), arg3Result("");

                    if (mem.classExists(arg3before))
                    {
                        if (mem.getClass(arg3before).hasMethod(beforeParams(arg3after)))
                            exec.executeTemplate(mem.getClass(arg3before).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (mem.methodExists(beforeParams(arg1)))
                            exec.executeTemplate(mem.getMethod(beforeParams(arg1)), getParams(arg1));

                        arg1Result = State.LastValue;

                        if (isNumeric(arg1Result) && isNumeric(arg3Result))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(arg1Result) == stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg1Result) != stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg1Result) < stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg1Result) > stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg1Result) <= stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg1Result) >= stod(arg3Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg1Result == arg3Result)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg1Result != arg3Result)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg3before, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    string arg1Result(""), arg3Result("");

                    if (mem.methodExists(beforeParams(arg1)))
                        exec.executeTemplate(mem.getMethod(beforeParams(arg1)), getParams(arg1));

                    arg1Result = State.LastValue;

                    if (mem.methodExists(beforeParams(arg3)))
                        exec.executeTemplate(mem.getMethod(beforeParams(arg3)), getParams(arg3));

                    arg3Result = State.LastValue;

                    if (isNumeric(arg1Result) && isNumeric(arg3Result))
                    {
                        if (arg2 == "==")
                        {
                            if (stod(arg1Result) == stod(arg3Result))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == "!=")
                        {
                            if (stod(arg1Result) != stod(arg3Result))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == "<")
                        {
                            if (stod(arg1Result) < stod(arg3Result))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == ">")
                        {
                            if (stod(arg1Result) > stod(arg3Result))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == "<=")
                        {
                            if (stod(arg1Result) <= stod(arg3Result))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == ">=")
                        {
                            if (stod(arg1Result) >= stod(arg3Result))
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
                        }
                    }
                    else
                    {
                        if (arg2 == "==")
                        {
                            if (arg1Result == arg3Result)
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else if (arg2 == "!=")
                        {
                            if (arg1Result != arg3Result)
                                mem.createIfStatement(true);
                            else
                                mem.createIfStatement(false);
                        }
                        else
                        {
                            error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                            mem.createIfStatement(false);
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
                    if (mem.methodExists(beforeParams(arg1)))
                    {
                        exec.executeTemplate(mem.getMethod(beforeParams(arg1)), getParams(arg1));

                        arg1Result = State.LastValue;

                        if (mem.methodExists(arg3))
                        {
                            parse(arg3);
                            arg3Result = State.LastValue;
                        }
                        else if (mem.variableExists(arg3))
                        {
                            if (mem.isString(arg3))
                                arg3Result = mem.varString(arg3);
                            else if (mem.isNumber(arg3))
                                arg3Result = dtos(mem.varNumber(arg3));
                            else
                            {
                                pass = false;
                                error(ErrorMessage::IS_NULL, arg3, false);
                                mem.createIfStatement(false);
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
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(false);
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg1Result == arg3Result)
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg1Result != arg3Result)
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(false);
                                }
                            }
                        }
                        else
                            mem.createIfStatement(false);
                    }
                    else if (stackReady(arg1))
                    {
                        string stackValue("");

                        if (isStringStack(arg1))
                            stackValue = getStringStack(arg1);
                        else
                            stackValue = dtos(getStack(arg1));

                        string comp("");

                        if (mem.variableExists(arg3))
                        {
                            if (mem.isString(arg3))
                                comp = mem.varString(arg3);
                            else if (mem.isNumber(arg3))
                                comp = dtos(mem.varNumber(arg3));
                        }
                        else if (mem.methodExists(arg3))
                        {
                            parse(arg3);

                            comp = State.LastValue;
                        }
                        else if (containsParams(arg3))
                        {
                            exec.executeTemplate(mem.getMethod(beforeParams(arg3)), getParams(arg3));

                            comp = State.LastValue;
                        }
                        else
                            comp = arg3;

                        if (isNumeric(stackValue) && isNumeric(comp))
                        {
                            if (arg2 == "==")
                            {
                                if (stod(stackValue) == stod(comp))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(stackValue) != stod(comp))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(stackValue) < stod(comp))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(stackValue) > stod(comp))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(stackValue) <= stod(comp))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(stackValue) >= stod(comp))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (stackValue == comp)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stackValue != comp)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::METHOD_UNDEFINED, beforeParams(arg1), false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    string arg1before(beforeDot(arg1)), arg1after(afterDot(arg1));

                    if (mem.classExists(arg1before))
                    {
                        if (mem.getClass(arg1before).hasMethod(beforeParams(arg1after)))
                            exec.executeTemplate(mem.getClass(arg1before).getMethod(beforeParams(arg1after)), getParams(arg1after));

                        arg1Result = State.LastValue;

                        if (mem.variableExists(arg3))
                        {
                            if (mem.isString(arg3))
                                arg3Result = mem.varString(arg3);
                            else if (mem.isNumber(arg3))
                                arg3Result = dtos(mem.varNumber(arg3));
                            else
                            {
                                pass = false;
                                error(ErrorMessage::IS_NULL, arg3, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else if (mem.methodExists(arg3))
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
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg1Result) != stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg1Result) < stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg1Result) > stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg1Result) <= stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg1Result) >= stod(arg3Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(false);
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg1Result == arg3Result)
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg1Result != arg3Result)
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(false);
                                }
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg1before, false);
                        mem.createIfStatement(false);
                    }
                }
            }
            else if (!containsParams(arg1) && containsParams(arg3))
            {
                string arg1Result(""), arg3Result("");

                bool pass = true;

                if (zeroDots(arg3))
                {
                    if (mem.methodExists(beforeParams(arg3)))
                    {
                        exec.executeTemplate(mem.getMethod(beforeParams(arg3)), getParams(arg3));

                        arg3Result = State.LastValue;

                        if (mem.methodExists(arg1))
                        {
                            parse(arg1);
                            arg1Result = State.LastValue;
                        }
                        else if (mem.variableExists(arg1))
                        {
                            if (mem.isString(arg1))
                                arg1Result = mem.varString(arg1);
                            else if (mem.isNumber(arg1))
                                arg1Result = dtos(mem.varNumber(arg1));
                            else
                            {
                                pass = false;
                                error(ErrorMessage::IS_NULL, arg1, false);
                                mem.createIfStatement(false);
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
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (stod(arg3Result) != stod(arg1Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "<")
                                {
                                    if (stod(arg3Result) < stod(arg1Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == ">")
                                {
                                    if (stod(arg3Result) > stod(arg1Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "<=")
                                {
                                    if (stod(arg3Result) <= stod(arg1Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == ">=")
                                {
                                    if (stod(arg3Result) >= stod(arg1Result))
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(false);
                                }
                            }
                            else
                            {
                                if (arg2 == "==")
                                {
                                    if (arg3Result == arg1Result)
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else if (arg2 == "!=")
                                {
                                    if (arg3Result != arg1Result)
                                        mem.createIfStatement(true);
                                    else
                                        mem.createIfStatement(false);
                                }
                                else
                                {
                                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                    mem.createIfStatement(false);
                                }
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::METHOD_UNDEFINED, beforeParams(arg3), false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    string arg3before(beforeDot(arg3)), arg3after(afterDot(arg3));

                    if (mem.classExists(arg3before))
                    {
                        if (mem.getClass(arg3before).hasMethod(beforeParams(arg3after)))
                            exec.executeTemplate(mem.getClass(arg3before).getMethod(beforeParams(arg3after)), getParams(arg3after));

                        arg3Result = State.LastValue;

                        if (mem.variableExists(arg1))
                        {
                            if (mem.isString(arg1))
                                arg1Result = mem.varString(arg1);
                            else if (mem.isNumber(arg3))
                                arg1Result = dtos(mem.varNumber(arg1));
                            else
                            {
                                error(ErrorMessage::IS_NULL, arg1, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else if (mem.methodExists(arg1))
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
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (stod(arg3Result) != stod(arg1Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<")
                            {
                                if (stod(arg3Result) < stod(arg1Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">")
                            {
                                if (stod(arg3Result) > stod(arg1Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "<=")
                            {
                                if (stod(arg3Result) <= stod(arg1Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == ">=")
                            {
                                if (stod(arg3Result) >= stod(arg1Result))
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                        else
                        {
                            if (arg2 == "==")
                            {
                                if (arg3Result == arg1Result)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else if (arg2 == "!=")
                            {
                                if (arg3Result != arg1Result)
                                    mem.createIfStatement(true);
                                else
                                    mem.createIfStatement(false);
                            }
                            else
                            {
                                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                                mem.createIfStatement(false);
                            }
                        }
                    }
                    else
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, arg3before, false);
                        mem.createIfStatement(false);
                    }
                }
            }
        }
        else if ((mem.methodExists(arg1) && arg3 != "method?") || mem.methodExists(arg3))
        {
            string arg1Result(""), arg3Result("");

            if (mem.methodExists(arg1))
            {
                parse(arg1);
                arg1Result = State.LastValue;
            }
            else if (mem.variableExists(arg1))
            {
                if (mem.isString(arg1))
                    arg1Result = mem.varString(arg1);
                else if (mem.isNumber(arg1))
                    arg1Result = dtos(mem.varNumber(arg1));
                else
                {
                    error(ErrorMessage::IS_NULL, arg1, false);
                    mem.createIfStatement(false);
                }
            }
            else
                arg1Result = arg1;

            if (mem.methodExists(arg3))
            {
                parse(arg3);
                arg3Result = State.LastValue;
            }
            else if (mem.variableExists(arg3))
            {
                if (mem.isString(arg3))
                    arg3Result = mem.varString(arg3);
                else if (mem.isNumber(arg3))
                    arg3Result = dtos(mem.varNumber(arg3));
                else
                {
                    error(ErrorMessage::IS_NULL, arg3, false);
                    mem.createIfStatement(false);
                }
            }
            else
                arg3Result = arg3;

            if (isNumeric(arg1Result) && isNumeric(arg3Result))
            {
                if (arg2 == "==")
                {
                    if (stod(arg1Result) == stod(arg3Result))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "!=")
                {
                    if (stod(arg1Result) != stod(arg3Result))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<")
                {
                    if (stod(arg1Result) < stod(arg3Result))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">")
                {
                    if (stod(arg1Result) > stod(arg3Result))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "<=")
                {
                    if (stod(arg1Result) <= stod(arg3Result))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == ">=")
                {
                    if (stod(arg1Result) >= stod(arg3Result))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
            else
            {
                if (arg2 == "==")
                {
                    if (arg1Result == arg3Result)
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else if (arg2 == "!=")
                {
                    if (arg1Result != arg3Result)
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
        }
        else
        {
            if (arg3 == "class?")
            {
                if (mem.classExists(arg1))
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
            }
            else if (arg3 == "variable?")
            {
                if (mem.variableExists(arg1))
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
            }
            else if (arg3 == "method?")
            {
                if (mem.methodExists(arg1))
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
            }
            else if (arg3 == "list?")
            {
                if (mem.listExists(arg1))
                {
                    if (arg2 == "==")
                        mem.createIfStatement(true);
                    else if (arg2 == "!=")
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    if (arg2 == "==")
                        mem.createIfStatement(false);
                    else if (arg2 == "!=")
                        mem.createIfStatement(true);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
            }
            else if (arg2 == "==")
            {
                if (arg1 == arg3)
                    mem.createIfStatement(true);
                else
                    mem.createIfStatement(false);
            }
            else if (arg2 == "!=")
            {
                if (arg1 != arg3)
                    mem.createIfStatement(true);
                else
                    mem.createIfStatement(false);
            }
            else if (arg2 == ">")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) > stod(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    if (arg1.length() > arg3.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
            }
            else if (arg2 == "<")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) < stod(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    if (arg1.length() < arg3.length())
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
            }
            else if (arg2 == ">=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) >= stod(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
            else if (arg2 == "<=")
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) <= stod(arg3))
                        mem.createIfStatement(true);
                    else
                        mem.createIfStatement(false);
                }
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
            else if (arg2 == "begins_with")
            {
                if (startsWith(arg1, arg3))
                    mem.createIfStatement(true);
                else
                    mem.createIfStatement(false);
            }
            else if (arg2 == "ends_with")
            {
                if (endsWith(arg1, arg3))
                    mem.createIfStatement(true);
                else
                    mem.createIfStatement(false);
            }
            else if (arg2 == "contains")
            {
                if (contains(arg1, arg3))
                    mem.createIfStatement(true);
                else
                    mem.createIfStatement(false);
            }
            else
            {
                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                mem.createIfStatement(false);
            }
        }
    }
    else if (arg0 == "for")
    {
        if (arg2 == "<")
        {
            if (mem.variableExists(arg1) && mem.variableExists(arg3))
            {
                if (mem.isNumber(arg1) && mem.isNumber(arg3))
                {
                    if (mem.varNumber(arg1) < mem.varNumber(arg3))
                        mem.createForLoop(mem.varNumber(arg1), mem.varNumber(arg3), "<");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else if (mem.variableExists(arg1) && !mem.variableExists(arg3))
            {
                if (mem.isNumber(arg1) && isNumeric(arg3))
                {
                    if (mem.varNumber(arg1) < stod(arg3))
                        mem.createForLoop(mem.varNumber(arg1), stod(arg3), "<");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else if (!mem.variableExists(arg1) && mem.variableExists(arg3))
            {
                if (isNumeric(arg1) && mem.isNumber(arg3))
                {
                    if (stod(arg1) < mem.varNumber(arg3))
                        mem.createForLoop(stod(arg1), mem.varNumber(arg3), "<");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) < stod(arg3))
                        mem.createForLoop(stod(arg1), stod(arg3), "<");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
        }
        else if (arg2 == ">")
        {
            if (mem.variableExists(arg1) && mem.variableExists(arg3))
            {
                if (mem.isNumber(arg1) && mem.isNumber(arg3))
                {
                    if (mem.varNumber(arg1) > mem.varNumber(arg3))
                        mem.createForLoop(mem.varNumber(arg1), mem.varNumber(arg3), ">");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else if (mem.variableExists(arg1) && !mem.variableExists(arg3))
            {
                if (mem.isNumber(arg1) && isNumeric(arg3))
                {
                    if (mem.varNumber(arg1) > stod(arg3))
                        mem.createForLoop(mem.varNumber(arg1), stod(arg3), ">");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else if (!mem.variableExists(arg1) && mem.variableExists(arg3))
            {
                if (isNumeric(arg1) && mem.isNumber(arg3))
                {
                    if (stod(arg1) > mem.varNumber(arg3))
                        mem.createForLoop(stod(arg1), mem.varNumber(arg3), ">");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) > stod(arg3))
                        mem.createForLoop(stod(arg1), stod(arg3), ">");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
        }
        else if (arg2 == "<=")
        {
            if (mem.variableExists(arg1) && mem.variableExists(arg3))
            {
                if (mem.isNumber(arg1) && mem.isNumber(arg3))
                {
                    if (mem.varNumber(arg1) <= mem.varNumber(arg3))
                        mem.createForLoop(mem.varNumber(arg1), mem.varNumber(arg3), "<=");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else if (mem.variableExists(arg1) && !mem.variableExists(arg3))
            {
                if (mem.isNumber(arg1) && isNumeric(arg3))
                {
                    if (mem.varNumber(arg1) <= stod(arg3))
                        mem.createForLoop(mem.varNumber(arg1), stod(arg3), "<=");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else if (!mem.variableExists(arg1) && mem.variableExists(arg3))
            {
                if (isNumeric(arg1) && mem.isNumber(arg3))
                {
                    if (stod(arg1) <= mem.varNumber(arg3))
                        mem.createForLoop(stod(arg1), mem.varNumber(arg3), "<=");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) <= stod(arg3))
                        mem.createForLoop(stod(arg1), stod(arg3), "<=");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
        }
        else if (arg2 == ">=")
        {
            if (mem.variableExists(arg1) && mem.variableExists(arg3))
            {
                if (mem.isNumber(arg1) && mem.isNumber(arg3))
                {
                    if (mem.varNumber(arg1) >= mem.varNumber(arg3))
                        mem.createForLoop(mem.varNumber(arg1), mem.varNumber(arg3), ">=");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else if (mem.variableExists(arg1) && !mem.variableExists(arg3))
            {
                if (mem.isNumber(arg1) && isNumeric(arg3))
                {
                    if (mem.varNumber(arg1) >= stod(arg3))
                        mem.createForLoop(mem.varNumber(arg1), stod(arg3), ">=");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else if (!mem.variableExists(arg1) && mem.variableExists(arg3))
            {
                if (isNumeric(arg1) && mem.isNumber(arg3))
                {
                    if (stod(arg1) >= mem.varNumber(arg3))
                        mem.createForLoop(stod(arg1), mem.varNumber(arg3), ">=");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
                }
            }
            else
            {
                if (isNumeric(arg1) && isNumeric(arg3))
                {
                    if (stod(arg1) >= stod(arg3))
                        mem.createForLoop(stod(arg1), stod(arg3), ">=");
                    else
                        mem.createFailedForLoop();
                }
                else
                {
                    error(ErrorMessage::CONV_ERR, s, false);
                    mem.createFailedForLoop();
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

                    for (int i = 0; i < mem.getArgCount(); i++)
                        newList.add(mem.getArg(i));

                    mem.createForLoop(newList);
                }
                else if (mem.classExists(before) && after == "get_methods")
                {
                    List newList;

                    vector<Method> objMethods = mem.getClass(before).getMethods();

                    for (int i = 0; i < (int)objMethods.size(); i++)
                        newList.add(objMethods.at(i).name());

                    mem.createForLoop(newList);
                }
                else if (mem.classExists(before) && after == "get_variables")
                {
                    List newList;

                    vector<Variable> objVars = mem.getClass(before).getVariables();

                    for (int i = 0; i < (int)objVars.size(); i++)
                        newList.add(objVars.at(i).name());

                    mem.createForLoop(newList);
                }
                else if (mem.variableExists(before) && after == "length")
                {
                    if (mem.isString(before))
                    {
                        List newList;
                        string tempVarStr = mem.varString(before);
                        int len = tempVarStr.length();

                        for (int i = 0; i < len; i++)
                        {
                            string tempStr("");
                            tempStr.push_back(tempVarStr[i]);
                            newList.add(tempStr);
                        }

                        mem.createForLoop(newList);
                    }
                }
                else
                {
                    if (before.length() != 0 && after.length() != 0)
                    {
                        if (mem.variableExists(before))
                        {
                            if (after == "get_dirs")
                            {
                                if (Env::directoryExists(mem.varString(before)))
                                    mem.createForLoop(getDirectoryList(before, false));
                                else
                                {
                                    error(ErrorMessage::READ_FAIL, mem.varString(before), false);
                                    mem.createFailedForLoop();
                                }
                            }
                            else if (after == "get_files")
                            {
                                if (Env::directoryExists(mem.varString(before)))
                                    mem.createForLoop(getDirectoryList(before, true));
                                else
                                {
                                    error(ErrorMessage::READ_FAIL, mem.varString(before), false);
                                    mem.createFailedForLoop();
                                }
                            }
                            else if (after == "read")
                            {
                                if (Env::fileExists(mem.varString(before)))
                                {
                                    List newList;

                                    ifstream file(mem.varString(before).c_str());
                                    string line("");

                                    if (file.is_open())
                                    {
                                        while (!file.eof())
                                        {
                                            getline(file, line);
                                            newList.add(line);
                                        }

                                        file.close();

                                        mem.createForLoop(newList);
                                    }
                                    else
                                    {
                                        error(ErrorMessage::READ_FAIL, mem.varString(before), false);
                                        mem.createFailedForLoop();
                                    }
                                }
                            }
                            else
                            {
                                error(ErrorMessage::METHOD_UNDEFINED, after, false);
                                mem.createFailedForLoop();
                            }
                        }
                        else
                        {
                            error(ErrorMessage::VAR_UNDEFINED, before, false);
                            mem.createFailedForLoop();
                        }
                    }
                    else
                    {
                        if (mem.listExists(arg3))
                            mem.createForLoop(mem.getList(arg3));
                        else
                        {
                            error(ErrorMessage::LIST_UNDEFINED, arg3, false);
                            mem.createFailedForLoop();
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

                    if (mem.variableExists(firstRangeSpecifier))
                    {
                        if (mem.isNumber(firstRangeSpecifier))
                            firstRangeSpecifier = mem.varNumberString(firstRangeSpecifier);
                        else
                            mem.createFailedForLoop();
                    }

                    if (mem.variableExists(lastRangeSpecifier))
                    {
                        if (mem.isNumber(lastRangeSpecifier))
                            lastRangeSpecifier = mem.varNumberString(lastRangeSpecifier);
                        else
                            mem.createFailedForLoop();
                    }

                    if (isNumeric(firstRangeSpecifier) && isNumeric(lastRangeSpecifier))
                    {
                        State.DefaultLoopSymbol = arg1;

                        int ifrs = stoi(firstRangeSpecifier), ilrs(stoi(lastRangeSpecifier));

                        if (ifrs < ilrs)
                            mem.createForLoop(stod(firstRangeSpecifier), stod(lastRangeSpecifier), "<=");
                        else if (ifrs > ilrs)
                            mem.createForLoop(stod(firstRangeSpecifier), stod(lastRangeSpecifier), ">=");
                        else
                            mem.createFailedForLoop();
                    }
                    else
                        mem.createFailedForLoop();
                }
            }
            else if (containsBrackets(arg3))
            {
                string before(beforeBrackets(arg3));

                if (mem.variableExists(before))
                {
                    if (mem.isString(before))
                    {
                        string tempVarString(mem.varString(before));

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

                                            mem.createForLoop(newList);

                                            mem.removeList("&l&i&s&t&");
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

                                            mem.createForLoop(newList);

                                            mem.removeList("&l&i&s&t&");
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
                        mem.createFailedForLoop();
                    }
                }
            }
            else if (mem.listExists(arg3))
            {
                State.DefaultLoopSymbol = arg1;

                mem.createForLoop(mem.getList(arg3));
            }
            else if (!zeroDots(arg3))
            {
                string _b(beforeDot(arg3)), _a(afterDot(arg3));

                if (_b == "args" && _a == "size")
                {
                    List newList;

                    State.DefaultLoopSymbol = arg1;

                    for (int i = 0; i < mem.getArgCount(); i++)
                        newList.add(mem.getArg(i));

                    mem.createForLoop(newList);
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
                    mem.createForLoop(newList);
                }
                else if (mem.classExists(_b) && _a == "get_methods")
                {
                    List newList;

                    vector<Method> objMethods = mem.getClass(_b).getMethods();

                    for (int i = 0; i < (int)objMethods.size(); i++)
                        newList.add(objMethods.at(i).name());

                    State.DefaultLoopSymbol = arg1;
                    mem.createForLoop(newList);
                }
                else if (mem.classExists(_b) && _a == "get_variables")
                {
                    List newList;

                    vector<Variable> objVars = mem.getClass(_b).getVariables();

                    for (int i = 0; i < (int)objVars.size(); i++)
                        newList.add(objVars.at(i).name());

                    State.DefaultLoopSymbol = arg1;
                    mem.createForLoop(newList);
                }
                else if (mem.variableExists(_b) && _a == "length")
                {
                    if (mem.isString(_b))
                    {
                        State.DefaultLoopSymbol = arg1;
                        List newList;
                        string _t = mem.varString(_b);
                        int _l = _t.length();

                        for (int i = 0; i < _l; i++)
                        {
                            string tmpStr("");
                            tmpStr.push_back(_t[i]);
                            newList.add(tmpStr);
                        }

                        mem.createForLoop(newList);
                    }
                }
                else
                {
                    if (_b.length() != 0 && _a.length() != 0)
                    {
                        if (mem.variableExists(_b))
                        {
                            if (_a == "get_dirs")
                            {
                                if (Env::directoryExists(mem.varString(_b)))
                                {
                                    State.DefaultLoopSymbol = arg1;
                                    mem.createForLoop(getDirectoryList(_b, false));
                                }
                                else
                                {
                                    error(ErrorMessage::READ_FAIL, mem.varString(_b), false);
                                    mem.createFailedForLoop();
                                }
                            }
                            else if (_a == "get_files")
                            {
                                if (Env::directoryExists(mem.varString(_b)))
                                {
                                    State.DefaultLoopSymbol = arg1;
                                    mem.createForLoop(getDirectoryList(_b, true));
                                }
                                else
                                {
                                    error(ErrorMessage::READ_FAIL, mem.varString(_b), false);
                                    mem.createFailedForLoop();
                                }
                            }
                            else if (_a == "read")
                            {
                                if (Env::fileExists(mem.varString(_b)))
                                {
                                    List newList;

                                    ifstream file(mem.varString(_b).c_str());
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
                                        mem.createForLoop(newList);
                                    }
                                    else
                                    {
                                        error(ErrorMessage::READ_FAIL, mem.varString(_b), false);
                                        mem.createFailedForLoop();
                                    }
                                }
                            }
                            else
                            {
                                error(ErrorMessage::METHOD_UNDEFINED, _a, false);
                                mem.createFailedForLoop();
                            }
                        }
                        else
                        {
                            error(ErrorMessage::VAR_UNDEFINED, _b, false);
                            mem.createFailedForLoop();
                        }
                    }
                }
            }
            else
            {
                error(ErrorMessage::INVALID_OP, s, false);
                mem.createFailedForLoop();
            }
        }
        else
        {
            error(ErrorMessage::INVALID_OP, s, false);
            mem.createFailedForLoop();
        }
    }
    else if (arg0 == "while")
    {
        if (mem.variableExists(arg1) && mem.variableExists(arg3))
        {
            if (mem.isNumber(arg1) && mem.isNumber(arg3))
            {
                if (arg2 == "<" || arg2 == "<=" || arg2 == ">=" || arg2 == ">" || arg2 == "==" || arg2 == "!=")
                    mem.createWhileLoop(arg1, arg2, arg3);
                else
                {
                    error(ErrorMessage::INVALID_OP, s, false);
                    mem.createFailedWhileLoop();
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, arg1 + arg2 + arg3, false);
                mem.createFailedWhileLoop();
            }
        }
        else if (isNumeric(arg3) && mem.variableExists(arg1))
        {
            if (mem.isNumber(arg1))
            {
                if (arg2 == "<" || arg2 == "<=" || arg2 == ">=" || arg2 == ">" || arg2 == "==" || arg2 == "!=")
                    mem.createWhileLoop(arg1, arg2, arg3);
                else
                {
                    error(ErrorMessage::INVALID_OP, s, false);
                    mem.createFailedWhileLoop();
                }
            }
            else
            {
                error(ErrorMessage::CONV_ERR, arg1 + arg2 + arg3, false);
                mem.createFailedWhileLoop();
            }
        }
        else if (isNumeric(arg1) && isNumeric(arg3))
        {
            if (arg2 == "<" || arg2 == "<=" || arg2 == ">=" || arg2 == ">" || arg2 == "==" || arg2 == "!=")
                mem.createWhileLoop(arg1, arg2, arg3);
            else
            {
                error(ErrorMessage::INVALID_OP, s, false);
                mem.createFailedWhileLoop();
            }
        }
        else
        {
            error(ErrorMessage::INVALID_OP, s, false);
            mem.createFailedWhileLoop();
        }
    }
    else
        Env::sysExec(s, command);
}

#endif
