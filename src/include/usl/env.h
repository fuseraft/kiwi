#ifndef NOCTIS_ENVIRONMENT_H
#define NOCTIS_ENVIRONMENT_H

#include "error.h"

std::string pre_parse(std::string st);
std::string get_parsed_stdout(std::string cmd);
std::string get_stdin_quiet(std::string text);

class Env
{
public:
    Env() {}
    ~Env() {}

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
        system(cmd.c_str());
    }

    static int shellExec(std::string s, std::vector<std::string> command)
    {
        exec(pre_parse(s));
        return 0;
    }

    static std::vector<std::string> getDirectoryContents(std::string path, bool filesOnly)
    {
        const std::string PathSeparator = "/";
        std::vector<std::string> newList;

        DIR *pd;
        struct dirent *pe;

        std::string dir = path;

        if ((pd = opendir(dir.c_str())) != NULL)
        {
            while ((pe = readdir(pd)) != NULL)
            {
                if (std::string(pe->d_name) != Keywords.Dot && std::string(pe->d_name) != Keywords.RangeSeparator)
                {
                    std::string tmp;

                    if (dir == PathSeparator)
                        dir = "";

                    tmp = dir + PathSeparator + std::string(pe->d_name);

                    if (filesOnly)
                    {
                        if (FileIO::fileExists(tmp))
                        {
                            newList.push_back(tmp);
                        }
                    }
                    else
                    {
                        if (FileIO::directoryExists(tmp))
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
        if (p == Keywords.InitialDirectory)
            changeDirectory(State.InitialDirectory);
        else if (chdir(p.c_str()) != 0)
            error(ErrorCode::READ_FAIL, p, false);
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