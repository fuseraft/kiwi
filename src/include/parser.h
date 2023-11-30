#include "prototypes.h"
#ifndef PARSER_H
#define PARSER_H

/**
    The heart of it all. Parse a string and send for interpretation.
**/
void parse(std::string s)
{
    std::vector<std::string> command;  // a tokenized command container
    State.CurrentLine = s; // store a copy of the current line

    StringContainer stringContainer; // contains separate commands
    std::string bigString("");       // a string to build upon

    int length = s.length(), //	length of the line
        count = 0,           // command token counter
        size = 0;            // final size of tokenized command container
    bool quoted = false,     // flag: parsing string literals
        broken = false,      // flag: end of a command
        uncomment = false,   // flag: end a command
        parenthesis = false; // flag: parsing contents within parentheses

    command.push_back(""); // push back an empty string to begin.
    // iterate each char in the initial string
    char prevChar = 'a';     // previous character in string

    tokenize(length, s, parenthesis, quoted, command, count, prevChar, bigString, uncomment, broken, stringContainer);

    size = (int)command.size();

    if (State.IsCommented)
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

                std::string commentString("");
                parse_commentstring(bigString, commentString);
                
                if (!broken)
                {
                    parse(ltrim_ws(commentString));
                }
                else
                {
                    stringContainer.add(ltrim_ws(commentString));
                    parse_stringcontainer(stringContainer);
                }
            }
        }
    }
    else if (!broken)
    {
        parse_args(size, command);

        if (State.DefiningSwitchBlock)
        {
            parse_switchstatement(s, command);
        }
        else if (State.DefiningModule)
        {
            parse_moduledefinition(s);
        }
        else if (State.DefiningScript)
        {
            parse_scriptdefinition(s);
        }
        else if (State.RaiseCatchBlock)
        {
            if (s == Keywords.Catch)
                State.RaiseCatchBlock = false;
        }
        else if (State.ExecutedTryBlock && s == Keywords.Catch)
            State.SkipCatchBlock = true;
        else if (State.ExecutedTryBlock && State.SkipCatchBlock)
        {
            if (s == Keywords.Caught)
            {
                State.SkipCatchBlock = false;
                parse(Keywords.Caught);
            }
        }
        else if (State.DefiningMethod)
        {
            parse_method_def(s);
        }
        else if (State.DefiningIfStatement)
        {
            parse_ifstatement_def(command, s, size);
        }
        else if (State.DefiningWhileLoop)
        {
            parse_whileloop_def(command, s);
        }
        else if (State.DefiningForLoop)
        {
            parse_forloop_def(command, s);
        }
        else
        {
            parse_default(size, command, s);
        }
    }
    else
    {
        stringContainer.add(bigString);
        parse_stringcontainer(stringContainer);
    }
}

std::string get_parsed_stdout(std::string cmd)
{
    State.CaptureParse = true;
    parse(cmd);
    std::string ret = State.ParsedOutput;
    State.ParsedOutput.clear();
    State.CaptureParse = false;

    return ret.length() == 0 ? State.LastValue : ret;
}

void parse_stringcontainer(StringContainer &stringContainer)
{
    for (int i = 0; i < (int)stringContainer.get().size(); i++)
        parse(stringContainer.at(i));
}

void parse_commentstring(std::string &bigString, std::string &commentString)
{
    bool commentFound = false;

    for (int i = 0; i < (int)bigString.length(); i++)
    {
        if (bigString[i] == '#')
            commentFound = true;

        if (!commentFound)
            commentString.push_back(bigString[i]);
    }
}

void parse_method_def(std::string &s)
{
    if (contains(s, Keywords.While))
        State.DefiningLocalWhileLoop = true;

    if (contains(s, Keywords.Switch))
        State.DefiningLocalSwitchBlock = true;

    if (State.DefiningParameterizedMethod)
    {
        if (s == Keywords.End)
        {
            if (State.DefiningLocalWhileLoop)
            {
                State.DefiningLocalWhileLoop = false;

                if (State.DefiningClass)
                    mem.getClass(State.CurrentClass).addToCurrentMethod(s);
                else
                    mem.addToCurrentMethod(s);
            }
            else if (State.DefiningLocalSwitchBlock)
            {
                State.DefiningLocalSwitchBlock = false;

                if (State.DefiningClass)
                    mem.getClass(State.CurrentClass).addToCurrentMethod(s);
                else
                    mem.addToCurrentMethod(s);
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
            std::string freshLine("");
            preparse_line_classdef(s, freshLine);

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
        if (s == Keywords.End)
        {
            if (State.DefiningLocalWhileLoop)
            {
                State.DefiningLocalWhileLoop = false;

                if (State.DefiningClass)
                    mem.addToCurrentClassMethod(s);
                else
                    mem.addToCurrentMethod(s);
            }
            else if (State.DefiningLocalSwitchBlock)
            {
                State.DefiningLocalSwitchBlock = false;

                if (State.DefiningClass)
                    mem.addToCurrentClassMethod(s);
                else
                    mem.addToCurrentMethod(s);
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
                parse_class_decl(s);
            }
            else
            {
                if (State.DefiningClassMethod)
                {
                    parse_classmethod_decl(s);
                }
                else
                    mem.addToCurrentMethod(s);
            }
        }
    }
}

void parse_ifstatement_def(std::vector<std::string> &command, std::string &s, int size)
{
    if (State.DefiningNest)
    {
        parse_nestedif_def(command, s);
    }
    else
    {
        if (command.at(0) == Keywords.If)
        {
            State.DefiningNest = true;

            if (size == 4)
                threeSpace(Keywords.If, command.at(1), command.at(2), command.at(3), command);
            else
            {
                mem.createIfStatement(false);
                State.DefiningNest = false;
            }
        }
        else if (command.at(0) == Keywords.Endif)
        {
            parse_ifstatement();
        }
        else if (command.at(0) == Keywords.Elsif)
        {
            if (size == 4)
                threeSpace(Keywords.If, command.at(1), command.at(2), command.at(3), command);
            else
                mem.createIfStatement(false);
        }
        else if (s == Keywords.Else)
            threeSpace(Keywords.If, Keywords.True, Operators.Equal, Keywords.True, command);
        else if (s == Keywords.Failif)
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

void parse_nestedif_def(std::vector<std::string> &command, std::string &s)
{
    if (command.at(0) == Keywords.Endif)
        exec.executeNest(mem.getIfStatement(mem.getIfStatementCount() - 1).getNest());
    else
        mem.getIfStatement(mem.getIfStatementCount() - 1).inNest(s);
}

void parse_whileloop_def(std::vector<std::string> &command, std::string &s)
{
    if (command.at(0) == Keywords.End)
        parse_whileloops();
    else
        mem.getWhileLoop(mem.getWhileLoopCount() - 1).add(s);
}

void parse_forloop_def(std::vector<std::string> &command, std::string &s)
{
    // TODO: I want to use `next` as `continue`.
    // `next if {condition}`
    // `next`
    if (command.at(0) == Keywords.Next || command.at(0) == Keywords.EndFor)
    {
        parse_forloop();
    }
    else
    {
        mem.addToCurrentForLoop(s);
    }
}

void parse_default(int size, std::vector<std::string> &command, std::string &s)
{
    if (size == 1)
    {
        parse_0space(command, s);
    }
    else if (size == 2)
    {
        parse_1space(command, s);
    }
    else if (size == 3)
    {
        parse_2space(command, s);
    }
    else if (size == 4)
        parse_3space(command);
    else if (size == 5)
    {
        parse_4space(command, s);
    }
    else
        Env::shellExec(s, command);
}

void parse_4space(std::vector<std::string> &command, std::string &s)
{
    // for each in
    if (command.at(0) == Keywords.For)
    {
        if (has_params(command.at(4)))
        {
            State.DefaultLoopSymbol = command.at(4);
            State.DefaultLoopSymbol = subtract_char(State.DefaultLoopSymbol, '(');
            State.DefaultLoopSymbol = subtract_char(State.DefaultLoopSymbol, ')');

            threeSpace(command.at(0), command.at(1), command.at(2), command.at(3), command);
            State.DefaultLoopSymbol = "$";
        }
        else
            Env::shellExec(s, command);
    }
    else
        Env::shellExec(s, command);
}

void parse_3space(std::vector<std::string> &command)
{
    threeSpace(command.at(0), command.at(1), command.at(2), command.at(3), command);
}

void parse_2space(std::vector<std::string> &command, std::string &s)
{
    // TODO: refactor
    if (unrecognized_2space(command.at(1)))
    {
        if (command.at(0) == Keywords.FileAppend)
            FileIO::appendText(command.at(1), command.at(2), false);
        else if (command.at(0) == Keywords.FileAppendLine)
            FileIO::appendText(command.at(1), command.at(2), true);
        else if ((command.at(0) == Keywords.FileWrite))
            FileIO::writeText(command.at(1), command.at(2));
        else if (command.at(0) == Keywords.Redefine)
            mem.redefine(command.at(1), command.at(2));
        else if (command.at(0) == Keywords.Loop)
        {
            if (has_params(command.at(2)))
            {
                State.DefaultLoopSymbol = command.at(2);
                State.DefaultLoopSymbol = subtract_char(State.DefaultLoopSymbol, '(');
                State.DefaultLoopSymbol = subtract_char(State.DefaultLoopSymbol, ')');

                oneSpace(command.at(0), command.at(1), command);
                State.DefaultLoopSymbol = "$";
            }
            else
                Env::shellExec(s, command);
        }
        else
            Env::shellExec(s, command);
    }
    else
        twoSpace(command.at(0), command.at(1), command.at(2), command);
}

void parse_1space(std::vector<std::string> &command, std::string &s)
{
    if (unrecognized_1space(command.at(0)))
        Env::shellExec(s, command);
    else
    {
        oneSpace(command.at(0), command.at(1), command);
    }
}

void parse_0space(std::vector<std::string> &command, std::string &s)
{
    if (unrecognized_0space(command.at(0)))
    {
        std::string before(before_dot(s)), after(after_dot(s));

        if (before.length() != 0 && after.length() != 0)
        {
            if (mem.classExists(before) && after.length() != 0)
            {
                if (has_params(after))
                {
                    s = subtract_char(s, '"');

                    if (mem.getClass(before).hasMethod(before_params(after)))
                        exec.executeTemplate(mem.getClass(before).getMethod(before_params(after)), parse_params(after));
                    else
                        Env::shellExec(s, command);
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
                else if (after == Keywords.GC)
                    mem.getClass(before).clear();
                else
                    error(ErrorMessage::UNDEFINED, "", false);
            }
            else
            {
                if (before == Keywords.Env)
                {
                    internal_env_builtins("", after, 3);
                }
                else if (mem.variableExists(before))
                {
                    if (after == Keywords.Clear)
                        parse(before + " = State.Null");
                }
                else if (mem.listExists(before))
                {
                    // REFACTOR HERE
                    if (after == Keywords.Clear)
                        mem.getList(before).clear();
                    else if (after == Keywords.Sort)
                        mem.getList(before).sort();
                    else if (after == Keywords.Reverse)
                        mem.getList(before).reverse();
                    else if (after == Keywords.Revert)
                        mem.getList(before).revert();
                }
                else if (before == Keywords.Self)
                {
                    if (State.ExecutedMethod)
                        exec.executeMethod(mem.getClass(State.CurrentMethodClass).getMethod(after));
                }
                else
                    Env::shellExec(s, command);
            }
        }
        else if (ends_with(s, "::"))
        {
            if (State.CurrentScript != "")
            {
                std::string newMark(s);
                newMark = subtract_string(s, "::");
                mem.getScript().addMark(newMark);
            }
        }
        else if (mem.methodExists(s))
            exec.executeMethod(mem.getMethod(s));
        else if (begins_with(s, "[") && ends_with(s, "]"))
        {
            mem.createModule(s);
        }
        else
        {
            s = subtract_char(s, '"');

            if (mem.methodExists(before_params(s)))
                exec.executeTemplate(mem.getMethod(before_params(s)), parse_params(s));
            else
                Env::shellExec(s, command);
        }
    }
    else
        zeroSpace(command.at(0), command);
}

void parse_ifstatement()
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

void preparse_line_classdef(std::string &s, std::string &freshLine)
{
    int _len = s.length();
    std::vector<std::string> words;
    std::string word("");

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
}

void parse_classmethod_decl(std::string &s)
{
    mem.addToCurrentClassMethod(s);

    if (State.DefiningPublicCode)
        mem.getClass(mem.getClassCount() - 1).setPublic();
    else if (State.DefiningPrivateCode)
        mem.getClass(mem.getClassCount() - 1).setPrivate();
    else
        mem.getClass(mem.getClassCount() - 1).setPublic();
}

void parse_class_decl(std::string &s)
{
    mem.addToCurrentClassMethod(s);

    if (State.DefiningPublicCode)
        mem.getClass(mem.getClassCount() - 1).setPublic();
    else if (State.DefiningPrivateCode)
        mem.getClass(mem.getClassCount() - 1).setPrivate();
    else
        mem.getClass(mem.getClassCount() - 1).setPublic();
}

void parse_scriptdefinition(std::string &s)
{
    if (s == Keywords.EndInlineScript)
    {
        State.CurrentScriptName = "";
        State.DefiningScript = false;
    }
    else
        Env::appendToFile(State.CurrentScriptName, s + "\n");
}

void parse_moduledefinition(std::string &s)
{
    if (s == ("[/" + State.CurrentModule + "]"))
    {
        State.DefiningModule = false;
        State.CurrentModule = "";
    }
    else
        mem.getModule(State.CurrentModule).add(s);
}

void parse_switchstatement(std::string &s, std::vector<std::string> &command)
{
    if (begins_with(s, Keywords.Case))
        mem.getMainSwitch().addCase(command.at(1));
    else if (s == Keywords.Default)
        State.InDefaultCase = true;
    else if (s == Keywords.End)
    {
        std::string switch_value("");

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

void parse_args(int size, std::vector<std::string> &command)
{
    for (int i = 0; i < size; i++)
    {
        // handle arguments
        // args[0], args[1], ..., args[n-1]
        if (contains(command.at(i), Keywords.Args) && command.at(i) != Keywords.ArgValues)
        {
            std::vector<std::string> params = parse_bracketrange(command.at(i));

            if (is_numeric(params.at(0)))
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
}

void parse_forloop()
{
    State.DefiningForLoop = false;

    for (int i = 0; i < mem.getForLoopCount(); i++)
        if (mem.getForLoop(i).isForLoop())
            exec.executeForLoop(mem.getForLoop(i));

    mem.clearFor();

    State.ForLoopCount = 0;
}

template<typename condition>
void parse_whileloop(std::string v1, std::string v2, condition cond)
{
    while (cond(v1, v2))
    {
        exec.executeWhileLoop(mem.getWhileLoop(mem.getWhileLoopCount() - 1));

        if (State.Breaking)
            break;
    }

    mem.clearWhile();

    State.WhileLoopCount = 0;
}

void parse_whileloops()
{
    State.DefiningWhileLoop = false;

    std::string v1 = mem.getWhileLoop(mem.getWhileLoopCount() - 1).valueOne(),
            v2 = mem.getWhileLoop(mem.getWhileLoopCount() - 1).valueTwo(),
            op = mem.getWhileLoop(mem.getWhileLoopCount() - 1).logicOperator();

    if (mem.variableExists(v1) && mem.variableExists(v2))
    {
        if (op == Operators.Equal)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) == mem.varNumber(v2);
            });
        }
        else if (op == Operators.LessThan)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) < mem.varNumber(v2);
            });
        }
        else if (op == Operators.GreaterThan)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) > mem.varNumber(v2);
            });
        }
        else if (op == Operators.LessThanOrEqual)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) <= mem.varNumber(v2);
            });
        }
        else if (op == Operators.GreaterThanOrEqual)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) >= mem.varNumber(v2);
            });
        }
        else if (op == Operators.NotEqual)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) != mem.varNumber(v2);
            });
        }
    }
    else if (mem.variableExists(v1))
    {
        if (op == Operators.Equal)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) == stoi(v2);
            });
        }
        else if (op == Operators.LessThan)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) < stoi(v2);
            });
        }
        else if (op == Operators.GreaterThan)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) > stoi(v2);
            });
        }
        else if (op == Operators.LessThanOrEqual)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) <= stoi(v2);
            });
        }
        else if (op == Operators.GreaterThanOrEqual)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) >= stoi(v2);
            });
        }
        else if (op == Operators.NotEqual)
        {
            parse_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return mem.varNumber(v1) != stoi(v2);
            });
        }
    }
}

void tokenize(int length, std::string &s, bool &parenthesis, bool &quoted, std::vector<std::string> &command, int &count, char &prevChar, std::string &bigString, bool &uncomment, bool &broken, StringContainer &stringContainer)
{
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
}

void zeroSpace(std::string arg0, std::vector<std::string> command)
{
    if (arg0 == Keywords.Pass)
    {
        return;
    }
    else if (arg0 == Keywords.Caught)
    {
        handleCaught();
    }
    else if (arg0 == Keywords.Exit)
    {
        handleExit();
    }
    else if (arg0 == Keywords.Break)
        State.Breaking = true;
    else if (arg0 == Keywords.End)
    {
        handleEnd();
    }
    else if (arg0 == Keywords.Parser)
        load_repl();
    else if (arg0 == Keywords.Private)
    {
        handlePrivateDecl();
    }
    else if (arg0 == Keywords.Public)
    {
        handlePublicDecl();
    }
    else if (arg0 == Keywords.Try)
        State.ExecutedTryBlock = true;
    else if (arg0 == Keywords.Failif)
    {
        handleFailedIfStatement();
    }
    else
        Env::shellExec(arg0, command);
}

void oneSpace(std::string arg0, std::string arg1, std::vector<std::string> command)
{
    std::string before(before_dot(arg1)), after(after_dot(arg1));

    // Refactor
    if (contains(arg1, Keywords.SelfDot))
    {
        arg1 = replace(arg1, Keywords.Self, State.CurrentMethodClass);
    }

    if (arg0 == Keywords.GC)
    {
        parse_clear(arg1);
    }
    else if (arg0 == Keywords.Switch)
    {
        handleSwitch(arg1);
    }
    else if (arg0 == Keywords.Goto)
    {
        handleGoto(arg1);
    }
    else if (arg0 == Keywords.If)
    {
        handleIfStatement(arg1);
    }
    else if (arg0 == Keywords.Prompt)
    {
        handlePrompt(arg1);
    }
    else if (arg0 == Keywords.Err)
    {
        handleErr(arg1);
    }
    else if (arg0 == Keywords.Delay)
    {
        handleDelay(arg1);
    }
    else if (arg0 == Keywords.Loop)
        threeSpace(Keywords.For, Keywords.Each, Keywords.In, arg1, command);
    else if (arg0 == Keywords.For && arg1 == Keywords.Infinity)
        mem.createForLoop();
    else if (arg0 == Keywords.Remove)
    {
        handleRemove(arg1);
    }
    else if (arg0 == Keywords.BeginInlineScript)
    {
        handleInlineScriptDecl(arg1);
    }
    else if (arg0 == Keywords.Globalize)
    {
        mem.globalize(arg1);
    }
    else if (arg0 == Keywords.Load)
    {
        handleLoad(arg1);
    }
    else if (arg0 == Keywords.Print || arg0 == Keywords.PrintLn)
    {
        internal_puts(arg0, arg1, arg0 == Keywords.PrintLn);
    }
    else if (arg0 == Keywords.ChangeDirectory)
    {
        handleChangeDir(arg1);
    }
    else if (arg0 == Keywords.List)
    {
        handleListDecl(arg1);
    }
    else if (arg0 == Keywords.InlineParse)
    {
        handleInlineParse(arg1);
    }
    else if (arg0 == Keywords.ShellExec)
    {
        handleInlineShellExec(arg1, command);
    }
    else if (arg0 == Keywords.InitialDirectory)
    {
        handleInitialDir(arg1);
    }
    else if (arg0 == Keywords.IsMethod)
    {
        handleMethodInspect(before, after, arg1);
    }
    else if (arg0 == Keywords.IsClass)
    {
        handleClassInspect(arg1);
    }
    else if (arg0 == Keywords.IsVariable)
    {
        handleVariableInspect(before, after, arg1);
    }
    else if (arg0 == Keywords.IsList)
    {
        handleListInspect(arg1);
    }
    else if (arg0 == Keywords.IsDirectory)
    {
        handleDirectoryInspect(before, after, arg1);
    }
    else if (arg0 == Keywords.IsFile)
    {
        handleFileInspect(before, after, arg1);
    }
    else if (arg0 == Keywords.IsCollectable)
    {
        handleCollectInspect(arg1);
    }
    else if (arg0 == Keywords.IsNumber)
    {
        handleNumberInspect(before, after, arg1);
    }
    else if (arg0 == Keywords.IsString)
    {
        handleStringInspect(before, after, arg1);
    }
    else if (arg0 == Keywords.Template)
    {
        handleTemplateDecl(arg1);
    }
    else if (arg0 == Keywords.Lock)
    {
        handleLockAssignment(arg1);
    }
    else if (arg0 == Keywords.Unlock)
    {
        handleUnlockAssignment(arg1);
    }
    else if (arg0 == Keywords.Method || arg0 == Keywords.LockedMethod)
    {
        mem.createMethod(arg0, arg1);
    }
    else if (arg0 == Keywords.InvokeMethod)
    {
        exec.executeMethod(arg1, before, after);
    }
    else if (arg0 == Keywords.Class)
    {
        mem.createClass(arg1);
    }
    else if (arg0 == Keywords.CreateFile)
    {
        handleFilePush(arg1);
    }
    else if (arg0 == Keywords.RemoveFile)
    {
        handleFilePop(arg1);
    }
    else if (arg0 == Keywords.CreateDirectory)
    {
        handleDirPush(arg1);
    }
    else if (arg0 == Keywords.RemoveDirectory)
    {
        handleDirPop(arg1);
    }
    else
        Env::shellExec(arg0, command);
}

void twoSpace(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command)
{
    std::string last_val = "";

    if (contains(arg2, Keywords.SelfDot))
        arg2 = replace(arg2, Keywords.Self, State.CurrentMethodClass);

    if (contains(arg0, Keywords.SelfDot))
        arg0 = replace(arg0, Keywords.Self, State.CurrentMethodClass);

    if (mem.variableExists(arg0))
    {
        initializeVariable(arg0, arg1, arg2, command);
    }
    else if (mem.listExists(arg0) || mem.listExists(before_brackets(arg0)))
    {
        init_listvalues(arg0, arg1, arg2, command);
    }
    else
    {
        if (begins_with(arg0, "@") && is_dotless(arg0))
        {
            init_globalvar(arg0, arg1, arg2, command);
        }
        else if (begins_with(arg0, "@") && !is_dotless(arg2))
        {
            init_classvar(arg0, arg1, arg2, command);
        }
        else if (!mem.classExists(arg0) && mem.classExists(arg2))
        {
            copy_class(arg0, arg1, arg2, command);
        }
        else if (valid_const_name(arg0))
        {
            init_const(arg0, arg1, arg2);
        }
        else
        {
            exec.executeSimpleStatement(arg0, arg1, arg2);
        }
    }
}

void threeSpace(std::string arg0, std::string arg1, std::string arg2, std::string arg3, std::vector<std::string> command)
{
    // isNumber(arg3)
    // isString(arg3)

    if (arg0 == Keywords.Class)
    {
        handleClassDecl(arg1, arg3, arg2);
    }
    else if (arg0 == Keywords.If)
    {
        checkCondition(arg1, arg2, arg3);
    }
    else if (arg0 == Keywords.For)
    {
        if (arg2 == Operators.LessThan || arg2 == Operators.GreaterThan || arg2 == Operators.LessThanOrEqual || arg2 == Operators.GreaterThanOrEqual)
        {
            handleLoopInit_For(arg1, arg2, arg3, arg0);
        }
        else if (arg2 == Keywords.In)
        {
            bool retFlag;
            handleLoopInit_ForIn(arg1, arg3, arg0, retFlag);
            if (retFlag)
                return;
        }
        else
        {
            error(ErrorMessage::INVALID_OP, arg0, false);
            mem.createFailedForLoop();
        }
    }
    else if (arg0 == Keywords.While)
    {
        handleLoopInit_While(arg1, arg3, arg2, arg0);
    }
    else
        Env::shellExec(arg0, command);
}

void handleLoopInit_For(std::string &arg1, std::string &arg2, std::string &arg3, std::string &arg0)
{
    double first = 0, second = 0;
    bool failed = false;
    if (mem.variableExists(arg1) && mem.variableExists(arg3) && (mem.isNumber(arg1) && mem.isNumber(arg3)))
    {
        first = mem.varNumber(arg1);
        second = mem.varNumber(arg3);
    }
    else if (mem.variableExists(arg1) && !mem.variableExists(arg3) && (mem.isNumber(arg1) && is_numeric(arg3)))
    {
        first = mem.varNumber(arg1);
        second = stod(arg3);
    }
    else if (!mem.variableExists(arg1) && mem.variableExists(arg3) && (is_numeric(arg1) && mem.isNumber(arg3)))
    {
        first = stod(arg1);
        second = mem.varNumber(arg3);
    }
    else if (is_numeric(arg1) && is_numeric(arg3))
    {
        first = stod(arg1);
        second = stod(arg3);
    }
    else
    {
        error(ErrorMessage::CONV_ERR, arg0, false);
        mem.createFailedForLoop();
        failed = true;
    }

    if (failed) return;

    if ((arg2 == Operators.LessThan && first < second)
        || (arg2 == Operators.GreaterThan && first > second)
        || (arg2 == Operators.LessThanOrEqual && first <= second)
        || (arg2 == Operators.GreaterThanOrEqual && first >= second))
        mem.createForLoop(first, second, arg2);
    else
        mem.createFailedForLoop();
}

void handleLoopInit_ForIn(std::string &arg1, std::string &arg3, std::string &arg0, bool &retFlag)
{
    retFlag = true;
    if (arg1 == Keywords.Each)
    {
        std::string before(before_dot(arg3)), after(after_dot(arg3));

        if (before == Keywords.Args && after == Keywords.Values)
        {
            handleLoopInit_CommandLineArgs();
        }
        else if (before == Keywords.Env && after == Keywords.InternalVariables)
        {
            handleLoopInit_Environment_BuiltIns();
        }
        else if (mem.classExists(before) && after == Keywords.InternalMethods)
        {
            handleLoopInit_ClassMembers_Methods(before);
        }
        else if (mem.classExists(before) && after == Keywords.InternalVariables)
        {
            handleLoopInit_ClassMembers_Variables(before);
        }
        else if (mem.variableExists(before) && after == Keywords.Size)
        {
            if (mem.isString(before))
            {
                handleLoopInit_Variable_Length(before);
            }
        }
        else
        {
            if (before.length() != 0 && after.length() != 0)
            {
                if (mem.variableExists(before))
                {
                    if (after == Keywords.GetDirectories)
                    {
                        handleLoopInit_Variable_Directories(before);
                    }
                    else if (after == Keywords.GetFiles)
                    {
                        handleLoopInit_Variable_Files(before);
                    }
                    else if (after == Keywords.Read)
                    {
                        handleLoopInit_Variable_FileRead(before);
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
    else if (has_params(arg3))
    {
        handleLoopInit_Params(arg3, arg1);
    }
    else if (has_brackets(arg3))
    {
        bool retFlag;
        handleLoopInit_Brackets(arg3, arg1, retFlag);
        if (retFlag)
            return;
    }
    else if (mem.listExists(arg3))
    {
        State.DefaultLoopSymbol = arg1;
        mem.createForLoop(mem.getList(arg3));
    }
    else if (!is_dotless(arg3))
    {
        State.DefaultLoopSymbol = arg1;
        std::string _b(before_dot(arg3)), _a(after_dot(arg3));

        if (_b == Keywords.Args && _a == Keywords.Values)
        {
            handleLoopInit_CommandLineArgs();
        }
        else if (_b == Keywords.Env && _a == Keywords.InternalVariables)
        {
            handleLoopInit_Environment_BuiltIns();
        }
        else if (mem.classExists(_b) && _a == Keywords.InternalMethods)
        {
            handleLoopInit_ClassMembers_Methods(_b);
        }
        else if (mem.classExists(_b) && _a == Keywords.InternalVariables)
        {
            handleLoopInit_ClassMembers_Variables(_b);
        }
        else if (mem.variableExists(_b) && _a == Keywords.Size)
        {
            handleLoopInit_Variable_Length(_b);
        }
        else
        {
            if (_b.length() != 0 && _a.length() != 0)
            {
                if (mem.variableExists(_b))
                {
                    if (_a == Keywords.GetDirectories)
                    {
                        handleLoopInit_Variable_Directories(_b);
                    }
                    else if (_a == Keywords.GetFiles)
                    {
                        handleLoopInit_Variable_Files(_b);
                    }
                    else if (_a == Keywords.Read)
                    {
                        handleLoopInit_Variable_FileRead(_b);
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
        error(ErrorMessage::INVALID_OP, arg0, false);
        mem.createFailedForLoop();
    }
    retFlag = false;
}

void handleLoopInit_Environment_BuiltIns()
{
    List newList;

    newList.add(Keywords.CurrentDirectory);
    newList.add(Keywords.UslangApp);
    newList.add(Keywords.CurrentUser);
    newList.add(Keywords.CurrentMachine);
    newList.add(Keywords.InitialDirectory);
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
    newList.add(Keywords.LastError);
    newList.add(Keywords.LastValue);
    mem.createForLoop(newList);
}

void handleLoopInit_Brackets(std::string &arg3, std::string &arg1, bool &retFlag)
{
    retFlag = true;
    std::string before(before_brackets(arg3));

    if (!mem.variableExists(before) || !mem.isString(before))
    {
        error(ErrorMessage::NULL_STRING, before, false);
        mem.createFailedForLoop();
        return;
    }

    std::string tempVarString(mem.varString(before));

    std::vector<std::string> range = parse_bracketrange(arg3);

    if (range.size() != 2)
    {
        error(ErrorMessage::OUT_OF_BOUNDS, arg3, false);
        return;
    }

    std::string rangeBegin(range.at(0)), rangeEnd(range.at(1));

    if ((rangeBegin.length() == 0 || rangeEnd.length() == 0) || !(is_numeric(rangeBegin) && is_numeric(rangeEnd)))
    {
        error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
        return;
    }

    if (stoi(rangeBegin) < stoi(rangeEnd))
    {
        if ((int)tempVarString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
        {
            List newList("&l&i&s&t&");

            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
            {
                std::string tempString("");
                tempString.push_back(tempVarString[i]);
                newList.add(tempString);
            }

            State.DefaultLoopSymbol = arg1;

            mem.createForLoop(newList);

            mem.removeList("&l&i&s&t&");
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
    }
    else if (stoi(rangeBegin) > stoi(rangeEnd))
    {
        if ((int)tempVarString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)
        {
            List newList("&l&i&s&t&");

            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
            {
                std::string tempString("");
                tempString.push_back(tempVarString[i]);
                newList.add(tempString);
            }

            State.DefaultLoopSymbol = arg1;

            mem.createForLoop(newList);

            mem.removeList("&l&i&s&t&");
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
    }
    else
        error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
    retFlag = false;
}

void handleLoopInit_Params(std::string &arg3, std::string &arg1)
{
    std::vector<std::string> rangeSpecifiers;

    rangeSpecifiers = parse_range(arg3);

    if (rangeSpecifiers.size() == 2)
    {
        std::string firstRangeSpecifier(rangeSpecifiers.at(0)), lastRangeSpecifier(rangeSpecifiers.at(1));

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

        if (is_numeric(firstRangeSpecifier) && is_numeric(lastRangeSpecifier))
        {
            State.DefaultLoopSymbol = arg1;

            int ifrs = stoi(firstRangeSpecifier), ilrs(stoi(lastRangeSpecifier));

            if (ifrs < ilrs)
                mem.createForLoop(stod(firstRangeSpecifier), stod(lastRangeSpecifier), Operators.LessThanOrEqual);
            else if (ifrs > ilrs)
                mem.createForLoop(stod(firstRangeSpecifier), stod(lastRangeSpecifier), Operators.GreaterThanOrEqual);
            else
                mem.createFailedForLoop();
        }
        else
            mem.createFailedForLoop();
    }
}

void handleLoopInit_Variable_FileRead(std::string &before)
{
    if (Env::fileExists(mem.varString(before)))
    {
        List newList;

        std::ifstream file(mem.varString(before).c_str());
        std::string line("");

        if (file.is_open())
        {
            while (!file.eof())
            {
                std::getline(file, line);
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

void handleLoopInit_Variable_Files(std::string &before)
{
    if (Env::directoryExists(mem.varString(before)))
        mem.createForLoop(getDirectoryList(before, true));
    else
    {
        error(ErrorMessage::READ_FAIL, mem.varString(before), false);
        mem.createFailedForLoop();
    }
}

void handleLoopInit_Variable_Directories(std::string &before)
{
    if (Env::directoryExists(mem.varString(before)))
        mem.createForLoop(getDirectoryList(before, false));
    else
    {
        error(ErrorMessage::READ_FAIL, mem.varString(before), false);
        mem.createFailedForLoop();
    }
}

void handleLoopInit_Variable_Length(std::string &before)
{
    List newList;
    std::string tempVarStr = mem.varString(before);
    int len = tempVarStr.length();

    for (int i = 0; i < len; i++)
    {
        std::string tempStr("");
        tempStr.push_back(tempVarStr[i]);
        newList.add(tempStr);
    }

    mem.createForLoop(newList);
}

void handleLoopInit_ClassMembers_Variables(std::string &before)
{
    List newList;

    std::vector<Variable> objVars = mem.getClass(before).getVariables();

    for (int i = 0; i < (int)objVars.size(); i++)
        newList.add(objVars.at(i).name());

    mem.createForLoop(newList);
}

void handleLoopInit_ClassMembers_Methods(std::string &before)
{
    List newList;

    std::vector<Method> objMethods = mem.getClass(before).getMethods();

    for (int i = 0; i < (int)objMethods.size(); i++)
        newList.add(objMethods.at(i).name());

    mem.createForLoop(newList);
}

void handleLoopInit_CommandLineArgs()
{
    List newList;

    for (int i = 0; i < mem.getArgCount(); i++)
        newList.add(mem.getArg(i));

    mem.createForLoop(newList);
}

void handleLoopInit_While(std::string &arg1, std::string &arg3, std::string &arg2, std::string &arg0)
{
    if (mem.variableExists(arg1) && mem.variableExists(arg3))
    {
        if (mem.isNumber(arg1) && mem.isNumber(arg3))
        {
            if (arg2 == Operators.LessThan || arg2 == Operators.LessThanOrEqual || arg2 == Operators.GreaterThanOrEqual || arg2 == Operators.GreaterThan || arg2 == Operators.Equal || arg2 == Operators.NotEqual)
                mem.createWhileLoop(arg1, arg2, arg3);
            else
            {
                error(ErrorMessage::INVALID_OP, arg0, false);
                mem.createFailedWhileLoop();
            }
        }
        else
        {
            error(ErrorMessage::CONV_ERR, arg1 + arg2 + arg3, false);
            mem.createFailedWhileLoop();
        }
    }
    else if (is_numeric(arg3) && mem.variableExists(arg1))
    {
        if (mem.isNumber(arg1))
        {
            if (arg2 == Operators.LessThan || arg2 == Operators.LessThanOrEqual || arg2 == Operators.GreaterThanOrEqual || arg2 == Operators.GreaterThan || arg2 == Operators.Equal || arg2 == Operators.NotEqual)
                mem.createWhileLoop(arg1, arg2, arg3);
            else
            {
                error(ErrorMessage::INVALID_OP, arg0, false);
                mem.createFailedWhileLoop();
            }
        }
        else
        {
            error(ErrorMessage::CONV_ERR, arg1 + arg2 + arg3, false);
            mem.createFailedWhileLoop();
        }
    }
    else if (is_numeric(arg1) && is_numeric(arg3))
    {
        if (arg2 == Operators.LessThan || arg2 == Operators.LessThanOrEqual || arg2 == Operators.GreaterThanOrEqual || arg2 == Operators.GreaterThan || arg2 == Operators.Equal || arg2 == Operators.NotEqual)
            mem.createWhileLoop(arg1, arg2, arg3);
        else
        {
            error(ErrorMessage::INVALID_OP, arg0, false);
            mem.createFailedWhileLoop();
        }
    }
    else
    {
        error(ErrorMessage::INVALID_OP, arg0, false);
        mem.createFailedWhileLoop();
    }
}
void handleIfStatementDecl_Generic(std::string first, std::string second, std::string oper)
{
    if (is_numeric(first) && is_numeric(second))
    {
        if (oper == Operators.Equal)
        {
            mem.createIfStatement(stod(first) == stod(second));
        }
        else if (oper == Operators.NotEqual)
        {
            mem.createIfStatement(stod(first) != stod(second));
        }
        else if (oper == Operators.LessThan)
        {
            mem.createIfStatement(stod(first) < stod(second));
        }
        else if (oper == Operators.GreaterThan)
        {
            mem.createIfStatement(stod(first) > stod(second));
        }
        else if (oper == Operators.LessThanOrEqual)
        {
            mem.createIfStatement(stod(first) <= stod(second));
        }
        else if (oper == Operators.GreaterThanOrEqual)
        {
            mem.createIfStatement(stod(first) >= stod(second));
        }
        else
        {
            error(ErrorMessage::INVALID_OPERATOR, oper, false);
            mem.createIfStatement(false);
        }
    }
    else
    {
        if (oper == Operators.Equal)
        {
            mem.createIfStatement(first == second);
        }
        else if (oper == Operators.NotEqual)
        {
            mem.createIfStatement(first != second);
        }
        else if (oper == Keywords.BeginsWith)
        {
            mem.createIfStatement(begins_with(first, second));
        }
        else if (oper == Keywords.EndsWith)
        {
            mem.createIfStatement(ends_with(first, second));
        }
        else if (oper == Keywords.Contains)
        {
            mem.createIfStatement(contains(first, second));
        }
        else
        {
            error(ErrorMessage::INVALID_OPERATOR, oper, false);
            mem.createIfStatement(false);
        }
    }
}

void handleIfStatementDecl_Method(std::string arg1, std::string arg1Result, std::string arg3, std::string arg3Result)
{
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
}

void handleClassDecl(std::string arg1, std::string arg3, std::string arg2)
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
            if (arg2 == Operators.Assign)
            {
                std::vector<Method> classMethods = mem.getClass(arg3).getMethods();
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

void handleFailedIfStatement()
{
    mem.createIfStatement(State.FailedIfStatement);
}

void checkCondition(const std::string arg1, const std::string arg2, const std::string arg3) {
    if (mem.listExists(arg1) && arg2 == Keywords.In) {
        checkListInCondition(arg1, arg2, arg3);
    }
    else if (mem.listExists(arg1) && arg2 == Keywords.Contains && arg3 != Keywords.IsList) {
        checkListContainsCondition(arg1, arg2, arg3);
    }
    else if (mem.variableExists(arg1) && mem.variableExists(arg3)) {
        checkVariableCondition(arg1, arg2, arg3);
    }
    else if ((mem.variableExists(arg1) && !mem.variableExists(arg3)) && !mem.methodExists(arg3) && mem.notClassMethod(arg3) && !has_params(arg3)) {
        checkNumericStringFileDirCondition(arg1, arg2, arg3);
    }
    else if ((mem.variableExists(arg1) && !mem.variableExists(arg3)) && !mem.methodExists(arg3) && mem.notClassMethod(arg3) && has_params(arg3)) {
        checkNumericStringFileDirCondition(arg1, arg2, getStackValue(arg3));
    }
    else if ((!mem.variableExists(arg1) && mem.variableExists(arg3)) && !mem.methodExists(arg1) && mem.notClassMethod(arg1) && !has_params(arg1)) {
        checkNumericStringFileDirCondition(arg3, arg2, arg1);
    }
    else if ((!mem.variableExists(arg1) && mem.variableExists(arg3)) && !mem.methodExists(arg1) && mem.notClassMethod(arg1) && has_params(arg1)) {
        checkNumericStringFileDirCondition(arg3, arg2, getStackValue(arg1));
    }
    else if (has_params(arg1) || has_params(arg3)) {
        checkParamsCondition(arg1, arg2, arg3);
    }
    else if ((mem.methodExists(arg1) && arg3 != Keywords.IsMethod) || mem.methodExists(arg3)) {
        checkMethodCondition(arg1, arg3, arg2);
    }
    else {
        checkGenericCondition(arg1, arg3, arg2);
    }
}

void checkNumericStringFileDirCondition(std::string arg1, std::string arg2, std::string arg3)
{
    if (mem.isNumber(arg1))
    {
        if (is_numeric(arg3))
        {
            handleIfStatementDecl_Generic(dtos(mem.varNumber(arg1)), arg3, arg2);
        }
        else if (arg3 == Keywords.IsNumber)
        {
            if (arg2 == Operators.Equal)
                mem.createIfStatement(true);
            else if (arg2 == Operators.NotEqual)
                mem.createIfStatement(false);
            else
                error(ErrorMessage::INVALID_OPERATOR, arg2, false);
        }
        else
        {
            error(ErrorMessage::CONV_ERR, arg2, false);
            mem.createIfStatement(false);
        }
    }
    else
    {
        if (arg3 == Keywords.IsString)
        {
            if (mem.isString(arg1))
            {
                if (arg2 == Operators.Equal)
                    mem.createIfStatement(true);
                else if (arg2 == Operators.NotEqual)
                    mem.createIfStatement(false);
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
            else
            {
                mem.createIfStatement(arg2 == Operators.NotEqual);
            }
        }
        else if (arg3 == Keywords.IsNumber)
        {
            if (mem.isNumber(arg1))
            {
                if (arg2 == Operators.Equal)
                    mem.createIfStatement(true);
                else if (arg2 == Operators.NotEqual)
                    mem.createIfStatement(false);
                else
                {
                    error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                    mem.createIfStatement(false);
                }
            }
            else
            {
                mem.createIfStatement(arg2 == Operators.NotEqual);
            }
        }
        else if (arg3 == Keywords.IsFile)
        {
            if (mem.isString(arg1))
            {
                if (Env::fileExists(mem.varString(arg1)))
                {
                    if (arg2 == Operators.Equal)
                        mem.createIfStatement(true);
                    else if (arg2 == Operators.NotEqual)
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    mem.createIfStatement(arg2 == Operators.NotEqual);
                }
            }
            else
            {
                error(ErrorMessage::IS_NULL, arg1, false);
                mem.createIfStatement(false);
            }
        }
        else if (arg3 == Keywords.IsDirectory)
        {
            if (mem.isString(arg1))
            {
                if (Env::directoryExists(mem.varString(arg1)))
                {
                    if (arg2 == Operators.Equal)
                        mem.createIfStatement(true);
                    else if (arg2 == Operators.NotEqual)
                        mem.createIfStatement(false);
                    else
                    {
                        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
                        mem.createIfStatement(false);
                    }
                }
                else
                {
                    mem.createIfStatement(arg2 == Operators.NotEqual);
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
            handleIfStatementDecl_Generic(mem.varString(arg1), arg3, arg2);
        }
    }
}

void checkListInCondition(const std::string listName, const std::string condition, const std::string testValue) {
    std::string testString = getTestString(mem.variableExists(testValue), listName);
    if (testString == "[none]") {
        mem.createIfStatement(false);
    }
    else {
        bool elementFound = checkListForElement(listName, testString, condition);
        mem.createIfStatement(!elementFound);
    }
}

void checkListContainsCondition(const std::string listName, const std::string condition, const std::string testValue) {
    std::string testString = getTestString(mem.variableExists(testValue), testValue);
    if (testString == "[none]") {
        mem.createIfStatement(false);
    }
    else {
        bool elementFound = checkListForElement(listName, testString, condition);
        mem.createIfStatement(!elementFound);
    }
}

bool checkListForElement(const std::string listName, const std::string testString, const std::string conditionType) {
    bool result = false;

    if (mem.listExists(listName)) {
        if (conditionType == Keywords.In) {
            result = checkListContains(listName, testString);
        }
        else if (conditionType == Keywords.Contains && testString != Keywords.IsList) {
            result = checkListContains(listName, testString);
        }
        else {
            error(ErrorMessage::INVALID_OP, conditionType, false);
        }
    }
    else {
        error(ErrorMessage::LIST_UNDEFINED, listName, false);
    }

    return result;
}

bool checkListContains(const std::string listName, const std::string testString) {
    bool elementFound = false;
    List list = mem.getList(listName);
    for (int i = 0; i < list.size(); i++)
    {
        if (list.at(i) == testString)
        {
            elementFound = true;
            mem.createIfStatement(true);
            State.LastValue = itos(i);
            break;
        }
    }

    mem.createIfStatement(elementFound);
    return elementFound;
}

void checkVariableCondition(const std::string arg1, const std::string arg2, const std::string arg3) {
    if (mem.isString(arg1) && mem.isString(arg3)) {
        handleIfStatementDecl_Generic(mem.varString(arg1), mem.varString(arg3), arg2);
    }
    else if (mem.isNumber(arg1) && mem.isNumber(arg3)) {
        handleIfStatementDecl_Generic(dtos(mem.varNumber(arg1)), dtos(mem.varNumber(arg3)), arg2);
    }
    else {
        error(ErrorMessage::CONV_ERR, arg1 + " " + arg2 + " " + arg3, false);
        mem.createIfStatement(false);
    }
}

// Continue with other conditions...

void checkParamsCondition(const std::string arg1, const std::string arg2, const std::string arg3) {
    // Implement the logic for conditions with parameters
    // ...

    // Example:
    // handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
}

void checkMethodCondition(const std::string arg1, const std::string arg3, const std::string arg2) {
    std::string arg1Result(""), arg3Result("");
    handleIfStatementDecl_Method(arg1, arg1Result, arg3, arg3Result);
    handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
}

void checkGenericCondition(const std::string arg1, const std::string arg3, const std::string arg2) {
    handleIfStatementDecl_Generic(arg1, arg3, arg2);
}

std::string getTestString(bool variableExists, const std::string variableName) {
    std::string testString("[none]");

    if (variableExists) {
        if (mem.isString(variableName))
            testString = mem.varString(variableName);
        else if (mem.isNumber(variableName))
            testString = dtos(mem.varNumber(variableName));
        else
            handleError(ErrorMessage::IS_NULL, variableName, false);
    }
    else {
        testString = variableName;
    }

    return testString;
}

void handleError(int errorType, const std::string variableName, bool isMethod) {
    error(errorType, variableName, isMethod);
    mem.createIfStatement(false);
}

void handlePublicDecl()
{
    State.DefiningPrivateCode = false;
    State.DefiningPublicCode = true;
}

void handlePrivateDecl()
{
    State.DefiningPrivateCode = true;
    State.DefiningPublicCode = false;
}

void handleEnd()
{
    State.DefiningPrivateCode = false,
    State.DefiningPublicCode = false;
    State.DefiningClass = false;
    State.DefiningClassMethod = false;
    State.CurrentClass = "";
}

void handleExit()
{
    mem.clearAll();
    exit(0);
}

void handleCaught()
{
    std::string to_remove = "remove ";
    to_remove.append(State.ErrorVarName);

    parse(to_remove);

    State.ExecutedTryBlock = false,
    State.RaiseCatchBlock = false;
    State.LastError = "";
    State.ErrorVarName = "";
}

void handleInlineScriptDecl(std::string &arg1)
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

void handleDirPop(std::string &arg1)
{
    if (mem.variableExists(arg1))
    {
        if (mem.isString(arg1))
        {
            if (Env::directoryExists(mem.varString(arg1)))
                Env::removeDirectory(mem.varString(arg1));
            else
                error(ErrorMessage::DIR_NOT_FOUND, mem.varString(arg1), false);
        }
        else
            error(ErrorMessage::NULL_STRING, arg1, false);
    }
    else
    {
        if (Env::directoryExists(arg1))
            Env::removeDirectory(arg1);
        else
            error(ErrorMessage::DIR_NOT_FOUND, arg1, false);
    }
}

void handleDirPush(std::string &arg1)
{
    if (mem.variableExists(arg1))
    {
        if (mem.isString(arg1))
        {
            if (!Env::directoryExists(mem.varString(arg1)))
                Env::makeDirectory(mem.varString(arg1));
            else
                error(ErrorMessage::DIR_EXISTS, mem.varString(arg1), false);
        }
        else
            error(ErrorMessage::NULL_STRING, arg1, false);
    }
    else
    {
        if (!Env::directoryExists(arg1))
            Env::makeDirectory(arg1);
        else
            error(ErrorMessage::DIR_EXISTS, arg1, false);
    }
}

void handleFilePop(std::string &arg1)
{
    if (mem.variableExists(arg1))
    {
        if (mem.isString(arg1))
        {
            if (Env::fileExists(mem.varString(arg1)))
                Env::removeFile(mem.varString(arg1));
            else
                error(ErrorMessage::FILE_NOT_FOUND, mem.varString(arg1), false);
        }
        else
            error(ErrorMessage::NULL_STRING, arg1, false);
    }
    else
    {
        if (Env::fileExists(arg1))
            Env::removeFile(arg1);
        else
            error(ErrorMessage::FILE_NOT_FOUND, arg1, false);
    }
}

void handleFilePush(std::string &arg1)
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

void handleUnlockAssignment(std::string &arg1)
{
    if (mem.variableExists(arg1))
        mem.getVar(arg1).setIndestructible(false);
    else if (mem.methodExists(arg1))
        mem.getMethod(arg1).setIndestructible(false);
}

void handleLockAssignment(std::string &arg1)
{
    if (mem.variableExists(arg1))
        mem.getVar(arg1).setIndestructible(true);
    else if (mem.methodExists(arg1))
        mem.getMethod(arg1).setIndestructible(true);
}

void handleTemplateDecl(std::string &arg1)
{
    if (mem.methodExists(arg1))
        error(ErrorMessage::METHOD_DEFINED, arg1, false);
    else
    {
        if (has_params(arg1))
        {
            std::vector<std::string> params = parse_params(arg1);
            Method method(before_params(arg1), true);
            method.setTemplateSize((int)params.size());
            mem.addMethod(method);
            State.DefiningMethod = true;
        }
    }
}

void handleStringInspect(std::string &before, std::string &after, std::string &arg1)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (mem.getClass(before).hasVariable(after))
        {
            if (mem.getClass(before).getVariable(after).getString() != State.Null)
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
        }
        else
            error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
    }
    else
    {
        if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
        }
        else
        {
            if (is_numeric(arg1))
                State.LastValue = Keywords.False;
            else
                State.LastValue = Keywords.True;
        }
    }
}

void handleNumberInspect(std::string &before, std::string &after, std::string &arg1)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (mem.getClass(before).hasVariable(after))
        {
            if (mem.getClass(before).getVariable(after).getNumber() != State.NullNum)
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
        }
        else
            error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
    }
    else
    {
        if (mem.variableExists(arg1))
        {
            if (mem.isNumber(arg1))
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
        }
        else
        {
            if (is_numeric(arg1))
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
        }
    }
}

void handleCollectInspect(std::string &arg1)
{
    if (mem.variableExists(arg1))
    {
        if (mem.getVar(arg1).isCollectable())
            State.LastValue = Keywords.True;
        else
            State.LastValue = Keywords.False;
    }
    else
        writeline("under construction...");
}

void handleFileInspect(std::string &before, std::string &after, std::string &arg1)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (mem.getClass(before).hasVariable(after))
        {
            if (Env::fileExists(mem.getClass(before).getVariable(after).getString()))
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
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
                    State.LastValue = Keywords.True;
                else
                    State.LastValue = Keywords.False;
            }
            else
                State.LastValue = Keywords.False;
        }
        else
        {
            if (Env::fileExists(arg1))
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
        }
    }
}

void handleDirectoryInspect(std::string &before, std::string &after, std::string &arg1)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (mem.getClass(before).hasVariable(after))
        {
            if (Env::directoryExists(mem.getClass(before).getVariable(after).getString()))
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
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
                    State.LastValue = Keywords.True;
                else
                    State.LastValue = Keywords.False;
            }
            else
                error(ErrorMessage::NULL_STRING, arg1, false);
        }
        else
        {
            if (Env::directoryExists(arg1))
                State.LastValue = Keywords.True;
            else
                State.LastValue = Keywords.False;
        }
    }
}

void handleListInspect(std::string &arg1)
{
    if (mem.listExists(arg1))
        State.LastValue = Keywords.True;
    else
        State.LastValue = Keywords.False;
}

void handleVariableInspect(std::string &before, std::string &after, std::string &arg1)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (mem.getClass(before).hasVariable(after))
            State.LastValue = Keywords.True;
        else
            State.LastValue = Keywords.False;
    }
    else
    {
        if (mem.variableExists(arg1))
            State.LastValue = Keywords.True;
        else
            State.LastValue = Keywords.False;
    }
}

void handleClassInspect(std::string &arg1)
{
    if (mem.classExists(arg1))
        State.LastValue = Keywords.True;
    else
        State.LastValue = Keywords.False;
}

void handleMethodInspect(std::string &before, std::string &after, std::string &arg1)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (mem.getClass(before).hasMethod(after))
            State.LastValue = Keywords.True;
        else
            State.LastValue = Keywords.False;
    }
    else
    {
        if (mem.methodExists(arg1))
            State.LastValue = Keywords.True;
        else
            State.LastValue = Keywords.False;
    }
}

void handleInitialDir(std::string &arg1)
{
    if (mem.variableExists(arg1))
    {
        if (mem.isString(arg1))
        {
            if (Env::directoryExists(mem.varString(arg1)))
            {
                State.InitialDirectory = mem.varString(arg1);
                Env::changeDirectory(State.InitialDirectory);
            }
            else
                error(ErrorMessage::READ_FAIL, State.InitialDirectory, false);
        }
        else
            error(ErrorMessage::NULL_STRING, arg1, false);
    }
    else
    {
        if (Env::directoryExists(arg1))
        {
            if (arg1 == Keywords.Dot)
                State.InitialDirectory = Env::getCurrentDirectory();
            else if (arg1 == Keywords.RangeSeparator)
                State.InitialDirectory = Env::getCurrentDirectory() + "\\..";
            else
                State.InitialDirectory = arg1;

            Env::changeDirectory(State.InitialDirectory);
        }
        else
            error(ErrorMessage::READ_FAIL, State.InitialDirectory, false);
    }
}

void handleInlineShellExec(std::string &arg1, std::vector<std::string> &command)
{
    if (mem.variableExists(arg1))
    {
        if (mem.isString(arg1))
            Env::shellExec(mem.varString(arg1), command);
        else
            error(ErrorMessage::IS_NULL, arg1, false);
    }
    else
        Env::shellExec(arg1, command);
}

void handleInlineParse(std::string &arg1)
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

void handleListDecl(std::string &arg1)
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

void handleChangeDir(std::string &arg1)
{
    if (mem.variableExists(arg1))
    {
        if (mem.isString(arg1))
        {
            if (Env::directoryExists(mem.varString(arg1)))
                Env::changeDirectory(mem.varString(arg1));
            else
                error(ErrorMessage::READ_FAIL, mem.varString(arg1), false);
        }
        else
            error(ErrorMessage::NULL_STRING, arg1, false);
    }
    else
    {
        if (arg1 == Keywords.InitialDirectory)
            Env::changeDirectory(State.InitialDirectory);
        else if (Env::directoryExists(arg1))
            Env::changeDirectory(arg1);
        else
            Env::changeDirectory(arg1);
    }
}

void handleLoad(std::string &arg1)
{
    if (Env::fileExists(arg1))
    {
        if (is_script(arg1))
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
        std::vector<std::string> lines = mem.getModule(arg1).get();

        for (int i = 0; i < (int)lines.size(); i++)
            parse(lines.at(i));
    }
    else
        error(ErrorMessage::BAD_LOAD, arg1, true);
}

void handleRemove(std::string &arg1)
{
    if (has_params(arg1))
    {
        std::vector<std::string> params = parse_params(arg1);

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

void handleDelay(std::string &arg1)
{
    if (is_numeric(arg1))
        DT::delay(stoi(arg1));
    else
        error(ErrorMessage::CONV_ERR, arg1, false);
}

void handleErr(std::string &arg1)
{
    std::string errorValue(arg1);
    
    if (mem.variableExists(arg1))
    {
        if (mem.isString(arg1))
            errorValue = mem.varString(arg1);
        else if (mem.isNumber(arg1))
            errorValue = dtos(mem.varNumber(arg1));
    }
    
    IO::printerrln(errorValue);
    State.LastError = errorValue;
}

void handlePrompt(std::string &arg1)
{
    if (arg1 == Keywords.InlineParse)
    {
        if (State.UseCustomPrompt == true)
            State.UseCustomPrompt = false;
        else
            State.UseCustomPrompt = true;
    }
    else
    {
        State.UseCustomPrompt = true;
        State.PromptStyle = arg1;
    }
}

void handleIfStatement(std::string &arg1)
{
    std::string tmpValue("");
    // if arg1 is a variable
    if (mem.variableExists(arg1))
    {
        // can we can assume that arg1 belongs to an object?
        if (!is_dotless(arg1))
        {
            std::string objName(before_dot(arg1)), varName(after_dot(arg1));
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
        if (is_numeric(arg1) || is_truthy(arg1) || is_falsey(arg1))
        {
            tmpValue = arg1;
        }
        else
        {
            std::string tmpCode("");

            if (begins_with(arg1, "(\"") && ends_with(arg1, "\")"))
            {
                tmpCode = substring(arg1, 2, arg1.length() - 3);
            }
            else
            {
                tmpCode = arg1;
            }
            tmpValue = get_parsed_stdout(tmpCode);
        }
    }

    if (is_truthy(tmpValue))
    {
        mem.createIfStatement(true);
    }
    else if (is_falsey(tmpValue))
    {
        mem.createIfStatement(false);
    }
    else
    {
        // error(ErrorMessage::INVALID_OP, arg1, true);
    }
}

void handleGoto(std::string &arg1)
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

void handleSwitch(std::string &arg1)
{
    if (mem.variableExists(arg1))
    {
        State.DefiningSwitchBlock = true;
        State.SwitchVarName = arg1;
    }
    else
        error(ErrorMessage::VAR_UNDEFINED, arg1, false);
}

#endif
