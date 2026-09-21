#include "BorrowRecord.h"

#include <sstream>
#include <vector>

#include "Config.h"
#include "TextUtils.h"

BorrowRecord::BorrowRecord()
    : borrowerId_(),
      barcode_(),
      borrowDate_(),
      dueDate_(),
      returnDate_(),
      returned_(false),
      overdueDays_(0),
      fine_(0.0) {}

BorrowRecord::BorrowRecord(const std::string& borrowerId,
                           const std::string& barcode,
                           const Date& borrowDate,
                           const Date& dueDate)
    : borrowerId_(borrowerId),
      barcode_(barcode),
      borrowDate_(borrowDate),
      dueDate_(dueDate),
      returnDate_(),
      returned_(false),
      overdueDays_(0),
      fine_(0.0) {}

long long BorrowRecord::overdueDaysAsOf(const Date& reference) const {
    if (returned_) {
        return overdueDays_;
    }
    if (!dueDate_.isValid() || !reference.isValid() || reference <= dueDate_) {
        return 0;
    }
    return reference.daysSince(dueDate_);
}

bool BorrowRecord::isOverdue(const Date& reference) const {
    return overdueDaysAsOf(reference) > 0;
}

std::string BorrowRecord::toString() const {
    std::ostringstream out;
    out << "借阅人: " << borrowerId_
        << " | 条码号: " << barcode_
        << " | 借出: " << borrowDate_.toString()
        << " | 应还: " << dueDate_.toString()
        << " | 归还: " << (returned_ ? returnDate_.toString() : std::string("未归还"));
    if (returned_) {
        out << " | 状态: " << (overdueDays_ > 0 ? "已逾期归还" : "按期归还")
            << " | 逾期: " << overdueDays_ << " 天"
            << " | 罚款: " << formatMoney(fine_) << " 元";
    } else {
        out << " | 状态: 在借";
    }
    return out.str();
}

std::string BorrowRecord::serialize() const {
    std::vector<std::string> fields;
    fields.push_back(text::sanitizeField(borrowerId_));
    fields.push_back(text::sanitizeField(barcode_));
    fields.push_back(borrowDate_.toString());
    fields.push_back(dueDate_.toString());
    fields.push_back(returned_ ? "1" : "0");
    fields.push_back(returned_ ? returnDate_.toString() : std::string("-"));
    fields.push_back(std::to_string(overdueDays_));
    fields.push_back(formatMoney(fine_));
    return text::join(fields, '|');
}

bool BorrowRecord::deserialize(const std::string& line, BorrowRecord& record) {
    const std::vector<std::string> fields = text::split(line, '|');
    if (fields.size() != 8) {
        return false;
    }

    BorrowRecord parsed;
    parsed.borrowerId_ = text::trim(fields[0]);
    parsed.barcode_ = text::trim(fields[1]);
    if (parsed.borrowerId_.empty() || parsed.barcode_.empty()) {
        return false;
    }

    if (!Date::tryParse(fields[2], parsed.borrowDate_) ||
        !Date::tryParse(fields[3], parsed.dueDate_)) {
        return false;
    }

    const std::string returnedFlag = text::trim(fields[4]);
    if (returnedFlag == "1") {
        Date returnDate;
        if (!Date::tryParse(fields[5], returnDate)) {
            return false;
        }
        long long overdueDays = 0;
        double fine = 0.0;
        if (!text::parseInt(fields[6], overdueDays) || !text::parseDouble(fields[7], fine)) {
            return false;
        }
        if (overdueDays < 0 || fine < 0.0) {
            return false;
        }
        parsed.markReturned(returnDate, roundMoney(fine), overdueDays);
    } else if (returnedFlag != "0") {
        return false;
    }

    record = parsed;
    return true;
}
