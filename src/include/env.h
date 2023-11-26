#ifndef NOCTIS_ENVIRONMENT_H
#define NOCTIS_ENVIRONMENT_H

#include "io.h"
#include "error.h"

std::string pre_parse(std::string st);
std::string get_parsed_stdout(std::string cmd);
std::string get_stdin_quiet(std::string text);

class Env
{
public:
    Env() {}
    ~Env() {}

    static bool directoryExists(std::string p)
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

    static bool fileExists(std::string p)
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

    static void createFile(std::string p)
    {
        std::ofstream f(p.c_str(), std::ios::out);

        if (f.is_open())
            f.close();
        else
            IO::printerrln("...could not create file: " + p);
    }

    static void appendToFile(std::string p, std::string a)
    {
        std::ofstream f(p.c_str(), std::ios::out | std::ios::app);

        if (!f.is_open())
            IO::printerrln("#!=read_fail");
        else
        {
            std::string cleaned("");
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
    }

    static std::string getStdout(std::string cmd)
    {
        std::string data;
        FILE *stream;
        char buffer[MAX_BUFFER];

        stream = popen(cmd.c_str(), "r");
        while (fgets(buffer, MAX_BUFFER, stream) != NULL)
            data.append(buffer);
        pclose(stream);

        return trim(data);
    }

    static void exec(std::string cmd)
    {
        FILE *stream;
        char buffer[MAX_BUFFER];

        stream = popen(cmd.c_str(), "r");
        while (fgets(buffer, MAX_BUFFER, stream) != NULL)
            IO::print(buffer);

        pclose(stream);
    }

    static int shellExec(std::string s, std::vector<std::string> command)
    {
        exec(pre_parse(s));
        return 0;
    }

    static std::vector<std::string> getDirectoryContents(std::string path, bool filesOnly)
    {
        std::vector<std::string> newList;

        DIR *pd;
        struct dirent *pe;

        std::string dir = path;

        if ((pd = opendir(dir.c_str())) != NULL)
        {
            while ((pe = readdir(pd)) != NULL)
            {
                if (std::string(pe->d_name) != "." && std::string(pe->d_name) != "..")
                {
                    std::string tmp("");

                    if (dir == "/")
                        dir = "";

                    tmp = dir + "/" + std::string(pe->d_name);

                    if (filesOnly)
                    {
                        if (Env::fileExists(tmp))
                        {
                            newList.push_back(tmp);
                        }
                    }
                    else
                    {
                        if (Env::directoryExists(tmp))
                        {
                            newList.push_back(tmp);
                        }
                    }
                }
            }
        }

        closedir(pd);

        return newList;
    }

    static std::string getCurrentDirectory()
    {
        char tmp[PATH_MAX];

        return getcwd(tmp, PATH_MAX) ? std::string(tmp) : std::string("");
    }

    static void changeDirectory(std::string p)
    {
        if (p == "init_dir" || p == "initial_directory")
            changeDirectory(State.InitialDirectory);
        else if (chdir(p.c_str()) != 0)
            error(ErrorMessage::READ_FAIL, p, false);
    }

    static std::string getEnvironmentVariable(std::string s)
    {
        char *cString;
        cString = getenv(s.c_str());

        if (cString != NULL)
            return std::string(cString);
        else
            return "[not_available]";
    }

    static void makeDirectory(std::string p)
    {
        if (mkdir(p.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
            error(ErrorMessage::MAKE_DIR_FAIL, p, false);
    }

    static void removeDirectory(std::string p)
    {
        if (rmdir(p.c_str()) != 0)
            error(ErrorMessage::REMOVE_DIR_FAIL, p, false);
    }

    static void removeFile(std::string p)
    {
        if (remove(p.c_str()) != 0)
            error(ErrorMessage::REMOVE_FILE_FAIL, p, false);
    }

    static std::string getUser()
    {
        char *pUser;
        pUser = getenv("USER");

        if (pUser != NULL)
            return pUser;

        return "";
    }

    static std::string getMachine()
    {
        const int MAXHOSTNAMELEN = 1024;
        char name[MAXHOSTNAMELEN];
        size_t namelen = MAXHOSTNAMELEN;

        if (gethostname(name, namelen) != -1)
            return name;

        return "";
    }
};

#endif