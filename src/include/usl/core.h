#ifndef CORE_H
#define CORE_H

void setList(std::string listName, std::string methodName,
             std::vector<std::string> params) {
    if (engine.methodExists(before_params(methodName))) {
        exec.executeTemplate(engine.getMethod(before_params(methodName)),
                             params);

        if (!has_params(State.LastValue)) {
            engine.addItemToList(listName, State.LastValue);
            return;
        }

        std::vector<std::string> last_params = interp_params(State.LastValue);

        for (int i = 0; i < (int)last_params.size(); i++)
            engine.addItemToList(listName, last_params.at(i));
    } else if (engine.classExists(before_dot(before_params(methodName)))) {
        exec.executeTemplate(
            engine.getClass(before_dot(before_params(methodName)))
                .getMethod(after_dot(before_params(methodName))),
            params);

        if (!has_params(State.LastValue)) {
            engine.addItemToList(listName, State.LastValue);
            return;
        }

        std::vector<std::string> last_params = interp_params(State.LastValue);

        for (int i = 0; i < (int)last_params.size(); i++)
            engine.addItemToList(listName, last_params.at(i));
    } else {
        for (int i = 0; i < (int)params.size(); i++) {
            if (!engine.variableExists(params.at(i))) {
                engine.addItemToList(listName, params.at(i));
                continue;
            }

            std::string value = engine.getVariableValueAsString(params.at(i));
            engine.addItemToList(listName, value);
        }
    }
}

std::string get_prompt() {
    if (!State.UseCustomPrompt || State.PromptStyle == "empty") {
        return std::string("> ");
    }

    std::string new_style;
    int length = State.PromptStyle.length();
    char prevChar = 'a';

    for (int i = 0; i < length; i++) {
        switch (State.PromptStyle[i]) {
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
                new_style.append(FileIO::getCurrentDirectory());
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

std::string clean_string(std::string builder) {
    if (engine.variableExists(builder) && is_dotless(builder))
        State.LastValue = engine.getVariableValueAsString(builder);
    else if (engine.methodExists(builder))
        parse(builder);
    else if (has_params(builder)) {
        if (stackReady(builder)) {
            if (isStringStack(builder))
                State.LastValue = getStringStack(builder);
            else
                State.LastValue = dtos(getStack(builder));
        } else if (!is_dotless(builder)) {
            std::string before(before_dot(builder)), after(after_dot(builder));

            if (!engine.classExists(before))
                error(ErrorCode::CLS_METHOD_UNDEFINED, before);

            if (!engine.getClass(before).hasMethod(before_params(after)))
                error(ErrorCode::METHOD_UNDEFINED,
                      before + Keywords.Dot + before_params(after));

            exec.executeTemplate(
                engine.getClass(before).getMethod(before_params(after)),
                interp_params(after));
        } else if (engine.methodExists(before_params(builder)))
            exec.executeTemplate(engine.getMethod(before_params(builder)),
                                 interp_params(builder));
        else
            State.LastValue = "";
    } else if (has_brackets(builder)) {
        // TODO: refactor
        std::string _beforeBrackets(before_brackets(builder)),
            afterBrackets(builder);
        std::string rangeBegin, rangeEnd, _build;

        std::vector<std::string> listRange = interp_bracketrange(afterBrackets);

        if (engine.variableExists(_beforeBrackets) &&
            engine.isString(_beforeBrackets)) {
            std::string tempString(engine.varString(_beforeBrackets));

            if (listRange.size() == 2) {
                rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                if (is_numeric(rangeBegin) && is_numeric(rangeEnd)) {
                    if (stoi(rangeBegin) < stoi(rangeEnd)) {
                        if ((int)tempString.length() - 1 >= stoi(rangeEnd) &&
                            stoi(rangeBegin) >= 0) {
                            for (int z = stoi(rangeBegin); z <= stoi(rangeEnd);
                                 z++)
                                _build.push_back(tempString[z]);

                            State.LastValue = _build;
                        } else
                            error(ErrorCode::OUT_OF_BOUNDS,
                                  rangeBegin + Keywords.RangeSeparator +
                                      rangeEnd);
                    } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
                        if ((int)tempString.length() - 1 >= stoi(rangeEnd) &&
                            stoi(rangeBegin) >= 0) {
                            for (int z = stoi(rangeBegin); z >= stoi(rangeEnd);
                                 z--)
                                _build.push_back(tempString[z]);

                            State.LastValue = _build;
                        } else
                            error(ErrorCode::OUT_OF_BOUNDS,
                                  rangeBegin + Keywords.RangeSeparator +
                                      rangeEnd);
                    } else
                        error(ErrorCode::OUT_OF_BOUNDS,
                              rangeBegin + Keywords.RangeSeparator + rangeEnd);
                }
            } else if (listRange.size() == 1) {
                rangeBegin = listRange.at(0);

                if (!is_numeric(rangeBegin))
                    error(ErrorCode::OUT_OF_BOUNDS, afterBrackets);

                if (stoi(rangeBegin) <= (int)tempString.length() - 1 &&
                    stoi(rangeBegin) >= 0) {
                    std::string _cstr;

                    _cstr.push_back(tempString[stoi(rangeBegin)]);

                    State.LastValue = _cstr;
                } else
                    error(ErrorCode::OUT_OF_BOUNDS, afterBrackets);
            } else
                error(ErrorCode::OUT_OF_BOUNDS, afterBrackets);
        } else if (engine.listExists(_beforeBrackets)) {
            if (listRange.size() == 2) {
                rangeBegin = listRange.at(0), rangeEnd = listRange.at(1);

                if (!(is_numeric(rangeBegin) && is_numeric(rangeEnd)))
                    error(ErrorCode::OUT_OF_BOUNDS,
                          rangeBegin + Keywords.RangeSeparator + rangeEnd);

                if (stoi(rangeBegin) < stoi(rangeEnd)) {
                    if (!(engine.getList(_beforeBrackets).size() - 1 >=
                              stoi(rangeEnd) &&
                          stoi(rangeBegin) >= 0))
                        error(ErrorCode::OUT_OF_BOUNDS,
                              rangeBegin + Keywords.RangeSeparator + rangeEnd);

                    std::string bigString("(");

                    for (int z = stoi(rangeBegin); z <= stoi(rangeEnd); z++) {
                        bigString.append("\"" +
                                         engine.getList(_beforeBrackets).at(z) +
                                         "\"");

                        if (z < stoi(rangeEnd))
                            bigString.push_back(',');
                    }

                    bigString.push_back(')');

                    State.LastValue = bigString;
                } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
                    if (!(engine.getList(_beforeBrackets).size() - 1 >=
                              stoi(rangeEnd) &&
                          stoi(rangeBegin) >= 0)) {
                        error(ErrorCode::OUT_OF_BOUNDS,
                              rangeBegin + Keywords.RangeSeparator + rangeEnd);
                        State.LastValue = "";
                    }

                    std::string bigString("(");

                    for (int z = stoi(rangeBegin); z >= stoi(rangeEnd); z--) {
                        bigString.append("\"" +
                                         engine.getList(_beforeBrackets).at(z) +
                                         "\"");

                        if (z > stoi(rangeEnd))
                            bigString.push_back(',');
                    }

                    bigString.push_back(')');

                    State.LastValue = bigString;
                } else
                    error(ErrorCode::OUT_OF_BOUNDS,
                          rangeBegin + Keywords.RangeSeparator + rangeEnd);
            } else if (listRange.size() == 1) {
                rangeBegin = listRange.at(0);

                if (!is_numeric(rangeBegin) ||
                    (!(stoi(rangeBegin) <=
                           (int)engine.getList(_beforeBrackets).size() - 1 &&
                       stoi(rangeBegin) >= 0)))
                    error(ErrorCode::OUT_OF_BOUNDS, afterBrackets);

                State.LastValue =
                    engine.getList(_beforeBrackets).at(stoi(rangeBegin));
            } else
                error(ErrorCode::OUT_OF_BOUNDS, afterBrackets);
        } else
            State.LastValue = "";
    } else if (!is_dotless(builder)) {
        std::string before(before_dot(builder)), after(after_dot(builder));

        if (!engine.classExists(before))
            error(ErrorCode::CLS_METHOD_UNDEFINED, before);

        if (engine.getClass(before).hasMethod(after))
            parse(before + Keywords.Dot + after);
        else if (engine.getClass(before).hasVariable(after))
            State.LastValue =
                engine.getClassVariableValueAsString(before, after);
        else
            error(ErrorCode::VAR_UNDEFINED, before + Keywords.Dot + after);
    } else
        State.LastValue = builder;

    return State.LastValue;
}

std::string pre_parse(std::string st) {
    std::string cleaned, builder;
    int l = st.length();
    bool buildSymbol = false;

    for (int i = 0; i < l; i++) {
        char curr = st[i], prev = st[i > 0 ? i - 1 : 0],
             next = st[i + 1 > l ? l : i + 1];

        if (buildSymbol) {
            if (curr == '}') {
                builder = subtract_char(builder, '{');
                cleaned += clean_string(builder);
                builder.clear();

                buildSymbol = false;
            } else
                builder.push_back(curr);
        } else {
            // REFACTOR HERE
            if (curr == '\\' && next == 'n') // begin new-line
                cleaned.push_back('\r');
            else if (curr == 'n' && prev == '\\') // end new-line
                cleaned.push_back('\n');
            else if (curr == 't' && prev == '\\') // end tab
                cleaned.push_back('\t');
            else if (curr == ';' && prev == '\\') // end semi-colon
                cleaned.push_back(';');
            else if (curr == '\'' && prev == '\\') // end apostrophe
                cleaned.push_back('\'');
            else if (curr == '#' && next == '{') // begin symbol
            {
                buildSymbol = true;
            } else if (curr == '\\' && next == 't') {
            } // begin tab
            else if (curr == '\\' && next == ';') {
            } // begin semi-colon
            else if (curr == '\\' && next == '\'') {
            } // begin apostrophe
            else
                cleaned.push_back(curr);
        }
    }

    return cleaned;
}

void write(std::string st) {
    if (State.CaptureParse)
        State.ParsedOutput.append(pre_parse(st));
    else
        std::cout << pre_parse(st);

    State.LastValue = st;
}

void writeline(std::string st) { write(st + "\n"); }

void writeline() { write("\n"); }

List getDirectoryList(std::string before, bool filesOnly) {
    List newList;
    std::vector<std::string> dirList =
        FileIO::getDirectoryContents(engine.varString(before), filesOnly);
    for (unsigned int i = 0; i < dirList.size(); i++) {
        newList.add(dirList.at(i));
    }
    if (newList.size() == 0) {
        State.DefiningForLoop = false;
    }
    return newList;
}

void show_version() {
    std::cout << uslang_name << " interpreter "
              << "v" << uslang_version << std::endl
              << std::endl;
}

void help(std::string app) {
    struct CommandInfo {
        std::string command;
        std::string description;
    };

    std::vector<CommandInfo> commands = {
        {"", "start the shell"},
        {"{args}", "start the shell with parameters"},
        {"{script}", "run a script"},
        {"{script} {args}", "run a script with parameters"},
        {"-v, --version", "show current version"},
        {"-p, --parse", "parse a command"},
        {"-h, --help", "show this message"}};

    show_version();

    for (const auto &cmd : commands) {
        std::cout << std::left << std::setw(30) << (app + " " + cmd.command)
                  << cmd.description << std::endl;
    }

    std::cout << std::endl;
}

int load_repl() {
    std::string s;
    bool active = true;

    while (active) {
        try {
            std::cout << get_prompt();

            s.clear();
            std::getline(std::cin, s);
            ++State.CurrentLineNumber;

            if (s != Keywords.Exit) {
                parse(ltrim_ws(s));
                continue;
            }

            if (State.DefiningClass || State.DefiningMethod) {
                parse(s);
                continue;
            }

            active = false;
            engine.clearAll();
            break;
        } catch (const std::exception &e) {
            print_error(e);
            return -1;
        }
    }

    return 0;
}

bool stackReady(std::string arg2) {
    return contains(arg2, Operators.Add) ||
           contains(arg2, Operators.Subtract) ||
           contains(arg2, Operators.Multiply) ||
           contains(arg2, Operators.Divide) ||
           contains(arg2, Operators.Modulus) ||
           contains(arg2, Operators.Exponent);
}

bool isStringStack(std::string arg2) {
    std::string tempArgTwo = arg2, temporaryBuild;
    tempArgTwo = subtract_char(tempArgTwo, '(');
    tempArgTwo = subtract_char(tempArgTwo, ')');

    for (int i = 0; i < (int)tempArgTwo.length(); i++) {
        if (tempArgTwo[i] == ' ' && temporaryBuild.length() != 0) {
            if (is_stackable(temporaryBuild))
                return true;
            continue;
        } else if (is_stack_op(tempArgTwo[i])) {
            if (is_stackable(temporaryBuild))
                return true;
            continue;
        }

        temporaryBuild.push_back(tempArgTwo[i]);
    }

    return false;
}

bool is_stackable(std::string &temporaryBuild) {
    if (engine.variableExists(temporaryBuild)) {
        if (engine.isNumber(temporaryBuild))
            temporaryBuild.clear();
        else if (engine.isString(temporaryBuild))
            return true;
    } else if (engine.methodExists(temporaryBuild)) {
        parse(temporaryBuild);
        if (is_numeric(State.LastValue))
            temporaryBuild.clear();
        else
            return true;
    } else if (!is_numeric(temporaryBuild))
        return true;
    else
        temporaryBuild.clear();

    return false;
}

std::string getStringStack(std::string arg2) {
    std::string tempArgTwo = arg2, temporaryBuild;
    tempArgTwo = subtract_char(tempArgTwo, '(');
    tempArgTwo = subtract_char(tempArgTwo, ')');

    std::string stackValue;

    std::vector<std::string> contents;
    std::vector<std::string> vars;

    bool quoted = false;

    for (int i = 0; i < (int)tempArgTwo.length(); i++) {
        if (tempArgTwo[i] == '\"') {
            quoted = !quoted;
            if (!quoted) {
                contents.push_back(temporaryBuild);
                temporaryBuild.clear();
            }
        } else if (tempArgTwo[i] == ' ' && temporaryBuild.length() != 0 &&
                   !quoted) {
            parseStringStack(contents, vars, temporaryBuild, tempArgTwo[i]);
        } else {
            switch (tempArgTwo[i]) {
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

    if (engine.variableExists(temporaryBuild)) {
        std::string value = engine.getVariableValueAsString(temporaryBuild);
        vars.push_back(temporaryBuild);
        contents.push_back(value);
        temporaryBuild.clear();
    } else {
        contents.push_back(temporaryBuild);
        temporaryBuild.clear();
    }

    bool startOperating = false, addNext = false, subtractNext = false,
         multiplyNext = false;

    for (int i = 0; i < (int)contents.size(); i++) {
        if (startOperating) {
            if (addNext) {
                stackValue.append(contents.at(i));
                addNext = false;
            } else if (subtractNext) {
                stackValue = subtract_string(stackValue, contents.at(i));
                subtractNext = false;
            } else if (multiplyNext && is_numeric(contents.at(i))) {
                stackValue = multiply_string(stackValue, stoi(contents.at(i)));
                multiplyNext = false;
            }

            if (contents.at(i) == Operators.Add)
                addNext = true;
            else if (contents.at(i) == Operators.Subtract)
                subtractNext = true;
            else if (contents.at(i) == Operators.Multiply)
                multiplyNext = true;
        } else {
            startOperating = true;
            stackValue = contents.at(i);
        }
    }

    return stackValue;
}

std::string getStackValue(std::string value) {
    std::string stackValue;

    if (isStringStack(value))
        stackValue = getStringStack(value);
    else if (stackReady(value))
        stackValue = dtos(getStack(value));
    else
        stackValue = value;

    return stackValue;
}

void parseNumberStack(std::vector<std::string> &contents,
                      std::vector<std::string> vars,
                      std::string &temporaryBuild, char currentChar) {
    if (engine.variableExists(temporaryBuild) &&
        engine.isNumber(temporaryBuild)) {
        vars.push_back(temporaryBuild);
        contents.push_back(dtos(engine.varNumber(temporaryBuild)));
        temporaryBuild.clear();
    } else if (engine.methodExists(temporaryBuild)) {
        parse(temporaryBuild);

        if (is_numeric(State.LastValue)) {
            contents.push_back(State.LastValue);
            temporaryBuild.clear();
        }
    } else {
        contents.push_back(temporaryBuild);
        temporaryBuild.clear();
    }

    if (currentChar != ' ') {
        std::string content;
        content = currentChar;
        contents.push_back(content);
    }
}

void parseStringStack(std::vector<std::string> &contents,
                      std::vector<std::string> vars,
                      std::string &temporaryBuild, char currentChar) {
    if (engine.variableExists(temporaryBuild)) {
        std::string value = engine.getVariableValueAsString(temporaryBuild);
        vars.push_back(temporaryBuild);
        contents.push_back(value);
        temporaryBuild.clear();
    } else if (engine.methodExists(temporaryBuild)) {
        parse(temporaryBuild);

        if (is_numeric(State.LastValue)) {
            contents.push_back(State.LastValue);
            temporaryBuild.clear();
        }
    } else {
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

double getStack(std::string arg2) {
    std::string tempArgTwo = arg2, temporaryBuild;
    tempArgTwo = subtract_char(tempArgTwo, '(');
    tempArgTwo = subtract_char(tempArgTwo, ')');

    double stackValue = (double)0.0;

    std::vector<std::string> contents;
    std::vector<std::string> vars;

    for (int i = 0; i < (int)tempArgTwo.length(); i++) {
        if (tempArgTwo[i] == ' ' && temporaryBuild.length() != 0) {
            parseNumberStack(contents, vars, temporaryBuild, ' ');
        } else {
            switch (tempArgTwo[i]) {
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

    if (engine.variableExists(temporaryBuild) &&
        engine.isNumber(temporaryBuild)) {
        vars.push_back(temporaryBuild);
        contents.push_back(dtos(engine.varNumber(temporaryBuild)));
        temporaryBuild.clear();
    } else {
        contents.push_back(temporaryBuild);
        temporaryBuild.clear();
    }

    bool startOperating = false, addNext = false, subtractNext = false,
         multiplyNext = false, divideNext = false, moduloNext = false,
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
        } else if (is_numeric(contents.at(i))) {
            startOperating = true;
            stackValue = stod(contents.at(i));
        }
    }

    return stackValue;
}

std::string getSubString(std::string arg2, std::string beforeBracket) {
    std::string returnValue;

    if (!engine.isString(beforeBracket)) {
        error(ErrorCode::NULL_STRING, beforeBracket);
        return returnValue;
    }

    std::vector<std::string> listRange = interp_bracketrange(arg2);
    std::string variableString = engine.varString(beforeBracket);

    if (listRange.size() < 1 || listRange.size() > 2) {
        error(ErrorCode::OUT_OF_BOUNDS, arg2);
    } else if (listRange.size() == 1) {
        std::string rangeBegin(listRange.at(0));

        if (rangeBegin.length() != 0 && is_numeric(rangeBegin)) {
            int beginIndex = stoi(rangeBegin);

            if ((int)variableString.length() - 1 >= beginIndex &&
                beginIndex >= 0) {
                returnValue = variableString[beginIndex];
            }
        }
    } else if (listRange.size() == 2) {
        std::string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

        if (!(rangeBegin.length() != 0 && rangeEnd.length() != 0) ||
            !((is_numeric(rangeBegin) && is_numeric(rangeEnd)) ||
              !((int)variableString.length() - 1 >= stoi(rangeEnd) &&
                stoi(rangeBegin) >= 0) ||
              !((int)variableString.length() >= stoi(rangeEnd) &&
                stoi(rangeBegin) >= 0))) {
            error(ErrorCode::OUT_OF_BOUNDS,
                  rangeBegin + Keywords.RangeSeparator + rangeEnd);
            return returnValue;
        }

        int beginIndex = stoi(rangeBegin), endIndex = stoi(rangeEnd);

        if (beginIndex < endIndex) {
            for (int i = beginIndex; i <= endIndex; i++) {
                returnValue.push_back(variableString[i]);
            }
        } else if (beginIndex > endIndex) {
            for (int i = beginIndex; i >= endIndex; i--) {
                returnValue.push_back(variableString[i]);
            }
        }
    }

    return returnValue;
}

void setSubString(std::string arg1, std::string arg2,
                  std::string beforeBracket) {
    if (!engine.isString(beforeBracket)) {
        error(ErrorCode::CONV_ERR, beforeBracket);
        return;
    }

    std::vector<std::string> listRange = interp_bracketrange(arg2);
    std::string variableString = engine.varString(beforeBracket);

    if (listRange.size() == 2) {
        std::string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

        if (rangeBegin.length() == 0 || rangeEnd.length() == 0 ||
            !(is_numeric(rangeBegin) && is_numeric(rangeEnd))) {
            error(ErrorCode::OUT_OF_BOUNDS,
                  rangeBegin + Keywords.RangeSeparator + rangeEnd);
            return;
        }

        int beginIndex = stoi(rangeBegin), endIndex = stoi(rangeEnd);

        std::string tempString;

        if (beginIndex < endIndex && beginIndex >= 0 &&
            endIndex <= (int)variableString.length() - 1) {
            for (int i = beginIndex; i <= endIndex; i++)
                tempString.push_back(variableString[i]);

            if (engine.variableExists(arg1))
                engine.setVariable(arg1, tempString);
            else
                engine.createVariable(arg1, tempString);
        } else if (beginIndex > endIndex && beginIndex >= 0 &&
                   endIndex <= (int)variableString.length()) {
            for (int i = beginIndex; i >= endIndex; i--)
                tempString.push_back(variableString[i]);

            if (engine.variableExists(arg1))
                engine.setVariable(arg1, tempString);
            else
                engine.createVariable(arg1, tempString);
        } else
            error(ErrorCode::OUT_OF_BOUNDS,
                  rangeBegin + Keywords.RangeSeparator + rangeEnd);
    } else if (listRange.size() == 1) {
        std::string rangeBegin(listRange.at(0));

        if (rangeBegin.length() != 0 && is_numeric(rangeBegin)) {
            int beginIndex = stoi(rangeBegin);
            if ((int)variableString.length() - 1 >= beginIndex &&
                beginIndex >= 0) {
                std::string tmp_;
                tmp_.push_back(variableString[beginIndex]);

                if (engine.variableExists(arg1))
                    engine.setVariable(arg1, tmp_);
                else
                    engine.createVariable(arg1, tmp_);
            }
        }
    } else
        error(ErrorCode::OUT_OF_BOUNDS, arg2);
}

std::string getStringValue(std::string arg1, std::string op, std::string arg2) {
    std::string firstValue, lastValue, returnValue;

    if (engine.variableExists(arg1) && engine.isString(arg1))
        firstValue = engine.varString(arg1);

    if (engine.variableExists(arg2))
        lastValue = engine.getVariableValueAsString(arg2);
    else if (engine.methodExists(arg2)) {
        parse(arg2);
        lastValue = State.LastValue;
    } else if (!is_dotless(arg2)) {
        std::string _beforeDot(before_dot(arg2)), _afterDot(after_dot(arg2));

        if (_beforeDot == Keywords.Env)
            interp_env_rhs("", _afterDot, 2);
        else if (_beforeDot == Keywords.Args && _afterDot == Keywords.Size) {
            lastValue = itos(engine.getArgCount());
        } else if (engine.classExists(_beforeDot)) {
            exec.executeTemplate(
                engine.getClass(_beforeDot).getMethod(_afterDot),
                interp_params(_afterDot));
            lastValue = State.LastValue;
        } else
            lastValue = arg2;
    } else if (has_brackets(arg2)) {
        std::string _beforeBrackets(before_brackets(arg2)),
            _afterBrackets(after_brackets(arg2));

        if (_beforeBrackets == Keywords.Args) {
            std::vector<std::string> params =
                interp_bracketrange(_afterBrackets);
            int index = stoi(params.at(0));

            if (is_numeric(params.at(0)) && engine.getArgCount() - 1 >= index &&
                index >= 0) {
                if (params.at(0) == "0")
                    lastValue = State.CurrentScript;
                else
                    lastValue = engine.getArg(index);
            }
        } else if (engine.listExists(_beforeBrackets)) {
            _afterBrackets = subtract_string(_afterBrackets, "]");
            int index = stoi(_afterBrackets);

            if (engine.getList(_beforeBrackets).size() >= index && index >= 0)
                lastValue = engine.getList(_beforeBrackets).at(index);
        }
    } else if (has_params(arg2)) {
        if (before_params(arg2).length() != 0) {
            exec.executeTemplate(engine.getMethod(arg2), interp_params(arg2));
            lastValue = State.LastValue;
        } else if (isStringStack(arg2))
            lastValue = getStringStack(arg2);
        else if (stackReady(arg2))
            lastValue = dtos(getStack(arg2));
    } else
        lastValue = arg2;

    if (op == Operators.AddAssign)
        returnValue = (firstValue + lastValue);
    else if (op == Operators.SubtractAssign)
        returnValue = subtract_string(firstValue, lastValue);
    else if (op == Operators.MultiplyAssign && is_numeric(lastValue))
        returnValue = multiply_string(firstValue, stoi(lastValue));
    else if (op == Operators.DivideAssign)
        returnValue = subtract_string(firstValue, lastValue);
    else if (op == Operators.ExponentAssign)
        returnValue = dtos(pow(stod(firstValue), stod(lastValue)));
    else if (op == Operators.Assign)
        returnValue = lastValue;

    State.LastValue = returnValue;
    return returnValue;
}

double getNumberValue(std::string arg1, std::string op, std::string arg2) {
    double firstValue = 0, lastValue = 0, returnValue = 0;

    if (engine.variableExists(arg1) && engine.isNumber(arg1)) {
        firstValue = engine.varNumber(arg1);
    }

    if (engine.variableExists(arg2) && engine.isNumber(arg2)) {
        lastValue = engine.varNumber(arg2);
    } else if (engine.methodExists(arg2)) {
        parse(arg2);
        lastValue = is_numeric(State.LastValue) ? stod(State.LastValue) : 0;
    } else if (!is_dotless(arg2)) {
        std::string _beforeDot(before_dot(arg2)), _afterDot(after_dot(arg2));
        if (_beforeDot == Keywords.Env) {
            interp_env_rhs("", _afterDot, 2);
        } else if (_beforeDot == Keywords.Args && _afterDot == Keywords.Size) {
            lastValue = stod(itos(engine.getArgCount()));
        } else if (engine.classExists(_beforeDot)) {
            exec.executeTemplate(
                engine.getClass(_beforeDot).getMethod(_afterDot),
                interp_params(_afterDot));
            lastValue = is_numeric(State.LastValue) ? stod(State.LastValue) : 0;
        } else if (is_numeric(State.LastValue))
            lastValue = stod(arg2);
    } else if (has_brackets(arg2)) {
        std::string _beforeBrackets(before_brackets(arg2)),
            _afterBrackets(after_brackets(arg2));

        if (engine.listExists(_beforeBrackets)) {
            _afterBrackets = subtract_string(_afterBrackets, "]");
            int index = stoi(_afterBrackets);

            if (engine.getList(_beforeBrackets).size() >= index) {
                if (index >= 0 &&
                    is_numeric(engine.getList(_beforeBrackets).at(index))) {
                    lastValue = stod(engine.getList(_beforeBrackets).at(index));
                }
            }
        }
    } else if (has_params(arg2)) {
        if (before_params(arg2).length() != 0) {
            exec.executeTemplate(engine.getMethod(arg2), interp_params(arg2));
            if (is_numeric(State.LastValue))
                lastValue = stod(State.LastValue);
        } else if (stackReady(arg2)) {
            lastValue = getStack(arg2);
        }
    } else if (is_numeric(arg2))
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

void initializeTemporaryVariable(std::string arg1, std::string arg2,
                                 std::vector<std::string> command,
                                 std::string tempClassVariableName,
                                 std::string className,
                                 std::string variableName) {
    twoSpace(tempClassVariableName, arg1, arg2, command);
    engine.getVar(tempClassVariableName).setName(variableName);
    engine.getClass(className).removeVariable(variableName);
    engine.getClass(className).addVariable(engine.getVar(variableName));
    engine.removeVariable(variableName);
}

void initializeTemporaryString(std::string arg1, std::string arg2,
                               std::vector<std::string> command,
                               std::string tempClassVariableName,
                               std::string className,
                               std::string variableName) {
    engine.createVariable(
        tempClassVariableName,
        engine.getClassVariableValueAsString(className, variableName));
    initializeTemporaryVariable(arg1, arg2, command, tempClassVariableName,
                                className, variableName);
}

void initializeTemporaryNumber(std::string arg1, std::string arg2,
                               std::vector<std::string> command,
                               std::string tempClassVariableName,
                               std::string className,
                               std::string variableName) {
    engine.createVariable(tempClassVariableName,
                          engine.getClass(before_dot(className))
                              .getVariable(after_dot(className))
                              .getNumber());
    initializeTemporaryVariable(arg1, arg2, command, tempClassVariableName,
                                className, variableName);
}

void initializeVariable(std::string arg0, std::string arg1, std::string arg2,
                        std::vector<std::string> command) {
    std::string tmpObjName = before_dot(arg0), tmpVarName = after_dot(arg0);
    bool tmpObjExists = engine.classExists(tmpObjName);
    if (tmpObjExists || begins_with(arg0, "@")) {
        if (tmpObjExists) {
            if (engine.getClass(tmpObjName).getVariable(tmpVarName).getType() ==
                ValueType::String) {
                std::string tempClassVariableName("@ " + tmpObjName +
                                                  tmpVarName + "_string");
                initializeTemporaryString(arg1, arg2, command,
                                          tempClassVariableName, tmpObjName,
                                          tmpVarName);
            } else if (engine.getClass(tmpObjName)
                           .getVariable(tmpVarName)
                           .getType() == ValueType::Double) {
                std::string tempClassVariableName("@____" + tmpObjName + "___" +
                                                  tmpVarName + "_number");
                initializeTemporaryNumber(arg1, arg2, command,
                                          tempClassVariableName, tmpObjName,
                                          tmpVarName);
            }
        } else if (arg1 == Operators.Assign) {
            std::string before(before_dot(arg2)), after(after_dot(arg2));

            if (has_brackets(arg2) &&
                (engine.variableExists(before_brackets(arg2)) ||
                 engine.listExists(before_brackets(arg2)))) {
                std::string beforeBracket(before_brackets(arg2)),
                    afterBracket(after_brackets(arg2));

                afterBracket = subtract_string(afterBracket, "]");

                if (engine.listExists(beforeBracket)) {
                    if (engine.getList(beforeBracket).size() >=
                        stoi(afterBracket)) {
                        if (engine.getList(beforeBracket)
                                .at(stoi(afterBracket)) == "#!=no_line")
                            error(ErrorCode::OUT_OF_BOUNDS, arg2);
                        else {
                            std::string listValue(engine.getList(beforeBracket)
                                                      .at(stoi(afterBracket)));

                            if (is_numeric(listValue)) {
                                if (!engine.isNumber(arg0))
                                    error(ErrorCode::CONV_ERR, arg0);
                                else
                                    engine.setVariable(arg0, stod(listValue));
                            } else {
                                if (!engine.isString(arg0))
                                    error(ErrorCode::CONV_ERR, arg0);
                                else
                                    engine.setVariable(arg0, listValue);
                            }
                        }
                    }
                } else if (engine.isString(beforeBracket))
                    setSubString(arg0, arg2, beforeBracket);
                else
                    error(ErrorCode::LIST_UNDEFINED, beforeBracket);
            } else if (before.length() != 0 && after.length() != 0) {
                if (has_params(arg2)) {
                    if (before_params(arg2) == Keywords.Random) {
                        if (!contains(arg2, Keywords.RangeSeparator)) {
                            error(ErrorCode::INVALID_SEQ_SEP, arg2);
                            return;
                        }

                        std::vector<std::string> range = interp_range(arg2);
                        std::string s0(range.at(0)), s2(range.at(1));

                        if (is_numeric(s0) && is_numeric(s2)) {
                            double n0 = stod(s0), n2 = stod(s2);
                            int rand = (int)RNG::getInstance().random(n0, n2);

                            if (engine.isNumber(arg0))
                                engine.setVariable(arg0, rand);
                            else if (engine.isString(arg0))
                                engine.setVariable(arg0, itos(rand));
                        } else if (engine.variableExists(s0) ||
                                   engine.variableExists(s2)) {
                            if (engine.variableExists(s0)) {
                                if (engine.isNumber(s0))
                                    s0 = dtos(engine.varNumber(s0));
                                else if (engine.isString(s0))
                                    s0 = engine.varString(s0);
                            }

                            if (engine.variableExists(s2)) {
                                if (engine.isNumber(s2))
                                    s2 = engine.varNumberString(s2);
                                else if (engine.isString(s2))
                                    s2 = engine.varString(s2);
                            }

                            if (is_numeric(s0) && is_numeric(s2)) {
                                double n0 = stod(s0), n2 = stod(s2);
                                int rand =
                                    (int)RNG::getInstance().random(n0, n2);
                                if (engine.isNumber(arg0))
                                    engine.setVariable(arg0, rand);
                                else if (engine.isString(arg0))
                                    engine.setVariable(arg0, itos(rand));
                            }
                        } else
                            error(ErrorCode::INVALID_SEQ, s0 + "_" + s2);
                    }
                } else if (engine.listExists(before) &&
                           after == Keywords.Size) {
                    if (engine.isNumber(arg0))
                        engine.setVariable(
                            arg0, stod(itos(engine.getList(before).size())));
                    else if (engine.isString(arg0))
                        engine.setVariable(arg0,
                                           itos(engine.getList(before).size()));
                    else
                        error(ErrorCode::IS_NULL, arg0);
                } else if (before == Keywords.Self) {
                    if (engine.classExists(State.CurrentMethodClass))
                        twoSpace(
                            arg0, arg1,
                            (State.CurrentMethodClass + Keywords.Dot + after),
                            command);
                    else
                        twoSpace(arg0, arg1, after, command);
                } else if (engine.classExists(before)) {
                    if (engine.getClass(before).hasVariable(after)) {
                        const auto classVariable =
                            engine.getClassVariable(before, after);
                        if (classVariable.getType() == ValueType::String)
                            engine.setVariable(arg0, classVariable.getString());
                        else if (classVariable.getType() == ValueType::Double)
                            engine.setVariable(arg0, classVariable.getNumber());
                        else
                            error(ErrorCode::IS_NULL, arg2);
                    } else if (engine.getClass(before).hasMethod(after) &&
                               !has_params(after)) {
                        parse(arg2);

                        if (engine.isString(arg0))
                            engine.setVariable(arg0, State.LastValue);
                        else if (engine.isNumber(arg0))
                            engine.setVariable(arg0, stod(State.LastValue));
                    } else if (has_params(after)) {
                        if (engine.getClass(before).hasMethod(
                                before_params(after))) {
                            exec.executeTemplate(
                                engine.getClass(before).getMethod(
                                    before_params(after)),
                                interp_params(after));

                            if (is_numeric(State.LastValue)) {
                                if (engine.isString(arg0))
                                    engine.setVariable(arg0, State.LastValue);
                                else if (engine.isNumber(arg0))
                                    engine.setVariable(arg0,
                                                       stod(State.LastValue));
                                else
                                    error(ErrorCode::IS_NULL, arg0);
                            } else {
                                if (engine.isString(arg0))
                                    engine.setVariable(arg0, State.LastValue);
                                else if (engine.isNumber(arg0))
                                    error(ErrorCode::CONV_ERR, arg0);
                                else
                                    error(ErrorCode::IS_NULL, arg0);
                            }
                        } else
                            Env::shellExec(arg0);
                    } else
                        error(ErrorCode::VAR_UNDEFINED, arg2);
                } else if (before == Keywords.Env) {
                    interp_env_rhs(arg0, after, 1);
                } else if (is_recognized_math_func(after)) {
                    interp_mathfunc_assignfromvar(arg0, before, after);
                } else if (after == Keywords.ToInteger) {
                    if (engine.variableExists(before)) {
                        if (engine.isString(before))
                            engine.setVariable(
                                arg0, (int)engine.varString(before)[0]);
                        else if (engine.isNumber(before)) {
                            int i = (int)engine.varNumber(before);
                            engine.setVariable(arg0, (double)i);
                        } else
                            error(ErrorCode::IS_NULL, before);
                    } else
                        error(ErrorCode::VAR_UNDEFINED, before);
                } else if (after == Keywords.ToDouble) {
                    if (engine.variableExists(before)) {
                        if (engine.isString(before))
                            engine.setVariable(
                                arg0, (double)engine.varString(before)[0]);
                        else if (engine.isNumber(before)) {
                            double i = engine.varNumber(before);
                            engine.setVariable(arg0, (double)i);
                        } else
                            error(ErrorCode::IS_NULL, before);
                    } else
                        error(ErrorCode::VAR_UNDEFINED, before);
                } else if (after == Keywords.ToString) {
                    if (engine.variableExists(before)) {
                        if (engine.isNumber(before))
                            engine.setVariable(arg0,
                                               dtos(engine.varNumber(before)));
                        else
                            error(ErrorCode::IS_NULL, before);
                    } else
                        error(ErrorCode::VAR_UNDEFINED, before);
                } else if (after == Keywords.ToNumber) {
                    if (!engine.variableExists(before)) {
                        error(ErrorCode::VAR_UNDEFINED, before);
                        return;
                    }

                    if (engine.isString(before))
                        engine.setVariable(arg0,
                                           stod(engine.varString(before)));
                    else
                        error(ErrorCode::IS_NULL, before);
                } else if (before == Keywords.ReadLine) {
                    if (engine.variableExists(after)) {
                        if (engine.isString(after))
                            write(pre_parse(engine.varString(after)));

                        std::string line;
                        std::getline(std::cin, line, '\n');

                        if (engine.isNumber(arg0)) {
                            if (is_numeric(line))
                                engine.setVariable(arg0, stod(line));
                            else
                                error(ErrorCode::CONV_ERR, line);
                        } else if (engine.isString(arg0))
                            engine.setVariable(arg0, line);
                        else
                            error(ErrorCode::IS_NULL, arg0);
                    } else {
                        std::string line;
                        std::cout << pre_parse(after);
                        std::getline(std::cin, line, '\n');

                        if (is_numeric(line))
                            engine.setVariable(arg0, stod(line));
                        else
                            engine.setVariable(arg0, line);
                    }
                } else if (before == Keywords.Mask) {
                    if (engine.variableExists(after)) {
                        std::string prompt = engine.isString(after)
                                                 ? engine.varString(after)
                                                 : "";
                        std::string line;
                        line = get_stdin_quiet(prompt);

                        if (engine.isNumber(arg0)) {
                            if (is_numeric(line))
                                engine.setVariable(arg0, stod(line));
                            else
                                error(ErrorCode::CONV_ERR, line);
                        } else if (engine.isString(arg0))
                            engine.setVariable(arg0, line);
                        else
                            error(ErrorCode::IS_NULL, arg0);

                        writeline();
                    } else {
                        std::string line;
                        line = get_stdin_quiet(pre_parse(after));

                        if (is_numeric(line))
                            engine.setVariable(arg0, stod(line));
                        else
                            engine.setVariable(arg0, line);

                        writeline();
                    }
                } else if (after == Keywords.ToLower &&
                           engine.variableExists(before)) {
                    if (!engine.isString(arg0) || !engine.isString(before)) {
                        error(ErrorCode::CONV_ERR, before);
                        return;
                    }

                    engine.setVariable(arg0,
                                       to_lower(engine.varString(before)));
                } else if (after == Keywords.Read) {
                    if (!engine.isString(arg0)) {
                        error(ErrorCode::CONV_ERR, arg0);
                        return;
                    }

                    if (engine.variableExists(before)) {
                        if (!engine.isString(before)) {
                            error(ErrorCode::NULL_STRING, before);
                            return;
                        }

                        if (!FileIO::fileExists(engine.varString(before))) {
                            error(ErrorCode::FILE_NOT_FOUND,
                                  engine.varString(before));
                            return;
                        }

                        std::string filePath = engine.varString(before);
                        engine.setVariable(arg0, FileIO::readText(filePath));
                    } else {
                        if (!FileIO::fileExists(before)) {
                            error(ErrorCode::FILE_NOT_FOUND, before);
                            return;
                        }

                        engine.setVariable(arg0, FileIO::readText(before));
                    }
                } else if (after == Keywords.ToUpper &&
                           engine.variableExists(before)) {
                    if (!engine.isString(arg0) || !engine.isString(before)) {
                        error(ErrorCode::CONV_ERR, before);
                        return;
                    }

                    engine.setVariable(arg0,
                                       to_upper(engine.varString(before)));
                } else if (after == Keywords.Size) {
                    if (engine.variableExists(before)) {
                        if (!engine.isNumber(arg0)) {
                            error(ErrorCode::CONV_ERR, arg0);
                            return;
                        }

                        if (engine.isString(before))
                            engine.setVariable(
                                arg0,
                                (double)engine.varString(before).length());
                        else
                            error(ErrorCode::CONV_ERR, before);
                    } else {
                        if (engine.isNumber(arg0))
                            engine.setVariable(arg0, (double)before.length());
                        else
                            error(ErrorCode::CONV_ERR, arg0);
                    }
                } else if (after == Keywords.FileSize) {
                    if (engine.isNumber(arg0)) {
                        if (engine.variableExists(before)) {
                            if (!engine.isString(before)) {
                                error(ErrorCode::CONV_ERR, before);
                                return;
                            }

                            if (FileIO::fileExists(engine.varString(before)))
                                engine.setVariable(
                                    arg0, FileIO::getFileSize(
                                              engine.varString(before)));
                            else
                                error(ErrorCode::READ_FAIL,
                                      engine.varString(before));
                        } else {
                            if (FileIO::fileExists(before))
                                engine.setVariable(arg0,
                                                   FileIO::getFileSize(before));
                            else
                                error(ErrorCode::READ_FAIL, before);
                        }
                    } else
                        error(ErrorCode::CONV_ERR, arg0);
                } else {
                    if (engine.isNumber(arg0)) {
                        if (is_numeric(arg2))
                            engine.setVariable(arg0, stod(arg2));
                        else
                            engine.setVariable(arg0, arg2);
                    } else if (engine.isString(arg0))
                        engine.setVariable(arg0, arg2);
                    else
                        error(ErrorCode::IS_NULL, arg0);
                }
            } else {
                if (arg2 == "null") {
                    if (engine.isString(arg0))
                        engine.getVar(arg0).setNull();
                    else if (engine.isNumber(arg0))
                        engine.getVar(arg0).setNull();
                    else
                        error(ErrorCode::IS_NULL, arg0);
                } else if (engine.constantExists(arg2)) {
                    if (engine.isString(arg0)) {
                        if (engine.getConstant(arg2).getType() ==
                            ValueType::Double)
                            engine.setVariable(
                                arg0,
                                dtos(engine.getConstant(arg2).getNumber()));
                        else if (engine.getConstant(arg2).getType() ==
                                 ValueType::String)
                            engine.setVariable(
                                arg0, engine.getConstant(arg2).getString());
                    } else if (engine.isNumber(arg0)) {
                        if (engine.getConstant(arg2).getType() ==
                            ValueType::Double)
                            engine.setVariable(
                                arg0, engine.getConstant(arg2).getNumber());
                        else
                            error(ErrorCode::CONV_ERR, arg2);
                    } else
                        error(ErrorCode::IS_NULL, arg0);
                } else if (engine.methodExists(arg2)) {
                    parse(arg2);

                    if (engine.isString(arg0))
                        engine.setVariable(arg0, State.LastValue);
                    else if (engine.isNumber(arg0))
                        engine.setVariable(arg0, stod(State.LastValue));
                } else if (engine.variableExists(arg2)) {
                    if (engine.isString(arg2)) {
                        if (engine.isString(arg0))
                            engine.setVariable(arg0, engine.varString(arg2));
                        else if (engine.isNumber(arg0))
                            error(ErrorCode::CONV_ERR, arg2);
                        else
                            error(ErrorCode::IS_NULL, arg0);
                    } else if (engine.isNumber(arg2)) {
                        if (engine.isString(arg0))
                            engine.setVariable(arg0,
                                               dtos(engine.varNumber(arg2)));
                        else if (engine.isNumber(arg0))
                            engine.setVariable(arg0, engine.varNumber(arg2));
                        else
                            error(ErrorCode::IS_NULL, arg0);
                    } else
                        error(ErrorCode::IS_NULL, arg2);
                } else if (arg2 == Keywords.Mask || arg2 == Keywords.ReadLine) {
                    if (arg2 == Keywords.Mask) {
                        std::string masked;
                        masked = get_stdin_quiet("");

                        if (engine.isNumber(arg0)) {
                            if (is_numeric(masked))
                                engine.setVariable(arg0, stod(masked));
                            else
                                error(ErrorCode::CONV_ERR, masked);
                        } else if (engine.isString(arg0))
                            engine.setVariable(arg0, masked);
                        else
                            engine.setVariable(arg0, masked);
                    } else {
                        std::string line;
                        std::getline(std::cin, line, '\n');

                        if (is_numeric(line))
                            engine.createVariable(arg0, stod(line));
                        else
                            engine.createVariable(arg0, line);
                    }
                } else if (has_params(arg2)) {
                    if (engine.methodExists(before_params(arg2))) {
                        exec.executeTemplate(
                            engine.getMethod(before_params(arg2)),
                            interp_params(arg2));

                        if (engine.isString(arg0))
                            engine.setVariable(arg0, State.LastValue);
                        else if (engine.isNumber(arg0))
                            engine.setVariable(arg0, stod(State.LastValue));
                    } else if (isStringStack(arg2)) {
                        if (engine.isString(arg0))
                            engine.setVariable(arg0, getStringStack(arg2));
                        else
                            error(ErrorCode::CONV_ERR, arg0);
                    } else if (stackReady(arg2)) {
                        if (engine.isString(arg0))
                            engine.setVariable(arg0, dtos(getStack(arg2)));
                        else if (engine.isNumber(arg0))
                            engine.setVariable(arg0, getStack(arg2));
                        else
                            error(ErrorCode::IS_NULL, arg0);
                    }
                } else {
                    if (is_numeric(arg2)) {
                        if (engine.isNumber(arg0))
                            engine.setVariable(arg0, stod(arg2));
                        else if (engine.isString(arg0))
                            engine.setVariable(arg0, arg2);
                    } else {
                        if (engine.isNumber(arg0))
                            error(ErrorCode::CONV_ERR, arg0);
                        else if (engine.isString(arg0))
                            engine.setVariable(arg0, pre_parse(arg2));
                    }
                }
            }
        } else if (is_recognized_2space(arg1))
            interp_assign(arg0, arg1, arg2);
        else
            print_underconstruction();
    }
}

void interp_assign(std::string arg0, std::string arg1, std::string arg2) {
    std::string first, second;

    bool arg0IsString = engine.isString(arg0),
         arg0IsNumber = engine.isNumber(arg0);

    if (!arg0IsString && !arg0IsNumber) {
        error(ErrorCode::IS_NULL, arg2);
        return;
    }

    first =
        arg0IsString ? engine.varString(arg0) : dtos(engine.varNumber(arg0));

    if (engine.variableExists(arg2))
        second = engine.getVariableValueAsString(arg2);
    else {
        if (has_params(arg2)) {
            if (isStringStack(arg2) && engine.isString(arg0)) {
                second = getStringStack(arg2);
            } else if (stackReady(arg2) && engine.isNumber(arg0)) {
                second = dtos(getStack(arg2));
            } else if (engine.methodExists(before_params(arg2))) {
                exec.executeTemplate(engine.getMethod(before_params(arg2)),
                                     interp_params(arg2));
                second = State.LastValue;
            } else if (engine.classExists(before_dot(arg2))) {
                exec.executeTemplate(engine.getMethod(before_params(arg2)),
                                     interp_params(arg2));
                second = State.LastValue;
            }
        } else if (engine.methodExists(arg2)) {
            parse(arg2);
            second = State.LastValue;
        } else if (is_numeric(arg2)) {
            second = arg2;
        } else {
            second = pre_parse(arg2);
        }
    }

    bool firstIsNumeric = is_numeric(first),
         secondIsNumeric = is_numeric(second);
    double firstNumber = firstIsNumeric ? stod(first) : 0,
           secondNumber = secondIsNumeric ? stod(second) : 0;

    if (firstIsNumeric && secondIsNumeric) {
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
        else {
            error(ErrorCode::INVALID_OP, arg1);
            return;
        }

        engine.setVariable(arg0, result);
    } else if (!firstIsNumeric && secondIsNumeric) {
        std::string result(first);
        if (arg1 == Operators.Assign)
            result = dtos(secondNumber);
        else if (arg1 == Operators.AddAssign)
            result = result + dtos(secondNumber);
        else if (arg1 == Operators.SubtractAssign)
            result = subtract_string(result, second);
        else if (arg1 == Operators.MultiplyAssign)
            result = multiply_string(result, (int)secondNumber);
        else {
            error(ErrorCode::INVALID_OP, arg1);
            return;
        }

        engine.setVariable(arg0, result);
    } else if (!firstIsNumeric && !secondIsNumeric) {
        std::string result(first);
        if (arg1 == Operators.Assign)
            result = second;
        else if (arg1 == Operators.AddAssign)
            result += second;
        else if (arg1 == Keywords.ShellExec)
            result = Env::getStdout(second.c_str());
        else if (arg1 == Keywords.InlineParse)
            result = get_parsed_stdout(second.c_str());
        else {
            error(ErrorCode::INVALID_OP, arg1);
            return;
        }

        engine.setVariable(arg0, result);
    } else
        error(ErrorCode::CONV_ERR, arg0 + " " + arg1 + " " + arg2);
}

void init_listvalues(std::string arg0, std::string arg1, std::string arg2) {
    std::string _b(before_dot(arg2)), _a(after_dot(arg2)),
        __b(before_params(arg2));

    if (has_brackets(arg0)) {
        std::string after(after_brackets(arg0)), before(before_brackets(arg0));
        after = subtract_string(after, "]");

        if (engine.getList(before).size() >= stoi(after)) {
            if (stoi(after) == 0) {
                if (arg1 == Operators.Assign) {
                    if (engine.variableExists(arg2)) {
                        if (engine.isString(arg2))
                            engine.replaceElement(before, after,
                                                  engine.varString(arg2));
                        else if (engine.isNumber(arg2))
                            engine.replaceElement(before, after,
                                                  dtos(engine.varNumber(arg2)));
                        else
                            error(ErrorCode::IS_NULL, arg2);
                    } else
                        engine.replaceElement(before, after, arg2);
                }
            } else if (engine.getList(before).at(stoi(after)) == "#!=no_line")
                error(ErrorCode::OUT_OF_BOUNDS, arg0);
            else {
                if (arg1 == Operators.Assign) {
                    if (engine.variableExists(arg2)) {
                        if (engine.isString(arg2))
                            engine.replaceElement(before, after,
                                                  engine.varString(arg2));
                        else if (engine.isNumber(arg2))
                            engine.replaceElement(before, after,
                                                  dtos(engine.varNumber(arg2)));
                        else
                            error(ErrorCode::IS_NULL, arg2);
                    } else
                        engine.replaceElement(before, after, arg2);
                }
            }
        } else
            error(ErrorCode::OUT_OF_BOUNDS, arg2);
    } else if (has_brackets(arg2)) // INITIALIZE LIST FROM RANGE
    {
        std::string listName(before_brackets(arg2));

        if (!engine.listExists(listName)) {
            error(ErrorCode::LIST_UNDEFINED, listName);
            return;
        }

        std::vector<std::string> listRange = interp_bracketrange(arg2);

        if (listRange.size() != 2) {
            error(ErrorCode::OUT_OF_BOUNDS, arg2);
            return;
        }

        std::string rangeBegin(listRange.at(0)), rangeEnd(listRange.at(1));

        if (!(rangeBegin.length() != 0 && rangeEnd.length() != 0)) {
            error(ErrorCode::OUT_OF_BOUNDS,
                  rangeBegin + Keywords.RangeSeparator + rangeEnd);
            return;
        }

        if (!(is_numeric(rangeBegin) && is_numeric(rangeEnd))) {
            error(ErrorCode::OUT_OF_BOUNDS,
                  rangeBegin + Keywords.RangeSeparator + rangeEnd);
            return;
        }

        if (stoi(rangeBegin) < stoi(rangeEnd)) {
            if (!(engine.getList(listName).size() >= stoi(rangeEnd) &&
                  stoi(rangeBegin) >= 0) ||
                !(stoi(rangeBegin) >= 0)) {
                error(ErrorCode::OUT_OF_BOUNDS, rangeEnd);
                return;
            }

            if (arg1 == Operators.AddAssign) {
                for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                    engine.addItemToList(arg0, engine.getList(listName).at(i));
            } else if (arg1 == Operators.Assign) {
                engine.getList(arg0).clear();

                for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++)
                    engine.addItemToList(arg0, engine.getList(listName).at(i));
            } else
                error(ErrorCode::INVALID_OPERATOR, arg1);
        } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
            if (!(engine.getList(listName).size() >= stoi(rangeEnd) &&
                  stoi(rangeBegin) >= 0) ||
                !(stoi(rangeBegin) >= 0)) {
                error(ErrorCode::OUT_OF_BOUNDS, rangeEnd);
                return;
            }

            if (arg1 == Operators.AddAssign) {
                for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                    engine.addItemToList(arg0, engine.getList(listName).at(i));
            } else if (arg1 == Operators.Assign) {
                engine.getList(arg0).clear();

                for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--)
                    engine.addItemToList(arg0, engine.getList(listName).at(i));
            } else
                error(ErrorCode::INVALID_OPERATOR, arg1);
        } else
            error(ErrorCode::OUT_OF_BOUNDS,
                  rangeBegin + Keywords.RangeSeparator + rangeEnd);
    } else if (engine.variableExists(_b) && contains(_a, Keywords.Split) &&
               arg1 == Operators.Assign) {
        if (!engine.isString(_b)) {
            error(ErrorCode::NULL_STRING, _b);
            return;
        }

        std::vector<std::string> params = interp_params(_a);
        std::vector<std::string> elements;

        if (params.at(0) == "")
            elements = split(engine.varString(_b), ' ');
        else {
            if (params.at(0)[0] == ';')
                elements = split(engine.varString(_b), ';');
            else
                elements = split(engine.varString(_b), params.at(0)[0]);
        }

        engine.getList(arg0).clear();

        for (int i = 0; i < (int)elements.size(); i++)
            engine.addItemToList(arg0, elements.at(i));
    } else if (has_params(arg2)) // ADD/REMOVE ARRAY FROM LIST
    {
        std::vector<std::string> params = interp_params(arg2);

        if (arg1 == Operators.Assign) {
            engine.getList(arg0).clear();
            setList(arg0, arg2, params);
        } else if (arg1 == Operators.AddAssign)
            setList(arg0, arg2, params);
        else if (arg1 == Operators.SubtractAssign) {
            for (int i = 0; i < (int)params.size(); i++) {
                if (engine.variableExists(params.at(i))) {
                    if (engine.isString(params.at(i)))
                        engine.getList(arg0).remove(
                            engine.varString(params.at(i)));
                    else if (engine.isNumber(params.at(i)))
                        engine.getList(arg0).remove(
                            dtos(engine.varNumber(params.at(i))));
                    else
                        error(ErrorCode::IS_NULL, params.at(i));
                } else
                    engine.getList(arg0).remove(params.at(i));
            }
        } else
            error(ErrorCode::INVALID_OPERATOR, arg1);
    } else if (engine.variableExists(
                   arg2)) // ADD/REMOVE VARIABLE VALUE TO/FROM LIST
    {
        if (arg1 == Operators.AddAssign) {
            if (engine.isString(arg2))
                engine.addItemToList(arg0, engine.varString(arg2));
            else if (engine.isNumber(arg2))
                engine.addItemToList(arg0, dtos(engine.varNumber(arg2)));
            else
                error(ErrorCode::CONV_ERR, arg2);
        } else if (arg1 == Operators.SubtractAssign) {
            if (engine.isString(arg2))
                engine.getList(arg0).remove(engine.varString(arg2));
            else if (engine.isNumber(arg2))
                engine.getList(arg0).remove(dtos(engine.varNumber(arg2)));
            else
                error(ErrorCode::CONV_ERR, arg2);
        } else
            error(ErrorCode::INVALID_OPERATOR, arg1);
    } else if (engine.methodExists(arg2)) // INITIALIZE LIST FROM METHOD RETURN
    {
        parse(arg2);

        std::vector<std::string> _p = interp_params(State.LastValue);

        if (arg1 == Operators.Assign) {
            engine.getList(arg0).clear();

            for (int i = 0; i < (int)_p.size(); i++)
                engine.addItemToList(arg0, _p.at(i));
        } else if (arg1 == Operators.AddAssign) {
            for (int i = 0; i < (int)_p.size(); i++)
                engine.addItemToList(arg0, _p.at(i));
        } else
            error(ErrorCode::INVALID_OPERATOR, arg1);
    } else // ADD/REMOVE STRING TO/FROM LIST
    {
        if (arg1 == Operators.AddAssign) {
            if (arg2.length() != 0)
                engine.addItemToList(arg0, arg2);
            else
                error(ErrorCode::IS_EMPTY, arg2);
        } else if (arg1 == Operators.SubtractAssign) {
            if (arg2.length() != 0)
                engine.getList(arg0).remove(arg2);
            else
                error(ErrorCode::IS_EMPTY, arg2);
        }
    }
}

void init_globalvar(std::string arg0, std::string arg1, std::string arg2,
                    std::vector<std::string> command) {
    if (arg1 == Operators.Assign) {
        std::string before(before_dot(arg2)), after(after_dot(arg2));

        if (has_brackets(arg2) &&
            (engine.variableExists(before_brackets(arg2)) ||
             engine.listExists(before_brackets(arg2)))) {
            std::string beforeBracket(before_brackets(arg2)),
                afterBracket(after_brackets(arg2));

            afterBracket = subtract_string(afterBracket, "]");

            if (engine.listExists(beforeBracket)) {
                if (engine.getList(beforeBracket).size() >=
                    stoi(afterBracket)) {
                    if (engine.getList(beforeBracket).at(stoi(afterBracket)) ==
                        "#!=no_line")
                        error(ErrorCode::OUT_OF_BOUNDS, arg2);
                    else {
                        std::string listValue(engine.getList(beforeBracket)
                                                  .at(stoi(afterBracket)));

                        if (is_numeric(listValue))
                            engine.createVariable(arg0, stod(listValue));
                        else
                            engine.createVariable(arg0, listValue);
                    }
                } else
                    error(ErrorCode::OUT_OF_BOUNDS, arg2);
            } else if (engine.variableExists(beforeBracket))
                setSubString(arg0, arg2, beforeBracket);
            else
                error(ErrorCode::LIST_UNDEFINED, beforeBracket);
        } else if (engine.listExists(before) && after == Keywords.Size)
            engine.createVariable(arg0,
                                  stod(itos(engine.getList(before).size())));
        else if (before == Keywords.Self) {
            if (engine.classExists(State.CurrentMethodClass))
                twoSpace(arg0, arg1,
                         (State.CurrentMethodClass + Keywords.Dot + after),
                         command);
            else
                twoSpace(arg0, arg1, after, command);
        } else if (after == Keywords.ToInteger) {
            if (!engine.variableExists(before)) {
                error(ErrorCode::VAR_UNDEFINED, before);
                return;
            }

            if (engine.isString(before))
                engine.createVariable(arg0, (int)engine.varString(before)[0]);
            else if (engine.isNumber(before)) {
                int i = (int)engine.varNumber(before);
                engine.createVariable(arg0, (double)i);
            } else
                error(ErrorCode::IS_NULL, before);
        } else if (after == Keywords.ToDouble) {
            if (!engine.variableExists(before)) {
                error(ErrorCode::VAR_UNDEFINED, before);
                return;
            }

            if (engine.isString(before))
                engine.createVariable(arg0,
                                      (double)engine.varString(before)[0]);
            else if (engine.isNumber(before)) {
                double i = engine.varNumber(before);
                engine.createVariable(arg0, (double)i);
            } else
                error(ErrorCode::IS_NULL, before);
        } else if (after == Keywords.ToString) {
            if (!engine.variableExists(before)) {
                error(ErrorCode::VAR_UNDEFINED, before);
                return;
            }

            if (engine.isNumber(before))
                engine.createVariable(arg0, dtos(engine.varNumber(before)));
            else
                error(ErrorCode::IS_NULL, before);
        } else if (after == Keywords.ToNumber) {
            if (!engine.variableExists(before)) {
                error(ErrorCode::VAR_UNDEFINED, before);
                return;
            }

            if (engine.isString(before))
                engine.createVariable(arg0, stod(engine.varString(before)));
            else
                error(ErrorCode::IS_NULL, before);
        } else if (engine.classExists(before)) {
            if (engine.getClass(before).hasMethod(after) &&
                !has_params(after)) {
                parse(arg2);

                if (is_numeric(State.LastValue))
                    engine.createVariable(arg0, stod(State.LastValue));
                else
                    engine.createVariable(arg0, State.LastValue);
            } else if (has_params(after)) {
                if (!engine.getClass(before).hasMethod(before_params(after))) {
                    Env::shellExec(arg0);
                    return;
                }

                exec.executeTemplate(
                    engine.getClass(before).getMethod(before_params(after)),
                    interp_params(after));

                if (is_numeric(State.LastValue))
                    engine.createVariable(arg0, stod(State.LastValue));
                else
                    engine.createVariable(arg0, State.LastValue);
            } else if (engine.getClass(before).hasVariable(after)) {
                const auto classVariable =
                    engine.getClassVariable(before, after);

                if (classVariable.getType() == ValueType::String)
                    engine.createVariable(arg0, classVariable.getString());
                else if (classVariable.getType() == ValueType::Double)
                    engine.createVariable(arg0, classVariable.getNumber());
                else
                    error(ErrorCode::IS_NULL, classVariable.name());
            }
        } else if (engine.variableExists(before) && after == Keywords.Read) {
            if (!engine.isString(before)) {
                error(ErrorCode::CONV_ERR, before);
                return;
            }

            if (!FileIO::fileExists(engine.varString(before))) {
                error(ErrorCode::READ_FAIL, engine.varString(before));
                return;
            }

            std::ifstream file(engine.varString(before).c_str());
            std::string line, bigString;

            if (!file.is_open()) {
                error(ErrorCode::READ_FAIL, engine.varString(before));
                return;
            }

            while (!file.eof()) {
                std::getline(file, line);
                bigString.append(line + "\r\n");
            }

            file.close();

            engine.createVariable(arg0, bigString);
        } else if (State.DefiningClass) {
            if (is_numeric(arg2)) {
                Variable newVariable(arg0, stod(arg2));

                if (State.DefiningPrivateCode)
                    newVariable.setPrivate();
                else if (State.DefiningPublicCode)
                    newVariable.setPublic();

                engine.getClass(State.CurrentClass).addVariable(newVariable);
            } else {
                Variable newVariable(arg0, arg2);

                if (State.DefiningPrivateCode)
                    newVariable.setPrivate();
                else if (State.DefiningPublicCode)
                    newVariable.setPublic();

                engine.getClass(State.CurrentClass).addVariable(newVariable);
            }
        } else if (arg2 == "null")
            engine.createVariable(arg0, arg2);
        else if (engine.methodExists(arg2)) {
            parse(arg2);

            if (is_numeric(State.LastValue))
                engine.createVariable(arg0, stod(State.LastValue));
            else
                engine.createVariable(arg0, State.LastValue);
        } else if (engine.constantExists(arg2)) {
            if (engine.getConstant(arg2).getType() == ValueType::Double)
                engine.createVariable(arg0,
                                      engine.getConstant(arg2).getNumber());
            else if (engine.getConstant(arg2).getType() == ValueType::String)
                engine.createVariable(arg0,
                                      engine.getConstant(arg2).getString());
            else
                error(ErrorCode::CONV_ERR, arg2);
        } else if (has_params(arg2)) {
            if (isStringStack(arg2))
                engine.createVariable(arg0, getStringStack(arg2));
            else if (stackReady(arg2))
                engine.createVariable(arg0, getStack(arg2));
            else if (before_params(arg2) == Keywords.Random) {
                if (!contains(arg2, Keywords.RangeSeparator)) {
                    error(ErrorCode::INVALID_RANGE_SEP, arg2);
                    return;
                }

                std::vector<std::string> range = interp_range(arg2);
                std::string s0(range.at(0)), s2(range.at(1));

                if (is_numeric(s0) && is_numeric(s2)) {
                    double n0 = stod(s0), n2 = stod(s2);
                    engine.createVariable(
                        arg0, (int)RNG::getInstance().random(std::min(n0, n2),
                                                             std::max(n0, n2)));
                } else if (engine.variableExists(s0) ||
                           engine.variableExists(s2)) {
                    if (engine.variableExists(s0))
                        s0 = engine.getVariableValueAsString(s0);

                    if (engine.variableExists(s2))
                        s2 = engine.getVariableValueAsString(s2);

                    if (!is_numeric(s0)) {
                        error(ErrorCode::CONV_ERR, s0);
                        return;
                    }

                    if (!is_numeric(s2)) {
                        error(ErrorCode::CONV_ERR, s2);
                        return;
                    }

                    double n0 = stod(s0), n2 = stod(s2);
                    engine.createVariable(
                        arg0, (int)RNG::getInstance().random(n0, n2));
                } else
                    error(ErrorCode::OUT_OF_BOUNDS,
                          s0 + Keywords.RangeSeparator + s2);
            } else {
                exec.executeTemplate(engine.getMethod(before_params(arg2)),
                                     interp_params(arg2));

                if (is_numeric(State.LastValue))
                    engine.createVariable(arg0, stod(State.LastValue));
                else
                    engine.createVariable(arg0, State.LastValue);
            }
        } else if (engine.variableExists(arg2)) {
            if (engine.isNumber(arg2))
                engine.createVariable(arg0, engine.varNumber(arg2));
            else if (engine.isString(arg2))
                engine.createVariable(arg0, engine.varString(arg2));
            else
                engine.createVariable(arg0, State.Null);
        } else if (arg2 == Keywords.Mask || arg2 == Keywords.ReadLine) {
            std::string line;
            if (arg2 == Keywords.Mask) {
                line = get_stdin_quiet("");

                if (is_numeric(line))
                    engine.createVariable(arg0, stod(line));
                else
                    engine.createVariable(arg0, line);
            } else {
                std::getline(std::cin, line, '\n');

                if (is_numeric(line))
                    engine.createVariable(arg0, stod(line));
                else
                    engine.createVariable(arg0, line);
            }
        } else if (arg2 == "args.size")
            engine.createVariable(arg0, (double)engine.getArgCount());
        else if (before == Keywords.ReadLine) {
            if (engine.variableExists(after) && engine.isString(after))
                std::cout << pre_parse(engine.varString(after));
            else
                std::cout << pre_parse(after);

            std::string line;
            std::getline(std::cin, line, '\n');

            if (is_numeric(line))
                engine.createVariable(arg0, stod(line));
            else
                engine.createVariable(arg0, line);
        } else if (before == Keywords.Mask) {
            if (engine.variableExists(after)) {
                if (engine.isString(after)) {
                    std::string line;
                    line = get_stdin_quiet(engine.varString(after));

                    if (is_numeric(line))
                        engine.createVariable(arg0, stod(line));
                    else
                        engine.createVariable(arg0, line);

                    writeline();
                } else {
                    std::string line;
                    line = get_stdin_quiet("");

                    if (is_numeric(line))
                        engine.createVariable(arg0, stod(line));
                    else
                        engine.createVariable(arg0, line);

                    writeline();
                }
            } else {
                std::string line;
                line = get_stdin_quiet(pre_parse(after));

                if (is_numeric(line))
                    engine.createVariable(arg0, stod(line));
                else
                    engine.createVariable(arg0, line);

                writeline();
            }
        } else if (after == Keywords.Size) {
            if (!engine.variableExists(before)) {
                engine.createVariable(arg0, (double)before.length());
                return;
            }

            if (engine.isString(before))
                engine.createVariable(
                    arg0, (double)engine.varString(before).length());
            else
                error(ErrorCode::CONV_ERR, before);
        } else if (is_recognized_math_func(after)) {
            interp_mathfunc(arg0, before, after);
        } else if (after == Keywords.ToUpper) {
            if (!engine.variableExists(before)) {
                error(ErrorCode::VAR_UNDEFINED, before);
                return;
            }

            if (engine.isString(before))
                engine.createVariable(arg0, to_upper(engine.varString(before)));
            else
                error(ErrorCode::CONV_ERR, before);
        } else if (after == Keywords.ToLower) {
            if (!engine.variableExists(before)) {
                error(ErrorCode::VAR_UNDEFINED, before);
                return;
            }

            if (engine.isString(before))
                engine.createVariable(arg0, to_lower(engine.varString(before)));
            else
                error(ErrorCode::CONV_ERR, before);
        } else if (after == Keywords.FileSize) {
            if (!engine.variableExists(before)) {
                if (FileIO::fileExists(before))
                    engine.createVariable(arg0, FileIO::getFileSize(before));
                else
                    error(ErrorCode::READ_FAIL, before);
                return;
            }

            if (!engine.isString(before)) {
                error(ErrorCode::CONV_ERR, before);
                return;
            }

            if (FileIO::fileExists(engine.varString(before)))
                engine.createVariable(
                    arg0, FileIO::getFileSize(engine.varString(before)));
            else
                error(ErrorCode::READ_FAIL, engine.varString(before));
        } else if (before == Keywords.Env) {
            interp_env_rhs(arg0, after, 0);
        } else {
            if (is_numeric(arg2))
                engine.createVariable(arg0, stod(arg2));
            else
                engine.createVariable(arg0, pre_parse(arg2));
        }
    } else if (arg1 == Operators.AddAssign) {
        if (engine.variableExists(arg2)) {
            if (engine.isString(arg2))
                engine.createVariable(arg0, engine.varString(arg2));
            else if (engine.isNumber(arg2))
                engine.createVariable(arg0, engine.varNumber(arg2));
            else
                engine.createVariable(arg0, State.Null);
        } else {
            if (is_numeric(arg2))
                engine.createVariable(arg0, stod(arg2));
            else
                engine.createVariable(arg0, pre_parse(arg2));
        }
    } else if (arg1 == Operators.SubtractAssign) {
        if (engine.variableExists(arg2)) {
            if (engine.isNumber(arg2))
                engine.createVariable(arg0, 0 - engine.varNumber(arg2));
            else
                engine.createVariable(arg0, State.Null);
        } else {
            if (is_numeric(arg2))
                engine.createVariable(arg0, stod(arg2));
            else
                engine.createVariable(arg0, pre_parse(arg2));
        }
    } else if (arg1 == Keywords.ShellExec) {
        if (!engine.variableExists(arg2)) {
            engine.createVariable(arg0, Env::getStdout(pre_parse(arg2)));
            return;
        }

        if (engine.isString(arg2))
            engine.createVariable(arg0, Env::getStdout(engine.varString(arg2)));
        else
            error(ErrorCode::CONV_ERR, arg2);
    } else if (arg1 == Keywords.InlineParse) {
        if (!engine.variableExists(arg2)) {
            engine.createVariable(arg0, get_parsed_stdout(pre_parse(arg2)));
            return;
        }

        if (engine.isString(arg2))
            engine.createVariable(arg0,
                                  get_parsed_stdout(engine.varString(arg2)));
        else
            error(ErrorCode::CONV_ERR, arg2);
    } else
        error(ErrorCode::INVALID_OPERATOR, arg2);
}

void interp_mathfunc_assignfromvar(std::string arg0, std::string before,
                                   std::string after) {
    if (!engine.variableExists(before)) {
        error(ErrorCode::VAR_UNDEFINED, before);
        return;
    }

    if (!engine.isNumber(before)) {
        error(ErrorCode::CONV_ERR, before);
        return;
    }

    double value = engine.varNumber(before);

    if (after == Math.Cos)
        value = cos(value);
    else if (after == Math.Acos)
        value = acos(value);
    else if (after == Math.Cosh)
        value = cosh(value);
    else if (after == Math.Log)
        value = log(value);
    else if (after == Math.Sqrt)
        value = sqrt(value);
    else if (after == Math.Abs)
        value = abs(value);
    else if (after == Math.Floor)
        value = floor(value);
    else if (after == Math.Ceil)
        value = ceil(value);
    else if (after == Math.Exp)
        value = exp(value);
    else if (after == Math.Sin)
        value = sin(value);
    else if (after == Math.Sinh)
        value = sinh(value);
    else if (after == Math.Asin)
        value = asin(value);
    else if (after == Math.Tan)
        value = tan(value);
    else if (after == Math.Tanh)
        value = tanh(value);
    else if (after == Math.Atan)
        value = atan(value);

    if (engine.isNumber(arg0))
        engine.setVariable(arg0, value);
    else if (engine.isString(arg0))
        engine.setVariable(arg0, dtos(value));
    else
        error(ErrorCode::IS_NULL, arg0);
}

void interp_mathfunc(std::string arg0, std::string before, std::string after) {
    if (!engine.variableExists(before)) {
        error(ErrorCode::VAR_UNDEFINED, before);
        return;
    }

    if (!engine.isNumber(before)) {
        error(ErrorCode::CONV_ERR, before);
        return;
    }

    double value = engine.varNumber(before);

    if (after == Math.Sin)
        engine.createVariable(arg0, sin(value));
    else if (after == Math.Sinh)
        engine.createVariable(arg0, sinh(value));
    else if (after == Math.Asin)
        engine.createVariable(arg0, asin(value));
    else if (after == Math.Tan)
        engine.createVariable(arg0, tan(value));
    else if (after == Math.Tanh)
        engine.createVariable(arg0, tanh(value));
    else if (after == Math.Atan)
        engine.createVariable(arg0, atan(value));
    else if (after == Math.Cos)
        engine.createVariable(arg0, cos(value));
    else if (after == Math.Acos)
        engine.createVariable(arg0, acos(value));
    else if (after == Math.Cosh)
        engine.createVariable(arg0, cosh(value));
    else if (after == Math.Log)
        engine.createVariable(arg0, log(value));
    else if (after == Math.Sqrt)
        engine.createVariable(arg0, sqrt(value));
    else if (after == Math.Abs)
        engine.createVariable(arg0, abs(value));
    else if (after == Math.Floor)
        engine.createVariable(arg0, floor(value));
    else if (after == Math.Ceil)
        engine.createVariable(arg0, ceil(value));
    else if (after == Math.Exp)
        engine.createVariable(arg0, exp(value));
}

void interp_init_classvar(std::string arg0, std::string arg1,
                          std::string arg2) {
    std::string className = before_dot(arg2), variableName = after_dot(arg2);

    if (engine.classExists(className)) {
        if (arg1 == Operators.Assign) {
            Variable classVariable =
                engine.getClassVariable(className, variableName);
            if (classVariable.getType() == ValueType::String)
                engine.createVariable(arg0, classVariable.getString());
            else if (classVariable.getType() == ValueType::Double)
                engine.createVariable(arg0, classVariable.getNumber());
        }
    }
}

void interp_class_clone(std::string arg0, std::string arg1, std::string arg2) {
    if (arg1 == Operators.Assign) {
        std::vector<Method> classMethods = engine.getClass(arg2).getMethods();
        Class newClass(arg0);

        for (int i = 0; i < (int)classMethods.size(); i++)
            newClass.addMethod(classMethods.at(i));

        std::vector<Variable> classVariables =
            engine.getClass(arg2).getVariables();

        for (int i = 0; i < (int)classVariables.size(); i++)
            newClass.addVariable(classVariables.at(i));

        newClass.setCollectable(State.ExecutedMethod);

        engine.addClass(newClass);

        State.CurrentClass = arg1;
        State.DefiningClass = false;

        newClass.clear();
        classMethods.clear();
    } else
        error(ErrorCode::INVALID_OPERATOR, arg1);
}

void interp_init_const(std::string arg0, std::string arg1, std::string arg2) {
    if (engine.constantExists(arg0)) {
        error(ErrorCode::CONST_DEFINED, arg0);
        return;
    }

    if (arg1 != Operators.Assign) {
        error(ErrorCode::INVALID_OPERATOR, arg1);
        return;
    }

    if (is_numeric(arg2))
        engine.addConstant(Constant(arg0, stod(arg2)));
    else
        engine.addConstant(Constant(arg0, arg2));
}

void interp_clear_command(std::string &arg) {
    if (arg == Keywords.Methods)
        engine.clearMethods();
    else if (arg == Keywords.Classes)
        engine.clearClasses();
    else if (arg == Keywords.Variables)
        engine.clearVariables();
    else if (arg == Keywords.Lists)
        engine.clearLists();
    else if (arg == Keywords.All)
        engine.clearAll();
    else if (arg == Keywords.Constants)
        engine.clearConstants();
}

// TODO: refactor
//	modes:
//		0 = createVariable
//		1 = setVariable
//		2 = setLastValue
void interp_env_rhs(std::string arg0, std::string after, int mode) {
    std::string defaultValue;
    std::string sValue(defaultValue);
    double dValue = 0;

    if (after == Keywords.CurrentDirectory)
        sValue = FileIO::getCurrentDirectory();
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

    // TODO: refactor into three different functions.
    switch (mode) {
    case 0:
        if (sValue != defaultValue)
            engine.createVariable(arg0, sValue);
        else
            engine.createVariable(arg0, dValue);
        break;
    case 1:
        if (sValue != defaultValue)
            engine.setVariable(arg0, sValue);
        else
            engine.setVariable(arg0, dValue);
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
void interp_internal_puts(std::string arg1, bool newline) {
    std::string text(arg1);

    // if parameter is variable, get it's value
    if (engine.variableExists(arg1)) {
        // set the value
        if (is_dotless(arg1))
            text = engine.getVariableValueAsString(arg1);
        else
            text = engine.getClassVariableValueAsString(before_dot(arg1),
                                                        after_dot(arg1));
    }

    if (newline)
        writeline(text);
    else
        write(text);
}

std::string get_stdin_quiet(std::string text) {
    char *s = getpass(pre_parse(text).c_str());
    return s;
}

void initialize_state(std::string uslang) {
    State.BadMethodCount = 0, State.BadClassCount = 0, State.BadVarCount = 0,
    State.CurrentLineNumber = 0, State.IfStatementCount = 0,
    State.ForLoopCount = 0, State.WhileLoopCount = 0, State.ParamVarCount = 0,
    State.LastErrorCode = 0;
    State.CaptureParse = false, State.IsCommented = false,
    State.CommentPosition = std::numeric_limits<int>::max();
    State.UseCustomPrompt = false, State.DontCollectMethodVars = false,
    State.FailedIfStatement = false, State.GoToLabel = false,
    State.ExecutedIfStatement = false, State.InDefaultCase = false,
    State.ExecutedMethod = false, State.DefiningSwitchBlock = false,
    State.DefiningIfStatement = false, State.DefiningForLoop = false,
    State.DefiningWhileLoop = false, State.DefiningModule = false,
    State.DefiningPrivateCode = false, State.DefiningPublicCode = false,
    State.DefiningScript = false,
    State.ExecutedTemplate = false, // remove
        State.ExecutedTryBlock = false, State.Breaking = false,
    State.DefiningMethod = false, State.IsMultilineComment = false,
    State.FailedNest = false, State.DefiningNest = false,
    State.DefiningClass = false, State.DefiningClassMethod = false,
    State.DefiningParameterizedMethod = false, State.SkipCatchBlock = false,
    State.RaiseCatchBlock = false, State.DefiningLocalSwitchBlock = false,
    State.DefiningLocalWhileLoop = false, State.DefiningLocalForLoop = false;

    State.CurrentClass = "", State.CurrentMethodClass = "",
    State.CurrentModule = "", State.CurrentScript = "", State.ErrorVarName = "",
    State.GoTo = "", State.LastError = "", State.LastValue = "",
    State.ParsedOutput = "", State.PreviousScript = "",
    State.CurrentScriptName = "", State.SwitchVarName = "",
    State.CurrentLine = "", State.DefaultLoopSymbol = "$";

    State.Null = "";
    State.NullNum = -DBL_MAX;

    State.Application = uslang;
    State.CurrentScript = uslang;
}

#endif
