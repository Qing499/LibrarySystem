#include "Library.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

#include "Student.h"
#include "Teacher.h"
#include "TextUtils.h"

namespace {

const int kMaxBorrowUpperBound = 99;

std::string formatMemberLine(const Person& member) {
    return member.toString();
}

}

Library::Library() {}

Library::Library(const BorrowPolicy& policy) : policy_(policy) {}

OperationResult Library::addMember(const std::string& role,
                                   const std::string& id,
                                   const std::string& name,
                                   const std::string& department,
                                   int maxBorrow) {
    OperationResult result;

    const std::string tag = text::trim(role);
    const bool isStudent = text::equalsIgnoreCaseAscii(tag, "STUDENT");
    const bool isTeacher = text::equalsIgnoreCaseAscii(tag, "TEACHER");
    if (!isStudent && !isTeacher) {
        result.message = "角色只能是 STUDENT（学生）或 TEACHER（教师）。";
        return result;
    }

    const std::string trimmedId = text::trim(id);
    const std::string trimmedName = text::trim(name);
    const std::string trimmedDepartment = text::trim(department);
    if (trimmedId.empty() || trimmedName.empty() || trimmedDepartment.empty()) {
        result.message = "编号、姓名、院系都不能为空。";
        return result;
    }
    if (maxBorrow < 1 || maxBorrow > kMaxBorrowUpperBound) {
        std::ostringstream out;
        out << "最大借阅数量必须在 1 ~ " << kMaxBorrowUpperBound << " 之间。";
        result.message = out.str();
        return result;
    }
    if (findMember(trimmedId) != nullptr) {
        result.message = "编号 " + trimmedId + " 已存在，不能重复添加。";
        return result;
    }

    if (isStudent) {
        members_.push_back(std::unique_ptr<Person>(
            new Student(trimmedId, trimmedName, trimmedDepartment, maxBorrow)));
    } else {
        members_.push_back(std::unique_ptr<Person>(
            new Teacher(trimmedId, trimmedName, trimmedDepartment, maxBorrow)));
    }

    result.ok = true;
    result.message = "添加成功：" + members_.back()->toString();
    return result;
}

OperationResult Library::updateMember(const std::string& id,
                                      const std::string& newName,
                                      const std::string& newDepartment,
                                      int newMaxBorrow) {
    OperationResult result;
    Person* member = findMember(text::trim(id));
    if (member == nullptr) {
        result.message = "借阅人不存在（编号: " + text::trim(id) + "）。";
        return result;
    }

    const std::string trimmedName = text::trim(newName);
    const std::string trimmedDepartment = text::trim(newDepartment);
    if (trimmedName.empty() || trimmedDepartment.empty()) {
        result.message = "姓名和院系都不能为空。";
        return result;
    }
    if (newMaxBorrow < 1 || newMaxBorrow > kMaxBorrowUpperBound) {
        std::ostringstream out;
        out << "最大借阅数量必须在 1 ~ " << kMaxBorrowUpperBound << " 之间。";
        result.message = out.str();
        return result;
    }
    if (newMaxBorrow < member->borrowedCount()) {
        std::ostringstream out;
        out << "该借阅人当前在借 " << member->borrowedCount()
            << " 本，最大借阅数量不能小于这个数。";
        result.message = out.str();
        return result;
    }

    member->setName(trimmedName);
    member->setDepartment(trimmedDepartment);
    member->setMaxBorrow(newMaxBorrow);

    result.ok = true;
    result.message = "修改成功：" + formatMemberLine(*member);
    return result;
}

OperationResult Library::removeMember(const std::string& id) {
    OperationResult result;
    const std::string trimmedId = text::trim(id);
    if (findMember(trimmedId) == nullptr) {
        result.message = "借阅人不存在（编号: " + trimmedId + "）。";
        return result;
    }
    if (!activeRecordsOfMember(trimmedId).empty()) {
        result.message = "该借阅人还有未归还的图书，请先办理归还再删除。";
        return result;
    }

    for (std::size_t i = 0; i < members_.size(); ++i) {
        if (members_[i]->id() == trimmedId) {
            members_.erase(members_.begin() + static_cast<std::ptrdiff_t>(i));
            result.ok = true;
            result.message = "已删除借阅人（编号: " + trimmedId + "）。";
            return result;
        }
    }
    result.message = "删除失败：内部数据异常。";
    return result;
}

Person* Library::findMember(const std::string& id) {
    const std::string trimmedId = text::trim(id);
    for (std::size_t i = 0; i < members_.size(); ++i) {
        if (members_[i]->id() == trimmedId) {
            return members_[i].get();
        }
    }
    return nullptr;
}

const Person* Library::findMember(const std::string& id) const {
    const std::string trimmedId = text::trim(id);
    for (std::size_t i = 0; i < members_.size(); ++i) {
        if (members_[i]->id() == trimmedId) {
            return members_[i].get();
        }
    }
    return nullptr;
}

std::vector<const Person*> Library::searchMembers(const std::string& keyword,
                                                  const std::string& role) const {
    std::vector<const Person*> found;
    const std::string key = text::trim(keyword);
    const std::string roleTag = text::trim(role);

    for (std::size_t i = 0; i < members_.size(); ++i) {
        const Person& member = *members_[i];
        if (!roleTag.empty() && !text::equalsIgnoreCaseAscii(member.typeTag(), roleTag)) {
            continue;
        }
        if (!key.empty() &&
            !text::containsIgnoreCaseAscii(member.id(), key) &&
            !text::containsIgnoreCaseAscii(member.name(), key) &&
            !text::containsIgnoreCaseAscii(member.department(), key)) {
            continue;
        }
        found.push_back(&member);
    }
    return found;
}

bool Library::validateBookFields(const Book& book, std::string& error) {
    if (text::trim(book.barcode()).empty()) {
        error = "图书条码号不能为空。";
        return false;
    }
    if (text::trim(book.isbn()).empty()) {
        error = "图书号 ISBN 不能为空。";
        return false;
    }
    if (text::trim(book.title()).empty()) {
        error = "书名不能为空。";
        return false;
    }
    if (text::trim(book.author()).empty()) {
        error = "作者不能为空。";
        return false;
    }
    if (text::trim(book.publisher()).empty()) {
        error = "出版社不能为空。";
        return false;
    }
    return true;
}

OperationResult Library::addBook(const Book& book) {
    OperationResult result;

    std::string error;
    if (!validateBookFields(book, error)) {
        result.message = error;
        return result;
    }
    const std::string barcode = text::trim(book.barcode());
    if (findBook(barcode) != nullptr) {
        result.message = "条码号 " + barcode + " 已存在，不能重复添加。";
        return result;
    }

    Book created = book;
    created.setBarcode(barcode);
    created.setIsbn(text::trim(book.isbn()));
    created.setTitle(text::trim(book.title()));
    created.setAuthor(text::trim(book.author()));
    created.setPublisher(text::trim(book.publisher()));
    created.setAvailable(true);

    books_.push_back(created);
    result.ok = true;
    result.message = "入库成功：" + books_.back().toString();
    return result;
}

OperationResult Library::updateBook(const std::string& barcode, const Book& values) {
    OperationResult result;
    Book* book = findBook(text::trim(barcode));
    if (book == nullptr) {
        result.message = "图书不存在（条码号: " + text::trim(barcode) + "）。";
        return result;
    }

    std::string error;
    if (!validateBookFields(values, error)) {
        result.message = error;
        return result;
    }

    book->setIsbn(text::trim(values.isbn()));
    book->setTitle(text::trim(values.title()));
    book->setAuthor(text::trim(values.author()));
    book->setPublisher(text::trim(values.publisher()));

    result.ok = true;
    result.message = "修改成功：" + book->toString();
    return result;
}

OperationResult Library::removeBook(const std::string& barcode) {
    OperationResult result;
    const std::string trimmedBarcode = text::trim(barcode);
    if (findBook(trimmedBarcode) == nullptr) {
        result.message = "图书不存在（条码号: " + trimmedBarcode + "）。";
        return result;
    }

    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (!records_[i].returned() && records_[i].barcode() == trimmedBarcode) {
            result.message = "该图书尚未归还（借阅人: " + records_[i].borrowerId() + "），不能删除。";
            return result;
        }
    }

    for (std::size_t i = 0; i < books_.size(); ++i) {
        if (books_[i].barcode() == trimmedBarcode) {
            books_.erase(books_.begin() + static_cast<std::ptrdiff_t>(i));
            result.ok = true;
            result.message = "已删除图书（条码号: " + trimmedBarcode + "），历史借阅记录保留。";
            return result;
        }
    }
    result.message = "删除失败：内部数据异常。";
    return result;
}

Book* Library::findBook(const std::string& barcode) {
    const std::string trimmedBarcode = text::trim(barcode);
    for (std::size_t i = 0; i < books_.size(); ++i) {
        if (books_[i].barcode() == trimmedBarcode) {
            return &books_[i];
        }
    }
    return nullptr;
}

const Book* Library::findBook(const std::string& barcode) const {
    const std::string trimmedBarcode = text::trim(barcode);
    for (std::size_t i = 0; i < books_.size(); ++i) {
        if (books_[i].barcode() == trimmedBarcode) {
            return &books_[i];
        }
    }
    return nullptr;
}

std::vector<const Book*> Library::searchBooks(const std::string& keyword) const {
    std::vector<const Book*> found;
    for (std::size_t i = 0; i < books_.size(); ++i) {
        if (books_[i].matches(keyword)) {
            found.push_back(&books_[i]);
        }
    }
    return found;
}

BorrowRecord* Library::findActiveRecord(const std::string& memberId, const std::string& barcode) {
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (!records_[i].returned() &&
            records_[i].borrowerId() == memberId &&
            records_[i].barcode() == barcode) {
            return &records_[i];
        }
    }
    return nullptr;
}

const BorrowRecord* Library::findActiveRecord(const std::string& memberId,
                                              const std::string& barcode) const {
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (!records_[i].returned() &&
            records_[i].borrowerId() == memberId &&
            records_[i].barcode() == barcode) {
            return &records_[i];
        }
    }
    return nullptr;
}

OperationResult Library::borrowBook(const std::string& memberId,
                                    const std::string& barcode,
                                    const Date& date) {
    OperationResult result;

    if (!date.isValid()) {
        result.message = "借书日期无效。";
        return result;
    }

    Person* member = findMember(memberId);
    if (member == nullptr) {
        result.message = "借阅人不存在（编号: " + text::trim(memberId) + "）。";
        return result;
    }

    Book* book = findBook(barcode);
    if (book == nullptr) {
        result.message = "图书不存在（条码号: " + text::trim(barcode) + "）。";
        return result;
    }

    if (!book->available()) {
        result.message = "图书《" + book->title() + "》当前已借出，无法借阅。";
        return result;
    }

    const int borrowDays = member->borrowDays(policy_);
    if (borrowDays < 1) {
        result.message = "借期参数不合法（小于 1 天），请先在参数设置中修改。";
        return result;
    }

    if (member->borrowedCount() >= member->maxBorrow()) {
        std::ostringstream out;
        out << "已达到最大借阅数量（" << member->maxBorrow() << " 本），请先归还后再借。";
        result.message = out.str();
        return result;
    }

    for (std::size_t i = 0; i < records_.size(); ++i) {
        const BorrowRecord& record = records_[i];
        if (!record.returned() && record.borrowerId() == member->id() && record.isOverdue(date)) {
            std::ostringstream out;
            out << "该借阅人有逾期未还图书（条码号: " << record.barcode()
                << "，应还日期 " << record.dueDate().toString()
                << "，已逾期 " << record.overdueDaysAsOf(date) << " 天），请先归还。";
            result.message = out.str();
            return result;
        }
    }

    const Date dueDate = date.addDays(borrowDays);
    if (!dueDate.isValid()) {
        result.message = "计算应还日期失败，请检查借期参数。";
        return result;
    }

    records_.push_back(BorrowRecord(member->id(), book->barcode(), date, dueDate));
    book->setAvailable(false);
    member->setBorrowedCount(member->borrowedCount() + 1);

    std::ostringstream out;
    out << "借阅成功：" << member->roleName() << " " << member->name()
        << "（" << member->idLabel() << " " << member->id() << "）"
        << " 借出《" << book->title() << "》（条码号 " << book->barcode() << "），"
        << "借出日期 " << date.toString() << "，应还日期 " << dueDate.toString()
        << "，借期 " << borrowDays << " 天。";
    result.ok = true;
    result.message = out.str();
    return result;
}

OperationResult Library::returnBook(const std::string& memberId,
                                    const std::string& barcode,
                                    const Date& date) {
    OperationResult result;

    if (!date.isValid()) {
        result.message = "归还日期无效。";
        return result;
    }

    Person* member = findMember(memberId);
    if (member == nullptr) {
        result.message = "借阅人不存在（编号: " + text::trim(memberId) + "）。";
        return result;
    }

    Book* book = findBook(barcode);
    if (book == nullptr) {
        result.message = "图书不存在（条码号: " + text::trim(barcode) + "）。";
        return result;
    }

    BorrowRecord* record = findActiveRecord(member->id(), book->barcode());
    if (record == nullptr) {
        result.message = "没有找到该借阅人在借的这本书（条码号: " + book->barcode() + "）。";
        return result;
    }

    if (date < record->borrowDate()) {
        result.message = "归还日期 " + date.toString() +
                         " 不能早于借出日期 " + record->borrowDate().toString() + "。";
        return result;
    }

    const long long overdueDays = record->overdueDaysAsOf(date);
    const double finePerDay = member->finePerDay(policy_);
    const double fine = roundMoney(static_cast<double>(overdueDays) * finePerDay);

    record->markReturned(date, fine, overdueDays);
    book->setAvailable(true);
    if (member->borrowedCount() > 0) {
        member->setBorrowedCount(member->borrowedCount() - 1);
    }

    std::ostringstream out;
    out << "归还成功：" << member->roleName() << " " << member->name()
        << " 归还《" << book->title() << "》，应还日期 " << record->dueDate().toString()
        << "，实际归还 " << date.toString() << "。";
    if (overdueDays > 0) {
        out << " 已逾期 " << overdueDays << " 天，罚款 "
            << formatMoney(fine) << " 元（" << formatMoney(finePerDay) << " 元/天）。";
    } else {
        out << " 按期归还，无罚款。";
    }

    result.ok = true;
    result.fine = fine;
    result.overdueDays = overdueDays;
    result.message = out.str();
    return result;
}

OperationResult Library::returnBookByBarcode(const std::string& barcode, const Date& date) {
    OperationResult result;
    const std::string trimmedBarcode = text::trim(barcode);
    if (findBook(trimmedBarcode) == nullptr) {
        result.message = "图书不存在（条码号: " + trimmedBarcode + "）。";
        return result;
    }

    const BorrowRecord* record = nullptr;
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (!records_[i].returned() && records_[i].barcode() == trimmedBarcode) {
            if (record != nullptr) {
                result.message = "该条码号存在多条未归还记录，数据异常，请用“按借阅人 + 条码号”方式归还。";
                return result;
            }
            record = &records_[i];
        }
    }
    if (record == nullptr) {
        result.message = "该图书当前没有未归还的借阅记录（条码号: " + trimmedBarcode + "）。";
        return result;
    }

    const std::string borrowerId = record->borrowerId();
    return returnBook(borrowerId, trimmedBarcode, date);
}

std::vector<const BorrowRecord*> Library::recordsOfMember(const std::string& memberId) const {
    std::vector<const BorrowRecord*> found;
    const std::string trimmedId = text::trim(memberId);
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (records_[i].borrowerId() == trimmedId) {
            found.push_back(&records_[i]);
        }
    }
    return found;
}

std::vector<const BorrowRecord*> Library::activeRecordsOfMember(const std::string& memberId) const {
    std::vector<const BorrowRecord*> found;
    const std::string trimmedId = text::trim(memberId);
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (!records_[i].returned() && records_[i].borrowerId() == trimmedId) {
            found.push_back(&records_[i]);
        }
    }
    return found;
}

std::vector<const BorrowRecord*> Library::overdueRecords(const Date& reference) const {
    std::vector<const BorrowRecord*> found;
    if (!reference.isValid()) {
        return found;
    }
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (!records_[i].returned() && records_[i].isOverdue(reference)) {
            found.push_back(&records_[i]);
        }
    }
    return found;
}

double Library::outstandingFineOf(const std::string& memberId, const Date& reference) const {
    const Person* member = findMember(memberId);
    if (member == nullptr) {
        return 0.0;
    }
    double total = 0.0;
    const double finePerDay = member->finePerDay(policy_);
    const std::vector<const BorrowRecord*> active = activeRecordsOfMember(member->id());
    for (std::size_t i = 0; i < active.size(); ++i) {
        total += static_cast<double>(active[i]->overdueDaysAsOf(reference)) * finePerDay;
    }
    return roundMoney(total);
}

double Library::totalOutstandingFine(const Date& reference) const {
    double total = 0.0;
    for (std::size_t i = 0; i < members_.size(); ++i) {
        total += outstandingFineOf(members_[i]->id(), reference);
    }
    return roundMoney(total);
}

double Library::totalCollectedFine() const {
    double total = 0.0;
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (records_[i].returned()) {
            total += records_[i].fine();
        }
    }
    return roundMoney(total);
}

std::string Library::statisticsReport(const Date& reference) const {
    std::size_t studentCount = 0;
    std::size_t teacherCount = 0;
    for (std::size_t i = 0; i < members_.size(); ++i) {
        if (members_[i]->typeTag() == "STUDENT") {
            ++studentCount;
        } else {
            ++teacherCount;
        }
    }

    std::size_t availableCount = 0;
    for (std::size_t i = 0; i < books_.size(); ++i) {
        if (books_[i].available()) {
            ++availableCount;
        }
    }

    std::size_t returnedCount = 0;
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (records_[i].returned()) {
            ++returnedCount;
        }
    }
    const std::size_t activeCount = records_.size() - returnedCount;
    const std::vector<const BorrowRecord*> overdue = overdueRecords(reference);

    std::ostringstream out;
    out << "统计基准日期: " << reference.toString() << "\n";
    out << "借阅人: 共 " << members_.size() << " 人（学生 " << studentCount
        << " 人，教师 " << teacherCount << " 人）\n";
    out << "馆藏图书: 共 " << books_.size() << " 种（可借 " << availableCount
        << " 种，已借出 " << (books_.size() - availableCount) << " 种）\n";
    out << "借阅记录: 共 " << records_.size() << " 条（在借 " << activeCount
        << " 条，已归还 " << returnedCount << " 条）\n";
    out << "逾期未还: " << overdue.size() << " 条，预计罚款合计 "
        << formatMoney(totalOutstandingFine(reference)) << " 元\n";
    out << "已收罚款合计: " << formatMoney(totalCollectedFine()) << " 元";
    return out.str();
}

std::string Library::defaultDataFile() {
    return std::string("library_data.txt");
}

int Library::refreshDerivedState() {
    std::map<std::string, int> activeCountByMember;
    std::set<std::string> borrowedBarcodes;
    for (std::size_t i = 0; i < records_.size(); ++i) {
        if (!records_[i].returned()) {
            ++activeCountByMember[records_[i].borrowerId()];
            borrowedBarcodes.insert(records_[i].barcode());
        }
    }

    int corrections = 0;
    for (std::size_t i = 0; i < members_.size(); ++i) {
        const std::map<std::string, int>::const_iterator it = activeCountByMember.find(members_[i]->id());
        const int expected = (it == activeCountByMember.end()) ? 0 : it->second;
        if (members_[i]->borrowedCount() != expected) {
            members_[i]->setBorrowedCount(expected);
            ++corrections;
        }
    }
    for (std::size_t i = 0; i < books_.size(); ++i) {
        const bool expected = borrowedBarcodes.find(books_[i].barcode()) == borrowedBarcodes.end();
        if (books_[i].available() != expected) {
            books_[i].setAvailable(expected);
            ++corrections;
        }
    }
    return corrections;
}

bool Library::saveToFile(const std::string& path, std::string& message) const {
    std::ofstream out(path.c_str(), std::ios::out | std::ios::trunc);
    if (!out) {
        message = "无法写入文件：" + path;
        return false;
    }

    out << "# 图书馆借阅管理系统 数据文件 v1\n";
    out << "# POLICY|学生最大借阅|教师最大借阅|学生借期|教师借期|学生罚款单价|教师罚款单价\n";
    out << "# MEMBER|角色|编号|姓名|院系|最大借阅|当前在借\n";
    out << "# BOOK|条码号|ISBN|书名|作者|出版社|是否可借\n";
    out << "# RECORD|借阅人编号|条码号|借出日期|应还日期|已归还|归还日期|逾期天数|罚款\n";

    out << "POLICY|" << policy_.studentMaxBorrow
        << '|' << policy_.teacherMaxBorrow
        << '|' << policy_.studentBorrowDays
        << '|' << policy_.teacherBorrowDays
        << '|' << formatMoney(policy_.studentFinePerDay)
        << '|' << formatMoney(policy_.teacherFinePerDay) << '\n';

    for (std::size_t i = 0; i < members_.size(); ++i) {
        const Person& member = *members_[i];
        out << "MEMBER|" << member.typeTag()
            << '|' << text::sanitizeField(member.id())
            << '|' << text::sanitizeField(member.name())
            << '|' << text::sanitizeField(member.department())
            << '|' << member.maxBorrow()
            << '|' << member.borrowedCount() << '\n';
    }

    for (std::size_t i = 0; i < books_.size(); ++i) {
        const Book& book = books_[i];
        out << "BOOK|" << text::sanitizeField(book.barcode())
            << '|' << text::sanitizeField(book.isbn())
            << '|' << text::sanitizeField(book.title())
            << '|' << text::sanitizeField(book.author())
            << '|' << text::sanitizeField(book.publisher())
            << '|' << (book.available() ? 1 : 0) << '\n';
    }

    for (std::size_t i = 0; i < records_.size(); ++i) {
        out << "RECORD|" << records_[i].serialize() << '\n';
    }

    if (!out.good()) {
        message = "写入文件过程中发生错误：" + path;
        return false;
    }
    out.close();

    std::ostringstream info;
    info << "数据已保存到 " << path << "（借阅人 " << members_.size()
         << " 人，图书 " << books_.size() << " 种，借阅记录 " << records_.size() << " 条）。";
    message = info.str();
    return true;
}

bool Library::loadFromFile(const std::string& path, std::string& message) {
    std::ifstream in(path.c_str(), std::ios::in);
    if (!in) {
        message = "无法打开文件：" + path;
        return false;
    }

    std::vector<std::unique_ptr<Person>> newMembers;
    std::vector<Book> newBooks;
    std::vector<BorrowRecord> newRecords;
    BorrowPolicy newPolicy = policy_;

    std::size_t lineNumber = 0;
    std::size_t skippedLines = 0;
    std::string line;
    while (std::getline(in, line)) {
        ++lineNumber;
        const std::string trimmed = text::trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }

        const std::vector<std::string> fields = text::split(trimmed, '|');
        const std::string& tag = fields[0];

        if (tag == "POLICY") {
            if (fields.size() != 7) {
                ++skippedLines;
                continue;
            }
            long long values[4] = {0, 0, 0, 0};
            double fines[2] = {0.0, 0.0};
            bool ok = text::parseInt(fields[1], values[0]) && text::parseInt(fields[2], values[1]) &&
                      text::parseInt(fields[3], values[2]) && text::parseInt(fields[4], values[3]) &&
                      text::parseDouble(fields[5], fines[0]) && text::parseDouble(fields[6], fines[1]);
            if (!ok || values[0] < 1 || values[1] < 1 || values[2] < 1 || values[3] < 1 ||
                fines[0] < 0.0 || fines[1] < 0.0) {
                ++skippedLines;
                continue;
            }
            newPolicy.studentMaxBorrow = static_cast<int>(values[0]);
            newPolicy.teacherMaxBorrow = static_cast<int>(values[1]);
            newPolicy.studentBorrowDays = static_cast<int>(values[2]);
            newPolicy.teacherBorrowDays = static_cast<int>(values[3]);
            newPolicy.studentFinePerDay = roundMoney(fines[0]);
            newPolicy.teacherFinePerDay = roundMoney(fines[1]);
            continue;
        }

        if (tag == "MEMBER") {
            if (fields.size() != 7) {
                ++skippedLines;
                continue;
            }
            const std::string role = text::trim(fields[1]);
            const std::string id = text::trim(fields[2]);
            const std::string name = text::trim(fields[3]);
            const std::string department = text::trim(fields[4]);
            long long maxBorrow = 0;
            if (id.empty() || name.empty() || department.empty() ||
                !text::parseInt(fields[5], maxBorrow) ||
                maxBorrow < 1 || maxBorrow > kMaxBorrowUpperBound) {
                ++skippedLines;
                continue;
            }

            long long borrowedCount = 0;
            if (!text::parseInt(fields[6], borrowedCount) || borrowedCount < 0) {
                borrowedCount = 0;
            }
            bool duplicated = false;
            for (std::size_t i = 0; i < newMembers.size(); ++i) {
                if (newMembers[i]->id() == id) {
                    duplicated = true;
                    break;
                }
            }
            if (duplicated) {
                ++skippedLines;
                continue;
            }
            if (text::equalsIgnoreCaseAscii(role, "STUDENT")) {
                newMembers.push_back(std::unique_ptr<Person>(
                    new Student(id, name, department, static_cast<int>(maxBorrow))));
            } else if (text::equalsIgnoreCaseAscii(role, "TEACHER")) {
                newMembers.push_back(std::unique_ptr<Person>(
                    new Teacher(id, name, department, static_cast<int>(maxBorrow))));
            } else {
                ++skippedLines;
                continue;
            }
            newMembers.back()->setBorrowedCount(static_cast<int>(borrowedCount));
            continue;
        }

        if (tag == "BOOK") {
            if (fields.size() != 7) {
                ++skippedLines;
                continue;
            }
            const std::string barcode = text::trim(fields[1]);
            bool duplicated = false;
            for (std::size_t i = 0; i < newBooks.size(); ++i) {
                if (newBooks[i].barcode() == barcode) {
                    duplicated = true;
                    break;
                }
            }
            Book book(barcode, text::trim(fields[2]), text::trim(fields[3]),
                      text::trim(fields[4]), text::trim(fields[5]),
                      text::trim(fields[6]) != "0");
            std::string error;
            if (duplicated || !validateBookFields(book, error)) {
                ++skippedLines;
                continue;
            }
            newBooks.push_back(book);
            continue;
        }

        if (tag == "RECORD") {
            std::string recordLine = trimmed.substr(tag.size() + 1);
            BorrowRecord record;
            if (!BorrowRecord::deserialize(recordLine, record)) {
                ++skippedLines;
                continue;
            }
            bool memberExists = false;
            for (std::size_t i = 0; i < newMembers.size(); ++i) {
                if (newMembers[i]->id() == record.borrowerId()) {
                    memberExists = true;
                    break;
                }
            }
            bool bookExists = false;
            for (std::size_t i = 0; i < newBooks.size(); ++i) {
                if (newBooks[i].barcode() == record.barcode()) {
                    bookExists = true;
                    break;
                }
            }
            bool conflicting = false;
            if (!record.returned()) {
                for (std::size_t i = 0; i < newRecords.size(); ++i) {
                    if (!newRecords[i].returned() &&
                        newRecords[i].borrowerId() == record.borrowerId() &&
                        newRecords[i].barcode() == record.barcode()) {
                        conflicting = true;
                        break;
                    }
                }
            }
            if (!memberExists || !bookExists || conflicting) {
                ++skippedLines;
                continue;
            }
            newRecords.push_back(record);
            continue;
        }

        ++skippedLines;
    }

    if (newMembers.empty() && newBooks.empty() && newRecords.empty()) {
        message = "文件 " + path + " 中没有可识别的数据，已保持原有数据不变。";
        return false;
    }

    members_.swap(newMembers);
    books_.swap(newBooks);
    records_.swap(newRecords);
    policy_ = newPolicy;

    const int corrections = refreshDerivedState();

    std::ostringstream info;
    info << "已从 " << path << " 读取数据（借阅人 " << members_.size()
         << " 人，图书 " << books_.size() << " 种，借阅记录 " << records_.size() << " 条）";
    if (skippedLines > 0) {
        info << "，忽略无效行 " << skippedLines << " 行";
    }
    if (corrections > 0) {
        info << "，按借阅记录修正在借状态 " << corrections << " 处";
    }
    info << "。";
    message = info.str();
    return true;
}

void Library::seedDemoData() {
    addMember("STUDENT", "20210001", "张三", "计算机科学与技术学院", policy_.studentMaxBorrow);
    addMember("STUDENT", "20210002", "李四", "计算机科学与技术学院", policy_.studentMaxBorrow);
    addMember("STUDENT", "20220003", "王五", "信息工程学院", policy_.studentMaxBorrow);
    addMember("TEACHER", "T1001", "赵老师", "计算机科学与技术学院", policy_.teacherMaxBorrow);
    addMember("TEACHER", "T1002", "钱老师", "信息工程学院", policy_.teacherMaxBorrow);

    addBook(Book("BC-0001", "978-7-111-40701-0", "C++ Primer（第5版）", "Stanley B. Lippman", "机械工业出版社"));
    addBook(Book("BC-0002", "978-7-115-48230-7", "C++程序设计教程", "钱能", "清华大学出版社"));
    addBook(Book("BC-0003", "978-7-302-52056-7", "数据结构（C++语言版）", "邓俊辉", "清华大学出版社"));
    addBook(Book("BC-0004", "978-7-121-31830-4", "Effective Modern C++", "Scott Meyers", "电子工业出版社"));
    addBook(Book("BC-0005", "978-7-111-60304-7", "操作系统概念", "Abraham Silberschatz", "机械工业出版社"));
    addBook(Book("BC-0006", "978-7-115-44663-3", "算法（第4版）", "Robert Sedgewick", "人民邮电出版社"));
}
