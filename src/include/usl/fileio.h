#ifndef NOCTIS_FILEIO_H
#define NOCTIS_FILEIO_H

class FileIO
{
public:
    static void appendText(std::string arg1, std::string arg2, bool newLine);
    static void writeText(std::string arg1, std::string arg2);
};

void FileIO::appendText(std::string arg1, std::string arg2, bool newLine)
{
    std::string target(""), text("");

    if (engine.variableExists(arg1))
    {
        if (!engine.isString(arg1))
        {
            error(ErrorCode::CONV_ERR, arg1, false);
            return;
        }

        target = engine.varString(arg1);

        if (!engine.variableExists(arg2))
        {
            text = arg2;
        }
        else
        {
            if (engine.isString(arg2))
            {
                text = engine.varString(arg2);
            }
            else if (engine.isNumber(arg2))
            {
                text = engine.varNumberString(arg2);
            }
            else
            {
                error(ErrorCode::IS_NULL, arg2, false);
                return;
            }
        }
    }
    else
    {
        target = arg1;

        if (!engine.variableExists(arg2))
        {
            text = arg2;
        }
        else
        {
            if (engine.isString(arg2))
            {
                text = engine.varString(arg2);
            }
            else if (engine.isNumber(arg2))
            {
                text = engine.varNumberString(arg2);
            }
            else
            {
                error(ErrorCode::IS_NULL, arg2, false);
                return;
            }
        }
    }

    if (!Env::fileExists(target))
    {
        error(ErrorCode::READ_FAIL, target, false);
        return;
    }

    if (newLine)
        Env::appendToFile(target, text + "\r\n");
    else
        Env::appendToFile(target, text);
}

void FileIO::writeText(std::string arg1, std::string arg2)
{
    std::string target(""), text("");

    if (engine.variableExists(arg1))
    {
        if (!engine.isString(arg1))
        {
            error(ErrorCode::CONV_ERR, arg1, false);
            State.LastValue = "-1";
            return;
        }

        target = engine.varString(arg1);

        if (!engine.variableExists(arg2))
        {
            text = arg2;
        }
        else
        {
            if (engine.isString(arg2))
            {
                text = engine.varString(arg2);
            }
            else if (engine.isNumber(arg2))
            {
                text = engine.varNumberString(arg2);
            }
            else
            {
                error(ErrorCode::IS_NULL, arg2, false);
                State.LastValue = "-1";
            }
        }
    }
    else
    {
        if (!engine.variableExists(arg2))
        {
            target = arg1;
            text = arg2;
        }
        else
        {
            if (engine.isString(arg2))
            {
                text = engine.varString(arg2);
            }
            else if (engine.isNumber(arg2))
            {
                text = engine.varNumberString(arg2);
            }
            else
            {
                error(ErrorCode::IS_NULL, arg2, false);
                State.LastValue = "-1";
                return;
            }
        }
    }

    if (Env::fileExists(target))
    {
        Env::appendToFile(target, text + "\r\n");
        State.LastValue = "0";
    }
    else
    {
        Env::createFile(target);
        Env::appendToFile(target, text + "\r\n");
        State.LastValue = "1";
    }
}

#endif