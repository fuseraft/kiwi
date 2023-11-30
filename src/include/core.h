#ifndef CORE_H
#define CORE_H

void setList(std::string listName, std::string methodName, std::vector<std::string> params)
{
    if (mem.methodExists(before_params(methodName)))
    {
        exec.executeTemplate(mem.getMethod(before_params(methodName)), params);

        if (!has_params(State.LastValue))
        {
            mem.addItemToList(listName, State.LastValue);
            return;
        }

        std::vector<std::string> last_params = parse_params(State.LastValue);

        for (int i = 0; i < (int)last_params.size(); i++)
            mem.addItemToList(listName, last_params.at(i));
    }
    else if (mem.classExists(before_dot(before_params(methodName))))
    {
        exec.executeTemplate(mem.getClass(before_dot(before_params(methodName))).getMethod(after_dot(before_params(methodName))), params);

        if (!has_params(State.LastValue))
        {
            mem.addItemToList(listName, State.LastValue);
            return;
        }

        std::vector<std::string> last_params = parse_params(State.LastValue);

        for (int i = 0; i < (int)last_params.size(); i++)
            mem.addItemToList(listName, last_params.at(i));
    }
    else
    {
        for (int i = 0; i < (int)params.size(); i++)
        {
            if (!mem.variableExists(params.at(i)))
            {
                mem.addItemToList(listName, params.at(i));
                return;
            }

            if (mem.isString(params.at(i)))
                mem.addItemToList(listName, mem.varString(params.at(i)));
            else if (mem.isNumber(params.at(i)))
                mem.addItemToList(listName, mem.varNumberString(params.at(i)));
            else
                error(ErrorMessage::IS_NULL, params.at(i), false);
        }
    }
}

std::string get_prompt()
{
    if (!State.UseCustomPrompt || State.PromptStyle == "empty")
    {
        return std::string("> ");
    }

    std::string new_style("");
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
                new_style.append(Env::getCurrentDirectory());
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

std::string pre_parse(std::string st)
{
    std::string cleaned(""), builder("");
    int l = st.length();
    bool buildSymbol = false;

    for (int i = 0; i < l; i++)
    {
        if (buildSymbol)
        {
            if (st[i] == '}')
            {
                builder = subtract_char(builder, '{');

                if (mem.variableExists(builder) && is_dotless(builder))
                {
                    if (mem.isString(builder))
                        cleaned.append(mem.varString(builder));
                    else if (mem.isNumber(builder))
                        cleaned.append(mem.varNumberString(builder));
                    else
                        cleaned.append("null");
                }
                else if (mem.methodExists(builder))
                {
                    parse(builder);

                    cleaned.append(State.LastValue);
                }
                else if (has_params(builder))
                {
                    if (stackReady(builder))
                    {
                        if (isStringStack(builder))
                            cleaned.append(getStringStack(builder));
                        else
                            cleaned.append(dtos(getStack(builder)));
                    }
                    else if (!is_dotless(builder))
                    {
                        std::string before(before_dot(builder)), after(after_dot(builder));

                        if (mem.classExists(before))
                        {
                            if (mem.getClass(before).hasMethod(before_params(after)))
                            {
                                exec.executeTemplate(mem.getClass(before).getMethod(before_params(after)), parse_params(after));

                                cleaned.append(State.LastValue);
                            }
                            else
                                error(ErrorMessage::METHOD_UNDEFINED, before + Keywords.Dot + before_params(after), false);
                        }
                        else
                            error(ErrorMessage::CLS_METHOD_UNDEFINED, before, false);
                    }
                    else if (mem.methodExists(before_params(builder)))
                    {
                        exec.executeTemplate(mem.getMethod(before_params(builder)), parse_params(builder));

                        cleaned.append(State.LastValue);
                    }
                    else
                        cleaned.append("null");
                }
                else if (has_brackets(builder))
                {
                    std::string _beforeBrackets(before_brackets(builder)), afterBrackets(builder);
                    std::string rangeBegin(""), rangeEnd(""), _build("");

                    std::vector<std::string> listRange = parse_bracketrange(afterBrackets);

                    if (mem.variableExists(_beforeBrackets))
                    {
                        if (mem.isString(_beforeBrackets))
                        {
                            std::string tempString(mem.varString(_beforeBrackets));

                            if (listRange.size() == 2)
                            {
                                rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                                if (is_numeric(rangeBegin) && is_numeric(rangeEnd))
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
                                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
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
                                            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
                                    }
                                    else
                                        error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
                                }
                            }
                            else if (listRange.size() == 1)
                            {
                                rangeBegin = listRange.at(0);

                                if (is_numeric(rangeBegin))
                                {
                                    if (stoi(rangeBegin) <= (int)tempString.length() - 1 && stoi(rangeBegin) >= 0)
                                    {
                                        std::string _cstr("");

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
                    else if (mem.listExists(_beforeBrackets))
                    {
                        if (listRange.size() == 2)
                        {
                            rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                            if (!(is_numeric(rangeBegin) && is_numeric(rangeEnd)))
                            {
                                error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
                                return Constants.Null;
                            }

                            if (stoi(rangeBegin) < stoi(rangeEnd))
                            {
                                if (!(mem.getList(_beforeBrackets).size() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0))
                                {
                                    error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
                                    return Constants.Null;
                                }

                                std::string bigString("(");

                                for (int z = stoi(rangeBegin); z <= stoi(rangeEnd); z++)
                                {
                                    bigString.append("\"" + mem.getList(_beforeBrackets).at(z) + "\"");

                                    if (z < stoi(rangeEnd))
                                        bigString.push_back(',');
                                }

                                bigString.push_back(')');

                                cleaned.append(bigString);
                            }
                            else if (stoi(rangeBegin) > stoi(rangeEnd))
                            {
                                if (!(mem.getList(_beforeBrackets).size() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0))
                                {
                                    error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
                                    return Constants.Null;
                                }

                                std::string bigString("(");

                                for (int z = stoi(rangeBegin); z >= stoi(rangeEnd); z--)
                                {
                                    bigString.append("\"" + mem.getList(_beforeBrackets).at(z) + "\"");

                                    if (z > stoi(rangeEnd))
                                        bigString.push_back(',');
                                }

                                bigString.push_back(')');

                                cleaned.append(bigString);
                            }
                            else
                                error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
                        }
                        else if (listRange.size() == 1)
                        {
                            rangeBegin = listRange.at(0);

                            if (!is_numeric(rangeBegin))
                            {
                                error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                                return Constants.Null;
                            }

                            if (!(stoi(rangeBegin) <= (int)mem.getList(_beforeBrackets).size() - 1 && stoi(rangeBegin) >= 0))
                            {
                                error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                                return Constants.Null;
                            }

                            cleaned.append(mem.getList(_beforeBrackets).at(stoi(rangeBegin)));
                        }
                        else
                            error(ErrorMessage::OUT_OF_BOUNDS, afterBrackets, false);
                    }
                    else
                        cleaned.append("null");
                }
                else if (!is_dotless(builder))
                {
                    std::string before(before_dot(builder)), after(after_dot(builder));

                    if (!mem.classExists(before))
                    {
                        error(ErrorMessage::CLS_METHOD_UNDEFINED, before, false);
                        return Constants.Null;
                    }

                    if (mem.getClass(before).hasMethod(after))
                    {
                        parse(before + Keywords.Dot + after);
                        cleaned.append(State.LastValue);
                    }
                    else if (mem.getClass(before).hasVariable(after))
                    {
                        if (mem.getClass(before).getVariable(after).getString() != State.Null)
                            cleaned.append(mem.getClass(before).getVariable(after).getString());
                        else if (mem.getClass(before).getVariable(after).getNumber() != State.NullNum)
                            cleaned.append(dtos(mem.getClass(before).getVariable(after).getNumber()));
                        else
                            cleaned.append("null");
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before + Keywords.Dot + after, false);
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
            else if (st[i] == 't' && st[i - 1] == '\\') // end tab
                cleaned.push_back('\t');
            else if (st[i] == ';' && st[i - 1] == '\\') // end semi-colon
                cleaned.push_back(';');
            else if (st[i] == '\'' && st[i - 1] == '\\') // end apostrophe
                cleaned.push_back('\'');
            else if (st[i] == '\\' && st[i + 1] == '{') // begin symbol
                buildSymbol = true;
            else if (st[i] == '\\' && st[i + 1] == 't')
            {
            } // begin tab
            else if (st[i] == '\\' && st[i + 1] == ';')
            {
            } // begin semi-colon
            else if (st[i] == '\\' && st[i + 1] == '\'')
            {
            } // begin apostrophe
            else
                cleaned.push_back(st[i]);
        }
    }

    return cleaned;
}

void write(std::string st)
{
    if (State.CaptureParse)
        State.ParsedOutput.append(pre_parse(st));
    else
        std::cout << pre_parse(st);

    State.LastValue = st;
}

void writeline(std::string st)
{
    write(st + "\n");
}

void writeline()
{
    write("\n");
}

// TODO: ugh
void show_version()
{
    writeline("uslang 0.1.1"); 
}

List getDirectoryList(std::string before, bool filesOnly)
{
    List newList;
    std::vector<std::string> dirList = Env::getDirectoryContents(mem.varString(before), filesOnly);
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

void error(int errorType, std::string errorInfo, bool quit)
{
    std::string completeError("\nError: ");
    completeError.append(Error::getErrorString(errorType));
    completeError.append("\n- line: ");
    completeError.append(itos(State.CurrentLineNumber));
    completeError.append("\n- code: ");
    completeError.append(State.CurrentLine);
    completeError.append("\n- near: ");
    completeError.append(errorInfo);
    completeError.append("\n");

    State.LastError = completeError;
    State.LastErrorCode = errorType;

    if (State.ExecutedTryBlock)
    {
        State.RaiseCatchBlock = true;
    }
    else
    {
        if (State.CaptureParse)
            State.ParsedOutput.append(completeError);
        else
            std::cerr << completeError;
    }

    if (quit)
    {
        mem.clearAll();
        exit(errorType);
    }
}

void help(std::string app)
{
    writeline("uslang interpreter");
    writeline();
    writeline("usage:\t" + app + "\t\t\tstart the shell");
    writeline("\t" + app + " {args}\t\t\tstart the shell with parameters");
    writeline("\t" + app + " {script}\t\trun a script");
    writeline("\t" + app + " {script} {args}\trun a script with parameters");
    writeline("\t" + app + " -v, --version\t\tshow current version");
    writeline("\t" + app + " -p, --parse\t\tparse a command");
    writeline("\t" + app + " -h, --help\t\tshow this message");
    writeline();
}

int load_repl()
{
    std::string s("");
    bool active = true;

    while (active)
    {
        try
        {
            IO::print(get_prompt());

            s.clear();
            std::getline(std::cin, s);
            ++State.CurrentLineNumber;

            if (s != Keywords.Exit)
            {
                parse(ltrim_ws(s));
                continue;
            }

            if (State.DefiningClass || State.DefiningMethod)
            {
                parse(s);
                continue;
            }

            active = false;
            mem.clearAll();
            break;
        }
        catch (const std::exception &e)
        {
            print_error(e);
            return -1;
        }
    }

    return 0;
}

bool stackReady(std::string arg2)
{
    return contains(arg2, Operators.Add) || contains(arg2, Operators.Subtract) || contains(arg2, Operators.Multiply) || contains(arg2, Operators.Divide) || contains(arg2, Operators.Modulus) || contains(arg2, Operators.Exponent);
}

bool isStringStack(std::string arg2)
{
    std::string tempArgTwo = arg2, temporaryBuild("");
    tempArgTwo = subtract_char(tempArgTwo, '(');
    tempArgTwo = subtract_char(tempArgTwo, ')');

    for (int i = 0; i < (int)tempArgTwo.length(); i++)
    {
        if (tempArgTwo[i] == ' ' && temporaryBuild.length() != 0)
        {
            if (mem.variableExists(temporaryBuild))
            {
                if (mem.isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (mem.isString(temporaryBuild))
                    return true;
            }
            else if (mem.methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (is_numeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '+')
        {
            if (mem.variableExists(temporaryBuild))
            {
                if (mem.isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (mem.isString(temporaryBuild))
                    return true;
            }
            else if (mem.methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (is_numeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!is_numeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '-')
        {
            if (mem.variableExists(temporaryBuild))
            {
                if (mem.isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (mem.isString(temporaryBuild))
                    return true;
            }
            else if (mem.methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (is_numeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!is_numeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '*')
        {
            if (mem.variableExists(temporaryBuild))
            {
                if (mem.isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (mem.isString(temporaryBuild))
                    return true;
            }
            else if (mem.methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (is_numeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!is_numeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '/')
        {
            if (mem.variableExists(temporaryBuild))
            {
                if (mem.isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (mem.isString(temporaryBuild))
                    return true;
            }
            else if (mem.methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (is_numeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!is_numeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '%')
        {
            if (mem.variableExists(temporaryBuild))
            {
                if (mem.isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (mem.isString(temporaryBuild))
                    return true;
            }
            else if (mem.methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (is_numeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!is_numeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else if (tempArgTwo[i] == '^')
        {
            if (mem.variableExists(temporaryBuild))
            {
                if (mem.isNumber(temporaryBuild))
                    temporaryBuild.clear();
                else if (mem.isString(temporaryBuild))
                    return true;
            }
            else if (mem.methodExists(temporaryBuild))
            {
                parse(temporaryBuild);

                if (is_numeric(State.LastValue))
                    temporaryBuild.clear();
                else
                    return true;
            }
            else if (!is_numeric(temporaryBuild))
                return true;
            else
                temporaryBuild.clear();
        }
        else
            temporaryBuild.push_back(tempArgTwo[i]);
    }

    return false;
}

std::string getStringStack(std::string arg2)
{
    std::string tempArgTwo = arg2, temporaryBuild("");
    tempArgTwo = subtract_char(tempArgTwo, '(');
    tempArgTwo = subtract_char(tempArgTwo, ')');

    std::string stackValue("");

    std::vector<std::string> contents;
    std::vector<std::string> vars;

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
        else if (tempArgTwo[i] == ' ' && temporaryBuild.length() != 0 && !quoted)
        {
            parseStringStack(contents, vars, temporaryBuild, tempArgTwo[i]);
        }
        else
        {
            switch (tempArgTwo[i])
            {
                case '+':
                case '-':
                case '*':
                    parseStringStack(contents, vars, temporaryBuild, tempArgTwo[i]);
                    break;

                default:
                    temporaryBuild.push_back(tempArgTwo[i]);
                    break;
            }
        }
    }

    if (mem.variableExists(temporaryBuild))
    {
        if (mem.isNumber(temporaryBuild))
        {
            vars.push_back(temporaryBuild);
            contents.push_back(dtos(mem.varNumber(temporaryBuild)));
            temporaryBuild.clear();
        }
        else if (mem.isString(temporaryBuild))
        {
            vars.push_back(temporaryBuild);
            contents.push_back(mem.varString(temporaryBuild));
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
                stackValue = subtract_string(stackValue, contents.at(i));
                subtractNext = false;
            }
            else if (multiplyNext && is_numeric(contents.at(i)))
            {
                stackValue = multiply_string(stackValue, stoi(contents.at(i)));
                multiplyNext = false;
            }

            if (contents.at(i) == Operators.Add)
                addNext = true;
            else if (contents.at(i) == Operators.Subtract)
                subtractNext = true;
            else if (contents.at(i) == Operators.Multiply)
                multiplyNext = true;
        }
        else
        {
            startOperating = true;
            stackValue = contents.at(i);
        }
    }

    return stackValue;
}

std::string getStackValue(std::string value)
{
    std::string stackValue("");

    if (isStringStack(value))
        stackValue = getStringStack(value);
    else if (stackReady(value))
        stackValue = dtos(getStack(value));
    else
        stackValue = value;

    return stackValue;
}

void parseNumberStack(std::vector<std::string> &contents, std::vector<std::string> vars, std::string &temporaryBuild, char currentChar)
{
    if (mem.variableExists(temporaryBuild) && mem.isNumber(temporaryBuild))
    {
        vars.push_back(temporaryBuild);
        contents.push_back(dtos(mem.varNumber(temporaryBuild)));
        temporaryBuild.clear();
    }
    else if (mem.methodExists(temporaryBuild))
    {
        parse(temporaryBuild);

        if (is_numeric(State.LastValue))
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

    if (currentChar != ' ')
    {
        contents.push_back("" + currentChar);
    }
}

void parseStringStack(std::vector<std::string> &contents, std::vector<std::string> vars, std::string &temporaryBuild, char currentChar)
{
    if (mem.variableExists(temporaryBuild))
    {
        if (mem.isNumber(temporaryBuild))
        {
            vars.push_back(temporaryBuild);
            contents.push_back(dtos(mem.varNumber(temporaryBuild)));
            temporaryBuild.clear();
        }
        else if (mem.isString(temporaryBuild))
        {
            vars.push_back(temporaryBuild);
            contents.push_back(mem.varString(temporaryBuild));
            temporaryBuild.clear();
        }
    }
    else if (mem.methodExists(temporaryBuild))
    {
        parse(temporaryBuild);

        if (is_numeric(State.LastValue))
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

    if (currentChar == '+')
        contents.push_back(Operators.Add);
    else if (currentChar == '-')
        contents.push_back(Operators.Subtract);
    else if (currentChar == '*')
        contents.push_back(Operators.Multiply);
}

double getStack(std::string arg2)
{
    std::string tempArgTwo = arg2, temporaryBuild("");
    tempArgTwo = subtract_char(tempArgTwo, '(');
    tempArgTwo = subtract_char(tempArgTwo, ')');

    double stackValue = (double)0.0;

    std::vector<std::string> contents;
    std::vector<std::string> vars;

    for (int i = 0; i < (int)tempArgTwo.length(); i++)
    {
        if (tempArgTwo[i] == ' ' && temporaryBuild.length() != 0)
        {
            parseNumberStack(contents, vars, temporaryBuild, ' ');
        }
        else
        {
            switch (tempArgTwo[i])
            {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '^':
                parseNumberStack(contents, vars, temporaryBuild, tempArgTwo[i]);
                break;

            default:
                temporaryBuild.push_back(tempArgTwo[i]);
                break;
            }
        }
    }

    if (mem.variableExists(temporaryBuild) && mem.isNumber(temporaryBuild))
    {
        vars.push_back(temporaryBuild);
        contents.push_back(dtos(mem.varNumber(temporaryBuild)));
        temporaryBuild.clear();
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

            if (contents.at(i) == Operators.Add)
                addNext = true;
            else if (contents.at(i) == Operators.Subtract)
                subtractNext = true;
            else if (contents.at(i) == Operators.Multiply)
                multiplyNext = true;
            else if (contents.at(i) == Operators.Divide)
                divideNext = true;
            else if (contents.at(i) == Operators.Modulus)
                moduloNext = true;
            else if (contents.at(i) == Operators.Exponent)
                powerNext = true;
        }
        else
        {
            if (is_numeric(contents.at(i)))
            {
                startOperating = true;
                stackValue = stod(contents.at(i));
            }
        }
    }

    return stackValue;
}

std::string getSubString(std::string arg1, std::string arg2, std::string beforeBracket)
{
    std::string returnValue("");

    if (mem.isString(beforeBracket))
    {
        std::vector<std::string> listRange = parse_bracketrange(arg2);
        std::string variableString = mem.varString(beforeBracket);

        if (listRange.size() < 1 || listRange.size() > 2)
        {
            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
        }
        else if (listRange.size() == 1)
        {
            std::string rangeBegin(listRange.at(0));

            if (rangeBegin.length() != 0 && is_numeric(rangeBegin))
            {
                int beginIndex = stoi(rangeBegin);

                if ((int)variableString.length() - 1 >= beginIndex && beginIndex >= 0)
                {
                    returnValue = "" + variableString[beginIndex];
                }
            }
        }
        else if (listRange.size() == 2)
        {
            std::string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

            if (!(rangeBegin.length() != 0 && rangeEnd.length() != 0) || !((is_numeric(rangeBegin) && is_numeric(rangeEnd)) || !((int)variableString.length() - 1 >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) || !((int)variableString.length() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0)))
            {
                error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
                return returnValue;
            }

            int beginIndex = stoi(rangeBegin),
                endIndex = stoi(rangeEnd);

            if (beginIndex < endIndex)
            {
                for (int i = beginIndex; i <= endIndex; i++)
                {
                    returnValue.push_back(variableString[i]);
                }
            }
            else if (beginIndex > endIndex)
            {
                for (int i = beginIndex; i >= endIndex; i--)
                {
                    returnValue.push_back(variableString[i]);
                }
            }
        }
    }
    else
        error(ErrorMessage::NULL_STRING, beforeBracket, false);

    return returnValue;
}

void setSubString(std::string arg1, std::string arg2, std::string beforeBracket)
{
    if (!mem.isString(beforeBracket))
    {
        error(ErrorMessage::NULL_STRING, beforeBracket, false);
        return;
    }

    std::vector<std::string> listRange = parse_bracketrange(arg2);
    std::string variableString = mem.varString(beforeBracket);

    if (listRange.size() == 2)
    {
        std::string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

        if (rangeBegin.length() == 0 || rangeEnd.length() == 0 || !(is_numeric(rangeBegin) && is_numeric(rangeEnd)))
        {
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
            return;
        }

        int beginIndex = stoi(rangeBegin),
            endIndex = stoi(rangeEnd);

        std::string tempString("");

        if (beginIndex < endIndex && beginIndex >= 0 && endIndex <= (int)variableString.length() - 1)
        {
            for (int i = beginIndex; i <= endIndex; i++)
                tempString.push_back(variableString[i]);

            if (mem.variableExists(arg1))
                mem.setVariable(arg1, tempString);
            else
                mem.createVariable(arg1, tempString);
        }
        else if (beginIndex > endIndex && beginIndex >= 0 && endIndex <= (int)variableString.length())
        {
            for (int i = beginIndex; i >= endIndex; i--)
                tempString.push_back(variableString[i]);

            if (mem.variableExists(arg1))
                mem.setVariable(arg1, tempString);
            else
                mem.createVariable(arg1, tempString);
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
    }
    else if (listRange.size() == 1)
    {
        std::string rangeBegin(listRange.at(0));

        if (rangeBegin.length() != 0 && is_numeric(rangeBegin))
        {
            int beginIndex = stoi(rangeBegin);
            if ((int)variableString.length() - 1 >= beginIndex && beginIndex >= 0)
            {
                std::string tmp_("");
                tmp_.push_back(variableString[beginIndex]);

                if (mem.variableExists(arg1))
                    mem.setVariable(arg1, tmp_);
                else
                    mem.createVariable(arg1, tmp_);
            }
        }
    }
    else
        error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
}

std::string getStringValue(std::string arg1, std::string op, std::string arg2)
{
    std::string firstValue(""), lastValue(""), returnValue("");

    if (mem.variableExists(arg1) && mem.isString(arg1))
    {
        firstValue = mem.varString(arg1);
    }

    if (mem.variableExists(arg2))
    {
        if (mem.isString(arg2))
            lastValue = mem.varString(arg2);
        else if (mem.isNumber(arg2))
            lastValue = dtos(mem.varNumber(arg2));
    }
    else if (mem.methodExists(arg2))
    {
        parse(arg2);
        lastValue = State.LastValue;
    }
    else if (!is_dotless(arg2))
    {
        std::string _beforeDot(before_dot(arg2)), _afterDot(after_dot(arg2));

        if (_beforeDot == Keywords.Env)
        {
            internal_env_builtins("", _afterDot, 2);
        }
        else if (_beforeDot == Keywords.Args && _afterDot == Keywords.Size)
        {
            lastValue = itos(mem.getArgCount());
        }
        else if (mem.classExists(_beforeDot))
        {
            exec.executeTemplate(mem.getClass(_beforeDot).getMethod(_afterDot), parse_params(_afterDot));
            lastValue = State.LastValue;
        }
        else
            lastValue = arg2;
    }
    else if (has_brackets(arg2))
    {
        std::string _beforeBrackets(before_brackets(arg2)), _afterBrackets(after_brackets(arg2));

        if (_beforeBrackets == Keywords.Args)
        {
            std::vector<std::string> params = parse_bracketrange(_afterBrackets);
            int index = stoi(params.at(0));

            if (is_numeric(params.at(0)) && mem.getArgCount() - 1 >= index && index >= 0)
            {
                if (params.at(0) == "0")
                    lastValue = State.CurrentScript;
                else
                    lastValue = mem.getArg(index);
            }
        }
        else if (mem.listExists(_beforeBrackets))
        {
            _afterBrackets = subtract_string(_afterBrackets, "]");
            int index = stoi(_afterBrackets);

            if (mem.getList(_beforeBrackets).size() >= index && index >= 0)
            {
                lastValue = mem.getList(_beforeBrackets).at(index);
            }
        }
    }
    else if (has_params(arg2))
    {
        if (before_params(arg2).length() != 0)
        {
            exec.executeTemplate(mem.getMethod(arg2), parse_params(arg2));
            lastValue = State.LastValue;
        }
        else if (isStringStack(arg2))
            lastValue = getStringStack(arg2);
        else if (stackReady(arg2))
            lastValue = dtos(getStack(arg2));
    }
    else
        lastValue = arg2;

    if (op == Operators.AddAssign)
        returnValue = (firstValue + lastValue);
    else if (op == Operators.SubtractAssign)
        returnValue = subtract_string(firstValue, lastValue);
    else if (op == Operators.MultiplyAssign && is_numeric(lastValue))
    {
        returnValue = multiply_string(firstValue, stoi(lastValue));
    }
    else if (op == Operators.DivideAssign)
        returnValue = subtract_string(firstValue, lastValue);
    else if (op == Operators.ExponentAssign)
        returnValue = dtos(pow(stod(firstValue), stod(lastValue)));
    else if (op == Operators.Assign)
        returnValue = lastValue;

    State.LastValue = returnValue;
    return returnValue;
}

double getNumberValue(std::string arg1, std::string op, std::string arg2)
{
    double firstValue = 0, lastValue = 0, returnValue = 0;

    if (mem.variableExists(arg1) && mem.isNumber(arg1))
    {
        firstValue = mem.varNumber(arg1);
    }

    if (mem.variableExists(arg2) && mem.isNumber(arg2))
    {
        lastValue = mem.varNumber(arg2);
    }
    else if (mem.methodExists(arg2))
    {
        parse(arg2);
        lastValue = is_numeric(State.LastValue) ? stod(State.LastValue) : 0;
    }
    else if (!is_dotless(arg2))
    {
        std::string _beforeDot(before_dot(arg2)), _afterDot(after_dot(arg2));
        if (_beforeDot == Keywords.Env)
        {
            internal_env_builtins("", _afterDot, 2);
        }
        else if (_beforeDot == Keywords.Args && _afterDot == Keywords.Size)
        {
            lastValue = stod(itos(mem.getArgCount()));
        }
        else if (mem.classExists(_beforeDot))
        {
            exec.executeTemplate(mem.getClass(_beforeDot).getMethod(_afterDot), parse_params(_afterDot));
            lastValue = is_numeric(State.LastValue) ? stod(State.LastValue) : 0;
        }
        else if (is_numeric(State.LastValue))
            lastValue = stod(arg2);
    }
    else if (has_brackets(arg2))
    {
        std::string _beforeBrackets(before_brackets(arg2)), _afterBrackets(after_brackets(arg2));

        if (mem.listExists(_beforeBrackets))
        {
            _afterBrackets = subtract_string(_afterBrackets, "]");
            int index = stoi(_afterBrackets);

            if (mem.getList(_beforeBrackets).size() >= index)
            {
                if (index >= 0 && is_numeric(mem.getList(_beforeBrackets).at(index)))
                {
                    lastValue = stod(mem.getList(_beforeBrackets).at(index));
                }
            }
        }
    }
    else if (has_params(arg2))
    {
        if (before_params(arg2).length() != 0)
        {
            exec.executeTemplate(mem.getMethod(arg2), parse_params(arg2));
            if (is_numeric(State.LastValue))
                lastValue = stod(State.LastValue);
        }
        else if (stackReady(arg2))
        {
            lastValue = getStack(arg2);
        }
    }
    else if (is_numeric(arg2))
        lastValue = stod(arg2);

    if (op == Operators.AddAssign)
        returnValue = (firstValue + lastValue);
    else if (op == Operators.SubtractAssign)
        returnValue = (firstValue - lastValue);
    else if (op == Operators.MultiplyAssign)
        returnValue = (firstValue * lastValue);
    else if (op == Operators.DivideAssign)
        returnValue = (firstValue / lastValue);
    else if (op == Operators.ExponentAssign)
        returnValue = pow(firstValue, lastValue);
    else if (op == Operators.Assign)
        returnValue = lastValue;

    State.LastValue = dtos(returnValue);
    return returnValue;
}

void initializeTemporaryVariable(std::string arg1, std::string arg2, std::vector<std::string> command, std::string tempClassVariableName, std::string className, std::string variableName)
{
    twoSpace(tempClassVariableName, arg1, arg2, command);
    mem.getVar(tempClassVariableName).setName(variableName);
    mem.getClass(className).removeVariable(variableName);
    mem.getClass(className).addVariable(mem.getVar(variableName));
    mem.removeVariable(variableName);
}

void initializeTemporaryString(std::string arg1, std::string arg2, std::vector<std::string> command, std::string tempClassVariableName, std::string className, std::string variableName)
{
    mem.createVariable(tempClassVariableName, mem.getClassVariable(className, variableName).getString());
    initializeTemporaryVariable(arg1, arg2, command, tempClassVariableName, className, variableName);
}

void initializeTemporaryNumber(std::string arg1, std::string arg2, std::vector<std::string> command, std::string tempClassVariableName, std::string className, std::string variableName)
{
    mem.createVariable(tempClassVariableName, mem.getClass(before_dot(className)).getVariable(after_dot(className)).getNumber());
    initializeTemporaryVariable(arg1, arg2, command, tempClassVariableName, className, variableName);
}

void initializeVariable(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command)
{
    std::string tmpObjName = before_dot(arg0), tmpVarName = after_dot(arg0);
    bool tmpObjExists = mem.classExists(tmpObjName);
    if (tmpObjExists || begins_with(arg0, "@"))
    {
        if (tmpObjExists)
        {
            if (mem.getClass(tmpObjName).getVariable(tmpVarName).getString() != State.Null)
            {
                std::string tempClassVariableName("@ " + tmpObjName + tmpVarName + "_string");
                initializeTemporaryString(arg1, arg2, command, tempClassVariableName, tmpObjName, tmpVarName);
            }
            else if (mem.getClass(tmpObjName).getVariable(tmpVarName).getNumber() != State.NullNum)
            {
                std::string tempClassVariableName("@____" + tmpObjName + "___" + tmpVarName + "_number");
                initializeTemporaryNumber(arg1, arg2, command, tempClassVariableName, tmpObjName, tmpVarName);
            }
        }
        else if (arg1 == Operators.Assign)
        {
            std::string before(before_dot(arg2)), after(after_dot(arg2));

            if (has_brackets(arg2) && (mem.variableExists(before_brackets(arg2)) || mem.listExists(before_brackets(arg2))))
            {
                std::string beforeBracket(before_brackets(arg2)), afterBracket(after_brackets(arg2));

                afterBracket = subtract_string(afterBracket, "]");

                if (mem.listExists(beforeBracket))
                {
                    if (mem.getList(beforeBracket).size() >= stoi(afterBracket))
                    {
                        if (mem.getList(beforeBracket).at(stoi(afterBracket)) == "#!=no_line")
                            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
                        else
                        {
                            std::string listValue(mem.getList(beforeBracket).at(stoi(afterBracket)));

                            if (is_numeric(listValue))
                            {
                                if (mem.isNumber(arg0))
                                    mem.setVariable(arg0, stod(listValue));
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                            else
                            {
                                if (mem.isString(arg0))
                                    mem.setVariable(arg0, listValue);
                                else
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                            }
                        }
                    }
                }
                else if (mem.isString(beforeBracket))
                    setSubString(arg0, arg2, beforeBracket);
                else
                    error(ErrorMessage::LIST_UNDEFINED, beforeBracket, false);
            }
            else if (before.length() != 0 && after.length() != 0)
            {
                if (has_params(arg2))
                {
                    if (before_params(arg2) == Keywords.Random)
                    {
                        if (!contains(arg2, Keywords.RangeSeparator))
                        {
                            error(ErrorMessage::INVALID_SEQ_SEP, arg2, false);
                            return;
                        }

                        std::vector<std::string> range = parse_range(arg2);
                        std::string s0(range.at(0)), s2(range.at(1));

                        if (is_numeric(s0) && is_numeric(s2))
                        {
                            double n0 = stod(s0), n2 = stod(s2);
                            int rand = (int)RNG::random(n0, n2);
                            
                            if (mem.isNumber(arg0))
                                mem.setVariable(arg0, rand);
                            else if (mem.isString(arg0))
                                mem.setVariable(arg0, itos(rand));
                        }
                        else if (mem.variableExists(s0) || mem.variableExists(s2))
                        {
                            if (mem.variableExists(s0))
                            {
                                if (mem.isNumber(s0))
                                    s0 = dtos(mem.varNumber(s0));
                                else if (mem.isString(s0))
                                    s0 = mem.varString(s0);
                            }

                            if (mem.variableExists(s2))
                            {
                                if (mem.isNumber(s2))
                                    s2 = mem.varNumberString(s2);
                                else if (mem.isString(s2))
                                    s2 = mem.varString(s2);
                            }

                            if (is_numeric(s0) && is_numeric(s2))
                            {
                                double n0 = stod(s0), n2 = stod(s2);
                                int rand = (int)RNG::random(n0, n2);
                                if (mem.isNumber(arg0))
                                    mem.setVariable(arg0, rand);
                                else if (mem.isString(arg0))
                                    mem.setVariable(arg0, itos(rand));
                            }
                        }
                        else
                            error(ErrorMessage::INVALID_SEQ, s0 + "_" + s2, false);
                    }
                }
                else if (mem.listExists(before) && after == Keywords.Size)
                {
                    if (mem.isNumber(arg0))
                        mem.setVariable(arg0, stod(itos(mem.getList(before).size())));
                    else if (mem.isString(arg0))
                        mem.setVariable(arg0, itos(mem.getList(before).size()));
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
                else if (before == Keywords.Self)
                {
                    if (mem.classExists(State.CurrentMethodClass))
                        twoSpace(arg0, arg1, (State.CurrentMethodClass + Keywords.Dot + after), command);
                    else
                        twoSpace(arg0, arg1, after, command);
                }
                else if (mem.classExists(before))
                {
                    if (mem.getClass(before).hasVariable(after))
                    {
                        if (mem.getClass(before).getVariable(after).getString() != State.Null)
                            mem.setVariable(arg0, mem.getClass(before).getVariable(after).getString());
                        else if (mem.getClass(before).getVariable(after).getNumber() != State.NullNum)
                            mem.setVariable(arg0, mem.getClass(before).getVariable(after).getNumber());
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else if (mem.getClass(before).hasMethod(after) && !has_params(after))
                    {
                        parse(arg2);

                        if (mem.isString(arg0))
                            mem.setVariable(arg0, State.LastValue);
                        else if (mem.isNumber(arg0))
                            mem.setVariable(arg0, stod(State.LastValue));
                    }
                    else if (has_params(after))
                    {
                        if (mem.getClass(before).hasMethod(before_params(after)))
                        {
                            exec.executeTemplate(mem.getClass(before).getMethod(before_params(after)), parse_params(after));

                            if (is_numeric(State.LastValue))
                            {
                                if (mem.isString(arg0))
                                    mem.setVariable(arg0, State.LastValue);
                                else if (mem.isNumber(arg0))
                                    mem.setVariable(arg0, stod(State.LastValue));
                                else
                                    error(ErrorMessage::IS_NULL, arg0, false);
                            }
                            else
                            {
                                if (mem.isString(arg0))
                                    mem.setVariable(arg0, State.LastValue);
                                else if (mem.isNumber(arg0))
                                    error(ErrorMessage::CONV_ERR, arg0, false);
                                else
                                    error(ErrorMessage::IS_NULL, arg0, false);
                            }
                        }
                        else
                            Env::shellExec(arg0, command);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, arg2, false);
                }
                else if (before == Keywords.Env)
                {
                    internal_env_builtins(arg0, after, 1);
                }
                else if (recognized_mathfunc(after))
                {
                    parse_mathfunc_assignfromvar(arg0, before, after);
                }
                else if (after == Keywords.ToInteger)
                {
                    if (mem.variableExists(before))
                    {
                        if (mem.isString(before))
                            mem.setVariable(arg0, (int)mem.varString(before)[0]);
                        else if (mem.isNumber(before))
                        {
                            int i = (int)mem.varNumber(before);
                            mem.setVariable(arg0, (double)i);
                        }
                        else
                            error(ErrorMessage::IS_NULL, before, false);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before, false);
                }
                else if (after == Keywords.ToDouble)
                {
                    if (mem.variableExists(before))
                    {
                        if (mem.isString(before))
                            mem.setVariable(arg0, (double)mem.varString(before)[0]);
                        else if (mem.isNumber(before))
                        {
                            double i = mem.varNumber(before);
                            mem.setVariable(arg0, (double)i);
                        }
                        else
                            error(ErrorMessage::IS_NULL, before, false);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before, false);
                }
                else if (after == Keywords.ToString)
                {
                    if (mem.variableExists(before))
                    {
                        if (mem.isNumber(before))
                            mem.setVariable(arg0, dtos(mem.varNumber(before)));
                        else
                            error(ErrorMessage::IS_NULL, before, false);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before, false);
                }
                else if (after == Keywords.ToNumber)
                {
                    if (mem.variableExists(before))
                    {
                        if (mem.isString(before))
                            mem.setVariable(arg0, stod(mem.varString(before)));
                        else
                            error(ErrorMessage::IS_NULL, before, false);
                    }
                    else
                        error(ErrorMessage::VAR_UNDEFINED, before, false);
                }
                else if (before == Keywords.ReadLine)
                {
                    if (mem.variableExists(after))
                    {
                        if (mem.isString(after))
                        {
                            std::string line("");
                            write(pre_parse(mem.varString(after)));
                            std::getline(std::cin, line, '\n');

                            if (mem.isNumber(arg0))
                            {
                                if (is_numeric(line))
                                    mem.setVariable(arg0, stod(line));
                                else
                                    error(ErrorMessage::CONV_ERR, line, false);
                            }
                            else if (mem.isString(arg0))
                                mem.setVariable(arg0, line);
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);
                        }
                        else
                        {
                            std::string line("");
                            IO::print("");
                            std::getline(std::cin, line, '\n');

                            if (mem.isNumber(arg0))
                            {
                                if (is_numeric(line))
                                    mem.setVariable(arg0, stod(line));
                                else
                                    error(ErrorMessage::CONV_ERR, line, false);
                            }
                            else if (mem.isString(arg0))
                                mem.setVariable(arg0, line);
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);
                        }
                    }
                    else
                    {
                        std::string line("");
                        IO::print(pre_parse(after));
                        std::getline(std::cin, line, '\n');

                        if (is_numeric(line))
                            mem.setVariable(arg0, stod(line));
                        else
                            mem.setVariable(arg0, line);
                    }
                }
                else if (before == Keywords.Mask)
                {
                    if (mem.variableExists(after))
                    {
                        if (mem.isString(after))
                        {
                            std::string line("");
                            line = get_stdin_quiet(mem.varString(after));

                            if (mem.isNumber(arg0))
                            {
                                if (is_numeric(line))
                                    mem.setVariable(arg0, stod(line));
                                else
                                    error(ErrorMessage::CONV_ERR, line, false);
                            }
                            else if (mem.isString(arg0))
                                mem.setVariable(arg0, line);
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);

                            writeline();
                        }
                        else
                        {
                            std::string line("");
                            line = get_stdin_quiet("");

                            if (mem.isNumber(arg0))
                            {
                                if (is_numeric(line))
                                    mem.setVariable(arg0, stod(line));
                                else
                                    error(ErrorMessage::CONV_ERR, line, false);
                            }
                            else if (mem.isString(arg0))
                                mem.setVariable(arg0, line);
                            else
                                error(ErrorMessage::IS_NULL, arg0, false);

                            writeline();
                        }
                    }
                    else
                    {
                        std::string line("");
                        line = get_stdin_quiet(pre_parse(after));

                        if (is_numeric(line))
                            mem.setVariable(arg0, stod(line));
                        else
                            mem.setVariable(arg0, line);

                        writeline();
                    }
                }
                else if (after == Keywords.ToLower)
                {
                    if (mem.variableExists(before))
                    {
                        if (mem.isString(arg0))
                        {
                            if (mem.isString(before))
                                mem.setVariable(arg0, to_lower(mem.varString(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == Keywords.Read)
                {
                    if (mem.isString(arg0))
                    {
                        if (mem.variableExists(before))
                        {
                            if (mem.isString(before))
                            {
                                if (Env::fileExists(mem.varString(before)))
                                {
                                    std::ifstream file(mem.varString(before).c_str());
                                    std::string line(""), bigString("");

                                    if (file.is_open())
                                    {
                                        while (!file.eof())
                                        {
                                            std::getline(file, line);
                                            bigString.append(line + "\r\n");
                                        }

                                        file.close();

                                        mem.setVariable(arg0, bigString);
                                    }
                                    else
                                        error(ErrorMessage::READ_FAIL, mem.varString(before), false);
                                }
                                else
                                    error(ErrorMessage::READ_FAIL, mem.varString(before), false);
                            }
                            else
                                error(ErrorMessage::NULL_STRING, before, false);
                        }
                        else
                        {
                            if (Env::fileExists(before))
                            {
                                std::ifstream file(before.c_str());
                                std::string line(""), bigString("");

                                if (file.is_open())
                                {
                                    while (!file.eof())
                                    {
                                        std::getline(file, line);
                                        bigString.append(line + "\r\n");
                                    }

                                    file.close();

                                    mem.setVariable(arg0, bigString);
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
                else if (after == Keywords.ToUpper)
                {
                    if (mem.variableExists(before))
                    {
                        if (mem.isString(arg0))
                        {
                            if (mem.isString(before))
                                mem.setVariable(arg0, to_upper(mem.varString(before)));
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else if (after == Keywords.Size)
                {
                    if (mem.variableExists(before))
                    {
                        if (mem.isNumber(arg0))
                        {
                            if (mem.isString(before))
                                mem.setVariable(arg0, (double)mem.varString(before).length());
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else
                    {
                        if (mem.isNumber(arg0))
                            mem.setVariable(arg0, (double)before.length());
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                }
                else if (after == Keywords.FileSize)
                {
                    if (mem.isNumber(arg0))
                    {
                        if (mem.variableExists(before))
                        {
                            if (mem.isString(before))
                            {
                                if (Env::fileExists(mem.varString(before)))
                                    mem.setVariable(arg0, get_filesize(mem.varString(before)));
                                else
                                    error(ErrorMessage::READ_FAIL, mem.varString(before), false);
                            }
                            else
                                error(ErrorMessage::CONV_ERR, before, false);
                        }
                        else
                        {
                            if (Env::fileExists(before))
                                mem.setVariable(arg0, get_filesize(before));
                            else
                                error(ErrorMessage::READ_FAIL, before, false);
                        }
                    }
                    else
                        error(ErrorMessage::CONV_ERR, arg0, false);
                }
                else
                {
                    if (mem.isNumber(arg0))
                    {
                        if (is_numeric(arg2))
                            mem.setVariable(arg0, stod(arg2));
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else if (mem.isString(arg0))
                        mem.setVariable(arg0, arg2);
                    else if (mem.getVar(arg0).waiting())
                    {
                        if (is_numeric(arg2))
                            mem.setVariable(arg0, stod(before + Keywords.Dot + after));
                        else
                            mem.setVariable(arg0, arg2);
                    }
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
            }
            else
            {
                if (mem.getVar(arg0).waiting())
                {
                    if (is_numeric(arg2))
                        mem.setVariable(arg0, stod(arg2));
                    else
                        mem.setVariable(arg0, arg2);
                }
                else if (arg2 == "null")
                {
                    if (mem.isString(arg0))
                        mem.getVar(arg0).setNull();
                    else if (mem.isNumber(arg0))
                        mem.getVar(arg0).setNull();
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
                else if (mem.constantExists(arg2))
                {
                    if (mem.isString(arg0))
                    {
                        if (mem.getConstant(arg2).ConstNumber())
                            mem.setVariable(arg0, dtos(mem.getConstant(arg2).getNumber()));
                        else if (mem.getConstant(arg2).ConstString())
                            mem.setVariable(arg0, mem.getConstant(arg2).getString());
                    }
                    else if (mem.isNumber(arg0))
                    {
                        if (mem.getConstant(arg2).ConstNumber())
                            mem.setVariable(arg0, mem.getConstant(arg2).getNumber());
                        else
                            error(ErrorMessage::CONV_ERR, arg2, false);
                    }
                    else
                        error(ErrorMessage::IS_NULL, arg0, false);
                }
                else if (mem.methodExists(arg2))
                {
                    parse(arg2);

                    if (mem.isString(arg0))
                        mem.setVariable(arg0, State.LastValue);
                    else if (mem.isNumber(arg0))
                        mem.setVariable(arg0, stod(State.LastValue));
                }
                else if (mem.variableExists(arg2))
                {
                    if (mem.isString(arg2))
                    {
                        if (mem.isString(arg0))
                            mem.setVariable(arg0, mem.varString(arg2));
                        else if (mem.isNumber(arg0))
                            error(ErrorMessage::CONV_ERR, arg2, false);
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else if (mem.isNumber(arg2))
                    {
                        if (mem.isString(arg0))
                            mem.setVariable(arg0, dtos(mem.varNumber(arg2)));
                        else if (mem.isNumber(arg0))
                            mem.setVariable(arg0, mem.varNumber(arg2));
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                    else
                        error(ErrorMessage::IS_NULL, arg2, false);
                }
                else if (arg2 == Keywords.Mask || arg2 == Keywords.ReadLine)
                {
                    if (arg2 == Keywords.Mask)
                    {
                        std::string masked("");
                        masked = get_stdin_quiet("");

                        if (mem.isNumber(arg0))
                        {
                            if (is_numeric(masked))
                                mem.setVariable(arg0, stod(masked));
                            else
                                error(ErrorMessage::CONV_ERR, masked, false);
                        }
                        else if (mem.isString(arg0))
                            mem.setVariable(arg0, masked);
                        else
                            mem.setVariable(arg0, masked);
                    }
                    else
                    {
                        std::string line("");
                        IO::print("");
                        std::getline(std::cin, line, '\n');

                        if (is_numeric(line))
                            mem.createVariable(arg0, stod(line));
                        else
                            mem.createVariable(arg0, line);
                    }
                }
                else if (has_params(arg2))
                {
                    if (mem.methodExists(before_params(arg2)))
                    {
                        // execute the method
                        exec.executeTemplate(mem.getMethod(before_params(arg2)), parse_params(arg2));
                        // set the variable = last value
                        if (mem.isString(arg0))
                        {
                            mem.setVariable(arg0, State.LastValue);
                        }
                        else if (mem.isNumber(arg0))
                        {
                            mem.setVariable(arg0, stod(State.LastValue));
                        }
                    }
                    else if (isStringStack(arg2))
                    {
                        if (mem.isString(arg0))
                            mem.setVariable(arg0, getStringStack(arg2));
                        else
                            error(ErrorMessage::CONV_ERR, arg0, false);
                    }
                    else if (stackReady(arg2))
                    {
                        if (mem.isString(arg0))
                            mem.setVariable(arg0, dtos(getStack(arg2)));
                        else if (mem.isNumber(arg0))
                            mem.setVariable(arg0, getStack(arg2));
                        else
                            error(ErrorMessage::IS_NULL, arg0, false);
                    }
                }
                else
                {
                    if (is_numeric(arg2))
                    {
                        if (mem.isNumber(arg0))
                            mem.setVariable(arg0, stod(arg2));
                        else if (mem.isString(arg0))
                            mem.setVariable(arg0, arg2);
                    }
                    else
                    {
                        if (mem.isNumber(arg0))
                            error(ErrorMessage::CONV_ERR, arg0, false);
                        else if (mem.isString(arg0))
                            mem.setVariable(arg0, pre_parse(arg2));
                    }
                }
            }
        }
        else if (!unrecognized_2space(arg1))
        {
            parse_assign(arg0, arg1, arg2);
        }
    }
}

void parse_assign(std::string arg0, std::string arg1, std::string arg2)
{
    std::string first(""), second("");

    bool arg0IsString = mem.isString(arg0),
        arg0IsNumber = mem.isNumber(arg0);

    if (!arg0IsString && !arg0IsNumber)
    {
        error(ErrorMessage::IS_NULL, arg2, false);
        return;
    }

    first = arg0IsString ? mem.varString(arg0) : dtos(mem.varNumber(arg0));

    if (mem.variableExists(arg2))
    {
        if (mem.isString(arg2))
            second = mem.varString(arg2);
        else if (mem.isNumber(arg2))
            second = dtos(mem.varNumber(arg2));
        else
        {
            error(ErrorMessage::IS_NULL, arg2, false);
            return;
        }
    }
    else
    {
        if (has_params(arg2))
        {
            if (isStringStack(arg2) && mem.isString(arg0))
            {
                second = getStringStack(arg2);
            }
            else if (stackReady(arg2) && mem.isNumber(arg0))
            {
                second = dtos(getStack(arg2));
            }
            else if (mem.methodExists(before_params(arg2)))
            {
                exec.executeTemplate(mem.getMethod(before_params(arg2)), parse_params(arg2));
                second = State.LastValue;
            }
            else if (mem.classExists(before_dot(arg2)))
            {
                exec.executeTemplate(mem.getMethod(before_params(arg2)), parse_params(arg2));
                second = State.LastValue;
            }
        }
        else if (mem.methodExists(arg2))
        {
            parse(arg2);
            second = State.LastValue;
        }
        else if (is_numeric(arg2))
        {
            second = arg2;
        }
        else
        {
            second = pre_parse(arg2);
        }
    }

    bool firstIsNumeric = is_numeric(first),
        secondIsNumeric = is_numeric(second);
    double firstNumber = firstIsNumeric ? stod(first) : 0,
        secondNumber = secondIsNumeric ? stod(second) : 0;

    if (firstIsNumeric && secondIsNumeric)
    {
        double result = 0;
        if (arg1 == Operators.Assign)
            result = secondNumber;
        else if (arg1 == Operators.AddAssign)
            result = firstNumber + secondNumber;
        else if (arg1 == Operators.SubtractAssign)
            result = firstNumber - secondNumber;
        else if (arg1 == Operators.MultiplyAssign)
            result = firstNumber * secondNumber;
        else if (arg1 == Operators.ModuloAssign)
            result = (int)firstNumber % (int)secondNumber;
        else if (arg1 == Operators.ExponentAssign)
            result = pow(firstNumber, secondNumber);
        else if (arg1 == Operators.DivideAssign)
            result = firstNumber / secondNumber;
        else 
        {
            error(ErrorMessage::INVALID_OP, arg1, true);
            return;
        }

        mem.setVariable(arg0, result);
    }
    else if (!firstIsNumeric && secondIsNumeric)
    {
        std::string result(first);
        if (arg1 == Operators.Assign)
            result = dtos(secondNumber);
        else if (arg1 == Operators.AddAssign)
            result = result + dtos(secondNumber);
        else if (arg1 == Operators.SubtractAssign)
            result = subtract_string(result, second);
        else if (arg1 == Operators.MultiplyAssign)
            result = multiply_string(result, (int)secondNumber);
        else 
        {
            error(ErrorMessage::INVALID_OP, arg1, true);
            return;
        }

        mem.setVariable(arg0, result);
    }
    else if (!firstIsNumeric && !secondIsNumeric)
    {
        std::string result(first);
        if (arg1 == Operators.Assign)
            result = second;
        else if (arg1 == Operators.AddAssign)
            result += second;
        else if (arg1 == Keywords.ShellExec)
            result = Env::getStdout(second.c_str());
        else if (arg1 == Keywords.InlineParse)
            result = get_parsed_stdout(second.c_str());
        else
        {
            error(ErrorMessage::INVALID_OP, arg1, true);
            return;
        }

        mem.setVariable(arg0, result);
    }
    else
    {
        error(ErrorMessage::CONV_ERR, "parseAssignment", true);
    }
}

void init_listvalues(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command)
{
    std::string _b(before_dot(arg2)), _a(after_dot(arg2)), __b(before_params(arg2));

    if (has_brackets(arg0))
    {
        std::string after(after_brackets(arg0)), before(before_brackets(arg0));
        after = subtract_string(after, "]");

        if (mem.getList(before).size() >= stoi(after))
        {
            if (stoi(after) == 0)
            {
                if (arg1 == Operators.Assign)
                {
                    if (mem.variableExists(arg2))
                    {
                        if (mem.isString(arg2))
                            mem.replaceElement(before, after, mem.varString(arg2));
                        else if (mem.isNumber(arg2))
                            mem.replaceElement(before, after, dtos(mem.varNumber(arg2)));
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else
                        mem.replaceElement(before, after, arg2);
                }
            }
            else if (mem.getList(before).at(stoi(after)) == "#!=no_line")
                error(ErrorMessage::OUT_OF_BOUNDS, arg0, false);
            else
            {
                if (arg1 == Operators.Assign)
                {
                    if (mem.variableExists(arg2))
                    {
                        if (mem.isString(arg2))
                            mem.replaceElement(before, after, mem.varString(arg2));
                        else if (mem.isNumber(arg2))
                            mem.replaceElement(before, after, dtos(mem.varNumber(arg2)));
                        else
                            error(ErrorMessage::IS_NULL, arg2, false);
                    }
                    else
                        mem.replaceElement(before, after, arg2);
                }
            }
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
    }
    else if (has_brackets(arg2)) // INITIALIZE LIST FROM RANGE
    {
        std::string listName(before_brackets(arg2));

        if (!mem.listExists(listName))
        {
            error(ErrorMessage::LIST_UNDEFINED, listName, false);
            return;
        }

        std::vector<std::string> listRange = parse_bracketrange(arg2);

        if (listRange.size() != 2)
        {
            error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
            return;
        }

        std::string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

        if (!(rangeBegin.length() != 0 && rangeEnd.length() != 0))
        {
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
            return;
        }

        if (!(is_numeric(rangeBegin) && is_numeric(rangeEnd)))
        {
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
            return;
        }

        if (stoi(rangeBegin) < stoi(rangeEnd))
        {
            if (!(mem.getList(listName).size() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) || !(stoi(rangeBegin) >= 0))
            {
                error(ErrorMessage::OUT_OF_BOUNDS, rangeEnd, false);
                return;
            }

            if (arg1 == Operators.AddAssign)
            {
                for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                    mem.addItemToList(arg0, mem.getList(listName).at(i));
            }
            else if (arg1 == Operators.Assign)
            {
                mem.getList(arg0).clear();

                for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                    mem.addItemToList(arg0, mem.getList(listName).at(i));
            }
            else
                error(ErrorMessage::INVALID_OPERATOR, arg1, false);
        }
        else if (stoi(rangeBegin) > stoi(rangeEnd))
        {
            if (!(mem.getList(listName).size() >= stoi(rangeEnd) && stoi(rangeBegin) >= 0) || !(stoi(rangeBegin) >= 0))
            {
                error(ErrorMessage::OUT_OF_BOUNDS, rangeEnd, false);
                return;
            }

            if (arg1 == Operators.AddAssign)
            {
                for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                    mem.addItemToList(arg0, mem.getList(listName).at(i));
            }
            else if (arg1 == Operators.Assign)
            {
                mem.getList(arg0).clear();

                for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                    mem.addItemToList(arg0, mem.getList(listName).at(i));
            }
            else
                error(ErrorMessage::INVALID_OPERATOR, arg1, false);
        }
        else
            error(ErrorMessage::OUT_OF_BOUNDS, rangeBegin + Keywords.RangeSeparator + rangeEnd, false);
    }
    else if (mem.variableExists(_b) && contains(_a, Keywords.Split) && arg1 == Operators.Assign)
    {
        if (!mem.isString(_b))
        {
            error(ErrorMessage::NULL_STRING, _b, false);
            return;
        }

        std::vector<std::string> params = parse_params(_a);
        std::vector<std::string> elements;

        if (params.at(0) == "")
            elements = split(mem.varString(_b), ' ');
        else
        {
            if (params.at(0)[0] == ';')
                elements = split(mem.varString(_b), ';');
            else
                elements = split(mem.varString(_b), params.at(0)[0]);
        }

        mem.getList(arg0).clear();

        for (int i = 0; i < (int)elements.size(); i++)
            mem.addItemToList(arg0, elements.at(i));
    }
    else if (has_params(arg2)) // ADD/REMOVE ARRAY FROM LIST
    {
        std::vector<std::string> params = parse_params(arg2);

        if (arg1 == Operators.Assign)
        {
            mem.getList(arg0).clear();
            setList(arg0, arg2, params);
        }
        else if (arg1 == Operators.AddAssign)
            setList(arg0, arg2, params);
        else if (arg1 == Operators.SubtractAssign)
        {
            for (int i = 0; i < (int)params.size(); i++)
            {
                if (mem.variableExists(params.at(i)))
                {
                    if (mem.isString(params.at(i)))
                        mem.getList(arg0).remove(mem.varString(params.at(i)));
                    else if (mem.isNumber(params.at(i)))
                        mem.getList(arg0).remove(dtos(mem.varNumber(params.at(i))));
                    else
                        error(ErrorMessage::IS_NULL, params.at(i), false);
                }
                else
                    mem.getList(arg0).remove(params.at(i));
            }
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
    else if (mem.variableExists(arg2)) // ADD/REMOVE VARIABLE VALUE TO/FROM LIST
    {
        if (arg1 == Operators.AddAssign)
        {
            if (mem.isString(arg2))
                mem.addItemToList(arg0, mem.varString(arg2));
            else if (mem.isNumber(arg2))
                mem.addItemToList(arg0, dtos(mem.varNumber(arg2)));
            else
                error(ErrorMessage::CONV_ERR, arg2, false);
        }
        else if (arg1 == Operators.SubtractAssign)
        {
            if (mem.isString(arg2))
                mem.getList(arg0).remove(mem.varString(arg2));
            else if (mem.isNumber(arg2))
                mem.getList(arg0).remove(dtos(mem.varNumber(arg2)));
            else
                error(ErrorMessage::CONV_ERR, arg2, false);
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
    else if (mem.methodExists(arg2)) // INITIALIZE LIST FROM METHOD RETURN
    {
        parse(arg2);

        std::vector<std::string> _p = parse_params(State.LastValue);

        if (arg1 == Operators.Assign)
        {
            mem.getList(arg0).clear();

            for (int i = 0; i < (int)_p.size(); i++)
                mem.addItemToList(arg0, _p.at(i));
        }
        else if (arg1 == Operators.AddAssign)
        {
            for (int i = 0; i < (int)_p.size(); i++)
                mem.addItemToList(arg0, _p.at(i));
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, arg1, false);
    }
    else // ADD/REMOVE STRING TO/FROM LIST
    {
        if (arg1 == Operators.AddAssign)
        {
            if (arg2.length() != 0)
                mem.addItemToList(arg0, arg2);
            else
                error(ErrorMessage::IS_EMPTY, arg2, false);
        }
        else if (arg1 == Operators.SubtractAssign)
        {
            if (arg2.length() != 0)
                mem.getList(arg0).remove(arg2);
            else
                error(ErrorMessage::IS_EMPTY, arg2, false);
        }
    }
}

void init_globalvar(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command)
{
    if (arg1 == Operators.Assign)
    {
        std::string before(before_dot(arg2)), after(after_dot(arg2));

        if (has_brackets(arg2) && (mem.variableExists(before_brackets(arg2)) || mem.listExists(before_brackets(arg2))))
        {
            std::string beforeBracket(before_brackets(arg2)), afterBracket(after_brackets(arg2));

            afterBracket = subtract_string(afterBracket, "]");

            if (mem.listExists(beforeBracket))
            {
                if (mem.getList(beforeBracket).size() >= stoi(afterBracket))
                {
                    if (mem.getList(beforeBracket).at(stoi(afterBracket)) == "#!=no_line")
                        error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
                    else
                    {
                        std::string listValue(mem.getList(beforeBracket).at(stoi(afterBracket)));

                        if (is_numeric(listValue))
                            mem.createVariable(arg0, stod(listValue));
                        else
                            mem.createVariable(arg0, listValue);
                    }
                }
                else
                    error(ErrorMessage::OUT_OF_BOUNDS, arg2, false);
            }
            else if (mem.variableExists(beforeBracket))
                setSubString(arg0, arg2, beforeBracket);
            else
                error(ErrorMessage::LIST_UNDEFINED, beforeBracket, false);
        }
        else if (mem.listExists(before) && after == Keywords.Size)
            mem.createVariable(arg0, stod(itos(mem.getList(before).size())));
        else if (before == Keywords.Self)
        {
            if (mem.classExists(State.CurrentMethodClass))
                twoSpace(arg0, arg1, (State.CurrentMethodClass + Keywords.Dot + after), command);
            else
                twoSpace(arg0, arg1, after, command);
        }
        else if (after == Keywords.ToInteger)
        {
            if (!mem.variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (mem.isString(before))
                mem.createVariable(arg0, (int)mem.varString(before)[0]);
            else if (mem.isNumber(before))
            {
                int i = (int)mem.varNumber(before);
                mem.createVariable(arg0, (double)i);
            }
            else
                error(ErrorMessage::IS_NULL, before, false);
        }
        else if (after == Keywords.ToDouble)
        {
            if (!mem.variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (mem.isString(before))
                mem.createVariable(arg0, (double)mem.varString(before)[0]);
            else if (mem.isNumber(before))
            {
                double i = mem.varNumber(before);
                mem.createVariable(arg0, (double)i);
            }
            else
                error(ErrorMessage::IS_NULL, before, false);
        }
        else if (after == Keywords.ToString)
        {
            if (!mem.variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (mem.isNumber(before))
                mem.createVariable(arg0, dtos(mem.varNumber(before)));
            else
                error(ErrorMessage::IS_NULL, before, false);
        }
        else if (after == Keywords.ToNumber)
        {
            if (!mem.variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (mem.isString(before))
                mem.createVariable(arg0, stod(mem.varString(before)));
            else
                error(ErrorMessage::IS_NULL, before, false);
        }
        else if (mem.classExists(before))
        {
            if (mem.getClass(before).hasMethod(after) && !has_params(after))
            {
                parse(arg2);

                if (is_numeric(State.LastValue))
                    mem.createVariable(arg0, stod(State.LastValue));
                else
                    mem.createVariable(arg0, State.LastValue);
            }
            else if (has_params(after))
            {
                if (!mem.getClass(before).hasMethod(before_params(after)))
                {
                    Env::shellExec(arg0, command);
                    return;
                }

                exec.executeTemplate(mem.getClass(before).getMethod(before_params(after)), parse_params(after));

                if (is_numeric(State.LastValue))
                    mem.createVariable(arg0, stod(State.LastValue));
                else
                    mem.createVariable(arg0, State.LastValue);
            }
            else if (mem.getClass(before).hasVariable(after))
            {
                if (mem.getClass(before).getVariable(after).getString() != State.Null)
                    mem.createVariable(arg0, mem.getClass(before).getVariable(after).getString());
                else if (mem.getClass(before).getVariable(after).getNumber() != State.NullNum)
                    mem.createVariable(arg0, mem.getClass(before).getVariable(after).getNumber());
                else
                    error(ErrorMessage::IS_NULL, mem.getClass(before).getVariable(after).name(), false);
            }
        }
        else if (mem.variableExists(before) && after == Keywords.Read)
        {
            if (!mem.isString(before))
            {
                error(ErrorMessage::NULL_STRING, before, false);
                return;
            }

            if (!Env::fileExists(mem.varString(before)))
            {
                error(ErrorMessage::READ_FAIL, mem.varString(before), false);
                return;
            }

            std::ifstream file(mem.varString(before).c_str());
            std::string line(""), bigString("");

            if (!file.is_open())
            {
                error(ErrorMessage::READ_FAIL, mem.varString(before), false);
                return;
            }

            while (!file.eof())
            {
                std::getline(file, line);
                bigString.append(line + "\r\n");
            }

            file.close();

            mem.createVariable(arg0, bigString);
        }
        else if (State.DefiningClass)
        {
            if (is_numeric(arg2))
            {
                Variable newVariable(arg0, stod(arg2));

                if (State.DefiningPrivateCode)
                    newVariable.setPrivate();
                else if (State.DefiningPublicCode)
                    newVariable.setPublic();

                mem.getClass(State.CurrentClass).addVariable(newVariable);
            }
            else
            {
                Variable newVariable(arg0, arg2);

                if (State.DefiningPrivateCode)
                    newVariable.setPrivate();
                else if (State.DefiningPublicCode)
                    newVariable.setPublic();

                mem.getClass(State.CurrentClass).addVariable(newVariable);
            }
        }
        else if (arg2 == "null")
            mem.createVariable(arg0, arg2);
        else if (mem.methodExists(arg2))
        {
            parse(arg2);

            if (is_numeric(State.LastValue))
                mem.createVariable(arg0, stod(State.LastValue));
            else
                mem.createVariable(arg0, State.LastValue);
        }
        else if (mem.constantExists(arg2))
        {
            if (mem.getConstant(arg2).ConstNumber())
                mem.createVariable(arg0, mem.getConstant(arg2).getNumber());
            else if (mem.getConstant(arg2).ConstString())
                mem.createVariable(arg0, mem.getConstant(arg2).getString());
            else
                error(ErrorMessage::CONV_ERR, arg2, false);
        }
        else if (has_params(arg2))
        {
            if (isStringStack(arg2))
                mem.createVariable(arg0, getStringStack(arg2));
            else if (stackReady(arg2))
                mem.createVariable(arg0, getStack(arg2));
            else if (before_params(arg2) == Keywords.Random)
            {
                if (!contains(arg2, Keywords.RangeSeparator))
                {
                    error(ErrorMessage::INVALID_RANGE_SEP, arg2, false);
                    return;
                }

                std::vector<std::string> range = parse_range(arg2);
                std::string s0(range.at(0)), s2(range.at(1));

                if (is_numeric(s0) && is_numeric(s2))
                {
                    double n0 = stod(s0), n2 = stod(s2);
                    mem.createVariable(arg0, (int)RNG::random(std::min(n0, n2), std::max(n0, n2)));
                }
                else if (mem.variableExists(s0) || mem.variableExists(s2))
                {
                    if (mem.variableExists(s0))
                    {
                        if (mem.isNumber(s0))
                            s0 = dtos(mem.varNumber(s0));
                        else if (mem.isString(s0))
                            s0 = mem.varString(s0);
                    }

                    if (mem.variableExists(s2))
                    {
                        if (mem.isNumber(s2))
                            s2 = mem.varNumberString(s2);
                        else if (mem.isString(s2))
                            s2 = mem.varString(s2);
                    }

                    if (is_numeric(s0) && is_numeric(s2))
                    {
                        double n0 = stod(s0), n2 = stod(s2);
                        mem.createVariable(arg0, (int)RNG::random(n0, n2));
                    }
                }
                else
                    error(ErrorMessage::OUT_OF_BOUNDS, s0 + Keywords.RangeSeparator + s2, false);
            }
            else
            {
                exec.executeTemplate(mem.getMethod(before_params(arg2)), parse_params(arg2));

                if (is_numeric(State.LastValue))
                    mem.createVariable(arg0, stod(State.LastValue));
                else
                    mem.createVariable(arg0, State.LastValue);
            }
        }
        else if (mem.variableExists(arg2))
        {
            if (mem.isNumber(arg2))
                mem.createVariable(arg0, mem.varNumber(arg2));
            else if (mem.isString(arg2))
                mem.createVariable(arg0, mem.varString(arg2));
            else
                mem.createVariable(arg0, State.Null);
        }
        else if (arg2 == Keywords.Mask || arg2 == Keywords.ReadLine)
        {
            std::string line("");
            if (arg2 == Keywords.Mask)
            {
                line = get_stdin_quiet("");

                if (is_numeric(line))
                    mem.createVariable(arg0, stod(line));
                else
                    mem.createVariable(arg0, line);
            }
            else
            {
                IO::print("");
                std::getline(std::cin, line, '\n');

                if (is_numeric(line))
                    mem.createVariable(arg0, stod(line));
                else
                    mem.createVariable(arg0, line);
            }
        }
        else if (arg2 == "args.size")
            mem.createVariable(arg0, (double)mem.getArgCount());
        else if (before == Keywords.ReadLine)
        {
            if (mem.variableExists(after))
            {
                if (mem.isString(after))
                {
                    std::string line("");
                    IO::print(pre_parse(mem.varString(after)));
                    std::getline(std::cin, line, '\n');

                    if (is_numeric(line))
                        mem.createVariable(arg0, stod(line));
                    else
                        mem.createVariable(arg0, line);
                }
                else
                {
                    std::string line("");
                    IO::print("");
                    std::getline(std::cin, line, '\n');

                    if (is_numeric(line))
                        mem.createVariable(arg0, stod(line));
                    else
                        mem.createVariable(arg0, line);
                }
            }
            else
            {
                std::string line("");
                IO::print(pre_parse(after));
                std::getline(std::cin, line, '\n');

                if (is_numeric(line))
                    mem.createVariable(arg0, stod(line));
                else
                    mem.createVariable(arg0, line);
            }
        }
        else if (before == Keywords.Mask)
        {
            if (mem.variableExists(after))
            {
                if (mem.isString(after))
                {
                    std::string line("");
                    line = get_stdin_quiet(mem.varString(after));

                    if (is_numeric(line))
                        mem.createVariable(arg0, stod(line));
                    else
                        mem.createVariable(arg0, line);

                    writeline();
                }
                else
                {
                    std::string line("");
                    line = get_stdin_quiet("");

                    if (is_numeric(line))
                        mem.createVariable(arg0, stod(line));
                    else
                        mem.createVariable(arg0, line);

                    writeline();
                }
            }
            else
            {
                std::string line("");
                line = get_stdin_quiet(pre_parse(after));

                if (is_numeric(line))
                    mem.createVariable(arg0, stod(line));
                else
                    mem.createVariable(arg0, line);

                writeline();
            }
        }
        else if (after == Keywords.Size)
        {
            if (!mem.variableExists(before))
            {
                mem.createVariable(arg0, (double)before.length());
                return;
            }

            if (mem.isString(before))
                mem.createVariable(arg0, (double)mem.varString(before).length());
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (recognized_mathfunc(after))
        {
            parse_mathfunc(arg0, before, after);
        }
        else if (after == Keywords.ToUpper)
        {
            if (!mem.variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (mem.isString(before))
                mem.createVariable(arg0, to_upper(mem.varString(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == Keywords.ToLower)
        {
            if (!mem.variableExists(before))
            {
                error(ErrorMessage::VAR_UNDEFINED, before, false);
                return;
            }

            if (mem.isString(before))
                mem.createVariable(arg0, to_lower(mem.varString(before)));
            else
                error(ErrorMessage::CONV_ERR, before, false);
        }
        else if (after == Keywords.FileSize)
        {
            if (!mem.variableExists(before))
            {
                if (Env::fileExists(before))
                    mem.createVariable(arg0, get_filesize(before));
                else
                    error(ErrorMessage::READ_FAIL, before, false);
                return;
            }

            if (!mem.isString(before))
            {
                error(ErrorMessage::CONV_ERR, before, false);
                return;
            }

            if (Env::fileExists(mem.varString(before)))
                mem.createVariable(arg0, get_filesize(mem.varString(before)));
            else
                error(ErrorMessage::READ_FAIL, mem.varString(before), false);
        }
        else if (before == Keywords.Env)
        {
            internal_env_builtins(arg0, after, 0);
        }
        else
        {
            if (is_numeric(arg2))
                mem.createVariable(arg0, stod(arg2));
            else
                mem.createVariable(arg0, pre_parse(arg2));
        }
    }
    else if (arg1 == Operators.AddAssign)
    {
        if (mem.variableExists(arg2))
        {
            if (mem.isString(arg2))
                mem.createVariable(arg0, mem.varString(arg2));
            else if (mem.isNumber(arg2))
                mem.createVariable(arg0, mem.varNumber(arg2));
            else
                mem.createVariable(arg0, State.Null);
        }
        else
        {
            if (is_numeric(arg2))
                mem.createVariable(arg0, stod(arg2));
            else
                mem.createVariable(arg0, pre_parse(arg2));
        }
    }
    else if (arg1 == Operators.SubtractAssign)
    {
        if (mem.variableExists(arg2))
        {
            if (mem.isNumber(arg2))
                mem.createVariable(arg0, 0 - mem.varNumber(arg2));
            else
                mem.createVariable(arg0, State.Null);
        }
        else
        {
            if (is_numeric(arg2))
                mem.createVariable(arg0, stod(arg2));
            else
                mem.createVariable(arg0, pre_parse(arg2));
        }
    }
    else if (arg1 == Keywords.ShellExec)
    {
        if (!mem.variableExists(arg2))
        {
            mem.createVariable(arg0, Env::getStdout(pre_parse(arg2)));
            return;
        }

        if (mem.isString(arg2))
            mem.createVariable(arg0, Env::getStdout(mem.varString(arg2)));
        else
            error(ErrorMessage::CONV_ERR, arg2, false);
    }
    else if (arg1 == Keywords.InlineParse)
    {
        if (!mem.variableExists(arg2))
        {
            mem.createVariable(arg0, get_parsed_stdout(pre_parse(arg2)));
            return;
        }

        if (mem.isString(arg2))
            mem.createVariable(arg0, get_parsed_stdout(mem.varString(arg2)));
        else
            error(ErrorMessage::CONV_ERR, arg2, false);
    }
    else
        error(ErrorMessage::INVALID_OPERATOR, arg2, false);
}

void parse_mathfunc_assignfromvar(std::string arg0, std::string before, std::string after)
{
    if (!mem.variableExists(before))
    {
        error(ErrorMessage::VAR_UNDEFINED, before, false);
        return;
    }

    if (!mem.isNumber(before))
    {
        error(ErrorMessage::CONV_ERR, before, false);
        return;
    }

    double value = mem.varNumber(before);

    if (after == Keywords.MathCos)
        value = cos(value);
    else if (after == Keywords.MathAcos)
        value = acos(value);
    else if (after == Keywords.MathCosh)
        value = cosh(value);
    else if (after == Keywords.MathLog)
        value = log(value);
    else if (after == Keywords.MathSqrt)
        value = sqrt(value);
    else if (after == Keywords.MathAbs)
        value = abs(value);
    else if (after == Keywords.MathFloor)
        value = floor(value);
    else if (after == Keywords.MathCeil)
        value = ceil(value);
    else if (after == Keywords.MathExp)
        value = exp(value);
    else if (after == Keywords.MathSin)
        value = sin(value);
    else if (after == Keywords.MathSinh)
        value = sinh(value);
    else if (after == Keywords.MathAsin)
        value = asin(value);
    else if (after == Keywords.MathTan)
        value = tan(value);
    else if (after == Keywords.MathTanh)
        value = tanh(value);
    else if (after == Keywords.MathAtan)
        value = atan(value);

    if (mem.isNumber(arg0))
        mem.setVariable(arg0, value);
    else if (mem.isString(arg0))
        mem.setVariable(arg0, dtos(value));
    else
        error(ErrorMessage::IS_NULL, arg0, false);
}

void parse_mathfunc(std::string arg0, std::string before, std::string after)
{
    if (!mem.variableExists(before))
    {
        error(ErrorMessage::VAR_UNDEFINED, before, false);
        return;
    }

    if (!mem.isNumber(before))
    {
        error(ErrorMessage::CONV_ERR, before, false);
        return;
    }

    double value = mem.varNumber(before);
    
    if (after == Keywords.MathSin)
        mem.createVariable(arg0, sin(value));
    else if (after == Keywords.MathSinh)
        mem.createVariable(arg0, sinh(value));
    else if (after == Keywords.MathAsin)
        mem.createVariable(arg0, asin(value));
    else if (after == Keywords.MathTan)
        mem.createVariable(arg0, tan(value));
    else if (after == Keywords.MathTanh)
        mem.createVariable(arg0, tanh(value));
    else if (after == Keywords.MathAtan)
        mem.createVariable(arg0, atan(value));
    else if (after == Keywords.MathCos)
        mem.createVariable(arg0, cos(value));
    else if (after == Keywords.MathAcos)
        mem.createVariable(arg0, acos(value));
    else if (after == Keywords.MathCosh)
        mem.createVariable(arg0, cosh(value));
    else if (after == Keywords.MathLog)
        mem.createVariable(arg0, log(value));
    else if (after == Keywords.MathSqrt)
        mem.createVariable(arg0, sqrt(value));
    else if (after == Keywords.MathAbs)
        mem.createVariable(arg0, abs(value));
    else if (after == Keywords.MathFloor)
        mem.createVariable(arg0, floor(value));
    else if (after == Keywords.MathCeil)
        mem.createVariable(arg0, ceil(value));
    else if (after == Keywords.MathExp)
        mem.createVariable(arg0, exp(value));
}

void init_classvar(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command)
{
    std::string className = before_dot(arg2),
           variableName = after_dot(arg2);

    if (mem.classExists(className))
    {
        if (arg1 == Operators.Assign)
        {
            Variable classVariable = mem.getClassVariable(className, variableName);
            if (classVariable.getString() != State.Null)
                mem.createVariable(arg0, classVariable.getString());
            else if (classVariable.getNumber() != State.NullNum)
                mem.createVariable(arg0, classVariable.getNumber());
        }
    }
}

void copy_class(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command)
{
    if (arg1 == Operators.Assign)
    {
        std::vector<Method> classMethods = mem.getClass(arg2).getMethods();
        Class newClass(arg0);

        for (int i = 0; i < (int)classMethods.size(); i++)
            newClass.addMethod(classMethods.at(i));

        std::vector<Variable> classVariables = mem.getClass(arg2).getVariables();

        for (int i = 0; i < (int)classVariables.size(); i++)
            newClass.addVariable(classVariables.at(i));

        newClass.setCollectable(State.ExecutedMethod);

        mem.addClass(newClass);

        State.CurrentClass = arg1;
        State.DefiningClass = false;

        newClass.clear();
        classMethods.clear();
    }
    else
        error(ErrorMessage::INVALID_OPERATOR, arg1, false);
}

void init_const(std::string arg0, std::string arg1, std::string arg2)
{
    if (mem.constantExists(arg0))
    {
        error(ErrorMessage::CONST_DEFINED, arg0, false);
        return;
    }

    if (arg1 != Operators.Assign)
    {
        error(ErrorMessage::INVALID_OPERATOR, arg1, false);
        return;
    }

    if (is_numeric(arg2))
        mem.addConstant(Constant(arg0, stod(arg2)));
    else
        mem.addConstant(Constant(arg0, arg2));
}

void parse_clear(std::string &arg)
{
    if (arg == Keywords.Methods)
        mem.clearMethods();
    else if (arg == Keywords.Classes)
        mem.clearClasses();
    else if (arg == Keywords.Variables)
        mem.clearVariables();
    else if (arg == Keywords.Lists)
        mem.clearLists();
    else if (arg == Keywords.All)
        mem.clearAll();
    else if (arg == Keywords.Constants)
        mem.clearConstants();
}

// TODO: refactor
//	modes:
//		0 = createVariable
//		1 = setVariable
//		2 = setLastValue
void internal_env_builtins(std::string arg0, std::string after, int mode)
{
    std::string defaultValue = DT::timeNow();
    std::string sValue(defaultValue);
    double dValue = 0;

    if (after == Keywords.CurrentDirectory)
        sValue = Env::getCurrentDirectory();
    else if (after == Keywords.UslangApp)
        sValue = State.Application;
    else if (after == Keywords.CurrentUser)
        sValue = Env::getUser();
    else if (after == Keywords.CurrentMachine)
        sValue = Env::getMachine();
    else if (after == Keywords.InitialDirectory)
        sValue = State.InitialDirectory;
    else if (after == Keywords.LastError)
        sValue = State.LastError;
    else if (after == Keywords.LastValue)
        sValue = State.LastValue;
    else
        sValue = Env::getEnvironmentVariable(after);

// TODO: Refactor
/*
    else if (after == "this_second")
        dValue = (double)DT::secondNow();
    else if (after == "this_minute")
        dValue = (double)DT::minuteNow();
    else if (after == "this_hour")
        dValue = (double)DT::hourNow();
    else if (after == "this_month")
        dValue = (double)DT::monthNow();
    else if (after == "this_year")
        dValue = (double)DT::yearNow();
    else if (after == "day_of_this_month")
        dValue = (double)DT::dayOfTheMonth();
    else if (after == "day_of_this_year")
        dValue = (double)DT::dayOfTheYear();
    else if (after == "day_of_this_week")
        sValue = DT::dayOfTheWeek();
    else if (after == "month_of_this_year")
        sValue = DT::monthOfTheYear();
    else if (after == "am_or_pm")
        sValue = DT::amOrPm();
    else if (after == "now")
        sValue = DT::timeNow();
*/

    // TODO: refactor into three different functions.    
    switch (mode)
    {
    case 0:
        if (sValue != defaultValue)
            mem.createVariable(arg0, sValue);
        else
            mem.createVariable(arg0, dValue);
        break;
    case 1:
        if (sValue != defaultValue)
            mem.setVariable(arg0, sValue);
        else
            mem.setVariable(arg0, dValue);
        break;
    case 2:
        if (sValue != defaultValue)
            State.LastValue = sValue;
        else
            State.LastValue = dtos(dValue);
        break;
    case 3:
        if (sValue != defaultValue)
            writeline(sValue);
        else
            writeline(dtos(dValue));
    }
}

// TODO: refactor
void internal_puts(std::string arg0, std::string arg1, bool newline)
{
    std::string text(arg1);
    
    // if parameter is variable, get it's value
    if (mem.variableExists(arg1))
    {
        // set the value
        if (!is_dotless(arg1))
        {
            std::string className = before_dot(arg1), variableName = after_dot(arg1);
            Variable classVariable = mem.getClassVariable(className, variableName);
            
            if (classVariable.getString() != State.Null)
                text = classVariable.getString();
            else if (classVariable.getNumber() != State.NullNum)
                text = dtos(classVariable.getNumber());
            else
            {
                error(ErrorMessage::IS_NULL, arg1, false);
                return;
            }
        }
        else
        {
            if (mem.isString(arg1))
                text = mem.varString(arg1);
            else if (mem.isNumber(arg1))
                text = dtos(mem.varNumber(arg1));
            else
            {
                error(ErrorMessage::IS_NULL, arg1, false);
                return;
            }
        }
    }

    if (newline)
        writeline(text);
    else
        write(text);
}

double get_filesize(std::string path)
{
    int bytes;

    std::ifstream file(path.c_str());

    if (!file.is_open())
    {
        error(ErrorMessage::READ_FAIL, path, false);
        return -DBL_MAX;
    }

    long begin, end;

    begin = file.tellg();
    file.seekg(0, std::ios::end);
    end = file.tellg();

    file.close();

    bytes = (end - begin);

    return bytes;
}

std::string get_stdin_quiet(std::string text)
{
    char *s = getpass(pre_parse(text).c_str());
    return s;
}

void initialize_state(std::string uslang)
{
    State.BadMethodCount = 0,
    State.BadClassCount = 0,
    State.BadVarCount = 0,
    State.CurrentLineNumber = 0,
    State.IfStatementCount = 0,
    State.ForLoopCount = 0,
    State.WhileLoopCount = 0,
    State.ParamVarCount = 0,
    State.LastErrorCode = 0;
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
    State.FailedNest = false,
    State.DefiningNest = false,
    State.DefiningClass = false,
    State.DefiningClassMethod = false,
    State.DefiningParameterizedMethod = false,
    State.SkipCatchBlock = false,
    State.RaiseCatchBlock = false,
    State.DefiningLocalSwitchBlock = false,
    State.DefiningLocalWhileLoop = false,
    State.DefiningLocalForLoop = false;

    State.CurrentClass = "",
    State.CurrentMethodClass = "",
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
    State.NullNum = -DBL_MAX;

    State.Application = uslang;
    State.CurrentScript = uslang;
}

#endif
