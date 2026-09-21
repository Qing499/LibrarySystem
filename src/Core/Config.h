#pragma once

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

struct BorrowPolicy {
    int studentMaxBorrow = 5;
    int teacherMaxBorrow = 10;
    int studentBorrowDays = 30;
    int teacherBorrowDays = 60;
    double studentFinePerDay = 0.20;
    double teacherFinePerDay = 0.10;
};

inline double roundMoney(double value) {
    return std::floor(value * 100.0 + 0.5) / 100.0;
}

inline std::string formatMoney(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}
