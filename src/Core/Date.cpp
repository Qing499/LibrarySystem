#include "Date.h"

#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

#include "TextUtils.h"

namespace {

const int kMinYear = 1900;
const int kMaxYear = 2999;

}

Date::Date() : year_(0), month_(0), day_(0) {}

Date::Date(int year, int month, int day) : year_(year), month_(month), day_(day) {
    if (!isValid()) {
        year_ = 0;
        month_ = 0;
        day_ = 0;
    }
}

bool Date::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int Date::daysInMonth(int year, int month) {
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return kDays[month - 1];
}

bool Date::isValid() const {
    if (year_ < kMinYear || year_ > kMaxYear) {
        return false;
    }
    if (month_ < 1 || month_ > 12) {
        return false;
    }
    if (day_ < 1 || day_ > daysInMonth(year_, month_)) {
        return false;
    }
    return true;
}

Date Date::today() {
    const std::time_t now = std::time(nullptr);
    std::tm local = std::tm();
#if defined(_WIN32)

    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    return Date(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
}

bool Date::tryParse(const std::string& text, Date& result) {
    std::string value = text::trim(text);
    if (value.empty()) {
        return false;
    }
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '/' || value[i] == '.') {
            value[i] = '-';
        }
    }

    long long year = 0;
    long long month = 0;
    long long day = 0;

    if (value.find('-') == std::string::npos) {

        if (value.size() != 8) {
            return false;
        }
        for (std::size_t i = 0; i < value.size(); ++i) {
            if (std::isdigit(static_cast<unsigned char>(value[i])) == 0) {
                return false;
            }
        }
        if (!text::parseInt(value.substr(0, 4), year) ||
            !text::parseInt(value.substr(4, 2), month) ||
            !text::parseInt(value.substr(6, 2), day)) {
            return false;
        }
    } else {
        const std::vector<std::string> parts = text::split(value, '-');
        if (parts.size() != 3) {
            return false;
        }
        if (!text::parseInt(parts[0], year) ||
            !text::parseInt(parts[1], month) ||
            !text::parseInt(parts[2], day)) {
            return false;
        }
    }

    const Date candidate(static_cast<int>(year), static_cast<int>(month), static_cast<int>(day));
    if (!candidate.isValid()) {
        return false;
    }
    result = candidate;
    return true;
}

std::string Date::toString() const {
    if (!isValid()) {
        return std::string("-");
    }
    std::ostringstream out;
    out << std::setfill('0') << std::setw(4) << year_ << '-'
        << std::setw(2) << month_ << '-' << std::setw(2) << day_;
    return out.str();
}

long long Date::toSerial(int year, int month, int day) {
    year -= (month <= 2) ? 1 : 0;
    const long long era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yearOfEra = static_cast<unsigned>(year - era * 400);
    const unsigned dayOfYear =
        (153u * static_cast<unsigned>(month + (month > 2 ? -3 : 9)) + 2u) / 5u + static_cast<unsigned>(day) - 1u;
    const unsigned dayOfEra =
        yearOfEra * 365u + yearOfEra / 4u - yearOfEra / 100u + dayOfYear;
    return era * 146097LL + static_cast<long long>(dayOfEra) - 719468LL;
}

void Date::fromSerial(long long serial, int& year, int& month, int& day) {
    serial += 719468;
    const long long era = (serial >= 0 ? serial : serial - 146096) / 146097;
    const unsigned dayOfEra = static_cast<unsigned>(serial - era * 146097);
    const unsigned yearOfEra =
        (dayOfEra - dayOfEra / 1460u + dayOfEra / 36524u - dayOfEra / 146096u) / 365u;
    const long long yearOfEraValue = static_cast<long long>(yearOfEra) + era * 400;
    const unsigned dayOfYear = dayOfEra - (365u * yearOfEra + yearOfEra / 4u - yearOfEra / 100u);
    const unsigned monthPrime = (5u * dayOfYear + 2u) / 153u;

    const int monthIndex = static_cast<int>(monthPrime);
    day = static_cast<int>(dayOfYear - (153u * monthPrime + 2u) / 5u + 1u);
    month = monthIndex + (monthIndex < 10 ? 3 : -9);
    year = static_cast<int>(yearOfEraValue + (month <= 2 ? 1 : 0));
}

long long Date::daysSince(const Date& other) const {
    return serial() - other.serial();
}

Date Date::addDays(long long days) const {
    int year = 0;
    int month = 0;
    int day = 0;
    fromSerial(serial() + days, year, month, day);
    return Date(year, month, day);
}

bool Date::operator==(const Date& other) const { return serial() == other.serial(); }
bool Date::operator!=(const Date& other) const { return serial() != other.serial(); }
bool Date::operator<(const Date& other) const { return serial() < other.serial(); }
bool Date::operator<=(const Date& other) const { return serial() <= other.serial(); }
bool Date::operator>(const Date& other) const { return serial() > other.serial(); }
bool Date::operator>=(const Date& other) const { return serial() >= other.serial(); }
