#ifndef NOCTIS_FILEIO_H
#define NOCTIS_FILEIO_H

class FileIO
{
public:
    static void appendText(const std::string &target, const std::string &text, bool newLine);
    static void writeText(const std::string &target, const std::string &text);
    static std::string readText(const std::string& filePath);
    static double getFileSize(const std::string& path);
    static bool directoryExists(const std::string& p);
    static bool fileExists(const std::string &p);
    static void appendToFile(const std::string &p, const std::string &a);
    static void createFile(const std::string &p);
    static void makeDirectory(const std::string &p);
    static void removeDirectory(const std::string &p);
    static void removeFile(const std::string &p);
};

double FileIO::getFileSize(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        error(ErrorCode::READ_FAIL, path, true);
        return -std::numeric_limits<double>::max();
    }

    file.seekg(0, std::ios::end);
    long fileSize = file.tellg();
    file.close();

    if (fileSize == -1) {
        error(ErrorCode::READ_FAIL, path, true);
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

void FileIO::appendText(const std::string &target, const std::string &text, bool newLine)
{
    if (!fileExists(target))
    {
        error(ErrorCode::READ_FAIL, target, false);
        return;
    }

    if (newLine)
        appendToFile(target, text + "\r\n");
    else
        appendToFile(target, text);
}

void FileIO::makeDirectory(const std::string &p)
{
    if (mkdir(p.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
        error(ErrorCode::MAKE_DIR_FAIL, p, false);
}

void FileIO::removeDirectory(const std::string &p)
{
    if (rmdir(p.c_str()) != 0)
        error(ErrorCode::REMOVE_DIR_FAIL, p, false);
}

void FileIO::removeFile(const std::string &p)
{
    if (remove(p.c_str()) != 0)
        error(ErrorCode::REMOVE_FILE_FAIL, p, false);
}

bool FileIO::directoryExists(const std::string &p)
{
    DIR *pd;
    if ((pd = opendir(p.c_str())) == NULL)
        return false;
    else
    {
        closedir(pd);
        return true;
    }
}

bool FileIO::fileExists(const std::string &p)
{
    if (!directoryExists(p))
    {
        std::ifstream f(p.c_str());
        if (f.is_open())
        {
            f.close();
            return true;
        }
    }

    return false;
}

void FileIO::createFile(const std::string &p)
{
    std::ofstream f(p.c_str(), std::ios::out);

    if (f.is_open())
        f.close();
    else
        error(ErrorCode::CREATE_FILE_FAIL, p, true);
}

void FileIO::appendToFile(const std::string &p, const std::string &a)
{
    std::ofstream f(p.c_str(), std::ios::out | std::ios::app);

    if (!f.is_open())
    {
        error(ErrorCode::READ_FAIL, p, true);
        return;
    }

    std::string cleaned;
    int l = a.length();

    for (int i = 0; i < l; i++)
    {
        if (a[i] == '\\' && a[i + 1] == 'n')
            cleaned.push_back('\r');
        else if (a[i] == 'n' && a[i - 1] == '\\')
            cleaned.push_back('\n');
        else if (a[i] == 't' && a[i - 1] == '\\')
            cleaned.push_back('\t');
        else if (a[i] == '\'' && a[i - 1] == '\\')
            cleaned.push_back('\"');
        else if (a[i] == '\\' && a[i + 1] == 't')
        {
        }
        else if (a[i] == '\\' && a[i + 1] == '\'')
        {
        }
        else
            cleaned.push_back(a[i]);
    }

    f << cleaned;
    f.close();
}

void FileIO::writeText(const std::string &target, const std::string &text)
{
    if (fileExists(target))
    {
        appendToFile(target, text + "\r\n");
        State.LastValue = "0";
    }
    else
    {
        createFile(target);
        appendToFile(target, text + "\r\n");
        State.LastValue = "1";
    }
}

#endif