#include <vector>
#include <stdexcept>

class StringList
{
private:
    std::vector<std::string> strings;

public:
    StringList() = default;
    ~StringList() = default;

    void clear()
    {
        strings.clear();
    }

    void add(const std::string& line)
    {
        strings.push_back(line);
    }

    const std::vector<std::string>& get() const
    {
        return strings;
    }

    std::string at(int index) const
    {
        if (index < 0 || index >= static_cast<int>(strings.size()))
            throw std::out_of_range("Index out of bounds");

        return strings.at(index);
    }
};
