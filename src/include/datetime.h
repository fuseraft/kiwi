#ifndef DATETIME_H
#define DATETIME_H

#include <ctime>
#include "io.h"

class DT
{
public:
    static void delay(int seconds)
    {
        clock_t ct;
        ct = clock() + seconds * CLOCKS_PER_SEC;

        while (clock() < ct)
        {
        }
    }

    static int secondNow()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);

        return t->tm_sec + 1;
    }

    static int minuteNow()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);

        return t->tm_min;
    }

    static int hourNow()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);

        if (t->tm_hour <= 11)
        {
            if (t->tm_hour == 0)
                return 12;
            else
                return t->tm_hour;
        }

        return t->tm_hour - 12;
    }

    static std::string amOrPm()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);

        return t->tm_hour > 11 ? "PM" : "AM";
    }

    static std::string timeNow()
    {
        std::string now("");
        now = itos(monthNow()) + "/" + itos(dayOfTheMonth()) + "/" + itos(yearNow()) + " ";

        if (hourNow() < 10)
            now.append("0" + itos(hourNow()));
        else
            now.append(itos(hourNow()));

        now.append(":");

        if (minuteNow() < 10)
            now.append("0" + itos(minuteNow()));
        else
            now.append(itos(minuteNow()));

        now.append(":");

        if (secondNow() < 10)
            now.append("0" + itos(secondNow()));
        else
            now.append(itos(secondNow()));

        now.append(" " + amOrPm());

        return now;
    }

    static int monthNow()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);

        return t->tm_mon + 1;
    }

    static int yearNow()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);

        return t->tm_year + 1900;
    }

    static int dayOfTheMonth()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);

        return t->tm_mday;
    }

    static int dayOfTheYear()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);

        return t->tm_yday + 2;
    }

    static std::string dayOfTheWeek()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);
        std::string day("");

        switch (t->tm_wday)
        {
        case 0:
            day = "Sunday";
            break;
        case 1:
            day = "Monday";
            break;
        case 2:
            day = "Tuesday";
            break;
        case 3:
            day = "Wednesday";
            break;
        case 4:
            day = "Thursday";
            break;
        case 5:
            day = "Friday";
            break;
        case 6:
            day = "Saturday";
            break;
        default:
            writeline("defaulted: " + t->tm_mday);
            break;
        }

        return day;
    }

    static std::string monthOfTheYear()
    {
        time_t currently;
        time(&currently);
        struct tm *t = localtime(&currently);
        std::string month("");

        switch (t->tm_mon)
        {
        case 0:
            return "January";
        case 1:
            return "February";
        case 2:
            return "March";
        case 3:
            return "April";
        case 4:
            return "May";
        case 5:
            return "June";
        case 6:
            return "July";
        case 7:
            return "August";
        case 8:
            return "September";
        case 9:
            return "October";
        case 10:
            return "November";
        case 11:
            return "December";
        default:
            return "Unknown";
        }

        return month;
    }
};

#endif
