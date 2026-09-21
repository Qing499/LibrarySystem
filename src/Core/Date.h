#pragma once

#include <string>

class Date {
public:

    Date();

    Date(int year, int month, int day);

    static Date today();
    static bool tryParse(const std::string& text, Date& result);

    int year() const { return year_; }
    int month() const { return month_; }
    int day() const { return day_; }

    bool isValid() const;
    std::string toString() const;

    long long daysSince(const Date& other) const;
    Date addDays(long long days) const;

    bool operator==(const Date& other) const;
    bool operator!=(const Date& other) const;
    bool operator<(const Date& other) const;
    bool operator<=(const Date& other) const;
    bool operator>(const Date& other) const;
    bool operator>=(const Date& other) const;

    static bool isLeapYear(int year);
    static int daysInMonth(int year, int month);

private:
    long long serial() const { return toSerial(year_, month_, day_); }
    static long long toSerial(int year, int month, int day);
    static void fromSerial(long long serial, int& year, int& month, int& day);

    int year_ = 0;
    int month_ = 0;
    int day_ = 0;
};
