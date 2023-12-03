#ifndef NOCTIS_FILEIO_H
#define NOCTIS_FILEIO_H

class FileIO
{
public:
    static void appendText(std::string arg1, std::string arg2, bool newLine);
    static void writeText(std::string arg1, std::string arg2);
    static std::string readText(const std::string& filePath);
    static double getFileSize(const std::string& path);
};

double FileIO::getFileSize(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        // Throw an exception or handle the error appropriately
        std::cerr << "Error opening file: " << path << std::endl;
        return -std::numeric_limits<double>::max();
    }

    file.seekg(0, std::ios::end);
    long fileSize = file.tellg();
    file.close();

    if (fileSize == -1) {
        // Handle error when getting file size
        std::cerr << "Error getting file size for: " << path << std::endl;
        return -std::numeric_limits<double>::max();
    }

    return static_cast<double>(fileSize);
}

std::string FileIO::readText(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);

    if (!file) {
        error(ErrorCode::READ_FAIL, filePath, true);
        return "";
    }

    std::ostringstream content;
    content << file.rdbuf();
    file.close();

    return content.str();
}

void FileIO::appendText(std::string arg1, std::string arg2, bool newLine)
{
    std::string target, text;

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
    std::string target, text;

    if (engine.variableExists(arg1))
    {
        if (!engine.isString(arg1))
        {
            error(ErrorCode::CONV_ERR, arg1, false);
            State.LastValue = "-1";
            return;
        }

        target = engine.varString(arg1);
        text = engine.variableExists(arg2) ? engine.getVariableValueAsString(arg2) : arg2;
    }
    else
    {
        if (engine.variableExists(arg2))
            text = engine.getVariableValueAsString(arg2);
        else
        {
            target = arg1;
            text = arg2;
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