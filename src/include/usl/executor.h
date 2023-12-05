#ifndef NOCTIS_EXEC_H
#define NOCTIS_EXEC_H

#include "memory.h"

class Executor {
  private:
    Engine &mem;

  public:
    Executor(Engine &mem_) : mem(mem_) {}
    ~Executor();

    void executeWhileLoop(Method m);
    void executeMethod(Method m);
    void executeMethod(std::string methodName, std::string className,
                       std::string classMethodName);
    void executeNest(Container n);
    void executeForLoop(Method m);
    void executeTemplate(Method m, std::vector<std::string> strings);
    void executeSimpleStatement(std::string left, std::string oper,
                                std::string right);
    void executeScript();
};

Executor::~Executor() {}

void Executor::executeScript() {
    Script script = mem.getScript();

    for (int i = 0; i < script.size(); i++) {
        State.CurrentLineNumber = i + 1;

        if (!State.GoToLabel)
            parse(script.at(i));
        else {
            bool startParsing = false;
            State.DefiningIfStatement = false;
            State.DefiningForLoop = false;
            State.GoToLabel = false;

            for (int z = 0; z < mem.getScript().size(); z++) {
                if (ends_with(mem.getScript().at(z), "::")) {
                    std::string s(mem.getScript().at(z));
                    s = subtract_string(s, "::");

                    if (s == State.GoTo)
                        startParsing = true;
                }

                if (startParsing)
                    parse(mem.getScript().at(z));
            }
        }
    }

    State.CurrentScript = State.PreviousScript;
}

void Executor::executeSimpleStatement(std::string left, std::string oper,
                                      std::string right) {
    if (is_numeric(left) && is_numeric(right)) {
        if (oper == Operators.Add)
            writeline(dtos(stod(left) + stod(right)));
        else if (oper == Operators.Subtract)
            writeline(dtos(stod(left) - stod(right)));
        else if (oper == Operators.Multiply)
            writeline(dtos(stod(left) * stod(right)));
        else if (oper == Operators.Divide)
            writeline(dtos(stod(left) / stod(right)));
        else if (oper == Operators.Exponent)
            writeline(dtos(pow(stod(left), stod(right))));
        else if (oper == Operators.Modulus) {
            if ((int)stod(right) == 0)
                error(ErrorCode::DIVIDED_BY_ZERO,
                      left + " " + oper + " " + right);
            else
                writeline(dtos((int)stod(left) % (int)stod(right)));
        } else
            error(ErrorCode::INVALID_OPERATOR, oper);
    } else {
        if (oper == Operators.Add)
            writeline(left + right);
        else if (oper == Operators.Subtract)
            writeline(subtract_string(left, right));
        else if (oper == Operators.Multiply) {
            if (is_numberless(right)) {
                error(ErrorCode::INVALID_OP, oper);
                return;
            }

            std::string bigstr;
            for (int i = 0; i < stoi(right); i++) {
                bigstr.append(left);
                write(left);
            }

            State.LastValue = bigstr;
        } else if (oper == Operators.Divide)
            writeline(subtract_string(left, right));
        else
            error(ErrorCode::INVALID_OPERATOR, oper);
    }
}

void Executor::executeTemplate(Method m, std::vector<std::string> strings) {
    std::vector<std::string> methodLines;

    State.ExecutedTemplate = true;
    State.DontCollectMethodVars = true;
    State.CurrentMethodClass = m.getClass();

    std::vector<Variable> methodVariables = m.getMethodVariables();

    for (int i = 0; i < (int)methodVariables.size(); i++) {
        if (mem.variableExists(strings.at(i))) {
            if (mem.isString(strings.at(i)))
                mem.createVariable(methodVariables.at(i).name(),
                                   mem.varString(strings.at(i)));
            else if (mem.isNumber(strings.at(i)))
                mem.createVariable(methodVariables.at(i).name(),
                                   mem.varNumber(strings.at(i)));
        } else if (mem.methodExists(strings.at(i))) {
            parse(strings.at(i));

            if (is_numeric(State.LastValue))
                mem.createVariable(methodVariables.at(i).name(),
                                   stod(State.LastValue));
            else
                mem.createVariable(methodVariables.at(i).name(),
                                   State.LastValue);
        } else {
            if (is_numeric(strings.at(i)))
                mem.createVariable(methodVariables.at(i).name(),
                                   stod(strings.at(i)));
            else
                mem.createVariable(methodVariables.at(i).name(), strings.at(i));
        }
    }

    for (int i = 0; i < (int)m.size(); i++) {
        std::string line = m.at(i), word;
        int len = line.length();
        std::vector<std::string> words;

        for (int x = 0; x < len; x++) {
            if (line[x] == ' ') {
                words.push_back(word);
                word.clear();
            } else
                word.push_back(line[x]);
        }

        words.push_back(word);

        std::vector<std::string> newWords;

        for (int x = 0; x < (int)words.size(); x++) {
            bool found = false;

            for (int a = 0; a < (int)strings.size(); a++) {
                std::string variableString("$");
                variableString.append(itos(a));

                if (words.at(x) == variableString) {
                    found = true;

                    newWords.push_back(strings.at(a));
                }
            }

            if (!found)
                newWords.push_back(words.at(x));
        }

        std::string freshLine;

        for (int b = 0; b < (int)newWords.size(); b++) {
            freshLine.append(newWords.at(b));

            if (b != (int)newWords.size() - 1)
                freshLine.push_back(' ');
        }

        methodLines.push_back(freshLine);
    }

    for (int i = 0; i < (int)methodLines.size(); i++)
        parse(methodLines.at(i));

    State.ExecutedTemplate = false, State.DontCollectMethodVars = false;

    mem.gc();
}

void Executor::executeNest(Container n) {
    State.DefiningNest = false;
    State.DefiningIfStatement = false;

    for (int i = 0; i < n.size(); i++) {
        if (!State.FailedNest)
            parse(n.at(i));
        else
            break;
    }

    State.DefiningIfStatement = true;
}

void Executor::executeMethod(std::string methodName, std::string className,
                             std::string classMethodName) {
    if (State.DefiningClass) {
        if (mem.getClass(State.CurrentClass).hasMethod(methodName))
            executeMethod(
                mem.getClass(State.CurrentClass).getMethod(methodName));
        else
            error(ErrorCode::METHOD_UNDEFINED, methodName);
        return;
    }

    if (className.length() != 0 && classMethodName.length() != 0) {
        if (!mem.classExists(className)) {
            error(ErrorCode::CLS_METHOD_UNDEFINED, className);
            return;
        }

        if (mem.getClass(className).hasMethod(classMethodName))
            executeMethod(mem.getClass(className).getMethod(classMethodName));
        else
            error(ErrorCode::METHOD_UNDEFINED, methodName);
    } else {
        if (mem.methodExists(methodName))
            executeMethod(mem.getMethod(methodName));
        else
            error(ErrorCode::METHOD_UNDEFINED, methodName);
    }
}

void Executor::executeMethod(Method m) {
    State.ExecutedMethod = true;
    State.CurrentMethodClass = m.getClass();

    if (State.DefiningParameterizedMethod) {
        std::vector<std::string> methodLines;

        for (int i = 0; i < (int)m.size(); i++) {
            std::string line = m.at(i), word;
            int len = line.length();
            std::vector<std::string> words;

            for (int x = 0; x < len; x++) {
                if (line[x] == ' ') {
                    words.push_back(word);
                    word.clear();
                } else
                    word.push_back(line[x]);
            }

            words.push_back(word);

            std::vector<std::string> newWords;

            for (int x = 0; x < (int)words.size(); x++) {
                bool found = false;

                for (int a = 0; a < (int)m.getMethodVariables().size(); a++) {
                    std::string variableString("$");
                    variableString.append(itos(a));

                    if (words.at(x) == m.getMethodVariables().at(a).name()) {
                        found = true;

                        if (m.getMethodVariables().at(a).getType() ==
                            VariableType::String)
                            newWords.push_back(
                                m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getType() ==
                                 VariableType::Double)
                            newWords.push_back(
                                dtos(m.getMethodVariables().at(a).getNumber()));
                    } else if (words.at(x) == variableString) {
                        found = true;

                        if (m.getMethodVariables().at(a).getType() ==
                            VariableType::String)
                            newWords.push_back(
                                m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getType() ==
                                 VariableType::Double)
                            newWords.push_back(
                                dtos(m.getMethodVariables().at(a).getNumber()));
                    }
                }

                if (!found)
                    newWords.push_back(words.at(x));
            }

            std::string freshLine;

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

    State.ExecutedMethod = false;

    mem.gc();
}

void Executor::executeWhileLoop(Method m) {
    for (int i = 0; i < m.size(); ++i) {
        if (m.at(i) == Keywords.Break)
            State.Breaking = true;
        else
            parse(m.at(i));
    }
}

void Executor::executeForLoop(Method m) {
    State.DefaultLoopSymbol = "$";
    int i = m.start(), limit = m.stop();

    if (m.isListLoop()) {
        limit = m.getList().size();

        while (i < limit) {
            interp_forloopmethod(m, m.getList().at(i));

            if (State.Breaking)
                break;

            ++i;
        }
    } else {
        if (m.isInfinite()) {
            while (true) {
                interp_forloopmethod(m, i);

                if (State.Breaking)
                    break;
            }
        } else if (i < limit) {
            while (i <= limit) {
                interp_forloopmethod(m, i);

                ++i;

                if (State.Breaking)
                    break;
            }
        } else if (i > limit) {
            while (i >= limit) {
                interp_forloopmethod(m, i);

                --i;

                if (State.Breaking)
                    break;
            }
        }
    }

    State.Breaking = false;
}

#endif