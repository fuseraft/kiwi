#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

class Date {
  private:
    std::tm dateInfo;

  public:
    Date() {
        std::time_t now = std::time(0);
        dateInfo = *std::localtime(&now);
    }

    int getYear() const { return dateInfo.tm_year + 1900; }

    int getMonth() const { return dateInfo.tm_mon + 1; }

    int getDay() const { return dateInfo.tm_mday; }

    void setDate(int year, int month, int day) {
        dateInfo.tm_year = year - 1900;
        dateInfo.tm_mon = month - 1;
        dateInfo.tm_mday = day;
        std::mktime(&dateInfo);
    }

    std::string toString(const std::string &format) const {
        std::stringstream ss;
        ss << std::setfill('0');

        for (char c : format) {
            switch (c) {
            case 'Y':
                ss << std::setw(4) << getYear();
                break;
            case 'M':
                ss << std::setw(2) << getMonth();
                break;
            case 'D':
                ss << std::setw(2) << getDay();
                break;
            default:
                ss << c;
            }
        }

        return ss.str();
    }
};