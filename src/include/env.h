#ifndef NOCTIS_ENVIRONMENT_H
#define NOCTIS_ENVIRONMENT_H

#include "io.h"
#include "error.h"

string cleanString(string st);
string getParsedOutput(string cmd);
string getSilentOutput(string text);

class Env
{
public:
    Env() {}
    ~Env() {}

    static bool directoryExists(string p)
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

    static bool fileExists(string p)
    {
        if (!directoryExists(p))
        {
            ifstream f(p.c_str());
            if (f.is_open())
            {
                f.close();
                return true;
            }
        }

        return false;
    }

    static void createFile(string p)
    {
        ofstream f(p.c_str(), ios::out);

        if (f.is_open())
            f.close();
        else
            IO::printerrln("...could not create file: " + p);
    }

    static void app(string p, string a)
    {
        ofstream f(p.c_str(), ios::out | ios::app);

        if (!f.is_open())
            IO::printerrln("#!=read_fail");
        else
        {
            string cleaned("");
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

    static string getStdout(string cmd)
    {
        string data;
        FILE *stream;
        char buffer[MAX_BUFFER];

        stream = popen(cmd.c_str(), "r");
        while (fgets(buffer, MAX_BUFFER, stream) != NULL)
            data.append(buffer);
        pclose(stream);

        return trim(data);
    }

    static void exec(string cmd)
    {
        FILE *stream;
        char buffer[MAX_BUFFER];

        stream = popen(cmd.c_str(), "r");
        while (fgets(buffer, MAX_BUFFER, stream) != NULL)
            IO::print(buffer);

        pclose(stream);
    }

    static int sysExec(string s, vector<string> command)
    {
        /*string _cleaned;
        _cleaned = cleanstring(s);
        for (int i = 0; i < (int)methods.size(); i++)
        {
            if (command.at(0) == methods.at(i).name())
            {
                if ((int)command.size() - 1 == (int)methods.at(i).getmethodvariables().size())
                {
                    // work
                }
            }
        }*/

        exec(cleanString(s));
        return 0;
    }

    static vector<string> getDirectoryContents(string path, bool filesOnly)
    {
        vector<string> newList;

        DIR *pd;
        struct dirent *pe;

        string dir = path;

        if ((pd = opendir(dir.c_str())) != NULL)
        {
            while ((pe = readdir(pd)) != NULL)
            {
                if (string(pe->d_name) != "." && string(pe->d_name) != "..")
                {
                    string tmp("");

                    if (dir == "/")
                        dir = "";

                    tmp = dir + "/" + string(pe->d_name);

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

    static string cwd()
    {
        char tmp[PATH_MAX];

        return getcwd(tmp, PATH_MAX) ? string(tmp) : string("");
    }

    static void cd(string p)
    {
        if (containsTilde(p))
        {
            if (Env::directoryExists(p))
                cd(p);
            else
                error(ErrorMessage::READ_FAIL, p, false);
        }
        else
        {
            if (p == "init_dir" || p == "initial_directory")
                cd(NoctisEnv.InitialDirectory);
            else
            {
                if (chdir(p.c_str()) != 0)
                    error(ErrorMessage::READ_FAIL, p, false);
            }
        }
    }

    static string getEnvironmentVariable(string s)
    {
        char *cString;
        cString = getenv(s.c_str());

        if (cString != NULL)
            return string(cString);
        else
            return "[not_available]";
    }

    static void md(string p)
    {
        if (mkdir(p.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
            error(ErrorMessage::MAKE_DIR_FAIL, p, false);
    }

    static void rd(string p)
    {
        if (rmdir(p.c_str()) != 0)
            error(ErrorMessage::REMOVE_DIR_FAIL, p, false);
    }

    static void rm(string p)
    {
        if (remove(p.c_str()) != 0)
            error(ErrorMessage::REMOVE_FILE_FAIL, p, false);
    }

    static string getUser()
    {
        char *pUser;
        pUser = getenv("USER");

        if (pUser != NULL)
            return pUser;

        return "";
    }

    static string getMachine()
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