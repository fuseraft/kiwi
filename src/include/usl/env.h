#ifndef NOCTIS_ENVIRONMENT_H
#define NOCTIS_ENVIRONMENT_H

#include "error.h"

std::string pre_parse(std::string st);
std::string get_parsed_stdout(std::string cmd);
std::string get_stdin_quiet(std::string text);

class Env {
  public:
    Env() {}
    ~Env() {}

    static std::string getStdout(std::string cmd) {
        std::string data;
        FILE *stream;
        char buffer[MAX_BUFFER];

        stream = popen(cmd.c_str(), "r");
        while (fgets(buffer, MAX_BUFFER, stream) != NULL)
            data.append(buffer);
        pclose(stream);

        return trim(data);
    }

    static void exec(std::string cmd) { system(cmd.c_str()); }

    static int shellExec(std::string s, std::vector<std::string> command) {
        exec(pre_parse(s));
        return 0;
    }

    static std::string getEnvironmentVariable(std::string s) {
        char *cString;
        cString = getenv(s.c_str());

        if (cString != NULL)
            return std::string(cString);
        else
            return "";
    }

    static std::string getUser() {
        char *pUser;
        pUser = getenv("USER");

        if (pUser != NULL)
            return pUser;

        return "";
    }

    static std::string getMachine() {
        const int MAXHOSTNAMELEN = 1024;
        char name[MAXHOSTNAMELEN];
        size_t namelen = MAXHOSTNAMELEN;

        if (gethostname(name, namelen) != -1)
            return name;

        return "";
    }
};

#endif