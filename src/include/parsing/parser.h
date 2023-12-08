#ifndef PARSER_H
#define PARSER_H

/**
    The heart of it all.
**/
void parse(std::string s) {
    if (s.empty())
        return;

    s = trim(s);

    std::vector<std::string> tokens;
    State.CurrentLine = s; // store a copy of the current line

    StringList stringList; // contains separate commands
    std::string builder;   // a string to build upon

    int length = s.length(),  //	length of the line
        count = 0,            // command token counter
        size = 0;             // final size of tokenized command container
    bool quoted = false,      // flag: parsing string literals
        endOfCommand = false, // flag: end of a command
        uncomment = false,    // flag: end a command
        parenthesis = false;  // flag: parsing contents within parentheses

    // tokens.push_back(""); // push back an empty string to begin.
    // iterate each char in the initial string
    char prevChar = 'a'; // previous character in string

    tokenize(length, s, parenthesis, quoted, tokens, count, prevChar, builder,
             uncomment, endOfCommand, stringList);

    size = (int)tokens.size();

    if (State.IsCommented) {
        if (State.IsMultilineComment && uncomment) {
            State.IsCommented = false;
            State.IsMultilineComment = false;
        } else if (uncomment) {
            State.IsCommented = false;
            uncomment = false;

            builder = preparse_stripcomment(builder);

            if (endOfCommand) {
                stringList.add(builder);
                interp_StringList(stringList);
            } else
                parse(builder);
        }
    } else if (!endOfCommand) {
        interp_args(size, tokens);

        if (State.DefiningSwitchBlock)
            interp_switchstatement(s, tokens);
        else if (State.DefiningModule)
            interp_moduledefinition(s);
        else if (State.DefiningScript)
            interp_scriptdefinition(s);
        else if (State.RaiseCatchBlock && s == Keywords.Catch)
            State.RaiseCatchBlock = false;
        else if (State.ExecutedTryBlock && s == Keywords.Catch)
            State.SkipCatchBlock = true;
        else if (State.ExecutedTryBlock && State.SkipCatchBlock &&
                 s == Keywords.Caught) {
            State.SkipCatchBlock = false;
            parse(Keywords.Caught);
        } else if (State.DefiningMethod)
            interp_method_def(s);
        else if (State.DefiningIfStatement)
            interp_ifstatement_def(tokens, s, size);
        else if (State.DefiningWhileLoop)
            interp_whileloop_def(tokens, s);
        else if (State.DefiningForLoop)
            interp_forloop_def(tokens, s);
        else
            interp_default(size, tokens, s);
    } else {
        stringList.add(builder);
        interp_StringList(stringList);
    }
}

std::string get_parsed_stdout(std::string cmd) {
    State.CaptureParse = true;
    parse(cmd);
    std::string ret = State.ParsedOutput;
    State.ParsedOutput.clear();
    State.CaptureParse = false;

    return ret.length() == 0 ? State.LastValue : ret;
}

void interp_StringList(StringList &StringList) {
    for (int i = 0; i < (int)StringList.get().size(); i++)
        parse(StringList.at(i));
}

std::string preparse_stripcomment(std::string &inputString) {
    std::string result;
    for (int i = 0; i < (int)inputString.length(); i++) {
        if (i == State.CommentPosition)
            break;

        result.push_back(inputString[i]);
    }
    State.CommentPosition = std::numeric_limits<int>::max();
    return result;
}

void interp_forloopmethod(Method &m, int iterVal) {
    for (int z = 0; z < m.size(); z++) {
        std::string cleanString(""), tmp(m.at(z));
        preparse_methodline(tmp, m, cleanString, itos(iterVal));

        parse(cleanString);
    }
}

void interp_forloopmethod(Method &m, std::string iterVal) {
    for (int z = 0; z < m.size(); z++) {
        std::string cleanString(""), tmp(m.at(z));
        preparse_methodline(tmp, m, cleanString, iterVal);

        parse(cleanString);
    }
}

void preparse_methodline(std::string &tmp, Method &m, std::string &cleanString,
                         std::string iterValue) {
    int l(tmp.length());
    bool buildSymbol = false, almostBuild = false, ended = false;
    std::string builder;

    for (int a = 0; a < l; a++) {
        if (almostBuild && tmp[a] == '{')
            buildSymbol = true;

        if (buildSymbol) {
            if (tmp[a] == '}') {
                almostBuild = false, buildSymbol = false;
                ended = true;

                builder = subtract_string(builder, "{");

                if (builder == m.getSymbolString())
                    cleanString.append(iterValue);

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
}

void interp_method_def(std::string &s) {
    if (contains(s, Keywords.While))
        State.DefiningLocalWhileLoop = true;

    if (contains(s, Keywords.Switch))
        State.DefiningLocalSwitchBlock = true;

    if (State.DefiningParameterizedMethod) {
        if (s == Keywords.End) {
            if (State.DefiningLocalWhileLoop) {
                State.DefiningLocalWhileLoop = false;

                if (State.DefiningClass)
                    engine.getClass(State.CurrentClass).addToCurrentMethod(s);
                else
                    engine.addToCurrentMethod(s);
            } else if (State.DefiningLocalSwitchBlock) {
                State.DefiningLocalSwitchBlock = false;

                if (State.DefiningClass)
                    engine.getClass(State.CurrentClass).addToCurrentMethod(s);
                else
                    engine.addToCurrentMethod(s);
            } else {
                State.DefiningMethod = false;

                if (State.DefiningClass) {
                    State.DefiningClassMethod = false;
                    engine.getClass(engine.getClassCount() - 1)
                        .setCurrentMethod("");
                }
            }
        } else {
            std::string freshLine;
            preparse_line_classdef(s, freshLine);

            if (State.DefiningClass) {
                engine.getClass(State.CurrentClass)
                    .addToCurrentMethod(freshLine);

                if (State.DefiningPublicCode)
                    engine.getClass(State.CurrentClass).setPublic();
                else if (State.DefiningPrivateCode)
                    engine.getClass(State.CurrentClass).setPrivate();
                else
                    engine.getClass(State.CurrentClass).setPublic();
            } else
                engine.getMethod(engine.getMethodCount() - 1).add(freshLine);
        }
    } else {
        if (s == Keywords.End) {
            if (State.DefiningLocalWhileLoop) {
                State.DefiningLocalWhileLoop = false;

                if (State.DefiningClass)
                    engine.addToCurrentClassMethod(s);
                else
                    engine.addToCurrentMethod(s);
            } else if (State.DefiningLocalSwitchBlock) {
                State.DefiningLocalSwitchBlock = false;

                if (State.DefiningClass)
                    engine.addToCurrentClassMethod(s);
                else
                    engine.addToCurrentMethod(s);
            } else {
                State.DefiningMethod = false;

                if (State.DefiningClass) {
                    State.DefiningClassMethod = false;
                    engine.getClass(engine.getClassCount() - 1)
                        .setCurrentMethod("");
                }
            }
        } else {
            if (State.DefiningClass)
                interp_class_decl(s);
            else {
                if (State.DefiningClassMethod)
                    interp_classmethod_decl(s);
                else
                    engine.addToCurrentMethod(s);
            }
        }
    }
}

void interp_ifstatement_def(std::vector<std::string> &command, std::string &s,
                            int size) {
    if (State.DefiningNest)
        interp_nestedif_def(command, s);
    else if (command.at(0) == Keywords.If) {
        State.DefiningNest = true;

        if (size == 4)
            threeSpace(Keywords.If, command.at(1), command.at(2),
                       command.at(3));
        else {
            engine.createIfStatement(false);
            State.DefiningNest = false;
        }
    } else if (command.at(0) == Keywords.Endif)
        interp_ifstatement();
    else if (command.at(0) == Keywords.Elsif) {
        if (size == 4)
            threeSpace(Keywords.If, command.at(1), command.at(2),
                       command.at(3));
        else
            engine.createIfStatement(false);
    } else if (s == Keywords.Else)
        threeSpace(Keywords.If, Keywords.True, Operators.Equal, Keywords.True);
    else if (s == Keywords.Failif) {
        if (State.FailedIfStatement == true)
            engine.createIfStatement(true);
        else
            engine.createIfStatement(false);
    } else
        engine.getIfStatement(engine.getIfStatementCount() - 1).add(s);
}

void interp_nestedif_def(std::vector<std::string> &command, std::string &s) {
    if (command.at(0) == Keywords.Endif)
        exec.executeNest(
            engine.getIfStatement(engine.getIfStatementCount() - 1).getNest());
    else
        engine.getIfStatement(engine.getIfStatementCount() - 1).addToNest(s);
}

void interp_whileloop_def(std::vector<std::string> &command, std::string &s) {
    if (command.at(0) == Keywords.End)
        interp_whileloops();
    else
        engine.getWhileLoop(engine.getWhileLoopCount() - 1).add(s);
}

void interp_forloop_def(std::vector<std::string> &command, std::string &s) {
    // TODO: I want to use `next` as `continue`.
    // `next if {condition}`
    // `next`
    if (command.at(0) == Keywords.Next || command.at(0) == Keywords.EndFor)
        interp_forloop();
    else
        engine.addToCurrentForLoop(s);
}

void interp_default(int size, std::vector<std::string> &command,
                    std::string &s) {
    if (size == 1)
        interp_0space(command, s);
    else if (size == 2)
        interp_1space(command, s);
    else if (size == 3)
        interp_2space(command, s);
    else if (size == 4)
        interp_3space(command);
    else if (size == 5)
        interp_4space(command, s);
    else
        Env::shellExec(s);
}

void interp_4space(std::vector<std::string> &command, std::string &s) {
    // for each in
    if (command.at(0) == Keywords.For && has_params(command.at(4))) {
        State.DefaultLoopSymbol = command.at(4);
        State.DefaultLoopSymbol = subtract_char(State.DefaultLoopSymbol, '(');
        State.DefaultLoopSymbol = subtract_char(State.DefaultLoopSymbol, ')');

        threeSpace(command.at(0), command.at(1), command.at(2), command.at(3));
        State.DefaultLoopSymbol = "$";
    } else
        Env::shellExec(s);
}

void interp_3space(std::vector<std::string> &command) {
    threeSpace(command.at(0), command.at(1), command.at(2), command.at(3));
}

void interp_targetandtext(const std::string &arg1, const std::string &arg2,
                          std::string &target, std::string &text) {
    if (engine.variableExists(arg1) && engine.isString(arg1)) {
        target = engine.varString(arg1);
        text = engine.variableExists(arg2)
                   ? engine.getVariableValueAsString(arg2)
                   : arg2;
    } else {
        target = arg1;
        text = engine.variableExists(arg2)
                   ? engine.getVariableValueAsString(arg2)
                   : arg2;
    }
}

void interp_2space(std::vector<std::string> &command, std::string &s) {
    if (is_recognized_2space(command.at(1))) {
        twoSpace(command.at(0), command.at(1), command.at(2), command);
        return;
    }

    std::string target, text;
    interp_targetandtext(command.at(1), command.at(2), target, text);

    if (command.at(0) == Keywords.FileAppend ||
        command.at(0) == Keywords.FileAppendLine)
        FileIO::appendText(target, text,
                           command.at(0) == Keywords.FileAppendLine);
    else if ((command.at(0) == Keywords.FileWrite))
        FileIO::writeText(target, text);
    else if (command.at(0) == Keywords.Redefine)
        engine.redefine(command.at(1), command.at(2));
    else
        Env::shellExec(s);
}

void interp_1space(std::vector<std::string> &command, std::string &s) {
    if (is_recognized_1space(command.at(0)))
        oneSpace(command.at(0), command.at(1));
    else
        Env::shellExec(s);
}

void interp_0space(std::vector<std::string> &command, std::string &s) {
    if (is_recognized_0space(command.at(0))) {
        zeroSpace(command.at(0));
        return;
    }

    DotSep dotsep(s);
    std::string before(dotsep.getBeforeDot()), after(dotsep.getAfterDot());

    if (dotsep.hasDot()) {
        if (engine.classExists(before)) {
            if (has_params(after)) {
                s = subtract_char(s, '"');

                if (engine.getClass(before).hasMethod(before_params(after)))
                    exec.executeTemplate(
                        engine.getClass(before).getMethod(before_params(after)),
                        interp_params(after));
                else
                    Env::shellExec(s);
            } else if (engine.getClass(before).hasMethod(after))
                exec.executeMethod(engine.getClass(before).getMethod(after));
            else if (engine.getClass(before).hasVariable(after)) {
                const auto &v = engine.getClassVariable(before, after);
                if (v.getType() == ValueType::String)
                    writeline(v.getString());
                else if (v.getType() == ValueType::Double)
                    writeline(dtos(v.getNumber()));
                else if (v.getType() == ValueType::Integer)
                    writeline(itos(v.getNumber()));
            } else if (after == Keywords.GC)
                engine.getClass(before).clear();
            else
                error(ErrorCode::UNDEFINED, after);
        } else if (before == Keywords.Env)
            interp_env_rhs("", after, 3);
        else if (engine.variableExists(before) && after == Keywords.Clear)
            engine.getVar(before).clear();
        else if (engine.listExists(before))
            interp_list_rhs(after, before);
        else if (before == Keywords.Self && State.ExecutedMethod)
            exec.executeMethod(
                engine.getClass(State.CurrentMethodClass).getMethod(after));
        else
            Env::shellExec(s);
    } else if (ends_with(s, "::")) {
        if (State.CurrentScript != "") {
            std::string newMark(s);
            newMark = subtract_string(s, "::");
            engine.getScript().addMark(newMark);
        }
    } else if (engine.methodExists(s))
        exec.executeMethod(engine.getMethod(s));
    else {
        s = subtract_char(s, '"');

        if (engine.methodExists(before_params(s)))
            exec.executeTemplate(engine.getMethod(before_params(s)),
                                 interp_params(s));
        else
            Env::shellExec(s);
    }
}

void interp_list_rhs(std::string &after, std::string &before) {
    if (after == Keywords.Clear)
        engine.getList(before).clear();
    else if (after == Keywords.Sort)
        engine.getList(before).sort();
    else if (after == Keywords.Reverse)
        engine.getList(before).reverse();
    else if (after == Keywords.Revert)
        engine.getList(before).revert();
}
void interp_ifstatement() {
    State.DefiningIfStatement = false;
    State.ExecutedIfStatement = true;

    for (int i = 0; i < engine.getIfStatementCount(); i++) {
        if (engine.getIfStatement(i).isIF()) {
            exec.executeMethod(engine.getIfStatement(i));

            if (!State.FailedIfStatement)
                break;
        }
    }

    engine.clearIf();

    State.ExecutedIfStatement = false;
    State.FailedIfStatement = false;
    State.IfStatementCount = 0;
}

void preparse_line_classdef(std::string &s, std::string &freshLine) {
    int _len = s.length();
    std::vector<std::string> words;
    std::string word;

    for (int z = 0; z < _len; z++) {
        if (s[z] == ' ') {
            words.push_back(word);
            word.clear();
        } else
            word.push_back(s[z]);
    }

    words.push_back(word);

    for (int z = 0; z < (int)words.size(); z++) {
        if (engine.variableExists(words.at(z)))
            freshLine.append(engine.getVariableValueAsString(words.at(z)));
        else
            freshLine.append(words.at(z));

        if (z != (int)words.size() - 1)
            freshLine.push_back(' ');
    }
}

void interp_classmethod_decl(std::string &s) {
    engine.addToCurrentClassMethod(s);

    if (State.DefiningPublicCode)
        engine.getClass(engine.getClassCount() - 1).setPublic();
    else if (State.DefiningPrivateCode)
        engine.getClass(engine.getClassCount() - 1).setPrivate();
    else
        engine.getClass(engine.getClassCount() - 1).setPublic();
}

void interp_class_decl(std::string &s) {
    engine.addToCurrentClassMethod(s);

    if (State.DefiningPublicCode)
        engine.getClass(engine.getClassCount() - 1).setPublic();
    else if (State.DefiningPrivateCode)
        engine.getClass(engine.getClassCount() - 1).setPrivate();
    else
        engine.getClass(engine.getClassCount() - 1).setPublic();
}

void interp_scriptdefinition(std::string &s) {
    if (s == Keywords.EndInlineScript) {
        State.CurrentScriptName = "";
        State.DefiningScript = false;
    } else
        FileIO::appendToFile(State.CurrentScriptName, s + "\n");
}

void interp_moduledefinition(std::string &s) {
    if (State.DefiningModule && s == Keywords.End) {
        State.DefiningModule = false;
        State.CurrentModule = "";
    } else
        engine.getModule(State.CurrentModule).add(s);
}

void interp_switchstatement(std::string &s, std::vector<std::string> &command) {
    if (begins_with(s, Keywords.Case))
        engine.getMainSwitch().addCase(command.at(1));
    else if (s == Keywords.Default)
        State.InDefaultCase = true;
    else if (s == Keywords.End) {
        std::string switch_value =
            engine.getVariableValueAsString(State.SwitchVarName);
        Container rightCase = engine.getMainSwitch().rightCase(switch_value);

        State.InDefaultCase = false;
        State.DefiningSwitchBlock = false;

        for (int i = 0; i < (int)rightCase.size(); i++)
            parse(rightCase.at(i));

        engine.getMainSwitch().clear();
    } else {
        if (State.InDefaultCase)
            engine.getMainSwitch().addToDefault(s);
        else
            engine.getMainSwitch().addToCase(s);
    }
}

void interp_args(int size, std::vector<std::string> &command) {
    for (int i = 0; i < size; i++) {
        // parse arguments
        // args[0], args[1], ..., args[n-1]
        if (contains(command.at(i), Keywords.Args) &&
            command.at(i) != Keywords.ArgValues) {
            std::vector<std::string> params =
                interp_bracketrange(command.at(i));

            if (!is_numeric(params.at(0))) {
                error(ErrorCode::CONV_ERR, command.at(i));
                return;
            }

            if (engine.getArgCount() - 1 >= stoi(params.at(0)) &&
                stoi(params.at(0)) >= 0) {
                if (params.at(0) == "0")
                    command.at(i) = State.CurrentScript;
                else
                    command.at(i) = engine.getArg(stoi(params.at(0)));
            } else
                error(ErrorCode::OUT_OF_BOUNDS, command.at(i));
        }
    }
}

void interp_forloop() {
    State.DefiningForLoop = false;

    for (int i = 0; i < engine.getForLoopCount(); i++)
        if (engine.getForLoop(i).isForLoop())
            exec.executeForLoop(engine.getForLoop(i));

    engine.clearFor();

    State.ForLoopCount = 0;
}

template <typename condition>
void interp_whileloop(std::string v1, std::string v2, condition cond) {
    while (cond(v1, v2)) {
        exec.executeWhileLoop(
            engine.getWhileLoop(engine.getWhileLoopCount() - 1));

        if (State.Breaking)
            break;
    }

    engine.clearWhile();

    State.WhileLoopCount = 0;
}

void interp_whileloops() {
    State.DefiningWhileLoop = false;

    std::string
        v1 = engine.getWhileLoop(engine.getWhileLoopCount() - 1).valueOne(),
        v2 = engine.getWhileLoop(engine.getWhileLoopCount() - 1).valueTwo(),
        op =
            engine.getWhileLoop(engine.getWhileLoopCount() - 1).logicOperator();

    if (engine.variableExists(v1) && engine.variableExists(v2)) {
        if (op == Operators.Equal) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) == engine.varNumber(v2);
            });
        } else if (op == Operators.LessThan) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) < engine.varNumber(v2);
            });
        } else if (op == Operators.GreaterThan) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) > engine.varNumber(v2);
            });
        } else if (op == Operators.LessThanOrEqual) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) <= engine.varNumber(v2);
            });
        } else if (op == Operators.GreaterThanOrEqual) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) >= engine.varNumber(v2);
            });
        } else if (op == Operators.NotEqual) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) != engine.varNumber(v2);
            });
        }
    } else if (engine.variableExists(v1)) {
        if (op == Operators.Equal) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) == stoi(v2);
            });
        } else if (op == Operators.LessThan) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) < stoi(v2);
            });
        } else if (op == Operators.GreaterThan) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) > stoi(v2);
            });
        } else if (op == Operators.LessThanOrEqual) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) <= stoi(v2);
            });
        } else if (op == Operators.GreaterThanOrEqual) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) >= stoi(v2);
            });
        } else if (op == Operators.NotEqual) {
            interp_whileloop(v1, v2, [](std::string v1, std::string v2) {
                return engine.varNumber(v1) != stoi(v2);
            });
        }
    }
}

void tokenize(int length, std::string &s, bool &parenthesis, bool &quoted,
              std::vector<std::string> &tokens, int &count, char &prevChar,
              std::string &builder, bool &uncomment, bool &broken,
              StringList &stringList) {
    // TODO: fix this
    tokens.push_back("");
    for (int i = 0; i < length; i++) {
        if (State.IsCommented) {
            builder.push_back(s[i]);
            continue;
        }

        switch (s[i]) {
        case ' ':
            if (!State.IsCommented) {
                if ((!parenthesis && quoted) || (parenthesis && quoted))
                    tokens.at(count).push_back(' ');
                else if (parenthesis && !quoted) {} 
                else if (prevChar != ' ') {
                    tokens.push_back("");
                    count++;
                }
            }

            builder.push_back(' ');
            break;

        case '\"':
            quoted = !quoted;
            if (parenthesis) {
                tokens.at(count).push_back('\"');
            }
            builder.push_back('\"');
            break;

        case '(':
            if (!parenthesis)
                parenthesis = true;

            tokens.at(count).push_back('(');

            builder.push_back('(');
            break;

        case ')':
            if (parenthesis)
                parenthesis = false;

            tokens.at(count).push_back(')');
            builder.push_back(')');
            break;

        case '\\':
            if (quoted || parenthesis) {
                if (!State.IsCommented)
                    tokens.at(count).push_back('\\');
            }

            builder.push_back('\\');
            break;

        case '\'':
            if (quoted || parenthesis) {
                if (prevChar == '\\')
                    tokens.at(count).append("\'");
                else
                    tokens.at(count).append("\"");

                builder.push_back('\'');
            }
            break;

        case '#':
            if (quoted || parenthesis)
                tokens.at(count).push_back('#');
            else if (prevChar == '#' && !State.IsMultilineComment) {
                State.IsMultilineComment = true;
                State.IsCommented = true;
                uncomment = false;
            } else if (prevChar == '#' && State.IsMultilineComment)
                uncomment = true;
            else if (prevChar != '#' && !State.IsMultilineComment) {
                if (State.CommentPosition == std::numeric_limits<int>::max())
                    State.CommentPosition = i;
                State.IsCommented = true;
                uncomment = true;

                // TODO: fix this, you can reproduce with: println "foo bar" #
                // baz qux
                if (tokens.back().empty())
                    tokens.pop_back();
            }

            builder.push_back('#');
            break;

        case ';':
            if (!quoted) {
                if (!State.IsCommented) {
                    broken = true;
                    stringList.add(builder);
                    builder = "";
                    count = 0;
                    tokens.clear();
                    tokens.push_back("");
                }
            } else {
                builder.push_back(';');
                tokens.at(count).push_back(';');
            }
            break;

        default:
            if (!State.IsCommented)
                tokens.at(count).push_back(s[i]);
            builder.push_back(s[i]);
            break;
        }

        prevChar = s[i];
    }
}

void zeroSpace(std::string arg0) {
    if (arg0 == Keywords.Pass)
        return;
    else if (arg0 == Keywords.Caught)
        handleCaught();
    else if (arg0 == Keywords.Exit)
        handleExit();
    else if (arg0 == Keywords.Break)
        State.Breaking = true;
    else if (arg0 == Keywords.End)
        handleEnd();
    else if (arg0 == Keywords.Parser)
        load_repl();
    else if (arg0 == Keywords.Private)
        handlePrivateDecl();
    else if (arg0 == Keywords.Public)
        handlePublicDecl();
    else if (arg0 == Keywords.Try)
        State.ExecutedTryBlock = true;
    else if (arg0 == Keywords.Failif)
        handleFailedIfStatement();
    else
        Env::shellExec(arg0);
}

void oneSpace(std::string arg0, std::string arg1) {
    // Refactor
    if (contains(arg1, Keywords.SelfDot))
        arg1 = replace(arg1, Keywords.Self, State.CurrentMethodClass);

    DotSep dotsep(arg1);
    std::string before(dotsep.getBeforeDot()), after(dotsep.getAfterDot());

    if (arg0 == Keywords.GC)
        interp_clear_command(arg1);
    else if (arg0 == Keywords.Switch)
        handleSwitch(arg1);
    else if (arg0 == Keywords.Goto)
        handleGoto(arg1);
    else if (arg0 == Keywords.If)
        handleIfStatement(arg1);
    else if (arg0 == Keywords.Prompt)
        handlePrompt(arg1);
    else if (arg0 == Keywords.Err)
        handleErr(arg1);
    else if (arg0 == Keywords.Delay)
        handleDelay(arg1);
    else if (arg0 == Keywords.For && arg1 == Keywords.Infinity)
        engine.createForLoop();
    else if (arg0 == Keywords.Remove)
        handleRemove(arg1);
    else if (arg0 == Keywords.BeginInlineScript)
        handleInlineScriptDecl(arg1);
    else if (arg0 == Keywords.Globalize)
        engine.globalize(arg1);
    else if (arg0 == Keywords.Load)
        handleLoad(arg1);
    else if (arg0 == Keywords.Print || arg0 == Keywords.PrintLn)
        interp_internal_puts(arg1, arg0 == Keywords.PrintLn);
    else if (arg0 == Keywords.ChangeDirectory)
        handleChangeDir(arg1);
    else if (arg0 == Keywords.List)
        handleListDecl(arg1);
    else if (arg0 == Keywords.InlineParse)
        handleInlineParse(arg1);
    else if (arg0 == Keywords.ShellExec)
        handleInlineShellExec(arg1);
    else if (arg0 == Keywords.InitialDirectory)
        handleInitialDir(arg1);
    else if (arg0 == Keywords.IsMethod)
        handleMethodInspect(before, after, arg1);
    else if (arg0 == Keywords.IsClass)
        handleClassInspect(arg1);
    else if (arg0 == Keywords.IsVariable)
        handleVariableInspect(before, after, arg1);
    else if (arg0 == Keywords.IsList)
        handleListInspect(arg1);
    else if (arg0 == Keywords.IsDirectory)
        handleDirectoryInspect(before, after, arg1);
    else if (arg0 == Keywords.IsFile)
        handleFileInspect(before, after, arg1);
    else if (arg0 == Keywords.IsCollectable)
        handleCollectInspect(arg1);
    else if (arg0 == Keywords.IsNumber)
        handleNumberInspect(before, after, arg1);
    else if (arg0 == Keywords.IsString)
        handleStringInspect(before, after, arg1);
    else if (arg0 == Keywords.Template)
        handleTemplateDecl(arg1);
    else if (arg0 == Keywords.Lock)
        handleLockAssignment(arg1);
    else if (arg0 == Keywords.Unlock)
        handleUnlockAssignment(arg1);
    else if (arg0 == Keywords.Method || arg0 == Keywords.LockedMethod)
        engine.createMethod(arg1, arg0 == Keywords.LockedMethod);
    else if (arg0 == Keywords.InvokeMethod)
        exec.executeMethod(arg1, before, after);
    else if (arg0 == Keywords.Class)
        engine.createClass(arg1);
    else if (arg0 == Keywords.Module)
        engine.createModule(arg1);
    else if (arg0 == Keywords.CreateFile)
        handleFilePush(arg1);
    else if (arg0 == Keywords.RemoveFile)
        handleFilePop(arg1);
    else if (arg0 == Keywords.CreateDirectory)
        handleDirPush(arg1);
    else if (arg0 == Keywords.RemoveDirectory)
        handleDirPop(arg1);
    else
        Env::shellExec(arg0);
}

void twoSpace(std::string arg0, std::string arg1, std::string arg2,
              std::vector<std::string> command) {
    std::string last_val = "";

    if (contains(arg2, Keywords.SelfDot))
        arg2 = replace(arg2, Keywords.Self, State.CurrentMethodClass);

    if (contains(arg0, Keywords.SelfDot))
        arg0 = replace(arg0, Keywords.Self, State.CurrentMethodClass);

    if (engine.variableExists(arg0)) {
        initializeVariable(arg0, arg1, arg2, command);
    } else if (engine.listExists(arg0) ||
               engine.listExists(before_brackets(arg0))) {
        init_listvalues(arg0, arg1, arg2);
    } else {
        if (begins_with(arg0, "@") && is_dotless(arg0)) {
            init_globalvar(arg0, arg1, arg2, command);
        } else if (begins_with(arg0, "@") && !is_dotless(arg2)) {
            interp_init_classvar(arg0, arg1, arg2);
        } else if (!engine.classExists(arg0) && engine.classExists(arg2)) {
            interp_class_clone(arg0, arg1, arg2);
        } else if (valid_const_name(arg0)) {
            interp_init_const(arg0, arg1, arg2);
        } else {
            exec.executeSimpleStatement(arg0, arg1, arg2);
        }
    }
}

void threeSpace(std::string arg0, std::string arg1, std::string arg2,
                std::string arg3) {
    if (arg0 == Keywords.Class) {
        handleClassDecl(arg1, arg3, arg2);
    } else if (arg0 == Keywords.If) {
        checkCondition(arg1, arg2, arg3);
    } else if (arg0 == Keywords.For) {
        if (arg2 == Operators.To) {
            handleLoopInit_For(arg1, arg3, arg0);
        } else if (arg2 == Keywords.In) {
            bool retFlag;
            handleLoopInit_ForIn(arg1, arg3, arg0, retFlag);
            if (retFlag)
                return;
        } else {
            error(ErrorCode::INVALID_OP, arg0);
            engine.createFailedForLoop();
        }
    } else if (arg0 == Keywords.While) {
        handleLoopInit_While(arg1, arg3, arg2, arg0);
    } else
        Env::shellExec(arg0);
}

void handleLoopInit_For(std::string &arg1, std::string &arg3,
                        std::string &arg0) {
    double first = 0, second = 0;
    bool failed = false;
    if (engine.variableExists(arg1) && engine.variableExists(arg3) &&
        (engine.isNumber(arg1) && engine.isNumber(arg3))) {
        first = engine.varNumber(arg1);
        second = engine.varNumber(arg3);
    } else if (engine.variableExists(arg1) && !engine.variableExists(arg3) &&
               (engine.isNumber(arg1) && is_numeric(arg3))) {
        first = engine.varNumber(arg1);
        second = stod(arg3);
    } else if (!engine.variableExists(arg1) && engine.variableExists(arg3) &&
               (is_numeric(arg1) && engine.isNumber(arg3))) {
        first = stod(arg1);
        second = engine.varNumber(arg3);
    } else if (is_numeric(arg1) && is_numeric(arg3)) {
        first = stod(arg1);
        second = stod(arg3);
    } else {
        error(ErrorCode::CONV_ERR, arg0);
        engine.createFailedForLoop();
        failed = true;
    }

    if (failed)
        return;

    engine.createForLoop(first, second);
}

void handleLoopInit_ForIn(std::string &arg1, std::string &arg3,
                          std::string &arg0, bool &retFlag) {
    retFlag = true;
    if (arg1 == Keywords.Each) {
        std::string before(before_dot(arg3)), after(after_dot(arg3));

        if (before == Keywords.Args && after == Keywords.Values) {
            handleLoopInit_CommandLineArgs();
        } else if (before == Keywords.Env &&
                   after == Keywords.InternalVariables) {
            handleLoopInit_Environment_BuiltIns();
        } else if (engine.classExists(before) &&
                   after == Keywords.InternalMethods) {
            handleLoopInit_ClassMembers_Methods(before);
        } else if (engine.classExists(before) &&
                   after == Keywords.InternalVariables) {
            handleLoopInit_ClassMembers_Variables(before);
        } else if (engine.variableExists(before) && after == Keywords.Size &&
                   engine.isString(before))
            handleLoopInit_Variable_Length(before);
        else if (before.length() != 0 && after.length() != 0) {
            if (engine.variableExists(before)) {
                if (after == Keywords.GetDirectories)
                    handleLoopInit_Variable_Directories(before);
                else if (after == Keywords.GetFiles)
                    handleLoopInit_Variable_Files(before);
                else if (after == Keywords.Read)
                    handleLoopInit_Variable_FileRead(before);
                else {
                    error(ErrorCode::METHOD_UNDEFINED, after);
                    engine.createFailedForLoop();
                }
            } else {
                error(ErrorCode::VAR_UNDEFINED, before);
                engine.createFailedForLoop();
            }
        } else {
            if (engine.listExists(arg3))
                engine.createForLoop(engine.getList(arg3));
            else {
                error(ErrorCode::LIST_UNDEFINED, arg3);
                engine.createFailedForLoop();
            }
        }
    } else if (has_params(arg3))
        handleLoopInit_Params(arg3, arg1);
    else if (has_brackets(arg3)) {
        bool retFlag;
        handleLoopInit_Brackets(arg3, arg1, retFlag);
        if (retFlag)
            return;
    } else if (engine.listExists(arg3)) {
        State.DefaultLoopSymbol = arg1;
        engine.createForLoop(engine.getList(arg3));
    } else if (!is_dotless(arg3)) {
        State.DefaultLoopSymbol = arg1;
        std::string _b(before_dot(arg3)), _a(after_dot(arg3));

        if (_b == Keywords.Args && _a == Keywords.Values)
            handleLoopInit_CommandLineArgs();
        else if (_b == Keywords.Env && _a == Keywords.InternalVariables)
            handleLoopInit_Environment_BuiltIns();
        else if (engine.classExists(_b) && _a == Keywords.InternalMethods)
            handleLoopInit_ClassMembers_Methods(_b);
        else if (engine.classExists(_b) && _a == Keywords.InternalVariables)
            handleLoopInit_ClassMembers_Variables(_b);
        else if (engine.variableExists(_b) && _a == Keywords.Size)
            handleLoopInit_Variable_Length(_b);
        else {
            if (_b.length() != 0 && _a.length() != 0) {
                if (engine.variableExists(_b)) {
                    if (_a == Keywords.GetDirectories)
                        handleLoopInit_Variable_Directories(_b);
                    else if (_a == Keywords.GetFiles)
                        handleLoopInit_Variable_Files(_b);
                    else if (_a == Keywords.Read)
                        handleLoopInit_Variable_FileRead(_b);
                    else {
                        error(ErrorCode::METHOD_UNDEFINED, _a);
                        engine.createFailedForLoop();
                    }
                } else {
                    error(ErrorCode::VAR_UNDEFINED, _b);
                    engine.createFailedForLoop();
                }
            }
        }
    } else {
        error(ErrorCode::INVALID_OP, arg0);
        engine.createFailedForLoop();
    }
    retFlag = false;
}

void handleLoopInit_Environment_BuiltIns() {
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
    engine.createForLoop(newList);
}

// TODO: refactor
void handleLoopInit_Brackets(std::string &arg3, std::string &arg1,
                             bool &retFlag) {
    retFlag = true;
    std::string before(before_brackets(arg3));

    if (!engine.variableExists(before) || !engine.isString(before)) {
        error(ErrorCode::NULL_STRING, before);
        engine.createFailedForLoop();
        return;
    }

    std::string tempVarString(engine.varString(before));

    std::vector<std::string> range = interp_bracketrange(arg3);

    if (range.size() != 2) {
        error(ErrorCode::OUT_OF_BOUNDS, arg3);
        return;
    }

    std::string rangeBegin(range.at(0)), rangeEnd(range.at(1));

    if ((rangeBegin.length() == 0 || rangeEnd.length() == 0) ||
        !(is_numeric(rangeBegin) && is_numeric(rangeEnd))) {
        error(ErrorCode::OUT_OF_BOUNDS,
              rangeBegin + Keywords.RangeSeparator + rangeEnd);
        return;
    }

    if (stoi(rangeBegin) < stoi(rangeEnd)) {
        if ((int)tempVarString.length() >= stoi(rangeEnd) &&
            stoi(rangeBegin) >= 0) {
            List newList("&l&i&s&t&");

            for (int i = stoi(rangeBegin); i <= stoi(rangeEnd); i++) {
                std::string tempString;
                tempString.push_back(tempVarString[i]);
                newList.add(tempString);
            }

            State.DefaultLoopSymbol = arg1;

            engine.createForLoop(newList);

            engine.removeList("&l&i&s&t&");
        } else
            error(ErrorCode::OUT_OF_BOUNDS,
                  rangeBegin + Keywords.RangeSeparator + rangeEnd);
    } else if (stoi(rangeBegin) > stoi(rangeEnd)) {
        if ((int)tempVarString.length() >= stoi(rangeEnd) &&
            stoi(rangeBegin) >= 0) {
            List newList("&l&i&s&t&");

            for (int i = stoi(rangeBegin); i >= stoi(rangeEnd); i--) {
                std::string tempString;
                tempString.push_back(tempVarString[i]);
                newList.add(tempString);
            }

            State.DefaultLoopSymbol = arg1;

            engine.createForLoop(newList);

            engine.removeList("&l&i&s&t&");
        } else
            error(ErrorCode::OUT_OF_BOUNDS,
                  rangeBegin + Keywords.RangeSeparator + rangeEnd);
    } else
        error(ErrorCode::OUT_OF_BOUNDS,
              rangeBegin + Keywords.RangeSeparator + rangeEnd);
    retFlag = false;
}

void handleLoopInit_Params(std::string &arg3, std::string &arg1) {
    std::vector<std::string> rangeSpecifiers;

    rangeSpecifiers = interp_range(arg3);

    if (rangeSpecifiers.size() != 2) {
        engine.createFailedForLoop();
        return;
    }

    std::string firstRangeSpecifier(rangeSpecifiers.at(0)),
        lastRangeSpecifier(rangeSpecifiers.at(1));

    if (engine.variableExists(firstRangeSpecifier)) {
        if (engine.isNumber(firstRangeSpecifier))
            firstRangeSpecifier = engine.varNumberString(firstRangeSpecifier);
        else
            engine.createFailedForLoop();
    }

    if (engine.variableExists(lastRangeSpecifier)) {
        if (engine.isNumber(lastRangeSpecifier))
            lastRangeSpecifier = engine.varNumberString(lastRangeSpecifier);
        else
            engine.createFailedForLoop();
    }

    if (is_numeric(firstRangeSpecifier) && is_numeric(lastRangeSpecifier)) {
        State.DefaultLoopSymbol = arg1;
        engine.createForLoop(stoi(firstRangeSpecifier),
                             stoi(lastRangeSpecifier));
    } else
        engine.createFailedForLoop();
}

void handleLoopInit_Variable_FileRead(std::string &before) {
    if (FileIO::fileExists(engine.varString(before))) {
        List newList;

        std::ifstream file(engine.varString(before).c_str());
        std::string line;

        if (file.is_open()) {
            while (!file.eof()) {
                std::getline(file, line);
                newList.add(line);
            }

            file.close();

            engine.createForLoop(newList);
        } else {
            error(ErrorCode::READ_FAIL, engine.varString(before));
            engine.createFailedForLoop();
        }
    }
}

void handleLoopInit_Variable_Files(std::string &before) {
    if (FileIO::directoryExists(engine.varString(before)))
        engine.createForLoop(getDirectoryList(before, true));
    else {
        error(ErrorCode::READ_FAIL, engine.varString(before));
        engine.createFailedForLoop();
    }
}

void handleLoopInit_Variable_Directories(std::string &before) {
    if (FileIO::directoryExists(engine.varString(before)))
        engine.createForLoop(getDirectoryList(before, false));
    else {
        error(ErrorCode::READ_FAIL, engine.varString(before));
        engine.createFailedForLoop();
    }
}

void handleLoopInit_Variable_Length(std::string &before) {
    List newList;
    std::string tempVarStr = engine.varString(before);
    int len = tempVarStr.length();

    for (int i = 0; i < len; i++) {
        std::string tempStr;
        tempStr.push_back(tempVarStr[i]);
        newList.add(tempStr);
    }

    engine.createForLoop(newList);
}

void handleLoopInit_ClassMembers_Variables(std::string &before) {
    List newList;

    std::vector<Variable> objVars = engine.getClass(before).getVariables();

    for (int i = 0; i < (int)objVars.size(); i++)
        newList.add(objVars.at(i).name());

    engine.createForLoop(newList);
}

void handleLoopInit_ClassMembers_Methods(std::string &before) {
    List newList;

    std::vector<Method> objMethods = engine.getClass(before).getMethods();

    for (int i = 0; i < (int)objMethods.size(); i++)
        newList.add(objMethods.at(i).name());

    engine.createForLoop(newList);
}

void handleLoopInit_CommandLineArgs() {
    List newList;

    for (int i = 0; i < engine.getArgCount(); i++)
        newList.add(engine.getArg(i));

    engine.createForLoop(newList);
}

void handleLoopInit_While(std::string &arg1, std::string &arg3,
                          std::string &arg2, std::string &arg0) {
    if (engine.variableExists(arg1) && engine.variableExists(arg3)) {
        if (engine.isNumber(arg1) && engine.isNumber(arg3)) {
            if (arg2 == Operators.LessThan ||
                arg2 == Operators.LessThanOrEqual ||
                arg2 == Operators.GreaterThanOrEqual ||
                arg2 == Operators.GreaterThan || arg2 == Operators.Equal ||
                arg2 == Operators.NotEqual)
                engine.createWhileLoop(arg1, arg2, arg3);
            else {
                error(ErrorCode::INVALID_OP, arg0);
                engine.createFailedWhileLoop();
            }
        } else {
            error(ErrorCode::CONV_ERR, arg1 + arg2 + arg3);
            engine.createFailedWhileLoop();
        }
    } else if (is_numeric(arg3) && engine.variableExists(arg1)) {
        if (engine.isNumber(arg1)) {
            if (arg2 == Operators.LessThan ||
                arg2 == Operators.LessThanOrEqual ||
                arg2 == Operators.GreaterThanOrEqual ||
                arg2 == Operators.GreaterThan || arg2 == Operators.Equal ||
                arg2 == Operators.NotEqual)
                engine.createWhileLoop(arg1, arg2, arg3);
            else {
                error(ErrorCode::INVALID_OP, arg0);
                engine.createFailedWhileLoop();
            }
        } else {
            error(ErrorCode::CONV_ERR, arg1 + arg2 + arg3);
            engine.createFailedWhileLoop();
        }
    } else if (is_numeric(arg1) && is_numeric(arg3)) {
        if (arg2 == Operators.LessThan || arg2 == Operators.LessThanOrEqual ||
            arg2 == Operators.GreaterThanOrEqual ||
            arg2 == Operators.GreaterThan || arg2 == Operators.Equal ||
            arg2 == Operators.NotEqual)
            engine.createWhileLoop(arg1, arg2, arg3);
        else {
            error(ErrorCode::INVALID_OP, arg0);
            engine.createFailedWhileLoop();
        }
    } else {
        error(ErrorCode::INVALID_OP, arg0);
        engine.createFailedWhileLoop();
    }
}
void handleIfStatementDecl_Generic(std::string first, std::string second,
                                   std::string oper) {
    if (is_numeric(first) && is_numeric(second)) {
        if (oper == Operators.Equal) {
            engine.createIfStatement(stod(first) == stod(second));
        } else if (oper == Operators.NotEqual) {
            engine.createIfStatement(stod(first) != stod(second));
        } else if (oper == Operators.LessThan) {
            engine.createIfStatement(stod(first) < stod(second));
        } else if (oper == Operators.GreaterThan) {
            engine.createIfStatement(stod(first) > stod(second));
        } else if (oper == Operators.LessThanOrEqual) {
            engine.createIfStatement(stod(first) <= stod(second));
        } else if (oper == Operators.GreaterThanOrEqual) {
            engine.createIfStatement(stod(first) >= stod(second));
        } else {
            error(ErrorCode::INVALID_OPERATOR, oper);
            engine.createIfStatement(false);
        }
    } else {
        if (oper == Operators.Equal) {
            engine.createIfStatement(first == second);
        } else if (oper == Operators.NotEqual) {
            engine.createIfStatement(first != second);
        } else if (oper == Keywords.BeginsWith) {
            engine.createIfStatement(begins_with(first, second));
        } else if (oper == Keywords.EndsWith) {
            engine.createIfStatement(ends_with(first, second));
        } else if (oper == Keywords.Contains) {
            engine.createIfStatement(contains(first, second));
        } else {
            error(ErrorCode::INVALID_OPERATOR, oper);
            engine.createIfStatement(false);
        }
    }
}

void handleIfStatementDecl_Method(std::string arg1, std::string arg1Result,
                                  std::string arg3, std::string arg3Result) {
    if (engine.methodExists(arg1)) {
        parse(arg1);
        arg1Result = State.LastValue;
    } else if (engine.variableExists(arg1)) {
        arg1Result = engine.getVariableValueAsString(arg1);

        if (!is_numeric(arg1Result)) {
            error(ErrorCode::IS_NULL, arg1);
            engine.createIfStatement(false);
        }
    } else
        arg1Result = arg1;

    if (engine.methodExists(arg3)) {
        parse(arg3);
        arg3Result = State.LastValue;
    } else if (engine.variableExists(arg3)) {
        arg3Result = engine.getVariableValueAsString(arg3);

        if (!is_numeric(arg3Result)) {
            error(ErrorCode::IS_NULL, arg3);
            engine.createIfStatement(false);
        }
    } else
        arg3Result = arg3;
}

void handleClassDecl(std::string arg1, std::string arg3, std::string arg2) {
    if (engine.classExists(arg1)) {
        State.DefiningClass = true;
        State.CurrentClass = arg1;
    } else {
        if (engine.classExists(arg3)) {
            if (arg2 == Operators.Assign) {
                std::vector<Method> classMethods =
                    engine.getClass(arg3).getMethods();
                Class newClass(arg1);

                for (int i = 0; i < (int)classMethods.size(); i++) {
                    if (classMethods.at(i).isPublic())
                        newClass.addMethod(classMethods.at(i));
                }

                engine.addClass(newClass);
                State.CurrentClass = arg1;
                State.DefiningClass = true;

                newClass.clear();
                classMethods.clear();
            } else
                error(ErrorCode::INVALID_OPERATOR, arg2);
        } else
            error(ErrorCode::CLS_METHOD_UNDEFINED, arg3);
    }
}

void handleFailedIfStatement() {
    engine.createIfStatement(State.FailedIfStatement);
}

void checkCondition(const std::string arg1, const std::string arg2,
                    const std::string arg3) {
    if (engine.listExists(arg1) && arg2 == Keywords.In) {
        checkListInCondition(arg1, arg2, arg3);
    } else if (engine.listExists(arg1) && arg2 == Keywords.Contains &&
               arg3 != Keywords.IsList) {
        checkListContainsCondition(arg1, arg2, arg3);
    } else if (engine.variableExists(arg1) && engine.variableExists(arg3)) {
        checkVariableCondition(arg1, arg2, arg3);
    } else if ((engine.variableExists(arg1) && !engine.variableExists(arg3)) &&
               !engine.methodExists(arg3) && engine.notClassMethod(arg3) &&
               !has_params(arg3)) {
        checkNumericStringFileDirCondition(arg1, arg2, arg3);
    } else if ((engine.variableExists(arg1) && !engine.variableExists(arg3)) &&
               !engine.methodExists(arg3) && engine.notClassMethod(arg3) &&
               has_params(arg3)) {
        checkNumericStringFileDirCondition(arg1, arg2, getStackValue(arg3));
    } else if ((!engine.variableExists(arg1) && engine.variableExists(arg3)) &&
               !engine.methodExists(arg1) && engine.notClassMethod(arg1) &&
               !has_params(arg1)) {
        checkNumericStringFileDirCondition(arg3, arg2, arg1);
    } else if ((!engine.variableExists(arg1) && engine.variableExists(arg3)) &&
               !engine.methodExists(arg1) && engine.notClassMethod(arg1) &&
               has_params(arg1)) {
        checkNumericStringFileDirCondition(arg3, arg2, getStackValue(arg1));
    } else if (has_params(arg1) || has_params(arg3)) {
        checkParamsCondition(arg1, arg2, arg3);
    } else if ((engine.methodExists(arg1) && arg3 != Keywords.IsMethod) ||
               engine.methodExists(arg3)) {
        checkMethodCondition(arg1, arg3, arg2);
    } else {
        checkGenericCondition(arg1, arg3, arg2);
    }
}

void checkNumericStringFileDirCondition(std::string arg1, std::string arg2,
                                        std::string arg3) {
    if (engine.isNumber(arg1)) {
        if (is_numeric(arg3)) {
            handleIfStatementDecl_Generic(dtos(engine.varNumber(arg1)), arg3,
                                          arg2);
        } else if (arg3 == Keywords.IsNumber) {
            if (arg2 == Operators.Equal)
                engine.createIfStatement(true);
            else if (arg2 == Operators.NotEqual)
                engine.createIfStatement(false);
            else
                error(ErrorCode::INVALID_OPERATOR, arg2);
        } else {
            error(ErrorCode::CONV_ERR, arg2);
            engine.createIfStatement(false);
        }
    } else {
        if (arg3 == Keywords.IsString) {
            if (!engine.isString(arg1)) {
                engine.createIfStatement(arg2 == Operators.NotEqual);
                return;
            }

            if (arg2 == Operators.Equal)
                engine.createIfStatement(true);
            else if (arg2 == Operators.NotEqual)
                engine.createIfStatement(false);
            else {
                error(ErrorCode::INVALID_OPERATOR, arg2);
                engine.createIfStatement(false);
            }
        } else if (arg3 == Keywords.IsNumber) {
            if (!engine.isNumber(arg1)) {
                engine.createIfStatement(arg2 == Operators.NotEqual);
                return;
            }

            if (arg2 == Operators.Equal)
                engine.createIfStatement(true);
            else if (arg2 == Operators.NotEqual)
                engine.createIfStatement(false);
            else {
                error(ErrorCode::INVALID_OPERATOR, arg2);
                engine.createIfStatement(false);
            }
        } else if (arg3 == Keywords.IsFile) {
            if (!engine.isString(arg1)) {
                error(ErrorCode::IS_NULL, arg1);
                engine.createIfStatement(false);
                return;
            }

            if (FileIO::fileExists(engine.varString(arg1))) {
                if (arg2 == Operators.Equal)
                    engine.createIfStatement(true);
                else if (arg2 == Operators.NotEqual)
                    engine.createIfStatement(false);
                else {
                    error(ErrorCode::INVALID_OPERATOR, arg2);
                    engine.createIfStatement(false);
                }
            } else {
                engine.createIfStatement(arg2 == Operators.NotEqual);
            }
        } else if (arg3 == Keywords.IsDirectory) {
            if (!engine.isString(arg1)) {
                error(ErrorCode::IS_NULL, arg1);
                engine.createIfStatement(false);
                return;
            }

            if (FileIO::directoryExists(engine.varString(arg1))) {
                if (arg2 == Operators.Equal)
                    engine.createIfStatement(true);
                else if (arg2 == Operators.NotEqual)
                    engine.createIfStatement(false);
                else {
                    error(ErrorCode::INVALID_OPERATOR, arg2);
                    engine.createIfStatement(false);
                }
            } else {
                engine.createIfStatement(arg2 == Operators.NotEqual);
            }
        } else {
            handleIfStatementDecl_Generic(engine.varString(arg1), arg3, arg2);
        }
    }
}

void checkListInCondition(const std::string listName,
                          const std::string condition,
                          const std::string testValue) {
    std::string testString =
        getTestString(engine.variableExists(testValue), listName);
    if (testString == listName) {
        bool elementFound =
            checkListForElement(listName, testString, condition);
        engine.createIfStatement(!elementFound);
    } else {
        engine.createIfStatement(false);
    }
}

void checkListContainsCondition(const std::string listName,
                                const std::string condition,
                                const std::string testValue) {
    std::string testString =
        getTestString(engine.variableExists(testValue), testValue);
    if (testString == "[none]") {
        engine.createIfStatement(false);
    } else {
        bool elementFound =
            checkListForElement(listName, testString, condition);
        engine.createIfStatement(!elementFound);
    }
}

bool checkListForElement(const std::string listName,
                         const std::string testString,
                         const std::string conditionType) {
    bool result = false;

    if (!engine.listExists(listName)) {
        error(ErrorCode::LIST_UNDEFINED, listName);
        return result;
    }

    if (conditionType == Keywords.In)
        result = checkListContains(listName, testString);
    else if (conditionType == Keywords.Contains &&
             testString != Keywords.IsList)
        result = checkListContains(listName, testString);
    else
        error(ErrorCode::INVALID_OP, conditionType);

    return result;
}

bool checkListContains(const std::string listName,
                       const std::string testString) {
    bool elementFound = false;
    List list = engine.getList(listName);
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i) == testString) {
            elementFound = true;
            engine.createIfStatement(true);
            State.LastValue = itos(i);
            break;
        }
    }

    engine.createIfStatement(elementFound);
    return elementFound;
}

void checkVariableCondition(const std::string arg1, const std::string arg2,
                            const std::string arg3) {
    if (engine.isString(arg1) && engine.isString(arg3)) {
        handleIfStatementDecl_Generic(engine.varString(arg1),
                                      engine.varString(arg3), arg2);
    } else if (engine.isNumber(arg1) && engine.isNumber(arg3)) {
        handleIfStatementDecl_Generic(dtos(engine.varNumber(arg1)),
                                      dtos(engine.varNumber(arg3)), arg2);
    } else {
        error(ErrorCode::CONV_ERR, arg1 + " " + arg2 + " " + arg3);
        engine.createIfStatement(false);
    }
}

void checkParamsCondition(const std::string &arg1, const std::string &arg2,
                          const std::string &arg3) {
    if (!arg1.empty() && !arg2.empty() && !arg3.empty()) {
        double val1 = getStack(arg1);
        double val3 = getStack(arg3);

        if (arg2 == Operators.Equal)
            engine.createIfStatement(val1 == val3);
        else if (arg2 == Operators.NotEqual)
            engine.createIfStatement(val1 != val3);
        else if (arg2 == Operators.LessThan)
            engine.createIfStatement(val1 < val3);
        else if (arg2 == Operators.LessThanOrEqual)
            engine.createIfStatement(val1 <= val3);
        else if (arg2 == Operators.GreaterThan)
            engine.createIfStatement(val1 > val3);
        else if (arg2 == Operators.GreaterThanOrEqual)
            engine.createIfStatement(val1 >= val3);
    }
}

void checkMethodCondition(const std::string arg1, const std::string arg3,
                          const std::string arg2) {
    std::string arg1Result, arg3Result;
    handleIfStatementDecl_Method(arg1, arg1Result, arg3, arg3Result);
    handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
}

void checkGenericCondition(const std::string arg1, const std::string arg3,
                           const std::string arg2) {
    handleIfStatementDecl_Generic(arg1, arg3, arg2);
}

std::string getTestString(bool variableExists, const std::string variableName) {
    std::string testString;

    if (variableExists)
        testString = engine.getVariableValueAsString(variableName);
    else
        testString = variableName;

    return testString;
}

void handleError(ErrorCode errorType, const std::string variableName) {
    error(errorType, variableName);
    engine.createIfStatement(false);
}

void handlePublicDecl() {
    State.DefiningPrivateCode = false;
    State.DefiningPublicCode = true;
}

void handlePrivateDecl() {
    State.DefiningPrivateCode = true;
    State.DefiningPublicCode = false;
}

void handleEnd() {
    State.DefiningPrivateCode = false, State.DefiningPublicCode = false;
    State.DefiningClass = false;
    State.DefiningClassMethod = false;
    State.CurrentClass = "";
}

void handleExit() {
    engine.clearAll();
    exit(0);
}

void handleCaught() {
    std::string to_remove = "remove ";
    to_remove.append(State.ErrorVarName);

    parse(to_remove);

    State.ExecutedTryBlock = false, State.RaiseCatchBlock = false;
    State.LastError = "";
    State.ErrorVarName = "";
}

void handleInlineScriptDecl(std::string &arg1) {
    if (engine.variableExists(arg1)) {
        if (!engine.isString(arg1)) {
            error(ErrorCode::CONV_ERR, arg1);
            return;
        }

        if (!FileIO::fileExists(engine.varString(arg1))) {
            FileIO::createFile(engine.varString(arg1));
            State.DefiningScript = true;
            State.CurrentScriptName = engine.varString(arg1);
        } else
            error(ErrorCode::FILE_EXISTS, engine.varString(arg1));
    } else if (!FileIO::fileExists(arg1)) {
        FileIO::createFile(arg1);
        State.DefiningScript = true;
        State.CurrentScriptName = arg1;
    } else
        error(ErrorCode::FILE_EXISTS, arg1);
}

void handleDirPop(std::string &arg1) {
    if (engine.variableExists(arg1)) {
        if (!engine.isString(arg1)) {
            error(ErrorCode::NULL_STRING, arg1);
            return;
        }

        if (FileIO::directoryExists(engine.varString(arg1)))
            FileIO::removeDirectory(engine.varString(arg1));
        else
            error(ErrorCode::DIR_NOT_FOUND, engine.varString(arg1));
    } else {
        if (FileIO::directoryExists(arg1))
            FileIO::removeDirectory(arg1);
        else
            error(ErrorCode::DIR_NOT_FOUND, arg1);
    }
}

void handleDirPush(std::string &arg1) {
    if (engine.variableExists(arg1)) {
        if (!engine.isString(arg1)) {
            error(ErrorCode::CONV_ERR, arg1);
            return;
        }

        if (!FileIO::directoryExists(engine.varString(arg1)))
            FileIO::makeDirectory(engine.varString(arg1));
        else
            error(ErrorCode::DIR_EXISTS, engine.varString(arg1));
    } else {
        if (!FileIO::directoryExists(arg1))
            FileIO::makeDirectory(arg1);
        else
            error(ErrorCode::DIR_EXISTS, arg1);
    }
}

void handleFilePop(std::string &arg1) {
    if (engine.variableExists(arg1)) {
        if (!engine.isString(arg1)) {
            error(ErrorCode::CONV_ERR, arg1);
            return;
        }

        if (FileIO::fileExists(engine.varString(arg1)))
            FileIO::removeFile(engine.varString(arg1));
        else
            error(ErrorCode::FILE_NOT_FOUND, engine.varString(arg1));
    } else {
        if (FileIO::fileExists(arg1))
            FileIO::removeFile(arg1);
        else
            error(ErrorCode::FILE_NOT_FOUND, arg1);
    }
}

void handleFilePush(std::string &arg1) {
    if (engine.variableExists(arg1)) {
        if (!engine.isString(arg1)) {
            error(ErrorCode::CONV_ERR, arg1);
            return;
        }

        if (!FileIO::fileExists(engine.varString(arg1)))
            FileIO::createFile(engine.varString(arg1));
        else
            error(ErrorCode::FILE_EXISTS, engine.varString(arg1));
    } else {
        if (!FileIO::fileExists(arg1))
            FileIO::createFile(arg1);
        else
            error(ErrorCode::FILE_EXISTS, arg1);
    }
}

void handleUnlockAssignment(std::string &arg1) {
    if (engine.variableExists(arg1))
        engine.getVar(arg1).setIndestructible(false);
    else if (engine.methodExists(arg1))
        engine.getMethod(arg1).setIndestructible(false);
}

void handleLockAssignment(std::string &arg1) {
    if (engine.variableExists(arg1))
        engine.getVar(arg1).setIndestructible(true);
    else if (engine.methodExists(arg1))
        engine.getMethod(arg1).setIndestructible(true);
}

void handleTemplateDecl(std::string &arg1) {
    if (engine.methodExists(arg1))
        error(ErrorCode::METHOD_DEFINED, arg1);
    else {
        if (has_params(arg1)) {
            std::vector<std::string> params = interp_params(arg1);
            Method method(before_params(arg1), true);
            method.setTemplateSize((int)params.size());
            engine.addMethod(method);
            State.DefiningMethod = true;
        }
    }
}

void handleStringInspect(std::string &before, std::string &after,
                         std::string &arg1) {
    if (before.length() != 0 && after.length() != 0) {
        if (!engine.getClass(before).hasVariable(after)) {
            error(ErrorCode::TARGET_UNDEFINED, arg1);
            return;
        }

        State.LastValue = engine.getClassVariable(before, after).getType() ==
                                  ValueType::String
                              ? Keywords.True
                              : Keywords.False;
    } else {
        if (engine.variableExists(arg1))
            State.LastValue =
                !engine.isString(arg1) ? Keywords.False : Keywords.True;
        else
            State.LastValue = is_numeric(arg1) ? Keywords.False : Keywords.True;
    }
}

void handleNumberInspect(std::string &before, std::string &after,
                         std::string &arg1) {
    if (before.length() != 0 && after.length() != 0) {
        if (!engine.getClass(before).hasVariable(after)) {
            error(ErrorCode::TARGET_UNDEFINED, arg1);
            return;
        }

        State.LastValue = engine.getClassVariable(before, after).getType() ==
                                  ValueType::Double
                              ? Keywords.True
                              : Keywords.False;
    } else {
        if (engine.variableExists(arg1))
            State.LastValue =
                engine.isNumber(arg1) ? Keywords.True : Keywords.False;
        else
            State.LastValue = is_numeric(arg1) ? Keywords.True : Keywords.False;
    }
}

void handleCollectInspect(std::string &arg1) {
    if (!engine.variableExists(arg1)) {
        error(ErrorCode::TARGET_UNDEFINED, arg1);
        return;
    }

    State.LastValue =
        engine.getVar(arg1).isCollectable() ? Keywords.True : Keywords.False;
}

void handleFileInspect(std::string &before, std::string &after,
                       std::string &arg1) {
    if (before.length() != 0 && after.length() != 0) {
        if (!engine.getClass(before).hasVariable(after)) {
            error(ErrorCode::TARGET_UNDEFINED, arg1);
            return;
        }

        State.LastValue =
            !FileIO::fileExists(
                engine.getClassVariable(before, after).getString())
                ? Keywords.False
                : Keywords.True;
    } else {
        if (!engine.variableExists(arg1)) {
            State.LastValue =
                !FileIO::fileExists(arg1) ? Keywords.False : Keywords.True;
            return;
        }

        if (!engine.isString(arg1)) {
            State.LastValue = Keywords.False;
            return;
        }

        State.LastValue = !FileIO::fileExists(engine.varString(arg1))
                              ? Keywords.False
                              : Keywords.True;
    }
}

void handleDirectoryInspect(std::string &before, std::string &after,
                            std::string &arg1) {
    if (before.length() != 0 && after.length() != 0) {
        if (!engine.getClass(before).hasVariable(after)) {
            error(ErrorCode::TARGET_UNDEFINED, arg1);
            return;
        }

        State.LastValue =
            FileIO::directoryExists(
                engine.getClassVariable(before, after).getString())
                ? Keywords.True
                : Keywords.False;
    } else {
        if (!engine.variableExists(arg1)) {
            State.LastValue =
                FileIO::directoryExists(arg1) ? Keywords.True : Keywords.False;
            return;
        }

        if (!engine.isString(arg1)) {
            error(ErrorCode::CONV_ERR, arg1);
            return;
        }

        State.LastValue = FileIO::directoryExists(engine.varString(arg1))
                              ? Keywords.True
                              : Keywords.False;
    }
}

void handleListInspect(std::string &arg1) {
    State.LastValue = engine.listExists(arg1) ? Keywords.True : Keywords.False;
}

void handleVariableInspect(std::string &before, std::string &after,
                           std::string &arg1) {
    if (before.length() != 0 && after.length() != 0)
        State.LastValue = engine.getClass(before).hasVariable(after)
                              ? Keywords.True
                              : Keywords.False;
    else
        State.LastValue =
            engine.variableExists(arg1) ? Keywords.True : Keywords.False;
}

void handleClassInspect(std::string &arg1) {
    State.LastValue = engine.classExists(arg1) ? Keywords.True : Keywords.False;
}

void handleMethodInspect(std::string &before, std::string &after,
                         std::string &arg1) {
    if (before.length() != 0 && after.length() != 0)
        State.LastValue = engine.getClass(before).hasMethod(after)
                              ? Keywords.True
                              : Keywords.False;
    else
        State.LastValue =
            engine.methodExists(arg1) ? Keywords.True : Keywords.False;
}

void handleInitialDir(std::string &arg1) {
    if (engine.variableExists(arg1)) {
        if (!engine.isString(arg1)) {
            error(ErrorCode::CONV_ERR, arg1);
            return;
        }

        if (!FileIO::directoryExists(engine.varString(arg1))) {
            error(ErrorCode::DIR_NOT_FOUND, engine.varString(arg1));
            return;
        }

        State.InitialDirectory = engine.varString(arg1);
        FileIO::changeDirectory(State.InitialDirectory);
    } else {
        if (!FileIO::directoryExists(arg1)) {
            error(ErrorCode::DIR_NOT_FOUND, arg1);
            return;
        }

        if (arg1 == Keywords.Dot)
            State.InitialDirectory = FileIO::getCurrentDirectory();
        else if (arg1 == Keywords.RangeSeparator)
            State.InitialDirectory = FileIO::getCurrentDirectory() + "\\..";
        else
            State.InitialDirectory = arg1;

        FileIO::changeDirectory(State.InitialDirectory);
    }
}

void handleInlineShellExec(std::string &arg1) {
    if (!engine.variableExists(arg1)) {
        Env::shellExec(arg1);
        return;
    }

    if (!engine.isString(arg1)) {
        error(ErrorCode::CONV_ERR, arg1);
        return;
    }

    Env::shellExec(engine.varString(arg1));
}

void handleInlineParse(std::string &arg1) {
    if (!engine.variableExists(arg1)) {
        parse(arg1.c_str());
        return;
    }

    if (!engine.isString(arg1)) {
        error(ErrorCode::CONV_ERR, arg1);
        return;
    }

    parse(engine.varString(arg1).c_str());
}

void handleListDecl(std::string &arg1) {
    if (engine.listExists(arg1)) {
        engine.getList(arg1).clear();
        return;
    }

    List newList(arg1);
    newList.setCollectable(State.ExecutedTemplate || State.ExecutedMethod);
    engine.addList(newList);
}

void handleChangeDir(std::string &arg1) {
    if (!engine.variableExists(arg1)) {
        if (arg1 == Keywords.InitialDirectory)
            FileIO::changeDirectory(State.InitialDirectory);
        else if (FileIO::directoryExists(arg1))
            FileIO::changeDirectory(arg1);
        else
            FileIO::changeDirectory(arg1);

        return;
    }

    if (!engine.isString(arg1)) {
        error(ErrorCode::CONV_ERR, arg1);
        return;
    }

    if (!FileIO::directoryExists(engine.varString(arg1))) {
        error(ErrorCode::DIR_NOT_FOUND, engine.varString(arg1));
        return;
    }

    FileIO::changeDirectory(engine.varString(arg1));
}

void handleLoad(std::string &arg1) {
    if (FileIO::fileExists(arg1)) {
        if (is_script(arg1)) {
            State.PreviousScript = State.CurrentScript;
            engine.loadScript(arg1);
            exec.executeScript();
        } else
            error(ErrorCode::BAD_LOAD, arg1);
    } else if (engine.moduleExists(arg1)) {
        std::vector<std::string> lines = engine.getModule(arg1).get();

        for (int i = 0; i < (int)lines.size(); i++)
            parse(lines.at(i));
    } else
        error(ErrorCode::BAD_LOAD, arg1);
}

void handleRemove(std::string &arg1) {
    if (has_params(arg1)) {
        std::vector<std::string> params = interp_params(arg1);

        for (int i = 0; i < (int)params.size(); i++) {
            if (engine.variableExists(params.at(i)))
                engine.removeVariable(params.at(i));
            else if (engine.listExists(params.at(i)))
                engine.removeList(params.at(i));
            else if (engine.classExists(params.at(i)))
                engine.removeClass(params.at(i));
            else if (engine.methodExists(params.at(i)))
                engine.removeMethod(params.at(i));
            else
                error(ErrorCode::TARGET_UNDEFINED, params.at(i));
        }
    } else if (engine.variableExists(arg1))
        engine.removeVariable(arg1);
    else if (engine.listExists(arg1))
        engine.removeList(arg1);
    else if (engine.classExists(arg1))
        engine.removeClass(arg1);
    else if (engine.methodExists(arg1))
        engine.removeMethod(arg1);
    else
        error(ErrorCode::TARGET_UNDEFINED, arg1);
}

void handleDelay(std::string &arg1) {
    if (is_numeric(arg1))
        Thread::sleep(stoi(arg1));
    else
        error(ErrorCode::CONV_ERR, arg1);
}

void handleErr(std::string &arg1) {
    std::string errorValue(arg1);

    if (engine.variableExists(arg1))
        errorValue = engine.getVariableValueAsString(arg1);

    State.LastError = errorValue;

    std::cerr << errorValue << std::endl;
}

void handlePrompt(std::string &arg1) {
    if (arg1 == Keywords.InlineParse) {
        if (State.UseCustomPrompt == true)
            State.UseCustomPrompt = false;
        else
            State.UseCustomPrompt = true;
    } else {
        State.UseCustomPrompt = true;
        State.PromptStyle = arg1;
    }
}

void handleIfStatement(std::string &arg1) {
    std::string tmpValue;

    if (engine.variableExists(arg1)) {
        // can we can assume that arg1 belongs to an object?
        if (!is_dotless(arg1)) {
            std::string objName(before_dot(arg1)), varName(after_dot(arg1));
            Variable tmpVar = engine.getClass(objName).getVariable(varName);
            tmpValue = engine.getVariableValueAsString(tmpVar);
        } else
            tmpValue = engine.getVariableValueAsString(arg1);
    } else if (is_numeric(arg1) || is_truthy(arg1) || is_falsey(arg1)) {
        tmpValue = arg1;
    } else {
        std::string tmpCode;

        if (begins_with(arg1, "(\"") && ends_with(arg1, "\")"))
            tmpCode = substring(arg1, 2, arg1.length() - 3);
        else
            tmpCode = arg1;

        tmpValue = get_parsed_stdout(tmpCode);
    }

    if (is_truthy(tmpValue))
        engine.createIfStatement(true);
    else if (is_falsey(tmpValue))
        engine.createIfStatement(false);
}

void handleGoto(std::string &arg1) {
    if (State.CurrentScript != "" && engine.getScript().markExists(arg1)) {
        State.GoTo = arg1;
        State.GoToLabel = true;
    }
}

void handleSwitch(std::string &arg1) {
    if (!engine.variableExists(arg1)) {
        error(ErrorCode::VAR_UNDEFINED, arg1);
        return;
    }

    State.DefiningSwitchBlock = true;
    State.SwitchVarName = arg1;
}

#endif
