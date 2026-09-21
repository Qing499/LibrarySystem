#pragma once

#include <string>

#include "Date.h"

class BorrowRecord {
public:

    BorrowRecord();

    BorrowRecord(const std::string& borrowerId,
                 const std::string& barcode,
                 const Date& borrowDate,
                 const Date& dueDate);

    const std::string& borrowerId() const { return borrowerId_; }
    const std::string& barcode() const { return barcode_; }
    const Date& borrowDate() const { return borrowDate_; }
    const Date& dueDate() const { return dueDate_; }
    const Date& returnDate() const { return returnDate_; }
    bool returned() const { return returned_; }
    double fine() const { return fine_; }
    long long overdueDays() const { return overdueDays_; }

    void markReturned(const Date& returnDate, double fine, long long overdueDays) {
        returnDate_ = returnDate;
        fine_ = fine;
        overdueDays_ = overdueDays;
        returned_ = true;
    }

    long long overdueDaysAsOf(const Date& reference) const;
    bool isOverdue(const Date& reference) const;

    std::string toString() const;
    std::string serialize() const;
    static bool deserialize(const std::string& line, BorrowRecord& record);

private:
    std::string borrowerId_;
    std::string barcode_;
    Date borrowDate_;
    Date dueDate_;
    Date returnDate_;
    bool returned_ = false;
    long long overdueDays_ = 0;
    double fine_ = 0.0;
};
