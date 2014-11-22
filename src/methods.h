/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com 
 **/

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
        printUSLHelp(),
        md(string p),
        rm(string p),
        rd(string p),
        createFile(string p);

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

void comprehensiveHelp(string keyword)
{
    // regular operators
    if (keyword == "+=")
    {
        cout << "The \"+=\" operator:\tincrements a numeric value,\r\n\t\t\tappends to a string value,\r\n\t\t\tappends an item to a list." << endl
             << endl
             << "Example:\r\n\r\n\t@str = \"string of words...\"\r\n\t@str += \" and another...\"" << endl
             << "\r\n\t@num = 65535\r\n\t@num += 6.5535" << endl
             << "\r\n\tlist @array\r\n\t@array += @str\r\n\t@array += @num\r\n" << endl;
    }
    else if (keyword == "-=")
    {
        cout << "The \"-=\" operator:\tdecrements a numeric value,\r\n\t\t\tsubtracts a string value,\r\n\t\t\tsubtracts an item from a list." << endl
             << endl
             << "Example:\r\n\r\n\t@str = \"string of words...\"\r\n\t@str -= \" of words...\"" << endl
             << "\r\n\t@num = (65535-6.5535)" << endl
             << "\r\n\tlist @array\r\n\t@array += @str\r\n\t@array += @num\r\n\t@array += @str\r\n\t@array -= @str\r\n" << endl;
    }
    else if (keyword == "*=")
    {
        cout << "The \"*=\" operator:\tmultiplies a numeric value." << endl
             << endl
             << "Example:\r\n\r\n\t@num = 65535\r\n\t@num *= 6.5535\r\n" << endl;
    }
    else if (keyword == "**=")
    {
        cout << "The \"**=\" operator:\tassigns a power of a value." << endl
             << endl
             << "Example:\r\n\r\n\t@num = 2.56\r\n\t@num **= 5\r\n" << endl;
    }
    else if (keyword == "/=")
    {
        cout << "The \"/=\" operator:\tdivides a numeric value." << endl
             << endl
             << "Example:\r\n\r\n\t@num = 65535\r\n\t@num /= 6.5535\r\n" << endl;
    }
    else if (keyword == "+")
    {
        cout << "The \"+\" operator:\tadds numeric values,\r\n\t\t\tconcatenates strings." << endl
             << endl
             << "Example:\r\n\r\n\t65535 + 65\r\n\t\"Hello, \" + \"World!\"" << endl
             << "\r\n\t@addition = (256+254)\r\n" << endl;
    }
    else if (keyword == "-")
    {
        cout << "The \"-\" operator:\tsubtracts numeric values,\r\n\t\t\tsubtracts strings." << endl
             << endl
             << "Example:\r\n\r\n\t65535 - 65\r\n\t\"Hello, \" - \"o, \"" << endl
             << "\r\n\t@subtraction = (256-254)\r\n" << endl;
    }
    else if (keyword == "*")
    {
        cout << "The \"*\" operator:\tmultiplies numeric values,\r\n\t\t\tmultiplies strings." << endl
             << endl
             << "Example:\r\n\r\n\t65535 * 65\r\n\t\"Hello, World! \" * 3" << endl
             << "\r\n\t@multiplication = (256*4)\r\n" << endl;
    }
    else if (keyword == "**")
    {
        cout << "The \"**\" operator:\tmultiplies the power of a value." << endl
             << endl
             << "Example:\r\n\r\n\t2.56 ** 2\r\n" << endl;
    }
    else if (keyword == "/")
    {
        cout << "The \"/\" operator:\tdivides numeric values." << endl
             << endl
             << "Example:\r\n\r\n\t65535 / 65" << endl
             << "\r\n\t@division = (256/4)\r\n" << endl;
    }
    else if (keyword == "%")
    {
        cout << "The \"%\" operator:\tcomputes the remainder of two numeric values." << endl
             << endl
             << "Example:\r\n\r\n\t11 % 3" << endl
             << "\r\n\t@divisible_by_four = (256%4)\r\n" << endl;
    }
    else if (keyword == "^")
    {
        cout << "The \"^\" operator:\tcomputes the power of a value." << endl
             << endl
             << "Example:\r\n\t@four_squared = (4^2)\r\n" << endl;
    }
    else if (keyword == "<")
    {
        cout << "The \"<\" operator:\ttests if a value is less than another,\r\n\t\t\tsets an incremental loop." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif" << endl
             << "\r\n\tfor 1 < 5 (i)\r\n\t\tsay \"Iteration: ${i}\"\r\n\tendfor\r\n" << endl;
    }
    else if (keyword == "<=")
    {
        cout << "The \"<=\" operator:\ttests if a value is less than or equal to another." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 <= 2\r\n\t\tsay \"One is less than or equal to two.\"\r\n\tendif" << endl
             << "\r\n\tfor 1 <= 5 (i)\r\n\t\tsay \"Iteration: ${i}\"\r\n\tendfor\r\n" << endl;
    }
    else if (keyword == ">")
    {
        cout << "The \">\" operator:\ttests if a value is greater than another,\r\n\t\t\tsets a decremental loop." << endl
             << endl
             << "Example:\r\n\r\n\tif 2 > 1\r\n\t\tsay \"Two is greater than one.\"\r\n\tendif" << endl
             << "\r\n\tfor 10 > 1 (i)\r\n\t\tout \"${i}, \"\r\n\tendfor\r\n\tsay \"blast off!\"\r\n" << endl;
    }
    else if (keyword == ">=")
    {
        cout << "The \">=\" operator:\ttests if a value is greater than or equal to another." << endl
             << endl
             << "Example:\r\n\r\n\tif 2 >= 1\r\n\t\tsay \"Two is greater than or equal to one.\"\r\n\tendif" << endl
             << "\r\n\tfor 5 >= 1 (i)\r\n\t\tsay \"Iteration: ${i}\"\r\n\tendfor\r\n" << endl;
    }
    else if (keyword == "=")
    {
        cout << "The \"<\" operator:\tinitializes variables,\r\n\t\t\tdenotes inheritance." << endl
             << endl
             << "Example:" << endl
             << "\r\n\t@var = \"A value.\"\r\n\r\n\tobject o\r\n\t\tmethod m\r\n\t\t\tsay \"Hello, World!\"\r\n\t\tend\r\n\tend" << endl
             << "\r\n\tobject oo = o;end\r\n" << endl;
    }
    else if (keyword == "==")
    {
        cout << "The \"<\" operator:\ttests if two values are equal." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 == 1\r\n\t\tsay \"One is equal to one.\"\r\n\tendif" << endl
             << "\r\n" << endl;
    }
    else if (keyword == "!")
    {
        cout << "The \"!\" operator:\tdirectly parses an argument." << endl
             << endl
             << "Example:" << endl
             << "\r\n\t! \"method m;say \'Hello, World!\';end;m;remove m\"\r\n" << endl;
    }
    else if (keyword == "!=")
    {
        cout << "The \"!\" operator:\ttests if two values are unequal." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 != 2\r\n\t\tsay \"One is unequal to two.\"\r\n\tendif\r\n" << endl;
    }
    // special operators
    else if (keyword == "begins_with")
    {
        cout << "The \"begins_with\" operator:\ttests if a string begins with another." << endl
             << endl
             << "Example:\r\n\r\n\tif \"Hello, World!\" begins_with \"Hello\"\r\n\t\tsay \"\'Hello, World!\' begins with \'Hello\'.\"\r\n\tendif" << endl
             << endl;
    }
    else if (keyword == "contains")
    {
        cout << "The \"contains\" operator:\ttests if a string contains another." << endl
             << endl
             << "Example:\r\n\r\n\tif \"Hello, World!\" contains \" \"\r\n\t\tsay \"\'Hello, World!\' contains \' \'.\"\r\n\tendif" << endl
             << endl;
    }
    else if (keyword == "ends_with")
    {
        cout << "The \"ends_with\" operator:\ttests if a string ends with another." << endl
             << endl
             << "Example:\r\n\r\n\tif \"Hello, World!\" ends_with \"World!\"\r\n\t\tsay \"\'Hello, World!\' ends with \'World!\'.\"\r\n\tendif" << endl
             << endl;
    }
    else if (keyword == "++=")
    {
        cout << "The \"++=\" operator:\tincrements the ascii value of every character in a string." << endl
             << endl
             << "Example:\r\n\r\n\t@var = \"Dinosaur\"\r\n\t@var ++= 2\r\n" << endl
             << endl;
    }
    else if (keyword == "--=")
    {
        cout << "The \"--=\" operator:\tdecrements the ascii value of every character in a string." << endl
             << endl
             << "Example:\r\n\r\n\t@var = \"Fkpqucwt\"\r\n\t@var --= 2\r\n" << endl
             << endl;
    }
    else if (keyword == "if")
    {
        cout << "The \"if\" statement:\texecutes a block if a condition is true." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "unless")
    {
        cout << "The \"unless\" statement:\texecutes a block if a condition is false." << endl
             << endl
             << "Example:\r\n\r\n\tunless 1 > 2\r\n\t\tsay \"One is not less than two.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "orif")
    {
        cout << "The \"orif\" statement:\texecutes a block if a condition is true and previous statements failed." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"" << endl
             << "\torif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "elif")
    {
        cout << "The \"elif\" statement:\texecutes a block if a condition is true and previous statements failed." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"" << endl
             << "\telif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "elsif")
    {
        cout << "The \"elsif\" statement:\texecutes a block if a condition is true and previous statements failed." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"" << endl
             << "\telsif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "else")
    {
        cout << "The \"orif\" statement:\texecutes a block if a condition is true and previous statements failed." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"" << endl
             << "\torif 1 < 0\r\n\t\tsay \"Failed statement.\"" << endl
             << "\telse\r\n\t\tsay \"Neither statement passed.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "failif")
    {
        cout << "The \"orif\" statement:\texecutes a block if a condition is true and previous statements failed." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 > 2\r\n\t\tsay \"Failed statement.\"" << endl
             << "\torif 1 < 0\r\n\t\tsay \"Failed statement.\"" << endl
             << "\tfailif\r\n\t\tsay \"Neither statement passed.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "endif")
    {
        cout << "The \"endif\" statement:\tends all conditional statements." << endl
             << endl
             << "Example:\r\n\r\n\tif 1 < 2\r\n\t\tsay \"One is less than two.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "for")
    {
        cout << "The \"for\" loop:\titerates by numerical setting." << endl
             << endl
             << "Example:\r\n\r\n\tfor infinity\r\n\t\tsay \"This will repeat infinitely\"\r\n\tendfor" << endl
             << "\r\n\tfor 1 < 5\r\n\t\tsay \"This will repeat 5 times\"\r\n\tendfor" << endl
             << "\r\n\tfor 5 > 1\r\n\t\tsay \"This will repeat 5 times\"\r\n\tendfor" << endl
             << "\r\n\tfor 1 < 5 (example)\r\n\t\tsay \"Current Iteration: ${example}\"\r\n\tendfor" << endl
             << "\r\n\tfor i in my_list\r\n\t\tsay \"Current Element: ${i}\"\r\n\tendfor" << endl
             << "\r\n\tfor i in (1..10)\r\n\t\tsay \"Current Iteration: ${i}\"\r\n\tendfor\r\n" << endl;
    }
    else if (keyword == "loop")
    {
        cout << "The \"loop\" loop:\titerates by enumerated setting." << endl
             << endl
             << "Example:\r\n\r\n\tlist @array;@array += \"abc\";@array += \"cba\"\r\n\tloop @array (i)\r\n\t\tsay \"${i}\"\r\n\tendfor" << endl
             << "\r\n\t@dir = env.cwd\r\n\tloop @dir.read_files (f)\r\n\t\tsay \"${f}\"\r\n\tendfor" << endl
             << "\r\n\t@dir = env.cwd\r\n\tloop @dir.read_dirs (d)\r\n\t\tsay \"${d}\"\r\n\tendfor" << endl
             << "\r\n\t@file = \"example.txt\"\r\n\tloop @file.read (line)\r\n\t\tsay \"${line}\"\r\n\tendfor" << endl
             << "\r\n\tloop @file.read (line)\r\n\t\tsay \"${line}\"\r\n\tendfor\r\n" << endl;
    }
    else if (keyword == "leave!")
    {
        cout << "The \"leave!\" keyword:\tbreaks an iteration." << endl
             << endl
             << "Example:\r\n\r\n\tfor 1 < 5 (i)\r\n\t\tif \"${i}\" = 3\r\n\t\t\tleave!\r\n\t\tendif\r\n\tendfor\r\n" << endl;
    }
    else if (keyword == "endfor")
    {
        cout << "The \"for\" loop:\titerates by numerical setting." << endl
             << endl
             << "Example:\r\n\r\n\tfor 1 < 5\r\n\t\tsay \"This will repeat 5 times\"\r\n\tendfor\r\n" << endl;
    }
    // Special Methods and Symbols
    // Memory Management
    else if (keyword == "clear_all!")
    {
        cout << "The \"clear_all!\" command:\tremoves all USL objects from memory." << endl
             << endl;
    }
    else if (keyword == "clear_lists!")
    {
        cout << "The \"clear_lists!\" command:\tremoves all lists from memory." << endl
             << endl;
    }
    else if (keyword == "clear_meths!")
    {
        cout << "The \"clear_meths!\" command:\tremoves all methods from memory." << endl
             << endl;
    }
    else if (keyword == "clear_objs!")
    {
        cout << "The \"clear_objs!\" command:\tremoves all objects from memory." << endl
             << endl;
    }
    else if (keyword == "clear_vars!")
    {
        cout << "The \"clear_vars!\" command:\tremoves all variables from memory." << endl
             << endl;
    }
    // Debug USL Objects
    else if (keyword == "is_list?")
    {
        cout << "The \"is_list?\" command:\ttests if a list exists." << endl
             << endl
             << "Example:\r\n\r\n\tlist @array\r\n\tis_list? @array\r\n" << endl;
    }
    else if (keyword == "is_method?")
    {
        cout << "The \"is_method?\" command:\ttests if a method exists." << endl
             << endl
             << "Example:\r\n\r\n\tmethod m;end\r\n\tis_method? m\r\n" << endl;
    }
    else if (keyword == "is_object?")
    {
        cout << "The \"is_object?\" command:\ttests if an object exists." << endl
             << endl
             << "Example:\r\n\r\n\tobject o;end\r\n\tis_object? o\r\n" << endl;
    }
    else if (keyword == "is_variable?")
    {
        cout << "The \"is_variable?\" command:\ttests if a variable exists." << endl
             << endl
             << "Example:\r\n\r\n\t@var = \"value\"\r\n\tis_variable? @var\r\n" << endl;
    }
    else if (keyword == "is_string?")
    {
        cout << "The \"is_string?\" command:\ttests if a variable is a string." << endl
             << endl
             << "Example:\r\n\r\n\t@var = \"value\"\r\n\tis_string? @var\r\n" << endl;
    }
    else if (keyword == "is_number?")
    {
        cout << "The \"is_number?\" command:\ttests if a variable is a number." << endl
             << endl
             << "Example:\r\n\r\n\t@var = 65535\r\n\tis_number? @var\r\n" << endl;
    }
    else if (keyword == "no_lists?")
    {
        cout << "The \"no_lists?\" command:\ttests the existence of list definitions." << endl
             << endl;
    }
    else if (keyword == "no_methods?")
    {
        cout << "The \"no_methods?\" command:\ttests the existence of method definitions." << endl
             << endl;
    }
    else if (keyword == "no_objects?")
    {
        cout << "The \"no_objects?\" command:\ttests the existence of object definitions." << endl
             << endl;
    }
    else if (keyword == "no_variables?")
    {
        cout << "The \"no_variables?\" command:\ttests the existence of variable definitions." << endl
             << endl;
    }
    else if (keyword == "see")
    {
        cout << "The \"see\" command:\tdisplays USL object definitions,\r\n\t\t\tdisplays defined USL objects." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tsee lists" << endl
             << "\tsee methods" << endl
             << "\tsee objects" << endl
             << "\tsee variables" << endl
             << endl
             << "\tobject o;end" << endl
             << "\tsee o\r\n" << endl
             << "\tmethod m;end" << endl
             << "\tsee m\r\n" << endl
             << "\tlist @l" << endl
             << "\tsee l\r\n" << endl
             << "\t@v = \"value\"" << endl
             << "\tsee @v\r\n" << endl;
    }
    // File/Directory Existence
    else if (keyword == "is_dir?")
    {
        cout << "The \"is_dir?\" command:\ttests if a string or variable is a directory." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@cwd = env.cwd\r\n\tif @cwd = is_dir?\r\n\t\tsay \"The directory exists.\"\r\n\tendif\r\n" << endl;
    }
    else if (keyword == "is_file?")
    {
        cout << "The \"is_file?\" command:\ttests if a string or variable is a file." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@file = \"/Windows/System32/cmd.exe\"\r\n\tif @file = is_file?\r\n\t\tsay \"The file exists.\"\r\n\tendif\r\n" << endl;
    }
    // Defining Memory
    else if (keyword == "__begin__")
    {
        cout << "The \"__begin__\" command:\tbegins a script definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t__begin__ \"script.us\"\r\n\tsay \"Hello, World!\"\r\n\t__end__\r\n" << endl;
    }
    else if (keyword == "__end__")
    {
        cout << "The \"__end__\" command:\tends a script definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t__begin__ \"script.us\"\r\n\tsay \"Hello, World!\"\r\n\t__end__\r\n" << endl;
    }
    else if (keyword == "list")
    {
        cout << "The \"list\" keyword:\tdeclares a list definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tlist @array\r\n\t@array += \"first element\"\r\n\t@array += \"last element\"\r\n" << endl;
    }
    else if (keyword == "[method]")
    {
        cout << "The \"[method]\" keyword:\tdeclares an indestructible method definition." << endl
             << endl
             << "The only way to remove an indestructible method is to use the \"remove\" command." << endl
             << "\r\nTo make a method destructible, use the \"unlock\" command." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t[method] indestructible_Method\r\n\t\tsay \"Hello. I am indestructible.\"\r\n\tend\r\n" << endl;
    }
    else if (keyword == "method")
    {
        cout << "The \"method\" keyword:\tdeclares a method definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tmethod m\r\n\t\tsay \"Hello, World!\"\r\n\tend\r\n" << endl;
    }
    else if (keyword == "template")
    {
        cout << "The \"template\" keyword:\tdeclares a template definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\ttemplate \"foo(b, a, r)\"\r\n\t\t@var = $0\r\n\t\t@var -= $1\r\n\t\t@var += $2\r\n\t\tsay @var\r\n\tend\r\n" << endl;
    }
    else if (keyword == "object")
    {
        cout << "The \"object\" keyword:\tdeclares an object definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tobject o\r\n\t\tmethod m\r\n\t\t\tsay \"Hello, World!\"\r\n\t\tend\r\n\tend\r\n" << endl;
    }
    else if (keyword == "lock")
    {
        cout << "The \"lock\" keyword:\tdeclares a method or variable as \"indestructible.\"" << endl
             << "\r\nUse the \"remove\" command to remove an indestructible method or variable." << endl
             << "\r\nTo make a method or variable destructible, use the \"unlock\" command." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tmethod indestructible_method\r\n\t\tsay \"I will be indestructible.\"\r\n\tend\r\n\r\n\tlock indestructible_method\r\n" << endl;
    }
    else if (keyword == "unlock")
    {
        cout << "The \"unlock\" keyword:\tdeclares a method or variable as \"destructible.\"" << endl
             << "\r\nTo make a method or variable indestructible, use the \"lock\" command." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t[method] indestructible_method\r\n\t\tsay \"I will be indestructible.\"\r\n\tend\r\n\r\n\tunlock indestructible_method\r\n" << endl;
    }
    else if (keyword == "public")
    {
        cout << "The \"public\" keyword:\tdeclares a public object definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tobject o\r\n\t\tpublic\r\n\t\t\tmethod m\r\n\t\t\t\tsay \"Hello, World!\"\r\n\t\t\tend\r\n\tend\r\n" << endl;
    }
    else if (keyword == "private")
    {
        cout << "The \"private\" keyword:\tdeclares a private object definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tobject o\r\n\t\tprivate\r\n\t\t\tmethod m\r\n\t\t\t\tsay \"Hello, World!\"\r\n\t\t\tend\r\n\tend\r\n" << endl;
    }
    else if (keyword == "@")
    {
        cout << "The \"@\" symbol:\tdenotes a variable declaration." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@var = \"Hello, World!\"\r\n" << endl;
    }
    // Time
    else if (keyword == "am_or_pm")
    {
        cout << "The \"am_or_pm\" initializer:\tsets a variable to before noon or after." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@am_or_pm = am_or_pm\r\n" << endl;
    }
    else if (keyword == "day_of_this_week")
    {
        cout << "The \"day_of_this_week\" initializer:\tsets a variable to the day of this week." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@day_of_this_week = day_of_this_week\r\n" << endl;
    }
    else if (keyword == "day_of_this_month")
    {
        cout << "The \"day_of_this_month\" initializer:\tsets a variable to the day of this month." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@day_of_this_month = day_of_this_month\r\n" << endl;
    }
    else if (keyword == "day_of_this_year")
    {
        cout << "The \"day_of_this_year\" initializer:\tsets a variable to the day of this year." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@day_of_this_year = day_of_this_year\r\n" << endl;
    }
    else if (keyword == "month_of_this_year")
    {
        cout << "The \"month_of_this_year\" initializer:\tsets a variable to the month of this year." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@month_of_this_year = month_of_this_year\r\n" << endl;
    }
    else if (keyword == "this_second")
    {
        cout << "The \"this_second\" initializer:\tsets a variable to the current second." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@this_second = this_second\r\n" << endl;
    }
    else if (keyword == "this_minute")
    {
        cout << "The \"this_minute\" initializer:\tsets a variable to the current minute." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@this_minute = this_minute\r\n" << endl;
    }
    else if (keyword == "this_hour")
    {
        cout << "The \"this_hour\" initializer:\tsets a variable to the current hour." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@this_hour = this_hour\r\n" << endl;
    }
    else if (keyword == "this_month")
    {
        cout << "The \"this_month\" initializer:\tsets a variable to the current month." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@this_month = this_month\r\n" << endl;
    }
    else if (keyword == "this_year")
    {
        cout << "The \"this_year\" initializer:\tsets a variable to the current year." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@this_year = this_year\r\n" << endl;
    }
    // Extras
    else if (keyword == "\\\'")
    {
        cout << "The \"\\\'\" symbol:\tis parsed as an apostrophe mark." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tsay \"\\\'Hello, World!\\\'\"\r\n" << endl;
    }
    else if (keyword == "\'")
    {
        cout << "The \"\'\" symbol:\tis parsed as a quotation mark." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tsay \"\'Hello, World!\'\"\r\n" << endl;
    }
    else if (keyword == "\\t")
    {
        cout << "The \"\\t\" symbol:\tis parsed as a tab sequence." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tsay \"\\tHello, World!\"\r\n" << endl;
    }
    else if (keyword == "\\n")
    {
        cout << "The \"\\n\" symbol:\tis parsed as a CRLF sequence." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tsay \"\\nHello, World!\\n\"\r\n" << endl;
    }
    else if (keyword == "#")
    {
        cout << "The \"#\" symbol:\tdenotes a comment." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t# This is a comment\r\n" << endl;
    }
    else if (keyword == "##")
    {
        cout << "The \"##\" symbol:\tdenotes multiline commentation." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t##\r\n\tThis is a multiline comment\r\n\t##\r\n" << endl;
    }
    else if (keyword == "?")
    {
        cout << "The \"?\" symbol:\texecutes an external command," << endl
             << "\t\t\tinitializes a variable with the stdout of an external command." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t? cmd" << endl
             << "\t@stdout ? \"ping example.com\"\r\n" << endl;
    }
    else if (keyword == ";")
    {
        cout << "The \";\" symbol:\tseparates code lines." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tmethod m;say \"Hello, World!\";end;m;remove m\r\n" << endl;
    }
    else if (keyword == "append")
    {
        cout << "The \"append\" command:\tappends text to a file." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tappend \"example.txt\" \"This is some text.\"\r\n" << endl;
    }
    else if (keyword == "appendl")
    {
        cout << "The \"appendl\" command:\tappends a newline of text to a file." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tappendl \"example.txt\" \"This is some text.\"\r\n" << endl;
    }
    else if (keyword == "args")
    {
        cout << "The \"args\" keyword:\tgets the parameters of a loaded script." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@script = args[0]\r\n" << endl;
    }
    else if (keyword == "args.size")
    {
        cout << "The \"args.size\" keyword:\tgets the parameter count of a loaded script." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@argc = args.size\r\n" << endl;
    }
    else if (keyword == "cd")
    {
        cout << "The \"cd\" keyword:\tchanges the current directory." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tcd /Windows/System32\r\n" << endl;
    }
    else if (keyword == "readline")
    {
        cout << "The \"readline\" keyword:\tbegins a loud stdin stream." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@fullname = \"readline.Enter your fullname:\"\r\n" << endl;
    }
    else if (keyword == "delay")
    {
        cout << "The \"delay\" command:\tdelays program flow for a set number of seconds." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tsay \"Delaying for 5 seconds...\"\r\n\tdelay 5\r\n" << endl;
    }
    else if (keyword == "dpush")
    {
        cout << "The \"dpush\" command:\tcreates a directory." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tdpush \"directory name\"\r\n" << endl;
    }
    else if (keyword == "dpop")
    {
        cout << "The \"dpop\" command:\tremoves a directory." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tdpop \"directory name\"\r\n" << endl;
    }
    else if (keyword == "env")
    {
        cout << "The \"env\" keyword:\tgets the value of an environmental variable." << endl
             << endl
             << "Example:" << endl
             << endl
             << "# USL Specific" << endl
             << "\t@cwd = env.cwd" << endl
             << "\t@usl = env.usl" << endl
             << "\t@user = env.user" << endl
             << "\t@machine = env.machine" << endl
             << "\t@os = env.os" << endl
             << "# OS Specific" << endl
             << "\t@windir = env.windir" << endl
             << "\t@prompt = env.prompt" << endl
             << "\t@sysdrive = env.systemdrive\r\n" << endl;
    }
    else if (keyword == "error")
    {
        cout << "The \"error\" command:\tprints to the stderr stream." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\terror \"An error occurred.\"\r\n" << endl;
    }
    else if (keyword == "forget")
    {
        cout << "The \"forget\" command:\tforgets a stored variable definition." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tforget @var\r\n" << endl;
    }
    else if (keyword == "fpush")
    {
        cout << "The \"fpush\" command:\tcreates a file." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tfpush \"file name\"\r\n" << endl;
    }
    else if (keyword == "fpop")
    {
        cout << "The \"fpop\" command:\tremoves a file." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tfpop \"file name\"\r\n" << endl;
    }
    else if (keyword == "in_dir")
    {
        cout << "The \"in_dir\" keyword:\tgets/sets the USL initial directory." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tin_dir /Windows/System32" << endl
             << "\tcd \"/Program Files\"" << endl
             << "\tcd in_dir\r\n" << endl;
    }
    else if (keyword == "load")
    {
        cout << "The \"load\" command:\tincludes and executes a script." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tload \"script name.us\"\r\n" << endl;
    }
    else if (keyword == "parser")
    {
        cout << "The \"parser\" command:\tbegins a separate USL session." << endl
             << endl;
    }
    else if (keyword == "prompt")
    {
        cout << "The \"prompt\" command:\tsets the USL shell prompt." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tprompt bash" << endl
             << "\tprompt !" << endl
             << "\tprompt \"[\\u@\\m(\\w)]$ \"" << endl
             << "\tprompt !" << endl
             << "\tprompt !\r\n" << endl;
    }
    else if (keyword == "random")
    {
        cout << "The \"random\" keyword:\tgets a random character or number." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@rand_num = random.1_12345" << endl
             << "\t@rand_char = random.a_z\r\n" << endl;
    }
    else if (keyword == "redefine")
    {
        cout << "The \"redefine\" command:\trenames a USL object name." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tobject o;method m;say \"Hello, World!\";end;end" << endl
             << "\tredefine o oo\r\n\too.m\r\n" << endl;
    }
    else if (keyword == "remember")
    {
        cout << "The \"remember\" command:\tstores a variable definition for future usage." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@var = \"a value\"\r\n\tremember @var\r\n" << endl;
    }
    else if (keyword == "remove")
    {
        cout << "The \"remove\" command:\tremoves a USL object definition from memory." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tobject o;method m;say \"Hello, World!\";end;end" << endl
             << "\tremove o\r\n" << endl;
    }
    else if (keyword == "say")
    {
        cout << "The \"say\" command:\tprints a newline of text to the stdout stream." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@words = \"This is a sentence.\"" << endl
             << "\tsay \"This sentence is ended with a newline.\\nSentence: \\{@words}\"\r\n" << endl;
    }
    else if (keyword == "password")
    {
        cout << "The \"password\" keyword:\tbegins a silent stdin stream." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@passwd = \"password.Enter your password:\"\r\n" << endl;
    }
    else if (keyword == "stdout")
    {
        cout << "The \"stdout\" command:\tprints text to the stdout stream." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tstdout \"This sentence is not ended with a newline.\"\r\n" << endl;
    }
    // Newly added
    else if (keyword == "bytes")
    {
        cout << "The \"bytes\" keyword:\tgets the size of a file in bytes." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@file = env.usl" << endl
             << "\t@size = @file.bytes\r\n" << endl;
    }
    else if (keyword == "kbytes")
    {
        cout << "The \"kbytes\" keyword:\tgets the size of a file in kilobytes." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@file = env.usl" << endl
             << "\t@size = @file.kbytes\r\n" << endl;
    }
    else if (keyword == "mbytes")
    {
        cout << "The \"mbytes\" keyword:\tgets the size of a file in megabytes." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@file = env.usl" << endl
             << "\t@size = @file.mbytes\r\n" << endl;
    }
    else if (keyword == "gbytes")
    {
        cout << "The \"gbytes\" keyword:\tgets the size of a file in gigabytes." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@file = env.usl" << endl
             << "\t@size = @file.gbytes\r\n" << endl;
    }
    else if (keyword == "tbytes")
    {
        cout << "The \"tbytes\" keyword:\tgets the size of a file in terabytes." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@file = env.usl" << endl
             << "\t@size = @file.tbytes\r\n" << endl;
    }
    else if (keyword == "try")
    {
        cout << "The \"try\" keyword:\ttries to execute a block of code." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\ttry" << endl
             << "\t\t@num = 123.456" << endl
             << "\tcatch" << endl
             << "\t\tsay \"An error occurred.\"" << endl
             << "\tcaught\r\n" << endl;
    }
    else if (keyword == "catch")
    {
        cout << "The \"catch\" keyword:\texecutes a block of code if an error occurred in the \"try\" block." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\ttry" << endl
             << "\t\t@num = 123.456" << endl
             << "\t\t@num = abcdef" << endl
             << "\tcatch" << endl
             << "\t\tsay \"An error occurred.\"" << endl
             << "\tcaught\r\n" << endl;
    }
    else if (keyword == "caught")
    {
        cout << "The \"caught\" keyword:\tends a \"try\" block." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\ttry" << endl
             << "\t\t@num = 123.456" << endl
             << "\tcatch" << endl
             << "\t\tsay \"An error occurred.\"" << endl
             << "\tcaught\r\n" << endl;
    }
    else if (keyword == "last_error")
    {
        cout << "The \"last_error\" symbol:\tcontains the error message of a failed \"try\" block." << endl
             << endl
             << "A variable created in the catch code will be removed." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\ttry" << endl
             << "\t\t@num = 123.456" << endl
             << "\t\t@num = abcdef" << endl
             << "\tcatch" << endl
             << "\t\t@e = last_error" << endl
             << "\t\tout \"Error occurred: \"" << endl
             << "\t\tsay @e" << endl
             << "\tcaught\r\n" << endl;
    }
    else if (keyword == "while")
    {
        cout << "The \"while\" keyword:\tdeclares a while loop." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@a = 1" << endl
             << "\t@b = 10" << endl
             << "\twhile @a <= @b" << endl
             << "\t\t@c += 1" << endl
             << "\t\tsay @c" << endl
             << "\t\t@a += 1" << endl
             << "\tend\r\n" << endl;
    }
    else if (keyword == "end")
    {
        cout << "The \"end\" keyword:\tends a while loop.\r\n\t\t\tends a switch statement.\r\n\t\t\tends a method, object, or template." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\t@a = 1" << endl
             << "\t@b = 10" << endl
             << "\twhile @a <= @b" << endl
             << "\t\t@c += 1" << endl
             << "\t\tsay @c" << endl
             << "\t\t@a += 1" << endl
             << "\tend" << endl
             << endl
             << "\tswitch @a" << endl
             << "\t\tcase 12" << endl
             << "\t\t\tsay \"Not quite 11.\"" << endl
             << "\t\tcase 11" << endl
             << "\t\t\tsay \"Match found!\"" << endl
             << "\t\tdefault" << endl
             << "\t\t\tsay \"No matches found.\"" << endl
             << "\tend\r\n" << endl;
    }
    else if (keyword == "switch")
    {
        cout << "The \"switch\" keyword:\tbegins a switch statement." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tmethod m" << endl
             << "\t\t@a = \"This is a string.\"" << endl
             << "\t\tswitch @a" << endl
             << "\t\t\tcase \"This\"" << endl
             << "\t\t\t\tsay \"Not quite...\"" << endl
             << "\t\t\tcase \"This is a string.\"" << endl
             << "\t\t\t\tsay \"Matched!\"" << endl
             << "\t\t\tdefault" << endl
             << "\t\t\t\tsay \"No match found...\"" << endl
             << "\t\tend" << endl
             << "\tend" << endl
             << endl
             << "m\r\n" << endl;
    }
    else if (keyword == "case")
    {
        cout << "The \"case\" keyword:\tprovides a test value for a switch statement." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tmethod m" << endl
             << "\t\t@a = \"This is a string.\"" << endl
             << "\t\tswitch @a" << endl
             << "\t\t\tcase \"This\"" << endl
             << "\t\t\t\tsay \"Not quite...\"" << endl
             << "\t\t\tcase \"This is a string.\"" << endl
             << "\t\t\t\tsay \"Matched!\"" << endl
             << "\t\t\tdefault" << endl
             << "\t\t\t\tsay \"No match found...\"" << endl
             << "\t\tend" << endl
             << "\tend" << endl
             << endl
             << "m\r\n" << endl;
    }
    else if (keyword == "default")
    {
        cout << "The \"default\" keyword:\tbegins a code block if no match is found when switching." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tmethod m" << endl
             << "\t\t@a = \"This is a string.\"" << endl
             << "\t\tswitch @a" << endl
             << "\t\t\tcase \"This\"" << endl
             << "\t\t\t\tsay \"Not quite...\"" << endl
             << "\t\t\tcase 3.14" << endl
             << "\t\t\t\tsay \"Matched!\"" << endl
             << "\t\t\tdefault" << endl
             << "\t\t\t\tsay \"No match found...\"" << endl
             << "\t\tend" << endl
             << "\tend" << endl
             << endl
             << "m\r\n" << endl;
    }
    else if (keyword == "return")
    {
        cout << "The \"return\" keyword:\tsets the value to be returned by a method." << endl
             << endl
             << "Any variable returned will be removed from memory." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tmethod m" << endl
             << "\t\treturn \"Return Value\"" << endl
             << "\tend" << endl
             << endl
             << "\tmethod pi" << endl
             << "\t\t@ret_val = 3.14" << endl
             << "\t\treturn @ret_val" << endl
             << "\tend" << endl
             << endl
             << "\t@pi = pi" << endl
             << "\t@value = m\r\n" << endl;
    }
    else if (keyword == "fwrite")
    {
        cout << "The \"fwrite\" command:\tcreates a file if it does not already exist and appends text to it." << endl
             << endl
             << "If the file exists and has been written to, fwrite returns 0." << endl
             << "If the file is created and has been written to, fwrite returns 1." << endl
             << "If the file could not be created, fwrite returns -1." << endl
             << endl
             << "Example:" << endl
             << endl
             << "\tmethod fwrite(file,contents)" << endl
             << "\t\tfwrite $0 $1" << endl
             << "\tend" << endl
             << endl
             << "\t@contents = \"This is a string of text that will be written to a file.\"" << endl
             << "\t@ret_val = fwrite(\"some file.txt\",@contents)" << endl
             << endl;
    }
    else
    {
        cout << "The keyword, symbol, or command could not be found." << endl
             << endl
             << "Please use the \"help\" command to see a list of keywords, symbols, and commands." << endl;
    }
}

void printUSLHelp() {
    cout << "[Parameter Key]" << endl
		<< "\t{s}\t\t(string)" << endl
		<< "\t{n}\t\t(number)" << endl
		<< "\t{v}\t\t(variable)" << endl
		<< "\t{m}\t\t(method)" << endl
		<< "\t{o}\t\t(object)" << endl
		<< "\t{l}\t\t(list)" << endl
		<< "\t{all}\t\t(any USL object)" << endl
		<< "\t{expression}\t(logical expression)" << endl
		<< endl
		<< "[Keywords and Symbols]" << endl
		<< "- Operators:" << endl
		<< "\t[Regular]" << endl
		<< "\t+=\t\t(increment || append string)" << endl
		<< "\t-=\t\t(decrement || remove string)" << endl
		<< "\t*=\t\t(multiply->increment || multiply string)" << endl
		<< "\t/=\t\t(divide->decrement)" << endl
		<< "\t**=\t\t(exponent)" << endl
		<< "\t+\t\t(addition)" << endl
		<< "\t-\t\t(subtraction)" << endl
		<< "\t*\t\t(multiplication)" << endl
		<< "\t/\t\t(division)" << endl
		<< "\t%\t\t(modulus)" << endl
		<< "\t**\t\t(exponent)" << endl
		<< "\t<\t\t(less than)" << endl
		<< "\t>\t\t(greater than)" << endl
		<< "\t<=\t\t(less than or equal to)" << endl
		<< "\t>=\t\t(greater than or equal to)" << endl
		<< "\t=\t\t(initializer)" << endl
		<< "\t==\t\t(equal)" << endl
		<< "\t!=\t\t(not equal)\r\n" << endl
		<< "\t[Special]" << endl
		<< "\tbegins_with\t(used in if expressions)" << endl
		<< "\tcontains\t(used in if expressions)" << endl
		<< "\tends_with\t(used in if expressions)" << endl
		<< "\t++=\t\t(increment ascii value)" << endl
		<< "\t--=\t\t(decrement ascii value)" << endl
		<< endl
		<< "- Conditionals:" << endl
		<< "\tunless {expression}\t(unless statement)" << endl
		<< "\tif {expression}\t\t(if statement)" << endl
		<< "\torif {expression}\t(else if statement)" << endl
		<< "\telse\t\t\t(else statement)" << endl
		<< "\tfailif\t\t\t(if failure occurred)" << endl
		<< "\tendif\t\t\t(end if statement)" << endl
		<< "\tswitch {v}\t\t(switch statement)" << endl
		<< "\tcase {n|s}\t\t(switch case)" << endl
		<< "\tdefault\t\t\t(default case)" << endl
		<< "\tend\t\t\t(end switch statement)" << endl
		<< "\ttry\t\t\t(begin try statement)" << endl
		<< "\tcatch\t\t\t(begin catch code)" << endl
		<< "\tcaught\t\t\t(end try statement)" << endl
		<< endl
		<< "\tfor infinity\t\t(infinite loop)" << endl
		<< "\tfor {n} < {n}\t\t(incremental loop)" << endl
		<< "\tfor {n} > {n}\t\t(decremental loop)" << endl
		<< "\twhile {v} {op} {n|v}\t(while loop)" << endl
		<< "\tloop {l}\t\t(list loop)" << endl
		<< "\tloop {v}.read\t\t(read lines in a file)" << endl
		<< "\tloop {v}.read_files\t(read files in a directory)" << endl
		<< "\tloop {v}.read_dirs\t(read directories in a directory)" << endl
		<< "\tleave!\t\t\t(break loop flow)" << endl
		<< "\tendfor\t\t\t(end for loop)" << endl
		<< "\tend\t\t\t(end while loop)" << endl
		<< endl
		<< "- Special Methods and Symbols:" << endl
		<< "\t[Memory Management]" << endl
		<< "\tclear_all!\t\t(clear everything in memory)" << endl
		<< "\tclear_lists!\t\t(clear all lists in memory)" << endl
		<< "\tclear_methods!\t\t(clear all methods in memory)" << endl
		<< "\tclear_objects!\t\t(clear all objects in memory)" << endl
		<< "\tclear_variables!\t(clear all variables in memory)" << endl
		<< "\tclear_constants!\t(clear all constants in memory)" << endl
		<< endl
		<< "\t[Debug USL Objects, Variables, and Methods]" << endl
		<< "\tis_list? {l}\t\t(is list defined?)" << endl
		<< "\tis_method? {m}\t\t(is method defined?)" << endl
		<< "\tis_object? {o}\t\t(is object defined?)" << endl
		<< "\tis_variable? {v}\t(is variable defined?)" << endl
		<< "\tis_number? {v}\t\t(is variable a number?)" << endl
		<< "\tis_string? {v}\t\t(is variable a string?)" << endl
		<< "\tno_lists?\t\t(do no lists exist?)" << endl
		<< "\tno_methods?\t\t(do no methods exist?)" << endl
		<< "\tno_objects?\t\t(do no objects exist?)" << endl
		<< "\tno_variables?\t\t(do no variables exist?)" << endl
		<< "\tsee {all}\t\t(see definition of declaration)" << endl
		<< "\tsee {option}\t\t(lists/methods/objects/variables)" << endl
		<< endl
		<< "\t[File/Directory Existence]" << endl
		<< "\tis_dir? {v|s}\t\t(is variable/string a directory?)" << endl
		<< "\tis_file? {v|s}\t\t(is variable/string a file?)" << endl
		<< endl
		<< "\t[File Size]" << endl
		<< "\t{v}.bytes\t\t(gets size of a file in bytes)" << endl
		<< "\t{v}.kbytes\t\t(gets size of a file in kilobytes)" << endl
		<< "\t{v}.mbytes\t\t(gets size of a file in megabytes)" << endl
		<< "\t{v}.gbytes\t\t(gets size of a file in gigabytes)" << endl
		<< "\t{v}.tbytes\t\t(gets size of a file in terabytes)" << endl
		<< endl
		<< "\t[Defining Memory]" << endl
		<< "\t__begin__ {v|s}\t\t(define a script)" << endl
		<< "\t__end__\t\t\t(end script definition)" << endl
		<< "\tlist {l}\t\t(define a list)" << endl
		<< "\t[method] {m}\t\t(define an indestructible method)" << endl
		<< "\tmethod {m}\t\t(define a method)" << endl
		<< "\ttemplate {m}\t\t(define a template method)" << endl
		<< "\tobject {o}\t\t(define an object)" << endl
		<< "\tpublic\t\t\t(begin defining public members)" << endl
		<< "\tprivate\t\t\t(begin defining private members)" << endl
		<< "\tend\t\t\t(end method/object/template definition)" << endl
		<< "\t@\t\t\t(variables begin with '@')" << endl
		<< endl
		<< "\t[Extras]" << endl
		<< "\t\\'\t\t\t(parsed as apostrophe mark)" << endl
		<< "\t'\t\t\t(parsed as quotation mark)" << endl
		<< "\t\\t\t\t\t(tab sequence)" << endl
		<< "\t\\n\t\t\t(carriage-return & line-feed)" << endl
		<< "\t#\t\t\t(denote commentation)" << endl
		<< "\t##\t\t\t(begin/end multiline commentation)" << endl
		<< "\t!\t\t\t(parse a string directly || initializer)" << endl
		<< "\t?\t\t\t(execute an external command || initializer)" << endl
		<< "\t;\t\t\t(line separator)" << endl
		<< "\tappend\t\t\t(append text to a file)" << endl
		<< "\tappendl\t\t\t(append a line to a file)" << endl
		<< "\targs[{n}]\t\t(parameter access)" << endl
		<< "\targs.size\t\t(parameter count)" << endl
		<< "\tcd\t\t\t(specialized \"chdir\")" << endl
		<< "\treadline.{v|s}\t\t(loudly receive input)" << endl
		<< "\tdelay {n}\t\t(pause script flow in seconds)" << endl
		<< "\tdpush {v|s}\t\t(create a directory)" << endl
		<< "\tdpop {v|s}\t\t(delete a directory)" << endl
		<< "\tenv.{s}\t\t\t(get environment variable)" << endl
		<< "\terror {v|s}\t\t(standard error stream)" << endl
		<< "\tforget {v}\t\t(forget variable definition)" << endl
		<< "\tfpush {v|s}\t\t(create a file)" << endl
		<< "\tfpop {v|s}\t\t(delete a file)" << endl
		<< "\tfwrite\t\t\t(use \"help fwrite\")" << endl
		<< "\thelp {keyword}\t\t(comprehensive help)" << endl
		<< "\tin_dir\t\t\t(USL Initial Directory)" << endl
		<< "\tload {us}\t\t(include and execute script definitions)" << endl
		<< "\tlock {v|m}\t\t(make method or variable indestructible)" << endl
		<< "\tparser\t\t\t(begin parsing input)" << endl
		<< "\tprompt {s}\t\t(customize prompt)" << endl
		<< "\trandom.{n|s}_{n|s}\t(random number or character)" << endl
		<< "\tredefine {all} {s}\t(rename an USL object)" << endl
		<< "\tremember {v}\t\t(store a variable definition)" << endl
		<< "\tremove {all}\t\t(remove an USL object from memory)" << endl
		<< "\treturn {v|s}\t\t(set return value of a method)" << endl
		<< "\tsay {v|s}\t\t(print a string with a newline)" << endl
		<< "\tpassword\t\t\t(quietly receive input)" << endl
		<< "\tstdout {v|s}\t\t(print a string)" << endl
		<< "\tunlock {v|m}\t\t(make method or variable destructible)" << endl
		<< endl
		<< "- Environment Variables:" << endl
		<< "\t[Time]" << endl
		<< "\tam_or_pm\t\t(before noon or after noon)" << endl
		<< "\tday_of_this_week\t(this day of this week: Sunday-Saturday)" << endl
		<< "\tday_of_this_month\t(this day of this month: 1-31)" << endl
		<< "\tday_of_this_year\t(this day of this year: 1-365)" << endl
		<< "\tmonth_of_this_year\t(this month of this year: January-December)" << endl
		<< "\tthis_second\t\t(this second of this minute: 1-60)" << endl
		<< "\tthis_minute\t\t(this minute of this hour: 1-60)" << endl
		<< "\tthis_hour\t\t(this hour of this day: 1-24)" << endl
		<< "\tthis_month\t\t(this month of this year: 1-12)" << endl
		<< "\tthis_year\t\t(this year)" << endl
		<< "\tnow\t\t\t(the current time)" << endl
		<< endl
		<< "\t[USL Specific]" << endl
		<< "\tcwd\t\t\t(current working directory)" << endl
		<< "\tin_dir\t\t\t(initial directory of shell)" << endl
		<< "\tnoctis\t\t\t(executable location of current shell)" << endl
		<< "\tos\t\t\t(guessed operating system)" << endl
		<< "\tuser\t\t\t(current user name)" << endl
		<< "\tmachine\t\t\t(current machine name)" << endl
		<< "\tempty_string\t\t(get an empty string)" << endl
		<< "\tempty_number\t\t(get an empty number)" << endl
		<< "\tlast_error\t\t(get last error)" << endl
		<< "\tlast_value\t\t(get last return value)" << endl 
		<< endl;
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

void createFile(string p)
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

void createFile(string p)
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

// experimental code

































