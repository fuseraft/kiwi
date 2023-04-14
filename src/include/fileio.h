#ifndef NOCTIS_FILEIO_H
#define NOCTIS_FILEIO_H

class FileIO {
    public:
    static void appendText(string arg1, string arg2, bool newLine);
    static void writeText(string arg1, string arg2);
};

void FileIO::appendText(string arg1, string arg2, bool newLine)
{
    string target(""), text("");

    if (mem.variableExists(arg1))
    {
        if (!mem.isString(arg1))
        {
            error(ErrorMessage::CONV_ERR, arg1, false);
            return;
        }
        
        target = mem.varString(arg1);

        if (!mem.variableExists(arg2))
        {
            text = arg2;
        }
        else
        {
            if (mem.isString(arg2))
            {
                text = mem.varString(arg2);
            }
            else if (mem.isNumber(arg2))
            {
                text = mem.varNumberString(arg2);
            }
            else
            {
                error(ErrorMessage::IS_NULL, arg2, false);
                return;
            }
        }
    }
    else
    {
        target = arg1;
        
        if (!mem.variableExists(arg2))
        {
            text = arg2;
        }
        else
        {
            if (mem.isString(arg2))
            {
                text = mem.varString(arg2);
            }
            else if (mem.isNumber(arg2))
            {
                text = mem.varNumberString(arg2);
            }
            else
            {
                error(ErrorMessage::IS_NULL, arg2, false);
                return;
            }
        }
    }

    if (!Env::fileExists(target))
    {
        error(ErrorMessage::READ_FAIL, target, false);
        return;
    }

    if (newLine)
        Env::app(target, text + "\r\n");
    else
        Env::app(target, text);
}

void FileIO::writeText(string arg1, string arg2)
{
    string target(""), text("");

    if (mem.variableExists(arg1))
    {
        if (!mem.isString(arg1))
        {
            error(ErrorMessage::CONV_ERR, arg1, false);
            State.LastValue = "-1";
            return;
        }

        target = mem.varString(arg1);
        
        if (!mem.variableExists(arg2))
        {
            text = arg2;
        }
        else
        {
            if (mem.isString(arg2))
            {
                text = mem.varString(arg2);
            }
            else if (mem.isNumber(arg2))
            {
                text = mem.varNumberString(arg2);
            }
            else
            {
                error(ErrorMessage::IS_NULL, arg2, false);
                State.LastValue = "-1";
            }
        }
    }
    else
    {
        if (!mem.variableExists(arg2))
        {
            target = arg1;
            text = arg2;
        }
        else
        {
            if (mem.isString(arg2))
            {
                text = mem.varString(arg2);
            }
            else if (mem.isNumber(arg2))
            {
                text = mem.varNumberString(arg2);
            }
            else
            {
                error(ErrorMessage::IS_NULL, arg2, false);
                State.LastValue = "-1";
                return;
            }
        }
    }

    if (Env::fileExists(target))
    {
        Env::app(target, text + "\r\n");
        State.LastValue = "0";
    }
    else
    {
        Env::createFile(target);
        Env::app(target, text + "\r\n");
        State.LastValue = "1";
    }
}

#endif