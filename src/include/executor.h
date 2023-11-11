#ifndef NOCTIS_EXEC_H
#define NOCTIS_EXEC_H

#include "memory.h"

class Executor {
    private:
    Memory& mem;

    public:
    Executor(Memory& mem_) : mem(mem_) {}
    ~Executor();

    void executeWhileLoop(Method m);
    void executeMethod(Method m);
    void executeMethod(string methodName, string className, string classMethodName);
    void executeNest(Container n);
    void executeForLoop(Method m);
    void executeTemplate(Method m, vector<string> vs);
    void executeSimpleStatement(string left, string oper, string right, string s);
    void executeInspection(string arg0, string arg1, string before, string after);
    void executeScript();
};

Executor::~Executor() {}

void Executor::executeScript()
{
    Script script = mem.getScript();
    
    for (int i = 0; i < script.size(); i++)
    {
        State.CurrentLineNumber = i + 1;

        if (!State.GoToLabel)
            parse(script.at(i));
        else
        {
            bool startParsing = false;
            State.DefiningIfStatement = false;
            State.DefiningForLoop = false;
            State.GoToLabel = false;

            for (int z = 0; z < mem.getScript().size(); z++)
            {
                if (endsWith(mem.getScript().at(z), "::"))
                {
                    string s(mem.getScript().at(z));
                    s = subtractString(s, "::");

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

void Executor::executeInspection(string arg0, string arg1, string before, string after)
{
    if (before.length() != 0 && after.length() != 0)
    {
        if (!mem.classExists(before))
        {
            error(ErrorMessage::CLS_METHOD_UNDEFINED, before, false);
            return;
        }

        if (mem.getClass(before).hasMethod(after))
        {
            for (int i = 0; i < mem.getClass(before).getMethod(after).size(); i++)
                write(mem.getClass(before).getMethod(after).at(i));
        }
        else if (mem.getClass(before).hasVariable(after))
        {
            if (mem.getClass(before).getVariable(after).getString() != State.Null)
                write(mem.getClass(before).getVariable(after).getString());
            else if (mem.getClass(before).getVariable(after).getNumber() != State.NullNum)
                write(dtos(mem.getClass(before).getVariable(after).getNumber()));
            else
                write(State.Null);
        }
        else
            error(ErrorMessage::TARGET_UNDEFINED, arg1, false);

    }
    else
    {
        if (mem.classExists(arg1))
        {
            for (int i = 0; i < mem.getClass(arg1).methodSize(); i++)
                write(mem.getClass(arg1).getMethod(mem.getClass(arg1).getMethodName(i)).name());
            for (int i = 0; i < mem.getClass(arg1).variableSize(); i++)
                write(mem.getClass(arg1).getVariable(mem.getClass(arg1).getVariableName(i)).name());
        }
        else if (mem.constantExists(arg1))
        {
            if (mem.getConstant(arg1).ConstNumber())
                write(dtos(mem.getConstant(arg1).getNumber()));
            else if (mem.getConstant(arg1).ConstString())
                write(mem.getConstant(arg1).getString());
        }
        else if (mem.methodExists(arg1))
        {
            for (int i = 0; i < mem.getMethod(arg1).size(); i++)
                write(mem.getMethod(arg1).at(i));
        }
        else if (mem.variableExists(arg1))
        {
            if (mem.isString(arg1))
                write(mem.varString(arg1));
            else if (mem.isNumber(arg1))
                write(dtos(mem.varNumber(arg1)));
        }
        else if (mem.listExists(arg1))
        {
            for (int i = 0; i < mem.getList(arg1).size(); i++)
                write(mem.getList(arg1).at(i));
        }
        else if (arg1 == "variables")
        {
            for (int i = 0; i < mem.getVariableCount(); i++)
            {
                if (mem.getVar(i).getString() != State.Null)
                    write(mem.getVar(i).name() + ":\t" + mem.getVar(i).getString());
                else if (mem.getVar(i).getNumber() != State.NullNum)
                    write(mem.getVar(i).name() + ":\t" + dtos(mem.getVar(i).getNumber()));
                else
                    write(mem.getVar(i).name() + ":\tis_null");
            }
        }
        else if (arg1 == "lists")
        {
            for (int i = 0; i < mem.getListCount(); i++)
                write(mem.getList(i).name());
        }
        else if (arg1 == "methods")
        {
            for (int i = 0; i < mem.getMethodCount(); i++)
                write(mem.getMethod(i).name());
        }
        else if (arg1 == "classes")
        {
            for (int i = 0; i < mem.getClassCount(); i++)
                write(mem.getClass(i).name());
        }
        else if (arg1 == "constants")
        {
            for (int i = 0; i < mem.getConstantCount(); i++)
                write(mem.getConstant(i).name());
        }
        else if (arg1 == "last")
            write(State.LastValue);
        else
            error(ErrorMessage::TARGET_UNDEFINED, arg1, false);
    }
}

void Executor::executeSimpleStatement(string left, string oper, string right, string s)
{
    if (isNumeric(left) && isNumeric(right))
    {
        if (oper == "+")
            writeline(dtos(stod(left) + stod(right)));
        else if (oper == "-")
            writeline(dtos(stod(left) - stod(right)));
        else if (oper == "*")
            writeline(dtos(stod(left) * stod(right)));
        else if (oper == "/")
            writeline(dtos(stod(left) / stod(right)));
        else if (oper == "**")
            writeline(dtos(pow(stod(left), stod(right))));
        else if (oper == "%")
        {
            if ((int)stod(right) == 0)
                error(ErrorMessage::DIVIDED_BY_ZERO, s, false);
            else
                writeline(dtos((int)stod(left) % (int)stod(right)));
        }
        else
            error(ErrorMessage::INVALID_OPERATOR, oper, false);
    }
    else
    {
        if (oper == "+")
            writeline(left + right);
        else if (oper == "-")
            writeline(subtractString(left, right));
        else if (oper == "*")
        {
            if (zeroNumbers(right))
            {
                error(ErrorMessage::INVALID_OP, s, false);
                return;
            }

            string bigstr("");
            for (int i = 0; i < stoi(right); i++)
            {
                bigstr.append(left);
                write(left);
            }

            State.LastValue = bigstr;
        }
        else if (oper == "/")
            writeline(subtractString(left, right));
        else
            error(ErrorMessage::INVALID_OPERATOR, oper, false);
    }
}

void Executor::executeTemplate(Method m, vector<string> strings)
{
    vector<string> methodLines;

    State.ExecutedTemplate = true;
    State.DontCollectMethodVars = true;
    State.CurrentMethodClass = m.getClass();

    vector<Variable> methodVariables = m.getMethodVariables();

    for (int i = 0; i < (int)methodVariables.size(); i++)
    {
        if (mem.variableExists(strings.at(i)))
        {
            if (mem.isString(strings.at(i)))
                mem.createVariable(methodVariables.at(i).name(), mem.varString(strings.at(i)));
            else if (mem.isNumber(strings.at(i)))
                mem.createVariable(methodVariables.at(i).name(), mem.varNumber(strings.at(i)));
        }
        else if (mem.methodExists(strings.at(i)))
        {
            parse(strings.at(i));

            if (isNumeric(State.LastValue))
                mem.createVariable(methodVariables.at(i).name(), stod(State.LastValue));
            else
                mem.createVariable(methodVariables.at(i).name(), State.LastValue);
        }
        else
        {
            if (isNumeric(strings.at(i)))
                mem.createVariable(methodVariables.at(i).name(), stod(strings.at(i)));
            else
                mem.createVariable(methodVariables.at(i).name(), strings.at(i));
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

    State.ExecutedTemplate = false, State.DontCollectMethodVars = false;

    mem.collectGarbage(); // if (!State.DontCollectMethodVars)
}

void Executor::executeNest(Container n)
{
    State.DefiningNest = false;
    State.DefiningIfStatement = false;

    for (int i = 0; i < n.size(); i++)
    {
        if (State.FailedNest == false)
            parse(n.at(i));
        else
            break;
    }

    State.DefiningIfStatement = true;
}

void Executor::executeMethod(string methodName, string className, string classMethodName)
{
    if (State.DefiningClass)
    {
        if (mem.getClass(State.CurrentClass).hasMethod(methodName))
            executeMethod(mem.getClass(State.CurrentClass).getMethod(methodName));
        else
            error(ErrorMessage::METHOD_UNDEFINED, methodName, false);
        return;
    }
    
    if (className.length() != 0 && classMethodName.length() != 0)
    {
        if (!mem.classExists(className))
        {
            error(ErrorMessage::CLS_METHOD_UNDEFINED, className, true);
            return;
        }

        if (mem.getClass(className).hasMethod(classMethodName))
            executeMethod(mem.getClass(className).getMethod(classMethodName));
        else
            error(ErrorMessage::METHOD_UNDEFINED, methodName, false);
    }
    else
    {
        if (mem.methodExists(methodName))
            executeMethod(mem.getMethod(methodName));
        else
            error(ErrorMessage::METHOD_UNDEFINED, methodName, true);
    }
}

void Executor::executeMethod(Method m)
{
    State.ExecutedMethod = true;
    State.CurrentMethodClass = m.getClass();

    if (State.DefiningParameterizedMethod)
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

                        if (m.getMethodVariables().at(a).getString() != State.Null)
                            newWords.push_back(m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getNumber() != State.NullNum)
                            newWords.push_back(dtos(m.getMethodVariables().at(a).getNumber()));
                    }
                    else if (words.at(x) == variableString)
                    {
                        found = true;

                        if (m.getMethodVariables().at(a).getString() != State.Null)
                            newWords.push_back(m.getMethodVariables().at(a).getString());
                        else if (m.getMethodVariables().at(a).getNumber() != State.NullNum)
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

    State.ExecutedMethod = false;

    mem.collectGarbage();
}

void Executor::executeWhileLoop(Method m)
{
    for (int i = 0; i < m.size(); ++i)
    {
        if (m.at(i) == "leave!")
            State.Breaking = true;
        else
            parse(m.at(i));
    }
}

void Executor::executeForLoop(Method m)
{
    State.DefaultLoopSymbol = "$";

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

            if (State.Breaking == true)
            {
                State.Breaking = false;
                break;
            }
        }
    }
    else
    {
        if (m.isInfinite())
        {
            for (;;)
            {
                for (int z = 0; z < m.size(); z++)
                    parse(m.at(z));

                if (State.Breaking == true)
                {
                    State.Breaking = false;
                    break;
                }
            }
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

                if (State.Breaking == true)
                {
                    State.Breaking = false;
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

                if (State.Breaking == true)
                {
                    State.Breaking = false;
                    break;
                }
            }
        }
    }
}

#endif