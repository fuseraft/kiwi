#ifndef STRINGS_H
#define STRINGS_H

const int MAX_BUFFER = 1024;

string after_brackets(std::string s);
string before_brackets(std::string s);
string after_dot(std::string s);
string before_dot(std::string s);
string before_params(std::string s);
string to_upper(std::string in);
string to_lower(std::string in);
string substring(std::string s, int left, int right);
string subtract_char(std::string s, char c);
string subtract_string(std::string s1, std::string s2);
string ltrim_ws(const string &str);

bool contains(std::string s1, std::string s2);
bool has_brackets(std::string s);
bool has_params(std::string s);

bool is_numeric(std::string s);
bool is_alpha(std::string s);
bool is_truthy(std::string s);
bool is_falsey(std::string s);
bool ends_with(std::string s, std::string end);
bool begins_with(std::string s1, std::string s2);
bool is_dotless(std::string s);
bool is_numberless(std::string s);

std::string itos(int i);

vector<std::string> parse_params(std::string s);
vector<std::string> parse_range(std::string s);
vector<std::string> parse_bracketrange(std::string s);

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
        elems.push_back(item);

    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

string &replace(std::string &original, std::string target, std::string replacement)
{
    size_t pos = original.find(target);

    if (pos != string::npos)
    {
        original.erase(pos, target.size());
        original.insert(pos, replacement);
    }

    return original;
}

static inline string &ltrim(std::string &s)
{
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
    return s;
}

static inline string &rtrim(std::string &s)
{
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
    return s;
}
static inline string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}

string ltrim_ws(const string &str)
{
    auto it = find_if_not(str.begin(), str.end(), [](int c) { return isspace(c); });
    return string(it, str.end());
}

bool valid_const_name(std::string in)
{
    for (int i = 0; i < (int)in.length(); i++)
    {
        char c = in[i];

        if (!isupper(c) && c != '_')
        {
            return false;
        }
    }

    return true;
}

string to_upper(std::string in)
{
    string builder("");
    for (int i = 0; i < (int)in.length(); i++)
    {
        char c = in[i];
        builder.push_back(toupper(c));
    }

    return builder;
}

string to_lower(std::string in)
{
    string builder("");
    for (int i = 0; i < (int)in.length(); i++)
    {
        char c = in[i];
        builder.push_back(tolower(c));
    }

    return builder;
}

bool contains(std::string s1, std::string s2)
{
    return s1.find(s2) != string::npos;
}

bool has_params(std::string s)
{
    int sl = s.length();

    for (int i = 0; i < sl; i++)
    {
        if (s[i] == '(')
            return true;
    }

    return false;
}

bool has_brackets(std::string s)
{
    int sl = s.length();

    for (int i = 0; i < sl; i++)
    {
        if (s[i] == '[')
            return true;
    }

    return false;
}

vector<std::string> parse_params(std::string s)
{
    std::vector<std::string> params;

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

    return params;
}

vector<std::string> parse_bracketrange(std::string s)
{
    std::vector<std::string> params;

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
            {
            }
            else
                new_name.push_back(s[i]);
        }
        else if (s[i] == '[')
            start_push = true;
    }

    params.push_back(new_name);

    return params;
}

vector<std::string> parse_range(std::string s)
{
    std::vector<std::string> params;

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
            else if (s[i] != ' ')
                new_name.push_back(s[i]);
        }
        else if (s[i] == '(')
            start_push = true;
    }

    params.push_back(new_name);

    return params;
}

string before_params(std::string s)
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

    return new_str;
}

string before_brackets(std::string s)
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

    return new_str;
}

string after_brackets(std::string s)
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

    s = subtract_char(s, ']');

    return var;
}

string after_dot(std::string s)
{
    string var("");
    int sl = s.length();
    bool start_push = false;

    for (int i = 0; i < sl; i++)
    {
        if (start_push)
            var.push_back(s[i]);
        else if (s[i] == '.')
            start_push = true;
    }

    return var;
}

string before_dot(std::string s)
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

    return var;
}

string substring(std::string s, int left, int right)
{
    string inner("");
    int len = s.length();
    if (left > len || right > len)
    {
        // overflow error
    }
    else if (left > right)
    {
        // invalid operation
    }
    else if (left == right)
    {
        inner.push_back(s[left]);
    }
    else
    {
        for (int i = left; i <= right; i++)
        {
            inner.push_back(s[i]);
        }
    }

    return inner;
}

string multiply_string(std::string input, int factor)
{
    string output("");

    for (int i = 0; i < factor; i++)
        output.append(input);

    return output;
}

string subtract_char(std::string s, char c)
{
    string r("");
    int len = s.length();

    for (int i = 0; i < len; i++)
    {
        if (s[i] != c)
            r.push_back(s[i]);
    }

    return r;
}

string subtract_string(std::string s1, std::string s2)
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

    return bs;
}

bool is_alpha(std::string s)
{
    int l = s.length();

    for (int i = 0; i < l; i++)
    {
        if (!isalpha(s[i]))
            return false;
    }

    return true;
}

bool ends_with(std::string s, std::string end)
{
    return s.size() > end.size() && s.substr(s.size() - end.size()) == end;
}

bool begins_with(std::string s, std::string start)
{
    return s.size() > start.size() && s.substr(0, start.size()) == start;
}

bool is_dotless(std::string s)
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

    return none;
}

bool is_numberless(std::string s)
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

bool is_numeric(std::string s)
{
    int l = s.length();
    bool foundDot = false, foundSign = false, foundNonEmpty = false;

    for (int i = 0; i < l; i++)
    {
        if (s[i] == ' ')
        {
            if (foundNonEmpty) 
            {
                return false;
            }
            else 
            {
                continue;
            }
        }

        if (!isdigit(s[i]))
        {
            foundNonEmpty = true;
            switch (s[i])
            {
            case '.':
                if (i == 0)
                    return false;
                else
                {
                    if (!foundDot)
                        foundDot = true;
                    else
                        return false;
                }
                break;

            case '-':
                if (i != 0)
                    return false;
                else
                {
                    if (!foundSign)
                        foundSign = true;
                    else
                        return false;
                }
                break;

            default:
                return false;
                break;
            }
        }
    }

    return true;
}

bool is_truthy(std::string s)
{
    return s == "true" || s == "1";
}

bool is_falsey(std::string s)
{
    return s == "false" || s == "0";
}

string itos(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

string dtos(double i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

int get_ascii_num(char c)
{
    return (int)c;
}

int get_alpha_num(char c)
{
    return tolower(c) - 'a' + 1;
}

bool unrecognized_0space(std::string arg)
{
    return !contains("break;caught;clear_all!;clear_constants!;clear_lists!;clear_methods!;clear_classes!;clear_variables!;else;end;exit;failif;leave!;no_methods?;no_classes?;no_variables?;parser;pass;private;public;try;", arg);
}

bool unrecognized_1space(std::string arg)
{
    return !contains("!;?;__begin__;call_method;cd;clear;chdir;collect?;decode;delay;directory?;dpush;dpop;encode;err;error;file?;for;fpush;fpop;garbage?;globalize;goto;if;init_dir;intial_directory;directory?;file?;list?;method?;number?;class?;string?;variable?;list;list?;load;lock;loop;lose;method;[method];class;out;print;println;prompt;remove;return;say;stdout;switch;template;unlock;", arg);
}

bool unrecognized_2space(std::string arg)
{
    return !contains("=;+=;-=;*=;%=;/=;**=;+;-;*;**;/;%;++=;--=;?;!", arg);
}

bool is(std::string s, std::string si)
{
    return s == ("-" + si) || s == ("--" + si) || s == ("/" + si);
}

bool is_script(std::string path)
{
    return ends_with(path, ".usl") || ends_with(path, ".uslang");
}

#endif
