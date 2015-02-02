/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#ifndef LOCAL_ENVIRONMENT_H
#define LOCAL_ENVIRONMENT_H

string __InitialDirectory;

int __GuessedOS;
const int OS_UNKNOWN	= 0x0000;
const int OS_NIX	    = 0x0001;
const int OS_WIN64	    = 0x0002;
const int OS_WIN32	    = 0x0004;
string getGuessedOS(string os);

string cwd();
string getEnvironmentVariable(string s);
string getUser();
string getMachine();

string getParsedOutput(string cmd);
string getSilentOutput(string text);
int sysExec(string s, vector<string> command);

vector<string> getDirectoryContents(string path, bool filesOnly);
void cd(string p);
void md(string p);
void rm(string p);
void rd(string p);

string getStdout(string cmd)
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

void exec(string cmd)
{
    FILE *stream;
    char buffer[MAX_BUFFER];

    stream = popen(cmd.c_str(), "r");
    while (fgets(buffer, MAX_BUFFER, stream) != NULL)
        cout << buffer;
		
    pclose(stream);
}

string getGuessedOS()
{
    string guessedOS("");

    switch (__GuessedOS)
    {
    case OS_NIX:
        guessedOS = "OS_NIX";
        break;
    case OS_WIN32:
        guessedOS = "OS_WIN32";
        break;
    case OS_WIN64:
        guessedOS = "OS_WIN64";
        break;
    case OS_UNKNOWN:
        guessedOS = "OS_UNKNOWN";
        break;
    }

    return guessedOS;
}

vector<string> getDirectoryContents(string path, bool filesOnly)
{
    vector<string> newList;

    DIR *pd;
    struct dirent *pe;

    string dir = path;

    if ((pd = opendir(dir.c_str())) == NULL)
        doNothing();
    else
    {
        while ((pe = readdir(pd)) != NULL)
        {
            if (string(pe->d_name) != "." && string(pe->d_name) != "..")
            {
                string tmp("");

                if (dir == "/")
                    dir = "";

                if (__GuessedOS == OS_NIX)
                    tmp = dir + "/" + string(pe->d_name);
                else
                    tmp = dir + "\\" + string(pe->d_name);

                if (filesOnly)
                {
                    if (fileExists(tmp))
                    {
                        newList.push_back(tmp);
                    }
                }
                else
                {
                    if (directoryExists(tmp))
                    {
                        newList.push_back(tmp);
                    }
                }
            }
        }
    }

    closedir(pd);

    return (newList);
}

#ifdef _WIN32
const int PATH_MAX = 1024;
#endif

string cwd()
{
    char tmp[PATH_MAX];

    return (getcwd(tmp, PATH_MAX) ? string(tmp) : string(""));
}

void cd(string p)
{
    if (containsTilde(p))
    {
        string cleaned("");
        int l = p.length();

        for (int i = 0; i < l; i++)
        {
            if (p[i] == '~')
            {
                switch (__GuessedOS)
                {
                case OS_NIX:
                    cleaned.append(getEnvironmentVariable("HOME"));
                    break;
                case OS_WIN32:
                case OS_WIN64:
                    cleaned.append(getEnvironmentVariable("HOMEPATH"));
                    break;
                case OS_UNKNOWN:
                    cleaned.append(getEnvironmentVariable("HOME"));
                    break;
                default:
                    error(UNDEFINED_OS, "", false);
                    break;
                }
            }
            else
                cleaned.push_back(p[i]);
        }

        if (directoryExists(cleaned))
            cd(cleaned);
        else
            error(READ_FAIL, p, false);
    }
    else
    {
        if (p == "~")
        {
            if (__GuessedOS == OS_NIX || __GuessedOS == OS_UNKNOWN)
                cd(getEnvironmentVariable("HOME"));
            else
                cd(getEnvironmentVariable("HOMEPATH"));
        }
        else if (p == "init_dir" || p == "initial_directory")
            cd(__InitialDirectory);
        else
        {
            if (chdir(p.c_str()) != 0)
                error(READ_FAIL, p, false);
        }
    }
}

string getEnvironmentVariable(string s)
{
    char * cString;
    cString = getenv(s.c_str());

    if (cString != NULL)
        return (string(cString));
    else
        return ("[not_available]");
}

#ifdef __linux__

void md(string p)
{
    if (mkdir(p.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0)
        cerr << "..could not create directory: " << p << endl;
}

void rd(string p)
{
    if (rmdir(p.c_str()) != 0)
        cerr << "..could not remove directory: " << p << endl;
}

void rm(string p)
{
    if (remove(p.c_str()) != 0)
        cout << "..could not remove file: " << p << endl;
}

string getUser()
{
    char * pUser;
    pUser = getenv("USER");

    if (pUser != NULL)
        return (pUser);
    return ("#!=no_user");
}

const int MAXHOSTNAMELEN = 1024;

string getMachine()
{
    char name[MAXHOSTNAMELEN];
    size_t namelen = MAXHOSTNAMELEN;

    if (gethostname(name, namelen) != -1)
        return (name);

    return ("#!=no_machine");
}

#elif defined _WIN32 || defined _WIN64

void md(string p)
{
    if (mkdir(p.c_str()) != 0)
        cerr << "..could not create directory: " << p << endl;
}

void rd(string p)
{
    if (rmdir(p.c_str()) != 0)
        cerr << "..could not remove directory: " << p << endl;
}

void rm(string p)
{
    if (remove(p.c_str()) != 0)
        cerr << "..could not remove file: " << p << endl;
}

string getUser()
{
    char lpszUsername[255];
    DWORD dUsername = sizeof(lpszUsername);

    if(GetUserName(lpszUsername, &dUsername))
        return(lpszUsername);

    return ("#!=no_user");
}

string getMachine()
{
    char lpszComputer[255];
    DWORD dComputer = sizeof(lpszComputer);

    if(GetComputerName(lpszComputer, &dComputer))
        return (lpszComputer);

    return ("#!=no_machine");
}

#endif

#endif
