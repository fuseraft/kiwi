/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#ifndef STRINGS_H
#define STRINGS_H

const int MAX_BUFFER = 1024;

string afterUS(string s);
string afterBrackets(string s);
string afterDot(string s);
string afterUS(string s);
string beforeDot(string s);
string beforeBrackets(string s);
string beforeParams(string s);
string beforeUS(string s);
string getStdout(string cmd);
string getUpper(string in);
string getLower(string in);
string getInner(string s, int left, int right);
string subtractChar(string s1, string s2);
string subtractString(string s1, string s2);
string trimLeadingWhitespace(char *str);

bool contains(string s1, string s2);
bool containsBrackets(string s);
bool containsParams(string s);

bool isNumeric(string s);
bool isAlpha(string s);
bool isUpper(string in);
bool isLower(string in);
bool isTrue(string s);
bool isFalse(string s);
bool oneDot(string s);
bool endsWith(string s, string end);
bool startsWith(string s1, string s2);
bool zeroDots(string s);
bool zeroNumbers(string s);

int     stoi(string s);
string  itos(int i);

vector<string> getParams(string s);
vector<string> getRange(string s);
vector<string> getBracketRange(string s);

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;

    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }

    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;

    return split(s, delim, elems);
}

string& replace(string &original, string target, string replacement)
{
    size_t pos = original.find(target);

    if (pos != string::npos)
    {
        original.erase(pos, target.size());
        original.insert(pos, replacement);
    }

    return original;
}

static inline string &ltrim(string &s)
{
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
    return s;
}
// trim from end
static inline string &rtrim(string &s)
{
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
    return s;
}
static inline string &trim(string &s)
{
    return ltrim(rtrim(s));
}

string trimLeadingWhitespace(char *str)
{
    size_t len = 0;
    char *start = str - 1;
    char *end = NULL;

    if (str == NULL)
        return NULL;

    if (str[0] == '\0')
        return str;

    len = strlen(str);
    end = str + len;

    while (isspace(*(++start)));
    while (isspace(*(--end)) && end != start);

    if (str + len - 1 != end)
	{
        *(end + 1) = '\0';
    }
	else if (start != str && end == start)
	{
        *str = '\0';
	}
	
    end = str;
    if (start != str)
    {
        while (*start) 
		{
			*end++ = *start++;
		}
        *end = '\0';
    }

    string ret_str(str);

    return ret_str;
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

string getInner(string s, int left, int right) {
	string inner("");
	int len = s.length();
	if (left > len || right > len) {
		// overflow error 
	} else if (left > right) {
		// invalid operation
	} else if (left == right) {
		inner.push_back(s[left]);
	} else {
		for (int i = left; i <= right; i++) {
			inner.push_back(s[i]);
		}
	}
	
	return inner;
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

bool endsWith(string s, string end)
{
    unsigned int lastMatchPos = s.rfind(end);
    bool isEnding = lastMatchPos != std::string::npos;

    int el = end.length(), fl = s.length();

    for (int i = lastMatchPos + el; (i < fl) && isEnding; i++)
    {
        if ((s[i] != '\n') && (s[i] != '\r'))
            return false;
    }

    return isEnding;
}

bool startsWith(string s1, string s2)
{
    if (s1.length() >= s2.length())
    {
        int s2l = s2.length();

        for (int i = 0; i < s2l; i++)
            if (s1[i] != s2[i])
                return false;

        return true;
    }

    return false;
}

bool zeroDots(string s)
{
    bool none = true;
    int l = s.length();

    for (int i = 0; i < l; i++)
    {
        if (s[i] == '.')
		{
            none = false;
		}
    }

    return (none);
}

bool zeroNumbers(string s)
{
	int start = '0', stop = '9';
	
    for (unsigned int i = 0; i < s.length(); i++)
    {
		if (s[i] >= start && s[i] <= stop)
		{
			return false;
		}
    }

    return true;
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

bool isNumeric(string s)
{
    int l = s.length();
    bool pFound = false, hFound = false;

    for (int i = 0; i < l; i++)
    {
		if (!isdigit(s[i])) 
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

			default:
				return (false);
				break;
			}
		}
    }

    return (true);
}

bool isTrue(string s)
{
	return s == "true" || s == "1";
}

bool isFalse(string s)
{
	return s == "false" || s == "0";
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
    return tolower(c) - 'a' + 1;
}

#endif
