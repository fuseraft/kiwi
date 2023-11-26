class Crypt
{
public:
    Crypt() {}
    ~Crypt() {}

    std::string d(std::string o)
    {
        return decode(o);
    }

    std::string e(std::string o)
    {
        return encode(o);
    }

private:
    std::string decode(std::string o)
    {
        int l = o.length(), s = 7;
        std::string ax("");

        for (int i = 0; i < l; i++)
        {
            if (s == 7)
            {
                ax.push_back(((char)((int)o[i] + 3)));
                s = 5;
            }
            else if (s == 5)
            {
                ax.push_back(((char)((int)o[i] - 1)));
                s = 0;
            }
            else if (s == 0)
            {
                ax.push_back(((char)((int)o[i] + 4)));
                s = 1;
            }
            else
            {
                ax.push_back(((char)((int)o[i] - 2)));
                s = 7;
            }
        }

        return ax;
    }

    std::string encode(std::string o)
    {
        int l = o.length(), s = 7;
        std::string ax("");

        for (int i = 0; i < l; i++)
        {
            if (s == 7)
            {
                ax.push_back(((char)((int)o[i] - 3)));
                s = 5;
            }
            else if (s == 5)
            {
                ax.push_back(((char)((int)o[i] + 1)));
                s = 0;
            }
            else if (s == 0)
            {
                ax.push_back(((char)((int)o[i] - 4)));
                s = 1;
            }
            else
            {
                ax.push_back(((char)((int)o[i] + 2)));
                s = 7;
            }
        }

        return ax;
    }
};