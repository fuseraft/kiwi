if (mem.listExists(arg3) && arg2 == "in")
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

    if (testString == "[none]")
    {
        mem.createIfStatement(false);
    }
    else
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

        mem.createIfStatement(!elementFound);
    }
}
else if (mem.listExists(arg1) && arg2 == "contains" && arg3 != "list?")
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

    if (testString == "[none]")
    {
        mem.createIfStatement(false);
    }
    else
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

        mem.createIfStatement(!elementFound);
    }
}
else if (mem.variableExists(arg1) && mem.variableExists(arg3))
{
    if (mem.isString(arg1) && mem.isString(arg3))
    {
        handleIfStatementDecl_Generic(mem.varString(arg1), mem.varString(arg3), arg2);
    }
    else if (mem.isNumber(arg1) && mem.isNumber(arg3))
    {
        handleIfStatementDecl_Generic(dtos(mem.varNumber(arg1)), dtos(mem.varNumber(arg3)), arg2);
    }
    else
    {
        error(ErrorMessage::CONV_ERR, arg0, false);
        mem.createIfStatement(false);
    }
}
else if ((mem.variableExists(arg1) && !mem.variableExists(arg3)) && !mem.methodExists(arg3) && mem.notClassMethod(arg3) && !containsParams(arg3))
{

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
            handleIfStatementDecl_Generic(dtos(mem.varNumber(arg1)), stackValue, arg2);
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
            error(ErrorMessage::CONV_ERR, arg0, false);
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
                mem.createIfStatement(arg2 == "!=");
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
                mem.createIfStatement(arg2 == "!=");
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
                    mem.createIfStatement(arg2 == "!=");
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
                    mem.createIfStatement(arg2 == "!=");
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
            handleIfStatementDecl_Generic(mem.varString(arg1), stackValue, arg2);
        }
    }
}
else if ((!mem.variableExists(arg1) && mem.variableExists(arg3)) && !mem.methodExists(arg1) && mem.notClassMethod(arg1) && !containsParams(arg1))
{
    if (mem.isNumber(arg3))
    {
        if (isNumeric(arg1))
        {
            handleIfStatementDecl_Generic(dtos(mem.varNumber(arg3)), arg1, arg2);
        }
        else
        {
            error(ErrorMessage::CONV_ERR, arg0, false);
            mem.createIfStatement(false);
        }
    }
    else
    {
        handleIfStatementDecl_Generic(mem.varString(arg3), arg1, arg2);
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
            handleIfStatementDecl_Generic(dtos(mem.varNumber(arg3)), stackValue, arg2);
        }
        else
        {
            error(ErrorMessage::CONV_ERR, arg0, false);
            mem.createIfStatement(false);
        }
    }
    else
    {
        handleIfStatementDecl_Generic(mem.varString(arg3), stackValue, arg2);
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

                handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
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

                handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
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

                handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
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

            handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
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
                    handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
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

                handleIfStatementDecl_Generic(stackValue, comp, arg2);
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
                arg3Result = arg3;

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

                if (pass)
                {
                    handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
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
        string arg1Result(arg1), arg3Result("");
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

                if (pass)
                {
                    handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
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

                arg1Result = arg1;
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

                handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
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
    handleIfStatementDecl_Method(arg1, arg1Result, arg3, arg3Result);
    handleIfStatementDecl_Generic(arg1Result, arg3Result, arg2);
}
else
{
    handleIfStatementDecl_Generic(arg1, arg3, arg2);
}