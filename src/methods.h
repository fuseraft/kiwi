/**
 * Project:		Unorthodox Scripting Language (USL)
 * Version:		0.0.0.1
 *
 * Filename:	methods.h
 *
 * Author:		Scott Christopher Stauffer
 * Email:		scstauf@gmail.com
 * Date:		Friday, August 23, 2013
 *
 * Copyright 2013
 */

void doNothing() { }

string  afterUS(string s),
        afterBrackets(string s),
        afterDot(string s),
        afterUS(string s),
        beforeDot(string s),
        beforeBrackets(string s),
        beforeParams(string s),
        beforeUS(string s),
        cwd(),
        getEnvironmentVariable(string s),
        getUser(),
        getMachine(),
        getStdout(string cmd),
        getUpper(string in),
        getLower(string in),
        subtractChar(string s1, string s2),
        subtractString(string s1, string s2),
        trim_leading_whitespace(char *str);

bool    contains(string s1, string s2),
        containsBrackets(string s),
        containsParams(string s),
        directoryExists(string p),
        exists(string p),
        fileExists(string p),
        isAlpha(string s),
        isUpper(string in),
        isLower(string in),
        oneDot(string s),
        endsWith(string fullString, string ending),
        startsWith(string s1, string s2),
        zeroDots(string s),
        zeroNumbers(string s);

void    app(string p, string a),
        cd(string p),
        p(string s),
        printUSLHelp(),
        md(string p),
        rm(string p),
        rd(string p),
        touch(string p);

int     stoi(string s);
string  itos(int i);

vector<string> getParams(string s);
vector<string> getRange(string s);
vector<string> getBracketRange(string s);

// Evan Teran @ stackoverflow.com
// trim from both ends
// trim from start
static inline std::string &ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}
// trim from end
static inline std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}
static inline std::string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}
// EO Evan Teran

// From indiv @ stackoverflow
// http://stackoverflow.com/users/19719/indiv
string trim_leading_whitespace(char *str)
{
    size_t len = 0;
    char *frontp = str - 1;
    char *endp = NULL;

    if( str == NULL )
        return NULL;

    if( str[0] == '\0' )
        return str;

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address
     * the first non-whitespace characters from
     * each end.
     */
    while( isspace(*(++frontp)) );
    while( isspace(*(--endp)) && endp != frontp );

    if( str + len - 1 != endp )
        *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
        *str = '\0';

    /* Shift the string so that it starts at str so
     * that if it's dynamically allocated, we can
     * still free it on the returned pointer.  Note
     * the reuse of endp to mean the front of the
     * string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
        while( *frontp ) *endp++ = *frontp++;
        *endp = '\0';
    }

    string ret_str(str);

    return (ret_str);
}
// EO indiv

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

bool isUpper(string in)
{
    for (int i = 0; i < (int)in.length(); i++)
    {
        char c = in[i];

        if (!isupper(c))
            return (false);
    }

    return (true);
}

bool isUpperConstant(string in)
{
    for (int i = 0; i < (int)in.length(); i++)
    {
        char c = in[i];

        if (!isupper(c))
        {
            if (c != '_')
                return (false);
        }
    }

    return (true);
}

bool isLower(string in)
{
    for (int i = 0; i < (int)in.length(); i++)
    {
        char c = in[i];

        if (!islower(c))
            return (false);
    }

    return (true);
}

string getUpper(string in)
{
    string builder("");
    for (int i = 0; i < (int)in.length(); i++)
    {
        char c = in[i];
        builder.push_back(toupper(c));
    }

    return (builder);
}

string getLower(string in)
{
    string builder("");
    for (int i = 0; i < (int)in.length(); i++)
    {
        char c = in[i];
        builder.push_back(tolower(c));
    }

    return (builder);
}

int dot_count(string s)
{
    int l = s.length(), c = 0;
    for (int i = 0; i < l; i++)
    {
        if (s[i] == '.')
            c++;
    }

    return (c);
}

#ifdef _WIN32
const int PATH_MAX = 1024;
#endif

string cwd()
{
    char tmp[PATH_MAX];

    return (getcwd(tmp, PATH_MAX) ? string(tmp) : string(""));
}

bool contains(string s1, string s2)
{
    return s1.find(s2) != string::npos;
}

bool containsTilde(string s)
{
    int l = s.length();

    if (s == "~")
        return true;
    else
        for (int i = 0; i < l; i++)
            if (s[i] == '~')
                return true;

    return false;
}

bool directoryExists(string p)
{
    DIR *pd;
    if ((pd = opendir(p.c_str())) == NULL)
        return (false);
    else
    {
        closedir(pd);
        return (true);
    }
}

bool fileExists(string p)
{
    if (!directoryExists(p))
    {
        ifstream f(p.c_str());
        if (f.is_open())
        {
            f.close();
            return (true);
        }
    }

    return (false);
}

void app(string p, string a)
{
    ofstream f(p.c_str(), ios::out|ios::app);

    if (!f.is_open())
        cout << ("#!=read_fail") << endl;
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
            else if (a[i] == '\\' && a[i + 1] == 't')
                doNothing();
            else if (a[i] == 't' && a[i - 1] == '\\')
                cleaned.push_back('\t');
            else if (a[i] == '\\' && a[i + 1] == '\'')
                doNothing();
            else if (a[i] == '\'' && a[i - 1] == '\\')
                cleaned.push_back('\"');
            else
                cleaned.push_back(a[i]);
        }

        f << cleaned;
        f.close();
    }
}

bool containsParams(string s)
{
    int sl = s.length();

    for (int i = 0; i < sl; i++)
    {
        if (s[i] == '(')
            return (true);
    }

    return (false);
}

bool containsBrackets(string s)
{
    int sl = s.length();

    for (int i = 0; i < sl; i++)
    {
        if (s[i] == '[')
            return (true);
    }

    return (false);
}


vector<string> getParams(string s)
{
    vector<string> params;

    int sl = s.length();
    bool start_push = false;

    string new_name("");

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
        {
            if (s[i] == ',')
            {
                params.push_back(new_name);
                new_name.clear();
            }
            else if (s[i] == ')')
                start_push = false;
            else
                new_name.push_back(s[i]);
        }
        else
        {
            if (s[i] == '(')
                start_push = true;
        }
    }

    params.push_back(new_name);

    return (params);
}

vector<string> getBracketRange(string s)
{
    vector<string> params;

    int sl = s.length();
    bool start_push = false, almost_push = false;

    string new_name("");

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
        {
            if (s[i] == '.')
            {
                if (!almost_push)
                    almost_push = true;
                else
                {
                    params.push_back(new_name);
                    new_name.clear();
                }
            }
            else if (s[i] == ']')
                start_push = false;
            else if (s[i] == ' ')
                doNothing();
            else
                new_name.push_back(s[i]);
        }
        else if (s[i] == '[')
            start_push = true;
    }

    params.push_back(new_name);

    return (params);
}

vector<string> getRange(string s)
{
    vector<string> params;

    int sl = s.length();
    bool start_push = false, almost_push = false;

    string new_name("");

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
        {
            if (s[i] == '.')
            {
                if (!almost_push)
                    almost_push = true;
                else
                {
                    params.push_back(new_name);
                    new_name.clear();
                }
            }
            else if (s[i] == ')')
                start_push = false;
            else if (s[i] == ' ')
                doNothing();
            else
                new_name.push_back(s[i]);
        }
        else if (s[i] == '(')
            start_push = true;
    }

    params.push_back(new_name);

    return (params);
}

string beforeParams(string s)
{
    int sl = s.length();
    bool stop_push = false;
    string new_str("");

    for (int i = 0; i < sl; i++)
    {
        if (s[i] == '(')
            stop_push = true;

        if (!stop_push)
            new_str.push_back(s[i]);
    }

    return (new_str);
}


string beforeBrackets(string s)
{
    int sl = s.length();
    bool stop_push = false;
    string new_str("");

    for (int i = 0; i < sl; i++)
    {
        if (s[i] == '[')
            stop_push = true;

        if (!stop_push)
            new_str.push_back(s[i]);
    }

    return (new_str);
}

string afterBrackets(string s)
{
    string var("");
    int sl = s.length();
    bool start_push = false;

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
            var.push_back(s[i]);
        else if (s[i] == '[')
            start_push = true;
    }

    s = subtractChar(s, "]");

    return (var);
}

string afterDot(string s)
{
    string var("");
    int sl = s.length();
    bool start_push = false;

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
            var.push_back(s[i]);
        else
        {
            if (s[i] == '.')
                start_push = true;
        }
    }

    return (var);
}

string beforeDot(string s)
{
    string var("");
    int sl = s.length();
    bool start_push = true;

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
        {
            if (s[i] == '.')
                start_push = false;
            else
                var.push_back(s[i]);
        }
    }

    return (var);
}

string afterUS(string s)
{
    string var("");
    int sl = s.length();
    bool start_push = false;

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
            var.push_back(s[i]);
        else
        {
            if (s[i] == '_')
                start_push = true;
        }
    }

    return (var);
}

string beforeUS(string s)
{
    string var("");
    int sl = s.length();
    bool start_push = true;

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
        {
            if (s[i] == '_')
                start_push = false;
            else
                var.push_back(s[i]);
        }
    }

    return (var);
}

string subtractChar(string s1, string s2)
{
    string r("");
    int len = s1.length();

    for (int i = 0; i < len; i++)
    {
        if (s1[i] != s2[0])
            r.push_back(s1[i]);
    }

    return (r);
}

bool isAlpha(string s)
{
    int l = s.length();

    for (int i = 0; i < l; i++)
    {
        if (!isalpha(s[i]))
            return (false);
    }

    return (true);
}

bool oneDot(string s)
{
    bool found = false;

    int l = s.length();

    for (int i = 0; i < l; i++)
    {
        if (s[i] == '.')
        {
            if (found)
                return (false);
            else
                found = true;
        }
    }

    return (true);
}

bool endsWith(string fullString, string ending)
{
    unsigned int lastMatchPos = fullString.rfind(ending);
    bool isEnding = lastMatchPos != std::string::npos;

    int el = ending.length(), fl = fullString.length();

    for (int i = lastMatchPos + el; (i < fl) && isEnding; i++)
    {
        if ((fullString[i] != '\n') && (fullString[i] != '\r'))
            isEnding = false;
    }

    return (isEnding);
}

bool startsWith(string s1, string s2)
{
    if (s1.length() > s2.length())
    {
        int s2l = s2.length();

        for (int i = 0; i < s2l; i++)
        {
            if (s1[i] != s2[i])
                return (false);
        }

        return (true);
    }
    else
        return (false);

    return (false);
}

bool zeroDots(string s)
{
    bool none = true;
    int l = s.length();

    for (int i = 0; i < l; i++)
    {
        if (s[i] == '.')
            none = false;
    }

    return (none);
}

bool zeroNumbers(string s)
{
    int l = s.length();

    for (int i = 0; i < l; i++)
    {
        switch (s[i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;

        default:
            return (true);
            break;
        }
    }

    return (false);
}

string subtractString(string s1, string s2)
{
    string bs("");

    int l1(s1.length()), l2(s2.length());

    for (int k = 0; k < l1; k++)
    {
        string tmp("");

        for (int z = k; z < l2 + k; z++)
            tmp.push_back(s1[z]);

        if (tmp == s2)
        {
            for (int q = 0; q < k; q++)
                bs.push_back(s1[q]);
            for (int y = k + l2; y < l1; y++)
                bs.push_back(s1[y]);
        }
    }

    return (bs);
}

// STRINTEGER TESTS
bool is_num_char(char c, bool &hFound, bool &pFound)
{
    switch (c)
    {
    case '-':
        if (hFound)
            return (false);
        else
            hFound = true;
        break;
    case '.':
        if (pFound)
            return (false);
        else
            pFound = true;
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return (true);
        break;

    default:
        return (false);
        break;
    }

    return (false);
}
bool is_num(string s)
{
    int l = s.length();
    for (int i = 0; i < l; i++)
    {
        if (!isdigit(s[i]))
            return (false);
    }
    return (true);
}
bool isNumeric(string s)
{
    int l = s.length();
    bool pFound = false, hFound = false;

    for (int i = 0; i < l; i++)
    {
        switch (s[i])
        {
        case '.':
            if (i == 0)
                return (false);
            else
            {
                if (!pFound)
                    pFound = true;
                else
                    return (false);
            }
            break;

        case '-':
            if (i != 0)
                return (false);
            else
            {
                if (!hFound)
                    hFound = true;
                else
                    return (false);
            }
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;

        default:
            return (false);
            break;
        }
    }

    return (true);
}

bool invalid_var_id(string s)
{
    if (s.length() > 1)
    {
        string c("");
        c.push_back(s[1]);

        if (s[0] != '$')
            return (true);

        if (isNumeric(c))
            return (true);
    }

    return (false);
}

// NUMBER > STRING & VICE-VERSA

int stoi(string s)
{
    stringstream ss(s);
    int i;
    ss >> i;
    return (i);
}

string itos(int i)
{
    stringstream ss;
    ss << i;
    return (ss.str());
}

double stod(string s)
{
    stringstream ss(s);
    double i;
    ss >> i;
    return (i);
}

string dtos(double i)
{
    stringstream ss;
    ss << i;
    return (ss.str());
}

int get_ascii_num(char c)
{
    return ((int)c);
}

int get_alpha_num(char c)
{
    if (tolower(c) == 'a')
        return (1);
    else if (tolower(c) == 'b')
        return (2);
    else if (tolower(c) == 'c')
        return (3);
    else if (tolower(c) == 'd')
        return (4);
    else if (tolower(c) == 'e')
        return (5);
    else if (tolower(c) == 'f')
        return (6);
    else if (tolower(c) == 'g')
        return (7);
    else if (tolower(c) == 'h')
        return (8);
    else if (tolower(c) == 'i')
        return (9);
    else if (tolower(c) == 'j')
        return (10);
    else if (tolower(c) == 'k')
        return (11);
    else if (tolower(c) == 'l')
        return (12);
    else if (tolower(c) == 'm')
        return (13);
    else if (tolower(c) == 'n')
        return (14);
    else if (tolower(c) == 'o')
        return (15);
    else if (tolower(c) == 'p')
        return (16);
    else if (tolower(c) == 'q')
        return (17);
    else if (tolower(c) == 'r')
        return (18);
    else if (tolower(c) == 's')
        return (19);
    else if (tolower(c) == 't')
        return (20);
    else if (tolower(c) == 'u')
        return (21);
    else if (tolower(c) == 'v')
        return (22);
    else if (tolower(c) == 'w')
        return (23);
    else if (tolower(c) == 'x')
        return (24);
    else if (tolower(c) == 'y')
        return (25);
    else if (tolower(c) == 'z')
        return (26);
    else
        return (0);
}

// ENVIRONMENTAL

string getEnvironmentVariable(string s)
{
    char * cString;
    cString = getenv(s.c_str());

    if (cString != NULL)
        return (string(cString));
    else
        return ("[not_available]");
}

void p(string s)
{
    cout << s << endl;
}

void comprehensiveHelp(string keyword)
{
    // regular operators
    if (keyword == "+=")
    {
        p("The \"+=\" operator:\tincrements a numeric value,\r\n\t\t\tappends to a string value,\r\n\t\t\tappends an item to a list.");
        p("");
        p("Example:\r\n\r\n\t@str = \"string of words...\"\r\n\t@str += \" and another...\"");
        p("\r\n\t@num = 65535\r\n\t@num += 6.5535");
        p("\r\n\tlist @array\r\n\t@array += @str\r\n\t@array += @num\r\n");
    }
    else if (keyword == "-=")
    {
        p("The \"-=\" operator:\tdecrements a numeric value,\r\n\t\t\tsubtracts a string value,\r\n\t\t\tsubtracts an item from a list.");
        p("");
        p("Example:\r\n\r\n\t@str = \"string of words...\"\r\n\t@str -= \" of words...\"");
        p("\r\n\t@num = (65535-6.5535)");
        p("\r\n\tlist @array\r\n\t@array += @str\r\n\t@array += @num\r\n\t@array += @str\r\n\t@array -= @str\r\n");
    }
    else if (keyword == "*=")
    {
        p("The \"*=\" operator:\tmultiplies a numeric value.");
        p("");
        p("Example:\r\n\r\n\t@num = 65535\r\n\t@num *= 6.5535\r\n");
    }
    else if (keyword == "**=")
    {
        p("The \"**=\" operator:\tassigns a power of a value.");
        p("");
        p("Example:\r\n\r\n\t@num = 2.56\r\n\t@num **= 5\r\n");
    }
    else if (keyword == "/=")
    {
        p("The \"/=\" operator:\tdivides a numeric value.");
        p("");
        p("Example:\r\n\r\n\t@num = 65535\r\n\t@num /= 6.5535\r\n");
    }
    else if (keyword == "+")
    {
        p("The \"+\" operator:\tadds numeric values,\r\n\t\t\tconcatenates strings.");
        p("");
        p("Example:\r\n\r\n\t65535 + 65\r\n\t\"Hello, \" + \"World!\"");
        p("\r\n\t@addition = (256+254)\r\n");
    }
    else if (keyword == "-")
    {
        p("The \"-\" operator:\tsubtracts numeric values,\r\n\t\t\tsubtracts strings.");
        p("");
        p("Example:\r\n\r\n\t65535 - 65\r\n\t\"Hello, \" - \"o, \"");
        p("\r\n\t@subtraction = (256-254)\r\n");
    }
    else if (keyword == "*")
    {
        p("The \"*\" operator:\tmultiplies numeric values,\r\n\t\t\tmultiplies strings.");
        p("");
        p("Example:\r\n\r\n\t65535 * 65\r\n\t\"Hello, World! \" * 3");
        p("\r\n\t@multiplication = (256*4)\r\n");
    }
    else if (keyword == "**")
    {
        p("The \"**\" operator:\tmultiplies the power of a value.");
        p("");
        p("Example:\r\n\r\n\t2.56 ** 2\r\n");
    }
    else if (keyword == "/")
    {
        p("The \"/\" operator:\tdivides numeric values.");
        p("");
        p("Example:\r\n\r\n\t65535 / 65");
        p("\r\n\t@division = (256/4)\r\n");
    }
    else if (keyword == "%")
    {
        p("The \"%\" operator:\tcomputes the remainder of two numeric values.");
        p("");
        p("Example:\r\n\r\n\t11 % 3");
        p("\r\n\t@divisible_by_four = (256%4)\r\n");
    }
    else if (keyword == "^")
    {
        p("The \"^\" operator:\tcomputes the power of a value.");
        p("");
        p("Example:\r\n\t@four_squared = (4^2)\r\n");
    }
    else if (keyword == "<")
    {
        p("The \"<\" operator:\ttests if a value is less than another,\r\n\t\t\tsets an incremental loop.");
        p("");
        p("Example:\r\n\r\n\tif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif");
        p("\r\n\tfor 1 < 5 (i)\r\n\t\tsay \"Iteration: ${i}\"\r\n\tendfor\r\n");
    }
    else if (keyword == "<=")
    {
        p("The \"<=\" operator:\ttests if a value is less than or equal to another.");
        p("");
        p("Example:\r\n\r\n\tif 1 <= 2\r\n\t\tsay \"One is less than or equal to two.\"\r\n\tendif");
        p("\r\n\tfor 1 <= 5 (i)\r\n\t\tsay \"Iteration: ${i}\"\r\n\tendfor\r\n");
    }
    else if (keyword == ">")
    {
        p("The \">\" operator:\ttests if a value is greater than another,\r\n\t\t\tsets a decremental loop.");
        p("");
        p("Example:\r\n\r\n\tif 2 > 1\r\n\t\tsay \"Two is greater than one.\"\r\n\tendif");
        p("\r\n\tfor 10 > 1 (i)\r\n\t\tout \"${i}, \"\r\n\tendfor\r\n\tsay \"blast off!\"\r\n");
    }
    else if (keyword == ">=")
    {
        p("The \">=\" operator:\ttests if a value is greater than or equal to another.");
        p("");
        p("Example:\r\n\r\n\tif 2 >= 1\r\n\t\tsay \"Two is greater than or equal to one.\"\r\n\tendif");
        p("\r\n\tfor 5 >= 1 (i)\r\n\t\tsay \"Iteration: ${i}\"\r\n\tendfor\r\n");
    }
    else if (keyword == "=")
    {
        p("The \"<\" operator:\tinitializes variables,\r\n\t\t\tdenotes inheritance.");
        p("");
        p("Example:");
        p("\r\n\t@var = \"A value.\"\r\n\r\n\tobject o\r\n\t\tmethod m\r\n\t\t\tsay \"Hello, World!\"\r\n\t\tend\r\n\tend");
        p("\r\n\tobject oo = o;end\r\n");
    }
    else if (keyword == "==")
    {
        p("The \"<\" operator:\ttests if two values are equal.");
        p("");
        p("Example:\r\n\r\n\tif 1 == 1\r\n\t\tsay \"One is equal to one.\"\r\n\tendif");
        p("\r\n");
    }
    else if (keyword == "!")
    {
        p("The \"!\" operator:\tdirectly parses an argument.");
        p("");
        p("Example:");
        p("\r\n\t! \"method m;say \'Hello, World!\';end;m;remove m\"\r\n");
    }
    else if (keyword == "!=")
    {
        p("The \"!\" operator:\ttests if two values are unequal.");
        p("");
        p("Example:\r\n\r\n\tif 1 != 2\r\n\t\tsay \"One is unequal to two.\"\r\n\tendif\r\n");
    }
    // special operators
    else if (keyword == "begins_with")
    {
        p("The \"begins_with\" operator:\ttests if a string begins with another.");
        p("");
        p("Example:\r\n\r\n\tif \"Hello, World!\" begins_with \"Hello\"\r\n\t\tsay \"\'Hello, World!\' begins with \'Hello\'.\"\r\n\tendif");
        p("");
    }
    else if (keyword == "contains")
    {
        p("The \"contains\" operator:\ttests if a string contains another.");
        p("");
        p("Example:\r\n\r\n\tif \"Hello, World!\" contains \" \"\r\n\t\tsay \"\'Hello, World!\' contains \' \'.\"\r\n\tendif");
        p("");
    }
    else if (keyword == "ends_with")
    {
        p("The \"ends_with\" operator:\ttests if a string ends with another.");
        p("");
        p("Example:\r\n\r\n\tif \"Hello, World!\" ends_with \"World!\"\r\n\t\tsay \"\'Hello, World!\' ends with \'World!\'.\"\r\n\tendif");
        p("");
    }
    else if (keyword == "++=")
    {
        p("The \"++=\" operator:\tincrements the ascii value of every character in a string.");
        p("");
        p("Example:\r\n\r\n\t@var = \"Dinosaur\"\r\n\t@var ++= 2\r\n");
        p("");
    }
    else if (keyword == "--=")
    {
        p("The \"--=\" operator:\tdecrements the ascii value of every character in a string.");
        p("");
        p("Example:\r\n\r\n\t@var = \"Fkpqucwt\"\r\n\t@var --= 2\r\n");
        p("");
    }
    else if (keyword == "if")
    {
        p("The \"if\" statement:\texecutes a block if a condition is true.");
        p("");
        p("Example:\r\n\r\n\tif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n");
    }
    else if (keyword == "unless")
    {
        p("The \"unless\" statement:\texecutes a block if a condition is false.");
        p("");
        p("Example:\r\n\r\n\tunless 1 > 2\r\n\t\tsay \"One is not less than two.\"\r\n\tendif\r\n");
    }
    else if (keyword == "orif")
    {
        p("The \"orif\" statement:\texecutes a block if a condition is true and previous statements failed.");
        p("");
        p("Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"");
        p("\torif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n");
    }
    else if (keyword == "elif")
    {
        p("The \"elif\" statement:\texecutes a block if a condition is true and previous statements failed.");
        p("");
        p("Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"");
        p("\telif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n");
    }
    else if (keyword == "elsif")
    {
        p("The \"elsif\" statement:\texecutes a block if a condition is true and previous statements failed.");
        p("");
        p("Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"");
        p("\telsif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n");
    }
    else if (keyword == "else")
    {
        p("The \"orif\" statement:\texecutes a block if a condition is true and previous statements failed.");
        p("");
        p("Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"");
        p("\torif 1 < 0\r\n\t\tsay \"Failed statement.\"");
        p("\telse\r\n\t\tsay \"Neither statement passed.\"\r\n\tendif\r\n");
    }
    else if (keyword == "failif")
    {
        p("The \"orif\" statement:\texecutes a block if a condition is true and previous statements failed.");
        p("");
        p("Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"");
        p("\torif 1 < 0\r\n\t\tsay \"Failed statement.\"");
        p("\tfailif\r\n\t\tsay \"Neither statement passed.\"\r\n\tendif\r\n");
    }
    else if (keyword == "endif")
    {
        p("The \"endif\" statement:\tends all conditional statements.");
        p("");
        p("Example:\r\n\r\n\tif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n");
    }
    else if (keyword == "for")
    {
        p("The \"for\" loop:\titerates by numerical setting.");
        p("");
        p("Example:\r\n\r\n\tfor infinity\r\n\t\tsay \"This will repeat infinitely\"\r\n\tendfor");
        p("\r\n\tfor 1 < 5\r\n\t\tsay \"This will repeat 5 times\"\r\n\tendfor");
        p("\r\n\tfor 5 > 1\r\n\t\tsay \"This will repeat 5 times\"\r\n\tendfor");
        p("\r\n\tfor 1 < 5 (example)\r\n\t\tsay \"Current Iteration: ${example}\"\r\n\tendfor");
        p("\r\n\tfor i in my_list\r\n\t\tsay \"Current Element: ${i}\"\r\n\tendfor");
        p("\r\n\tfor i in (1..10)\r\n\t\tsay \"Current Iteration: ${i}\"\r\n\tendfor\r\n");
    }
    else if (keyword == "loop")
    {
        p("The \"loop\" loop:\titerates by enumerated setting.");
        p("");
        p("Example:\r\n\r\n\tlist @array;@array += \"abc\";@array += \"cba\"\r\n\tloop @array (i)\r\n\t\tsay \"${i}\"\r\n\tendfor");
        p("\r\n\t@dir = env.cwd\r\n\tloop @dir.read_files (f)\r\n\t\tsay \"${f}\"\r\n\tendfor");
        p("\r\n\t@dir = env.cwd\r\n\tloop @dir.read_dirs (d)\r\n\t\tsay \"${d}\"\r\n\tendfor");
        p("\r\n\t@file = \"example.txt\"\r\n\tloop @file.read (line)\r\n\t\tsay \"${line}\"\r\n\tendfor");
        p("\r\n\tloop @file.read (line)\r\n\t\tsay \"${line}\"\r\n\tendfor\r\n");
    }
    else if (keyword == "leave!")
    {
        p("The \"leave!\" keyword:\tbreaks an iteration.");
        p("");
        p("Example:\r\n\r\n\tfor 1 < 5 (i)\r\n\t\tif \"${i}\" = 3\r\n\t\t\tleave!\r\n\t\tendif\r\n\tendfor\r\n");
    }
    else if (keyword == "endfor")
    {
        p("The \"for\" loop:\titerates by numerical setting.");
        p("");
        p("Example:\r\n\r\n\tfor 1 < 5\r\n\t\tsay \"This will repeat 5 times\"\r\n\tendfor\r\n");
    }
    // Special Methods and Symbols
    // Memory Management
    else if (keyword == "clear_all!")
    {
        p("The \"clear_all!\" command:\tremoves all USL objects from memory.");
        p("");
    }
    else if (keyword == "clear_lists!")
    {
        p("The \"clear_lists!\" command:\tremoves all lists from memory.");
        p("");
    }
    else if (keyword == "clear_meths!")
    {
        p("The \"clear_meths!\" command:\tremoves all methods from memory.");
        p("");
    }
    else if (keyword == "clear_objs!")
    {
        p("The \"clear_objs!\" command:\tremoves all objects from memory.");
        p("");
    }
    else if (keyword == "clear_vars!")
    {
        p("The \"clear_vars!\" command:\tremoves all variables from memory.");
        p("");
    }
    // Debug USL Objects
    else if (keyword == "is_list?")
    {
        p("The \"is_list?\" command:\ttests if a list exists.");
        p("");
        p("Example:\r\n\r\n\tlist @array\r\n\tis_list? @array\r\n");
    }
    else if (keyword == "is_method?")
    {
        p("The \"is_method?\" command:\ttests if a method exists.");
        p("");
        p("Example:\r\n\r\n\tmethod m;end\r\n\tis_method? m\r\n");
    }
    else if (keyword == "is_object?")
    {
        p("The \"is_object?\" command:\ttests if an object exists.");
        p("");
        p("Example:\r\n\r\n\tobject o;end\r\n\tis_object? o\r\n");
    }
    else if (keyword == "is_variable?")
    {
        p("The \"is_variable?\" command:\ttests if a variable exists.");
        p("");
        p("Example:\r\n\r\n\t@var = \"value\"\r\n\tis_variable? @var\r\n");
    }
    else if (keyword == "is_string?")
    {
        p("The \"is_string?\" command:\ttests if a variable is a string.");
        p("");
        p("Example:\r\n\r\n\t@var = \"value\"\r\n\tis_string? @var\r\n");
    }
    else if (keyword == "is_number?")
    {
        p("The \"is_number?\" command:\ttests if a variable is a number.");
        p("");
        p("Example:\r\n\r\n\t@var = 65535\r\n\tis_number? @var\r\n");
    }
    else if (keyword == "no_lists?")
    {
        p("The \"no_lists?\" command:\ttests the existence of list definitions.");
        p("");
    }
    else if (keyword == "no_methods?")
    {
        p("The \"no_methods?\" command:\ttests the existence of method definitions.");
        p("");
    }
    else if (keyword == "no_objects?")
    {
        p("The \"no_objects?\" command:\ttests the existence of object definitions.");
        p("");
    }
    else if (keyword == "no_variables?")
    {
        p("The \"no_variables?\" command:\ttests the existence of variable definitions.");
        p("");
    }
    else if (keyword == "see")
    {
        p("The \"see\" command:\tdisplays USL object definitions,\r\n\t\t\tdisplays defined USL objects.");
        p("");
        p("Example:");
        p("");
        p("\tsee lists");
        p("\tsee methods");
        p("\tsee objects");
        p("\tsee variables");
        p("");
        p("\tobject o;end");
        p("\tsee o\r\n");
        p("\tmethod m;end");
        p("\tsee m\r\n");
        p("\tlist @l");
        p("\tsee l\r\n");
        p("\t@v = \"value\"");
        p("\tsee @v\r\n");
    }
    // File/Directory Existence
    else if (keyword == "is_dir?")
    {
        p("The \"is_dir?\" command:\ttests if a string or variable is a directory.");
        p("");
        p("Example:");
        p("");
        p("\t@cwd = env.cwd\r\n\tif @cwd = is_dir?\r\n\t\tsay \"The directory exists.\"\r\n\tendif\r\n");
    }
    else if (keyword == "is_file?")
    {
        p("The \"is_file?\" command:\ttests if a string or variable is a file.");
        p("");
        p("Example:");
        p("");
        p("\t@file = \"/Windows/System32/cmd.exe\"\r\n\tif @file = is_file?\r\n\t\tsay \"The file exists.\"\r\n\tendif\r\n");
    }
    // Defining Memory
    else if (keyword == "__begin__")
    {
        p("The \"__begin__\" command:\tbegins a script definition.");
        p("");
        p("Example:");
        p("");
        p("\t__begin__ \"script.us\"\r\n\tsay \"Hello, World!\"\r\n\t__end__\r\n");
    }
    else if (keyword == "__end__")
    {
        p("The \"__end__\" command:\tends a script definition.");
        p("");
        p("Example:");
        p("");
        p("\t__begin__ \"script.us\"\r\n\tsay \"Hello, World!\"\r\n\t__end__\r\n");
    }
    else if (keyword == "list")
    {
        p("The \"list\" keyword:\tdeclares a list definition.");
        p("");
        p("Example:");
        p("");
        p("\tlist @array\r\n\t@array += \"first element\"\r\n\t@array += \"last element\"\r\n");
    }
    else if (keyword == "[method]")
    {
        p("The \"[method]\" keyword:\tdeclares an indestructible method definition.");
        p("");
        p("The only way to remove an indestructible method is to use the \"remove\" command.");
        p("\r\nTo make a method destructible, use the \"unlock\" command.");
        p("");
        p("Example:");
        p("");
        p("\t[method] indestructible_Method\r\n\t\tsay \"Hello. I am indestructible.\"\r\n\tend\r\n");
    }
    else if (keyword == "method")
    {
        p("The \"method\" keyword:\tdeclares a method definition.");
        p("");
        p("Example:");
        p("");
        p("\tmethod m\r\n\t\tsay \"Hello, World!\"\r\n\tend\r\n");
    }
    else if (keyword == "template")
    {
        p("The \"template\" keyword:\tdeclares a template definition.");
        p("");
        p("Example:");
        p("");
        p("\ttemplate \"foo(b, a, r)\"\r\n\t\t@var = $0\r\n\t\t@var -= $1\r\n\t\t@var += $2\r\n\t\tsay @var\r\n\tend\r\n");
    }
    else if (keyword == "object")
    {
        p("The \"object\" keyword:\tdeclares an object definition.");
        p("");
        p("Example:");
        p("");
        p("\tobject o\r\n\t\tmethod m\r\n\t\t\tsay \"Hello, World!\"\r\n\t\tend\r\n\tend\r\n");
    }
    else if (keyword == "lock")
    {
        p("The \"lock\" keyword:\tdeclares a method or variable as \"indestructible.\"");
        p("\r\nUse the \"remove\" command to remove an indestructible method or variable.");
        p("\r\nTo make a method or variable destructible, use the \"unlock\" command.");
        p("");
        p("Example:");
        p("");
        p("\tmethod indestructible_method\r\n\t\tsay \"I will be indestructible.\"\r\n\tend\r\n\r\n\tlock indestructible_method\r\n");
    }
    else if (keyword == "unlock")
    {
        p("The \"unlock\" keyword:\tdeclares a method or variable as \"destructible.\"");
        p("\r\nTo make a method or variable indestructible, use the \"lock\" command.");
        p("");
        p("Example:");
        p("");
        p("\t[method] indestructible_method\r\n\t\tsay \"I will be indestructible.\"\r\n\tend\r\n\r\n\tunlock indestructible_method\r\n");
    }
    else if (keyword == "public")
    {
        p("The \"public\" keyword:\tdeclares a public object definition.");
        p("");
        p("Example:");
        p("");
        p("\tobject o\r\n\t\tpublic\r\n\t\t\tmethod m\r\n\t\t\t\tsay \"Hello, World!\"\r\n\t\t\tend\r\n\tend\r\n");
    }
    else if (keyword == "private")
    {
        p("The \"private\" keyword:\tdeclares a private object definition.");
        p("");
        p("Example:");
        p("");
        p("\tobject o\r\n\t\tprivate\r\n\t\t\tmethod m\r\n\t\t\t\tsay \"Hello, World!\"\r\n\t\t\tend\r\n\tend\r\n");
    }
    else if (keyword == "@")
    {
        p("The \"@\" symbol:\tdenotes a variable declaration.");
        p("");
        p("Example:");
        p("");
        p("\t@var = \"Hello, World!\"\r\n");
    }
    // Time
    else if (keyword == "am_or_pm")
    {
        p("The \"am_or_pm\" initializer:\tsets a variable to before noon or after.");
        p("");
        p("Example:");
        p("");
        p("\t@am_or_pm = am_or_pm\r\n");
    }
    else if (keyword == "day_of_this_week")
    {
        p("The \"day_of_this_week\" initializer:\tsets a variable to the day of this week.");
        p("");
        p("Example:");
        p("");
        p("\t@day_of_this_week = day_of_this_week\r\n");
    }
    else if (keyword == "day_of_this_month")
    {
        p("The \"day_of_this_month\" initializer:\tsets a variable to the day of this month.");
        p("");
        p("Example:");
        p("");
        p("\t@day_of_this_month = day_of_this_month\r\n");
    }
    else if (keyword == "day_of_this_year")
    {
        p("The \"day_of_this_year\" initializer:\tsets a variable to the day of this year.");
        p("");
        p("Example:");
        p("");
        p("\t@day_of_this_year = day_of_this_year\r\n");
    }
    else if (keyword == "month_of_this_year")
    {
        p("The \"month_of_this_year\" initializer:\tsets a variable to the month of this year.");
        p("");
        p("Example:");
        p("");
        p("\t@month_of_this_year = month_of_this_year\r\n");
    }
    else if (keyword == "this_second")
    {
        p("The \"this_second\" initializer:\tsets a variable to the current second.");
        p("");
        p("Example:");
        p("");
        p("\t@this_second = this_second\r\n");
    }
    else if (keyword == "this_minute")
    {
        p("The \"this_minute\" initializer:\tsets a variable to the current minute.");
        p("");
        p("Example:");
        p("");
        p("\t@this_minute = this_minute\r\n");
    }
    else if (keyword == "this_hour")
    {
        p("The \"this_hour\" initializer:\tsets a variable to the current hour.");
        p("");
        p("Example:");
        p("");
        p("\t@this_hour = this_hour\r\n");
    }
    else if (keyword == "this_month")
    {
        p("The \"this_month\" initializer:\tsets a variable to the current month.");
        p("");
        p("Example:");
        p("");
        p("\t@this_month = this_month\r\n");
    }
    else if (keyword == "this_year")
    {
        p("The \"this_year\" initializer:\tsets a variable to the current year.");
        p("");
        p("Example:");
        p("");
        p("\t@this_year = this_year\r\n");
    }
    // Extras
    else if (keyword == "\\\'")
    {
        p("The \"\\\'\" symbol:\tis parsed as an apostrophe mark.");
        p("");
        p("Example:");
        p("");
        p("\tsay \"\\\'Hello, World!\\\'\"\r\n");
    }
    else if (keyword == "\'")
    {
        p("The \"\'\" symbol:\tis parsed as a quotation mark.");
        p("");
        p("Example:");
        p("");
        p("\tsay \"\'Hello, World!\'\"\r\n");
    }
    else if (keyword == "\\t")
    {
        p("The \"\\t\" symbol:\tis parsed as a tab sequence.");
        p("");
        p("Example:");
        p("");
        p("\tsay \"\\tHello, World!\"\r\n");
    }
    else if (keyword == "\\n")
    {
        p("The \"\\n\" symbol:\tis parsed as a CRLF sequence.");
        p("");
        p("Example:");
        p("");
        p("\tsay \"\\nHello, World!\\n\"\r\n");
    }
    else if (keyword == "#")
    {
        p("The \"#\" symbol:\tdenotes a comment.");
        p("");
        p("Example:");
        p("");
        p("\t# This is a comment\r\n");
    }
    else if (keyword == "##")
    {
        p("The \"##\" symbol:\tdenotes multiline commentation.");
        p("");
        p("Example:");
        p("");
        p("\t##\r\n\tThis is a multiline comment\r\n\t##\r\n");
    }
    else if (keyword == "?")
    {
        p("The \"?\" symbol:\texecutes an external command,");
        p("\t\t\tinitializes a variable with the stdout of an external command.");
        p("");
        p("Example:");
        p("");
        p("\t? cmd");
        p("\t@stdout ? \"ping example.com\"\r\n");
    }
    else if (keyword == ";")
    {
        p("The \";\" symbol:\tseparates code lines.");
        p("");
        p("Example:");
        p("");
        p("\tmethod m;say \"Hello, World!\";end;m;remove m\r\n");
    }
    else if (keyword == "append")
    {
        p("The \"append\" command:\tappends text to a file.");
        p("");
        p("Example:");
        p("");
        p("\tappend \"example.txt\" \"This is some text.\"\r\n");
    }
    else if (keyword == "appendl")
    {
        p("The \"appendl\" command:\tappends a newline of text to a file.");
        p("");
        p("Example:");
        p("");
        p("\tappendl \"example.txt\" \"This is some text.\"\r\n");
    }
    else if (keyword == "args")
    {
        p("The \"args\" keyword:\tgets the parameters of a loaded script.");
        p("");
        p("Example:");
        p("");
        p("\t@script = args[0]\r\n");
    }
    else if (keyword == "args.size")
    {
        p("The \"args.size\" keyword:\tgets the parameter count of a loaded script.");
        p("");
        p("Example:");
        p("");
        p("\t@argc = args.size\r\n");
    }
    else if (keyword == "cd")
    {
        p("The \"cd\" keyword:\tchanges the current directory.");
        p("");
        p("Example:");
        p("");
        p("\tcd /Windows/System32\r\n");
    }
    else if (keyword == "chomp")
    {
        p("The \"chomp\" keyword:\tbegins a loud stdin stream.");
        p("");
        p("Example:");
        p("");
        p("\t@fullname = \"chomp.Enter your fullname:\"\r\n");
    }
    else if (keyword == "delay")
    {
        p("The \"delay\" command:\tdelays program flow for a set number of seconds.");
        p("");
        p("Example:");
        p("");
        p("\tsay \"Delaying for 5 seconds...\"\r\n\tdelay 5\r\n");
    }
    else if (keyword == "dpush")
    {
        p("The \"dpush\" command:\tcreates a directory.");
        p("");
        p("Example:");
        p("");
        p("\tdpush \"directory name\"\r\n");
    }
    else if (keyword == "dpop")
    {
        p("The \"dpop\" command:\tremoves a directory.");
        p("");
        p("Example:");
        p("");
        p("\tdpop \"directory name\"\r\n");
    }
    else if (keyword == "env")
    {
        p("The \"env\" keyword:\tgets the value of an environmental variable.");
        p("");
        p("Example:");
        p("");
        p("# USL Specific");
        p("\t@cwd = env.cwd");
        p("\t@usl = env.usl");
        p("\t@user = env.user");
        p("\t@machine = env.machine");
        p("\t@os = env.os");
        p("# OS Specific");
        p("\t@windir = env.windir");
        p("\t@prompt = env.prompt");
        p("\t@sysdrive = env.systemdrive\r\n");
    }
    else if (keyword == "error")
    {
        p("The \"error\" command:\tprints to the stderr stream.");
        p("");
        p("Example:");
        p("");
        p("\terror \"An error occurred.\"\r\n");
    }
    else if (keyword == "forget")
    {
        p("The \"forget\" command:\tforgets a stored variable definition.");
        p("");
        p("Example:");
        p("");
        p("\tforget @var\r\n");
    }
    else if (keyword == "fpush")
    {
        p("The \"fpush\" command:\tcreates a file.");
        p("");
        p("Example:");
        p("");
        p("\tfpush \"file name\"\r\n");
    }
    else if (keyword == "fpop")
    {
        p("The \"fpop\" command:\tremoves a file.");
        p("");
        p("Example:");
        p("");
        p("\tfpop \"file name\"\r\n");
    }
    else if (keyword == "in_dir")
    {
        p("The \"in_dir\" keyword:\tgets/sets the USL initial directory.");
        p("");
        p("Example:");
        p("");
        p("\tin_dir /Windows/System32");
        p("\tcd \"/Program Files\"");
        p("\tcd in_dir\r\n");
    }
    else if (keyword == "load")
    {
        p("The \"load\" command:\tincludes and executes a script.");
        p("");
        p("Example:");
        p("");
        p("\tload \"script name.us\"\r\n");
    }
    else if (keyword == "parser")
    {
        p("The \"parser\" command:\tbegins a separate USL session.");
        p("");
    }
    else if (keyword == "prompt")
    {
        p("The \"prompt\" command:\tsets the USL shell prompt.");
        p("");
        p("Example:");
        p("");
        p("\tprompt bash");
        p("\tprompt !");
        p("\tprompt \"[\\u@\\m(\\w)]$ \"");
        p("\tprompt !");
        p("\tprompt !\r\n");
    }
    else if (keyword == "random")
    {
        p("The \"random\" keyword:\tgets a random character or number.");
        p("");
        p("Example:");
        p("");
        p("\t@rand_num = random.1_12345");
        p("\t@rand_char = random.a_z\r\n");
    }
    else if (keyword == "redefine")
    {
        p("The \"redefine\" command:\trenames a USL object name.");
        p("");
        p("Example:");
        p("");
        p("\tobject o;method m;say \"Hello, World!\";end;end");
        p("\tredefine o oo\r\n\too.m\r\n");
    }
    else if (keyword == "remember")
    {
        p("The \"remember\" command:\tstores a variable definition for future usage.");
        p("");
        p("Example:");
        p("");
        p("\t@var = \"a value\"\r\n\tremember @var\r\n");
    }
    else if (keyword == "remove")
    {
        p("The \"remove\" command:\tremoves a USL object definition from memory.");
        p("");
        p("Example:");
        p("");
        p("\tobject o;method m;say \"Hello, World!\";end;end");
        p("\tremove o\r\n");
    }
    else if (keyword == "say")
    {
        p("The \"say\" command:\tprints a newline of text to the stdout stream.");
        p("");
        p("Example:");
        p("");
        p("\t@words = \"This is a sentence.\"");
        p("\tsay \"This sentence is ended with a newline.\\nSentence: \\{@words}\"\r\n");
    }
    else if (keyword == "shomp")
    {
        p("The \"shomp\" keyword:\tbegins a silent stdin stream.");
        p("");
        p("Example:");
        p("");
        p("\t@passwd = \"shomp.Enter your password:\"\r\n");
    }
    else if (keyword == "stdout")
    {
        p("The \"stdout\" command:\tprints text to the stdout stream.");
        p("");
        p("Example:");
        p("");
        p("\tstdout \"This sentence is not ended with a newline.\"\r\n");
    }
    // Newly added
    else if (keyword == "bytes")
    {
        p("The \"bytes\" keyword:\tgets the size of a file in bytes.");
        p("");
        p("Example:");
        p("");
        p("\t@file = env.usl");
        p("\t@size = @file.bytes\r\n");
    }
    else if (keyword == "kbytes")
    {
        p("The \"kbytes\" keyword:\tgets the size of a file in kilobytes.");
        p("");
        p("Example:");
        p("");
        p("\t@file = env.usl");
        p("\t@size = @file.kbytes\r\n");
    }
    else if (keyword == "mbytes")
    {
        p("The \"mbytes\" keyword:\tgets the size of a file in megabytes.");
        p("");
        p("Example:");
        p("");
        p("\t@file = env.usl");
        p("\t@size = @file.mbytes\r\n");
    }
    else if (keyword == "gbytes")
    {
        p("The \"gbytes\" keyword:\tgets the size of a file in gigabytes.");
        p("");
        p("Example:");
        p("");
        p("\t@file = env.usl");
        p("\t@size = @file.gbytes\r\n");
    }
    else if (keyword == "tbytes")
    {
        p("The \"tbytes\" keyword:\tgets the size of a file in terabytes.");
        p("");
        p("Example:");
        p("");
        p("\t@file = env.usl");
        p("\t@size = @file.tbytes\r\n");
    }
    else if (keyword == "try")
    {
        p("The \"try\" keyword:\ttries to execute a block of code.");
        p("");
        p("Example:");
        p("");
        p("\ttry");
        p("\t\t@num = 123.456");
        p("\tcatch");
        p("\t\tsay \"An error occurred.\"");
        p("\tcaught\r\n");
    }
    else if (keyword == "catch")
    {
        p("The \"catch\" keyword:\texecutes a block of code if an error occurred in the \"try\" block.");
        p("");
        p("Example:");
        p("");
        p("\ttry");
        p("\t\t@num = 123.456");
        p("\t\t@num = abcdef");
        p("\tcatch");
        p("\t\tsay \"An error occurred.\"");
        p("\tcaught\r\n");
    }
    else if (keyword == "caught")
    {
        p("The \"caught\" keyword:\tends a \"try\" block.");
        p("");
        p("Example:");
        p("");
        p("\ttry");
        p("\t\t@num = 123.456");
        p("\tcatch");
        p("\t\tsay \"An error occurred.\"");
        p("\tcaught\r\n");
    }
    else if (keyword == "last_error")
    {
        p("The \"last_error\" symbol:\tcontains the error message of a failed \"try\" block.");
        p("");
        p("A variable created in the catch code will be removed.");
        p("");
        p("Example:");
        p("");
        p("\ttry");
        p("\t\t@num = 123.456");
        p("\t\t@num = abcdef");
        p("\tcatch");
        p("\t\t@e = last_error");
        p("\t\tout \"Error occurred: \"");
        p("\t\tsay @e");
        p("\tcaught\r\n");
    }
    else if (keyword == "while")
    {
        p("The \"while\" keyword:\tdeclares a while loop.");
        p("");
        p("Example:");
        p("");
        p("\t@a = 1");
        p("\t@b = 10");
        p("\twhile @a <= @b");
        p("\t\t@c += 1");
        p("\t\tsay @c");
        p("\t\t@a += 1");
        p("\tend\r\n");
    }
    else if (keyword == "end")
    {
        p("The \"end\" keyword:\tends a while loop.\r\n\t\t\tends a switch statement.\r\n\t\t\tends a method, object, or template.");
        p("");
        p("Example:");
        p("");
        p("\t@a = 1");
        p("\t@b = 10");
        p("\twhile @a <= @b");
        p("\t\t@c += 1");
        p("\t\tsay @c");
        p("\t\t@a += 1");
        p("\tend");
        p("");
        p("\tswitch @a");
        p("\t\tcase 12");
        p("\t\t\tsay \"Not quite 11.\"");
        p("\t\tcase 11");
        p("\t\t\tsay \"Match found!\"");
        p("\t\tdefault");
        p("\t\t\tsay \"No matches found.\"");
        p("\tend\r\n");
    }
    else if (keyword == "switch")
    {
        p("The \"switch\" keyword:\tbegins a switch statement.");
        p("");
        p("Example:");
        p("");
        p("\tmethod m");
        p("\t\t@a = \"This is a string.\"");
        p("\t\tswitch @a");
        p("\t\t\tcase \"This\"");
        p("\t\t\t\tsay \"Not quite...\"");
        p("\t\t\tcase \"This is a string.\"");
        p("\t\t\t\tsay \"Matched!\"");
        p("\t\t\tdefault");
        p("\t\t\t\tsay \"No match found...\"");
        p("\t\tend");
        p("\tend");
        p("");
        p("m\r\n");
    }
    else if (keyword == "case")
    {
        p("The \"case\" keyword:\tprovides a test value for a switch statement.");
        p("");
        p("Example:");
        p("");
        p("\tmethod m");
        p("\t\t@a = \"This is a string.\"");
        p("\t\tswitch @a");
        p("\t\t\tcase \"This\"");
        p("\t\t\t\tsay \"Not quite...\"");
        p("\t\t\tcase \"This is a string.\"");
        p("\t\t\t\tsay \"Matched!\"");
        p("\t\t\tdefault");
        p("\t\t\t\tsay \"No match found...\"");
        p("\t\tend");
        p("\tend");
        p("");
        p("m\r\n");
    }
    else if (keyword == "default")
    {
        p("The \"default\" keyword:\tbegins a code block if no match is found when switching.");
        p("");
        p("Example:");
        p("");
        p("\tmethod m");
        p("\t\t@a = \"This is a string.\"");
        p("\t\tswitch @a");
        p("\t\t\tcase \"This\"");
        p("\t\t\t\tsay \"Not quite...\"");
        p("\t\t\tcase 3.14");
        p("\t\t\t\tsay \"Matched!\"");
        p("\t\t\tdefault");
        p("\t\t\t\tsay \"No match found...\"");
        p("\t\tend");
        p("\tend");
        p("");
        p("m\r\n");
    }
    else if (keyword == "return")
    {
        p("The \"return\" keyword:\tsets the value to be returned by a method.");
        p("");
        p("Any variable returned will be removed from memory.");
        p("");
        p("Example:");
        p("");
        p("\tmethod m");
        p("\t\treturn \"Return Value\"");
        p("\tend");
        p("");
        p("\tmethod pi");
        p("\t\t@ret_val = 3.14");
        p("\t\treturn @ret_val");
        p("\tend");
        p("");
        p("\t@pi = pi");
        p("\t@value = m\r\n");
    }
    else if (keyword == "fwrite")
    {
        p("The \"fwrite\" command:\tcreates a file if it does not already exist and appends text to it.");
        p("");
        p("If the file exists and has been written to, fwrite returns 0.");
        p("If the file is created and has been written to, fwrite returns 1.");
        p("If the file could not be created, fwrite returns -1.");
        p("");
        p("Example:");
        p("");
        p("\tmethod fwrite(file,contents)");
        p("\t\tfwrite $0 $1");
        p("\tend");
        p("");
        p("\t@contents = \"This is a string of text that will be written to a file.\"");
        p("\t@ret_val = fwrite(\"some file.txt\",@contents)");
        p("");
    }
    else
    {
        p("The keyword, symbol, or command could not be found.");
        p("");
        p("Please use the \"help\" command to see a list of keywords, symbols, and commands.");
    }
}

void printUSLHelp()
{
    p("[Parameter Key]");
    p("\t{s}\t\t(string)");
    p("\t{n}\t\t(number)");
    p("\t{v}\t\t(variable)");
    p("\t{m}\t\t(method)");
    p("\t{o}\t\t(object)");
    p("\t{l}\t\t(list)");
    p("\t{all}\t\t(any USL object)");
    p("\t{expression}\t(logical expression)");
    p("");
    p("[Keywords and Symbols]");
    p("- Operators:");
    p("\t[Regular]");
    p("\t+=\t\t(increment || append string)");
    p("\t-=\t\t(decrement || remove string)");
    p("\t*=\t\t(multiply->increment || multiply string)");
    p("\t/=\t\t(divide->decrement)");
    p("\t**=\t\t(exponent)");
    p("\t+\t\t(addition)");
    p("\t-\t\t(subtraction)");
    p("\t*\t\t(multiplication)");
    p("\t/\t\t(division)");
    p("\t%\t\t(modulus)");
    p("\t**\t\t(exponent)");
    p("\t<\t\t(less than)");
    p("\t>\t\t(greater than)");
    p("\t<=\t\t(less than or equal to)");
    p("\t>=\t\t(greater than or equal to)");
    p("\t=\t\t(initializer)");
    p("\t==\t\t(equal)");
    p("\t!=\t\t(not equal)\r\n");
    p("\t[Special]");
    p("\tbegins_with\t(used in if expressions)");
    p("\tcontains\t(used in if expressions)");
    p("\tends_with\t(used in if expressions)");
    p("\t++=\t\t(increment ascii value)");
    p("\t--=\t\t(decrement ascii value)");
    p("");
    p("- Conditionals:");
    p("\tunless {expression}\t(unless statement)");
    p("\tif {expression}\t\t(if statement)");
    p("\torif {expression}\t(else if statement)");
    p("\telse\t\t\t(else statement)");
    p("\tfailif\t\t\t(if failure occurred)");
    p("\tendif\t\t\t(end if statement)");
    p("\tswitch {v}\t\t(switch statement)");
    p("\tcase {n|s}\t\t(switch case)");
    p("\tdefault\t\t\t(default case)");
    p("\tend\t\t\t(end switch statement)");
    p("\ttry\t\t\t(begin try statement)");
    p("\tcatch\t\t\t(begin catch code)");
    p("\tcaught\t\t\t(end try statement)");
    p("");
    p("\tfor infinity\t\t(infinite loop)");
    p("\tfor {n} < {n}\t\t(incremental loop)");
    p("\tfor {n} > {n}\t\t(decremental loop)");
    p("\twhile {v} {op} {n|v}\t(while loop)");
    p("\tloop {l}\t\t(list loop)");
    p("\tloop {v}.read\t\t(read lines in a file)");
    p("\tloop {v}.read_files\t(read files in a directory)");
    p("\tloop {v}.read_dirs\t(read directories in a directory)");
    p("\tleave!\t\t\t(break loop flow)");
    p("\tendfor\t\t\t(end for loop)");
    p("\tend\t\t\t(end while loop)");
    p("");
    p("- Special Methods and Symbols:");
    p("\t[Memory Management]");
    p("\tclear_all!\t\t(clear everything in memory)");
    p("\tclear_lists!\t\t(clear all lists in memory)");
    p("\tclear_methods!\t\t(clear all methods in memory)");
    p("\tclear_objects!\t\t(clear all objects in memory)");
    p("\tclear_variables!\t(clear all variables in memory)");
    p("\tclear_constants!\t(clear all constants in memory)");
    p("");
    p("\t[Debug USL Objects, Variables, and Methods]");
    p("\tis_list? {l}\t\t(is list defined?)");
    p("\tis_method? {m}\t\t(is method defined?)");
    p("\tis_object? {o}\t\t(is object defined?)");
    p("\tis_variable? {v}\t(is variable defined?)");
    p("\tis_number? {v}\t\t(is variable a number?)");
    p("\tis_string? {v}\t\t(is variable a string?)");
    p("\tno_lists?\t\t(do no lists exist?)");
    p("\tno_methods?\t\t(do no methods exist?)");
    p("\tno_objects?\t\t(do no objects exist?)");
    p("\tno_variables?\t\t(do no variables exist?)");
    p("\tsee {all}\t\t(see definition of declaration)");
    p("\tsee {option}\t\t(lists/methods/objects/variables)");
    p("");
    p("\t[File/Directory Existence]");
    p("\tis_dir? {v|s}\t\t(is variable/string a directory?)");
    p("\tis_file? {v|s}\t\t(is variable/string a file?)");
    p("");
    p("\t[File Size]");
    p("\t{v}.bytes\t\t(gets size of a file in bytes)");
    p("\t{v}.kbytes\t\t(gets size of a file in kilobytes)");
    p("\t{v}.mbytes\t\t(gets size of a file in megabytes)");
    p("\t{v}.gbytes\t\t(gets size of a file in gigabytes)");
    p("\t{v}.tbytes\t\t(gets size of a file in terabytes)");
    p("");
    p("\t[Defining Memory]");
    p("\t__begin__ {v|s}\t\t(define a script)");
    p("\t__end__\t\t\t(end script definition)");
    p("\tlist {l}\t\t(define a list)");
    p("\t[method] {m}\t\t(define an indestructible method)");
    p("\tmethod {m}\t\t(define a method)");
    p("\ttemplate {m}\t\t(define a template method)");
    p("\tobject {o}\t\t(define an object)");
    p("\tpublic\t\t\t(begin defining public members)");
    p("\tprivate\t\t\t(begin defining private members)");
    p("\tend\t\t\t(end method/object/template definition)");
    p("\t@\t\t\t(variables begin with '@')");
    p("");
    p("\t[Extras]");
    p("\t\\'\t\t\t(parsed as apostrophe mark)");
    p("\t'\t\t\t(parsed as quotation mark)");
    p("\t\\t\t\t\t(tab sequence)");
    p("\t\\n\t\t\t(carriage-return & line-feed)");
    p("\t#\t\t\t(denote commentation)");
    p("\t##\t\t\t(begin/end multiline commentation)");
    p("\t!\t\t\t(parse a string directly || initializer)");
    p("\t?\t\t\t(execute an external command || initializer)");
    p("\t;\t\t\t(line separator)");
    p("\tappend\t\t\t(append text to a file)");
    p("\tappendl\t\t\t(append a line to a file)");
    p("\targs[{n}]\t\t(parameter access)");
    p("\targs.size\t\t(parameter count)");
    p("\tcd\t\t\t(specialized \"chdir\")");
    p("\tchomp.{v|s}\t\t(loudly receive input)");
    p("\tdelay {n}\t\t(pause script flow in seconds)");
    p("\tdpush {v|s}\t\t(create a directory)");
    p("\tdpop {v|s}\t\t(delete a directory)");
    p("\tenv.{s}\t\t\t(get environment variable)");
    p("\terror {v|s}\t\t(standard error stream)");
    p("\tforget {v}\t\t(forget variable definition)");
    p("\tfpush {v|s}\t\t(create a file)");
    p("\tfpop {v|s}\t\t(delete a file)");
    p("\tfwrite\t\t\t(use \"help fwrite\")");
    p("\thelp {keyword}\t\t(comprehensive help)");
    p("\tin_dir\t\t\t(USL Initial Directory)");
    p("\tload {us}\t\t(include and execute script definitions)");
    p("\tlock {v|m}\t\t(make method or variable indestructible)");
    p("\tparser\t\t\t(begin parsing input)");
    p("\tprompt {s}\t\t(customize prompt)");
    p("\trandom.{n|s}_{n|s}\t(random number or character)");
    p("\tredefine {all} {s}\t(rename an USL object)");
    p("\tremember {v}\t\t(store a variable definition)");
    p("\tremove {all}\t\t(remove an USL object from memory)");
    p("\treturn {v|s}\t\t(set return value of a method)");
    p("\tsay {v|s}\t\t(print a string with a newline)");
    p("\tshomp\t\t\t(quietly receive input)");
    p("\tstdout {v|s}\t\t(print a string)");
    p("\tunlock {v|m}\t\t(make method or variable destructible)");
    p("");
    p("- Environment Variables:");
    p("\t[Time]");
    p("\tam_or_pm\t\t(before noon or after noon)");
    p("\tday_of_this_week\t(this day of this week: Sunday-Saturday)");
    p("\tday_of_this_month\t(this day of this month: 1-31)");
    p("\tday_of_this_year\t(this day of this year: 1-365)");
    p("\tmonth_of_this_year\t(this month of this year: January-December)");
    p("\tthis_second\t\t(this second of this minute: 1-60)");
    p("\tthis_minute\t\t(this minute of this hour: 1-60)");
    p("\tthis_hour\t\t(this hour of this day: 1-24)");
    p("\tthis_month\t\t(this month of this year: 1-12)");
    p("\tthis_year\t\t(this year)");
    p("\tnow\t\t\t(the current time)");
    p("");
    p("\t[USL Specific]");
    p("\tcwd\t\t\t(current working directory)");
    p("\tin_dir\t\t\t(initial directory of shell)");
    p("\tusl\t\t\t(executable location of current shell)");
    p("\tos\t\t\t(guessed operating system)");
    p("\tuser\t\t\t(current user name)");
    p("\tmachine\t\t\t(current machine name)");
    p("\tempty_string\t\t(get an empty string)");
    p("\tempty_number\t\t(get an empty number)");
    p("\tlast_error\t\t(get last error)");
    p("\tlast_value\t\t(get last return value)");
    p("");
}

#ifdef __linux__

void md(string p)
{
    if (mkdir(p.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0)
        cout << ("..could not create directory: " + p) << endl;
}

void rd(string p)
{
    if (rmdir(p.c_str()) != 0)
        cout << ("..could not remove directory: " + p) << endl;
}

void rm(string p)
{
    if (remove(p.c_str()) != 0)
        cout << ("..could not remove file: " + p) << endl;
}

void touch(string p)
{
    ofstream f(p.c_str(), ios::out);

    if (f.is_open())
        f.close();
    else
        cout << ("..could not create file: " + p) << endl;
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
        cout << ("..could not create directory: " + p) << endl;
}

void rd(string p)
{
    if (rmdir(p.c_str()) != 0)
        cout << ("..could not remove directory: " + p) << endl;
}

void rm(string p)
{
    if (remove(p.c_str()) != 0)
        cout << ("..could not remove file: " + p) << endl;
}

void touch(string p)
{
    ofstream f(p.c_str(), ios::out);

    if (f.is_open())
        f.close();
    else
        cout << ("..could not create file: " + p) << endl;
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
