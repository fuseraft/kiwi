/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#ifndef FILEIO_H
#define FILEIO_H

void app(string p, string a);
void createFile(string p);
bool directoryExists(string p);
bool fileExists(string p);	

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

void createFile(string p)
{
    ofstream f(p.c_str(), ios::out);

    if (f.is_open())
        f.close();
    else
        cout << "..could not create file: " << p << endl;
}

void app(string p, string a)
{
    ofstream f(p.c_str(), ios::out|ios::app);

    if (!f.is_open())
        cout << "#!=read_fail" << endl;
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

#endif
