/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#ifndef DATETIME_H
#define DATETIME_H

#include <ctime>

int secondNow();
int minuteNow();
int hourNow();
int monthNow();
int yearNow();
int dayOfTheMonth();
int dayOfTheYear();

string amOrPm();
string dayOfTheWeek();
string monthOfTheYear();
string timeNow();

int secondNow()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);
	
    return (t->tm_sec + 1);
}

int minuteNow()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);
	
    return (t->tm_min);
}

int hourNow()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);

    if (t->tm_hour <= 11)
    {
        if (t->tm_hour == 0)
            return (12);
        else
            return (t->tm_hour);
    }

    return (t->tm_hour - 12);
}

string amOrPm()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);

    if (t->tm_hour > 11)
        return ("PM");

    return ("AM");
}

string timeNow()
{
    string now("");
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

    return (now);
}

int monthNow()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);

    return (t->tm_mon + 1);
}

int yearNow()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);

    return (t->tm_year + 1900);
}

int dayOfTheMonth()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);

    return (t->tm_mday);
}

int dayOfTheYear()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);

    return (t->tm_yday + 2);
}

string dayOfTheWeek()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);
    string day("");

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
        cout << "defaulted: " << t->tm_mday << endl;
        break;
    }

    return (day);
}

string monthOfTheYear()
{
    time_t currently;
    time(&currently);
    struct tm * t = localtime(&currently);
    string month("");

    switch (t->tm_mon)
    {
    case 0:
        month = "January";
        break;
    case 1:
        month = "February";
        break;
    case 2:
        month = "March";
        break;
    case 3:
        month = "April";
        break;
    case 4:
        month = "May";
        break;
    case 5:
        month = "June";
        break;
    case 6:
        month = "July";
        break;
    case 7:
        month = "August";
        break;
    case 8:
        month = "September";
        break;
    case 9:
        month = "October";
        break;
    case 10:
        month = "November";
        break;
    case 11:
        month = "December";
        break;
    default:
        month = "Unknown";
        break;
    }

    return (month);
}

#endif
