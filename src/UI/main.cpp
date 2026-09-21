#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ClassDemo.h"
#include "ConsoleUtils.h"
#include "Date.h"
#include "Library.h"
#include "SelfTest.h"
#include "TextUtils.h"

namespace {

const int kMaxBorrowUpperBound = 99;
const int kMaxBorrowDaysUpperBound = 365;
const double kMaxFinePerDay = 100.0;

void report(const OperationResult& result) {
    std::cout << (result.ok ? "  [成功] " : "  [失败] ") << result.message << "\n";
}

void printUsage() {
    std::cout << "图书馆借阅管理系统\n"
                 "用法：\n"
                 "  LibrarySystem            启动菜单（退出时自动保存数据）\n"
                 "  LibrarySystem --selftest 运行自检程序，检查各功能是否正确\n"
                 "  LibrarySystem --demo     演示类与对象（构造函数 / 成员函数 / 多态）\n"
                 "  LibrarySystem --help     显示本帮助\n";
}

void printBanner() {
    console::printSeparator('=');
    std::cout << "                 图书馆借阅管理系统\n";
    std::cout << "         （学生 / 教师信息 · 图书信息 · 借还与逾期罚款）\n";
    console::printSeparator('=');
}

void printMemberDetail(const Library& library, const Person& member) {
    std::cout << "  " << member.toString() << "\n";
    const std::vector<const BorrowRecord*> active = library.activeRecordsOfMember(member.id());
    const Date today = Date::today();
    if (active.empty()) {
        std::cout << "  当前没有在借图书。\n";
    } else {
        std::cout << "  当前在借 " << active.size() << " 本：\n";
        for (std::size_t i = 0; i < active.size(); ++i) {
            const BorrowRecord& record = *active[i];
            const long long overdue = record.overdueDaysAsOf(today);
            const Book* book = library.findBook(record.barcode());
            std::cout << "    " << (i + 1) << ". " << record.barcode()
                      << " 《" << (book != nullptr ? book->title() : std::string("(已下架)")) << "》"
                      << " 应还 " << record.dueDate().toString();
            if (overdue > 0) {
                std::cout << "（已逾期 " << overdue << " 天，预计罚款 "
                          << formatMoney(static_cast<double>(overdue) *
                                         member.finePerDay(library.policy()))
                          << " 元）";
            }
            std::cout << "\n";
        }
        std::cout << "  预计罚款合计：" << formatMoney(library.outstandingFineOf(member.id(), today))
                  << " 元（按 " << today.toString() << " 计算）\n";
    }
}

void addMemberInteractive(Library& library, bool isStudent) {
    const std::string roleLabel = isStudent ? "学生" : "教师";
    const std::string roleTag = isStudent ? "STUDENT" : "TEACHER";
    const std::string idLabel = isStudent ? "学号" : "工号";
    const int defaultMaxBorrow =
        isStudent ? library.policy().studentMaxBorrow : library.policy().teacherMaxBorrow;

    console::printTitle("添加" + roleLabel);
    const std::string id = console::readNonEmpty(idLabel + ": ");
    if (console::inputEnded()) {
        return;
    }
    const std::string name = console::readNonEmpty("姓名: ");
    if (console::inputEnded()) {
        return;
    }
    const std::string department = console::readNonEmpty("院系: ");
    if (console::inputEnded()) {
        return;
    }
    const int maxBorrow = console::readIntOrDefault(
        "最大借阅图书数量", defaultMaxBorrow, 1, kMaxBorrowUpperBound);
    if (console::inputEnded()) {
        return;
    }
    report(library.addMember(roleTag, id, name, department, maxBorrow));
}

void updateMemberInteractive(Library& library) {
    console::printTitle("修改借阅人信息");
    const std::string id = console::readNonEmpty("请输入学号 / 工号: ");
    if (console::inputEnded()) {
        return;
    }
    const Person* found = library.findMember(id);
    if (found == nullptr) {
        std::cout << "  [失败] 借阅人不存在（编号: " << id << "）。\n";
        return;
    }
    std::cout << "  当前信息：" << found->toString() << "\n";

    const std::string name = console::readStringOrDefault("新姓名", found->name());
    const std::string department = console::readStringOrDefault("新院系", found->department());
    const int maxBorrow = console::readIntOrDefault(
        "新的最大借阅图书数量", found->maxBorrow(), 1, kMaxBorrowUpperBound);
    if (console::inputEnded()) {
        return;
    }
    report(library.updateMember(id, name, department, maxBorrow));
}

void removeMemberInteractive(Library& library) {
    console::printTitle("删除借阅人");
    const std::string id = console::readNonEmpty("请输入学号 / 工号: ");
    if (console::inputEnded()) {
        return;
    }
    const Person* found = library.findMember(id);
    if (found == nullptr) {
        std::cout << "  [失败] 借阅人不存在（编号: " << id << "）。\n";
        return;
    }
    std::cout << "  待删除：" << found->toString() << "\n";
    if (!console::readYesNo("确认删除吗？", false)) {
        std::cout << "  已取消。\n";
        return;
    }
    report(library.removeMember(id));
}

void queryMemberInteractive(const Library& library) {
    console::printTitle("按编号查询借阅人");
    const std::string id = console::readNonEmpty("请输入学号 / 工号: ");
    if (console::inputEnded()) {
        return;
    }
    const Person* found = library.findMember(id);
    if (found == nullptr) {
        std::cout << "  [失败] 借阅人不存在（编号: " << id << "）。\n";
        return;
    }
    printMemberDetail(library, *found);

    const std::vector<const BorrowRecord*> history = library.recordsOfMember(id);
    if (!history.empty()) {
        std::cout << "  借阅历史（共 " << history.size() << " 条）：\n";
        for (std::size_t i = 0; i < history.size(); ++i) {
            std::cout << "    " << (i + 1) << ". " << history[i]->toString() << "\n";
        }
    }
}

void searchMembersInteractive(const Library& library) {
    console::printTitle("按关键字查询借阅人");
    const std::string keyword = console::readNonEmpty("关键字（学号 / 工号 / 姓名 / 院系）: ");
    if (console::inputEnded()) {
        return;
    }
    const std::vector<const Person*> found = library.searchMembers(keyword);
    if (found.empty()) {
        std::cout << "  没有找到匹配的借阅人。\n";
        return;
    }
    for (std::size_t i = 0; i < found.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << found[i]->toString() << "\n";
    }
    std::cout << "  共 " << found.size() << " 条。\n";
}

void listMembersInteractive(const Library& library) {
    console::printTitle("列出借阅人");
    std::cout << "  1. 全部   2. 仅学生   3. 仅教师\n";
    const int choice = console::readIntInRange("请选择: ", 1, 3);
    if (console::inputEnded()) {
        return;
    }
    std::string role;
    if (choice == 2) {
        role = "STUDENT";
    } else if (choice == 3) {
        role = "TEACHER";
    }

    const std::vector<const Person*> found = library.searchMembers("", role);
    if (found.empty()) {
        std::cout << "  暂无数据。\n";
        return;
    }
    for (std::size_t i = 0; i < found.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << found[i]->toString() << "\n";
    }
    std::cout << "  共 " << found.size() << " 条。\n";
}

void memberMenu(Library& library) {
    for (;;) {
        console::printTitle("借阅人管理（学生 / 教师）");
        std::cout << "  1. 添加学生\n"
                     "  2. 添加教师\n"
                     "  3. 修改借阅人信息\n"
                     "  4. 删除借阅人\n"
                     "  5. 按学号 / 工号查询（含在借与借阅历史）\n"
                     "  6. 按关键字查询\n"
                     "  7. 列出借阅人（学生 / 教师）\n"
                     "  0. 返回主菜单\n";
        const int choice = console::readIntInRange("请选择: ", 0, 7);
        if (console::inputEnded()) {
            return;
        }
        switch (choice) {
            case 0:
                return;
            case 1:
                addMemberInteractive(library, true);
                break;
            case 2:
                addMemberInteractive(library, false);
                break;
            case 3:
                updateMemberInteractive(library);
                break;
            case 4:
                removeMemberInteractive(library);
                break;
            case 5:
                queryMemberInteractive(library);
                break;
            case 6:
                searchMembersInteractive(library);
                break;
            case 7:
                listMembersInteractive(library);
                break;
            default:
                break;
        }
        if (console::inputEnded()) {
            return;
        }
        console::pause();
    }
}

void printBookDetail(const Library& library, const Book& book) {
    std::cout << "  " << book.toString() << "\n";
    if (!book.available()) {
        const std::vector<BorrowRecord>& records = library.records();
        for (std::size_t i = 0; i < records.size(); ++i) {
            if (!records[i].returned() && records[i].barcode() == book.barcode()) {
                const Person* borrower = library.findMember(records[i].borrowerId());
                std::cout << "  当前借阅人：" << records[i].borrowerId();
                if (borrower != nullptr) {
                    std::cout << "（" << borrower->roleName() << " " << borrower->name() << "）";
                }
                std::cout << "，应还日期 " << records[i].dueDate().toString() << "\n";
                break;
            }
        }
    }
}

void addBookInteractive(Library& library) {
    console::printTitle("图书入库（添加图书）");
    const std::string barcode = console::readNonEmpty("图书条码号: ");
    if (console::inputEnded()) {
        return;
    }
    const std::string isbn = console::readNonEmpty("图书号 ISBN: ");
    if (console::inputEnded()) {
        return;
    }
    const std::string title = console::readNonEmpty("书名: ");
    if (console::inputEnded()) {
        return;
    }
    const std::string author = console::readNonEmpty("作者: ");
    if (console::inputEnded()) {
        return;
    }
    const std::string publisher = console::readNonEmpty("出版社: ");
    if (console::inputEnded()) {
        return;
    }
    report(library.addBook(Book(barcode, isbn, title, author, publisher)));
}

void updateBookInteractive(Library& library) {
    console::printTitle("修改图书信息");
    const std::string barcode = console::readNonEmpty("请输入图书条码号: ");
    if (console::inputEnded()) {
        return;
    }
    const Book* found = library.findBook(barcode);
    if (found == nullptr) {
        std::cout << "  [失败] 图书不存在（条码号: " << barcode << "）。\n";
        return;
    }
    std::cout << "  当前信息：" << found->toString() << "\n";
    std::cout << "  （条码号是唯一标识，不能修改）\n";

    const std::string isbn = console::readStringOrDefault("新 ISBN", found->isbn());
    const std::string title = console::readStringOrDefault("新书名", found->title());
    const std::string author = console::readStringOrDefault("新作者", found->author());
    const std::string publisher = console::readStringOrDefault("新出版社", found->publisher());
    if (console::inputEnded()) {
        return;
    }
    report(library.updateBook(barcode, Book(barcode, isbn, title, author, publisher)));
}

void removeBookInteractive(Library& library) {
    console::printTitle("删除图书");
    const std::string barcode = console::readNonEmpty("请输入图书条码号: ");
    if (console::inputEnded()) {
        return;
    }
    const Book* found = library.findBook(barcode);
    if (found == nullptr) {
        std::cout << "  [失败] 图书不存在（条码号: " << barcode << "）。\n";
        return;
    }
    std::cout << "  待删除：" << found->toString() << "\n";
    if (!console::readYesNo("确认删除吗？", false)) {
        std::cout << "  已取消。\n";
        return;
    }
    report(library.removeBook(barcode));
}

void queryBookInteractive(const Library& library) {
    console::printTitle("按条码号 / ISBN 查询图书");
    const std::string keyword = console::readNonEmpty("条码号或 ISBN: ");
    if (console::inputEnded()) {
        return;
    }
    const Book* found = library.findBook(keyword);
    if (found != nullptr) {
        printBookDetail(library, *found);
        return;
    }
    const std::vector<const Book*> matched = library.searchBooks(keyword);
    if (matched.empty()) {
        std::cout << "  [失败] 没有找到匹配的图书。\n";
        return;
    }
    for (std::size_t i = 0; i < matched.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << matched[i]->toString() << "\n";
    }
    std::cout << "  共 " << matched.size() << " 条。\n";
}

void searchBooksInteractive(const Library& library) {
    console::printTitle("按关键字查询图书");
    const std::string keyword = console::readNonEmpty("关键字（条码号 / ISBN / 书名 / 作者 / 出版社）: ");
    if (console::inputEnded()) {
        return;
    }
    const std::vector<const Book*> found = library.searchBooks(keyword);
    if (found.empty()) {
        std::cout << "  没有找到匹配的图书。\n";
        return;
    }
    for (std::size_t i = 0; i < found.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << found[i]->toString() << "\n";
    }
    std::cout << "  共 " << found.size() << " 条。\n";
}

void listBooksInteractive(const Library& library) {
    console::printTitle("列出全部图书");
    if (library.books().empty()) {
        std::cout << "  暂无数据。\n";
        return;
    }
    for (std::size_t i = 0; i < library.books().size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << library.books()[i].toString() << "\n";
    }
    std::cout << "  共 " << library.books().size() << " 条。\n";
}

void bookMenu(Library& library) {
    for (;;) {
        console::printTitle("图书管理");
        std::cout << "  1. 添加图书\n"
                     "  2. 修改图书信息\n"
                     "  3. 删除图书\n"
                     "  4. 按条码号 / ISBN 查询\n"
                     "  5. 按关键字查询\n"
                     "  6. 列出全部图书\n"
                     "  0. 返回主菜单\n";
        const int choice = console::readIntInRange("请选择: ", 0, 6);
        if (console::inputEnded()) {
            return;
        }
        switch (choice) {
            case 0:
                return;
            case 1:
                addBookInteractive(library);
                break;
            case 2:
                updateBookInteractive(library);
                break;
            case 3:
                removeBookInteractive(library);
                break;
            case 4:
                queryBookInteractive(library);
                break;
            case 5:
                searchBooksInteractive(library);
                break;
            case 6:
                listBooksInteractive(library);
                break;
            default:
                break;
        }
        if (console::inputEnded()) {
            return;
        }
        console::pause();
    }
}

void borrowInteractive(Library& library) {
    console::printTitle("借书");
    const std::string memberId = console::readNonEmpty("借阅人学号 / 工号: ");
    if (console::inputEnded()) {
        return;
    }
    const std::string barcode = console::readNonEmpty("图书条码号: ");
    if (console::inputEnded()) {
        return;
    }
    const Date date = console::readDateOrDefault("借书日期", Date::today());
    if (console::inputEnded()) {
        return;
    }
    report(library.borrowBook(memberId, barcode, date));
}

void returnByBarcodeInteractive(Library& library) {
    console::printTitle("还书（按图书条码号）");
    const std::string barcode = console::readNonEmpty("图书条码号: ");
    if (console::inputEnded()) {
        return;
    }
    const Date date = console::readDateOrDefault("归还日期", Date::today());
    if (console::inputEnded()) {
        return;
    }
    report(library.returnBookByBarcode(barcode, date));
}

void returnByMemberInteractive(Library& library) {
    console::printTitle("还书（按借阅人 + 图书条码号）");
    const std::string memberId = console::readNonEmpty("借阅人学号 / 工号: ");
    if (console::inputEnded()) {
        return;
    }
    const std::string barcode = console::readNonEmpty("图书条码号: ");
    if (console::inputEnded()) {
        return;
    }
    const Date date = console::readDateOrDefault("归还日期", Date::today());
    if (console::inputEnded()) {
        return;
    }
    report(library.returnBook(memberId, barcode, date));
}

void activeOfMemberInteractive(const Library& library) {
    console::printTitle("查询某借阅人当前在借图书");
    const std::string memberId = console::readNonEmpty("借阅人学号 / 工号: ");
    if (console::inputEnded()) {
        return;
    }
    const Person* member = library.findMember(memberId);
    if (member == nullptr) {
        std::cout << "  [失败] 借阅人不存在（编号: " << memberId << "）。\n";
        return;
    }
    printMemberDetail(library, *member);
}

void borrowMenu(Library& library) {
    for (;;) {
        console::printTitle("借书 / 还书");
        std::cout << "  1. 借书\n"
                     "  2. 还书（按图书条码号）\n"
                     "  3. 还书（按借阅人 + 图书条码号）\n"
                     "  4. 查询某借阅人当前在借图书\n"
                     "  0. 返回主菜单\n";
        const int choice = console::readIntInRange("请选择: ", 0, 4);
        if (console::inputEnded()) {
            return;
        }
        switch (choice) {
            case 0:
                return;
            case 1:
                borrowInteractive(library);
                break;
            case 2:
                returnByBarcodeInteractive(library);
                break;
            case 3:
                returnByMemberInteractive(library);
                break;
            case 4:
                activeOfMemberInteractive(library);
                break;
            default:
                break;
        }
        if (console::inputEnded()) {
            return;
        }
        console::pause();
    }
}

void overdueInteractive(const Library& library) {
    console::printTitle("逾期未还清单");
    const Date reference = console::readDateOrDefault("统计基准日期", Date::today());
    if (console::inputEnded()) {
        return;
    }
    const std::vector<const BorrowRecord*> found = library.overdueRecords(reference);
    if (found.empty()) {
        std::cout << "  截至 " << reference.toString() << "，没有逾期未还的图书。\n";
        return;
    }

    double totalFine = 0.0;
    for (std::size_t i = 0; i < found.size(); ++i) {
        const BorrowRecord& record = *found[i];
        const Person* member = library.findMember(record.borrowerId());
        const Book* book = library.findBook(record.barcode());
        const long long overdue = record.overdueDaysAsOf(reference);
        const double finePerDay = member != nullptr ? member->finePerDay(library.policy()) : 0.0;
        const double fine = roundMoney(static_cast<double>(overdue) * finePerDay);
        totalFine += fine;

        std::cout << "  " << (i + 1) << ". 借阅人 " << record.borrowerId();
        if (member != nullptr) {
            std::cout << "（" << member->roleName() << " " << member->name() << "）";
        }
        std::cout << " | 图书 " << record.barcode();
        if (book != nullptr) {
            std::cout << "《" << book->title() << "》";
        }
        std::cout << " | 借出 " << record.borrowDate().toString()
                  << " | 应还 " << record.dueDate().toString()
                  << " | 已逾期 " << overdue << " 天"
                  << " | 应缴罚款 " << formatMoney(fine) << " 元\n";
    }
    std::cout << "  共 " << found.size() << " 条逾期记录，罚款合计 "
              << formatMoney(roundMoney(totalFine)) << " 元。\n";
}

void historyInteractive(const Library& library) {
    console::printTitle("借阅历史查询");
    const std::string memberId = console::readNonEmpty("借阅人学号 / 工号: ");
    if (console::inputEnded()) {
        return;
    }
    if (library.findMember(memberId) == nullptr) {
        std::cout << "  [失败] 借阅人不存在（编号: " << memberId << "）。\n";
        return;
    }
    const std::vector<const BorrowRecord*> history = library.recordsOfMember(memberId);
    if (history.empty()) {
        std::cout << "  该借阅人还没有借阅记录。\n";
        return;
    }
    double collected = 0.0;
    for (std::size_t i = 0; i < history.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << history[i]->toString() << "\n";
        if (history[i]->returned()) {
            collected += history[i]->fine();
        }
    }
    std::cout << "  共 " << history.size() << " 条记录，已收罚款合计 "
              << formatMoney(roundMoney(collected)) << " 元；未结罚款 "
              << formatMoney(library.outstandingFineOf(memberId, Date::today())) << " 元。\n";
}

void statisticsInteractive(const Library& library) {
    console::printTitle("馆藏与借阅统计");
    const Date reference = console::readDateOrDefault("统计基准日期", Date::today());
    if (console::inputEnded()) {
        return;
    }
    std::cout << library.statisticsReport(reference) << "\n";
}

void allRecordsInteractive(const Library& library) {
    console::printTitle("全部借阅记录");
    const std::vector<BorrowRecord>& records = library.records();
    if (records.empty()) {
        std::cout << "  暂无借阅记录。\n";
        return;
    }
    for (std::size_t i = 0; i < records.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << records[i].toString() << "\n";
    }
    std::cout << "  共 " << records.size() << " 条。\n";
}

void queryMenu(const Library& library) {
    for (;;) {
        console::printTitle("查询与统计");
        std::cout << "  1. 逾期未还清单（含罚款）\n"
                     "  2. 某借阅人的借阅历史\n"
                     "  3. 馆藏与借阅统计\n"
                     "  4. 全部借阅记录\n"
                     "  0. 返回主菜单\n";
        const int choice = console::readIntInRange("请选择: ", 0, 4);
        if (console::inputEnded()) {
            return;
        }
        switch (choice) {
            case 0:
                return;
            case 1:
                overdueInteractive(library);
                break;
            case 2:
                historyInteractive(library);
                break;
            case 3:
                statisticsInteractive(library);
                break;
            case 4:
                allRecordsInteractive(library);
                break;
            default:
                break;
        }
        if (console::inputEnded()) {
            return;
        }
        console::pause();
    }
}

void showPolicy(const Library& library) {
    const BorrowPolicy& policy = library.policy();
    std::cout << "  学生：最大借阅 " << policy.studentMaxBorrow << " 本，借期 "
              << policy.studentBorrowDays << " 天，逾期罚款 "
              << formatMoney(policy.studentFinePerDay) << " 元/天\n";
    std::cout << "  教师：最大借阅 " << policy.teacherMaxBorrow << " 本，借期 "
              << policy.teacherBorrowDays << " 天，逾期罚款 "
              << formatMoney(policy.teacherFinePerDay) << " 元/天\n";
}

void editPolicyInteractive(Library& library) {
    console::printTitle("修改借阅规则");
    BorrowPolicy policy = library.policy();
    policy.studentMaxBorrow = console::readIntOrDefault(
        "学生最大借阅数量", policy.studentMaxBorrow, 1, kMaxBorrowUpperBound);
    policy.teacherMaxBorrow = console::readIntOrDefault(
        "教师最大借阅数量", policy.teacherMaxBorrow, 1, kMaxBorrowUpperBound);
    policy.studentBorrowDays = console::readIntOrDefault(
        "学生借期（天）", policy.studentBorrowDays, 1, kMaxBorrowDaysUpperBound);
    policy.teacherBorrowDays = console::readIntOrDefault(
        "教师借期（天）", policy.teacherBorrowDays, 1, kMaxBorrowDaysUpperBound);
    policy.studentFinePerDay = console::readDoubleOrDefault(
        "学生逾期罚款（元/天）", policy.studentFinePerDay, 0.0, kMaxFinePerDay);
    policy.teacherFinePerDay = console::readDoubleOrDefault(
        "教师逾期罚款（元/天）", policy.teacherFinePerDay, 0.0, kMaxFinePerDay);
    if (console::inputEnded()) {
        return;
    }
    library.setPolicy(policy);
    std::cout << "  [成功] 参数已更新：\n";
    showPolicy(library);
    std::cout << "  说明：新参数只影响之后新产生的借阅，已有记录仍按原应还日期结算。\n";
}

void policyMenu(Library& library) {
    for (;;) {
        console::printTitle("参数设置（借阅规则）");
        std::cout << "  1. 查看当前参数\n"
                     "  2. 修改参数\n"
                     "  0. 返回主菜单\n";
        const int choice = console::readIntInRange("请选择: ", 0, 2);
        if (console::inputEnded()) {
            return;
        }
        switch (choice) {
            case 0:
                return;
            case 1:
                showPolicy(library);
                break;
            case 2:
                editPolicyInteractive(library);
                break;
            default:
                break;
        }
        if (console::inputEnded()) {
            return;
        }
        console::pause();
    }
}

void saveInteractive(const Library& library) {
    std::string message;
    if (library.saveToFile(Library::defaultDataFile(), message)) {
        std::cout << "  [成功] " << message << "\n";
    } else {
        std::cout << "  [失败] " << message << "\n";
    }
}

void saveAsInteractive(const Library& library) {
    console::printTitle("另存为");
    const std::string path = console::readNonEmpty("文件名（例: my_library.txt）: ");
    if (console::inputEnded()) {
        return;
    }
    std::string message;
    if (library.saveToFile(path, message)) {
        std::cout << "  [成功] " << message << "\n";
    } else {
        std::cout << "  [失败] " << message << "\n";
    }
}

void loadInteractive(Library& library) {
    console::printTitle("读取数据文件");
    const std::string path = console::readLine(
        "文件名（回车使用 " + std::string(Library::defaultDataFile()) + "）: ");
    if (console::inputEnded()) {
        return;
    }
    const std::string target = path.empty() ? Library::defaultDataFile() : path;
    if (!console::readYesNo("读取将覆盖当前内存中的数据，继续吗？", false)) {
        std::cout << "  已取消。\n";
        return;
    }
    std::string message;
    if (library.loadFromFile(target, message)) {
        std::cout << "  [成功] " << message << "\n";
    } else {
        std::cout << "  [失败] " << message << "\n";
    }
}

void dataMenu(Library& library) {
    for (;;) {
        console::printTitle("数据存档");
        std::cout << "  1. 保存到默认文件（" << Library::defaultDataFile() << "）\n"
                     "  2. 另存为其它文件\n"
                     "  3. 从文件读取数据\n"
                     "  0. 返回主菜单\n";
        const int choice = console::readIntInRange("请选择: ", 0, 3);
        if (console::inputEnded()) {
            return;
        }
        switch (choice) {
            case 0:
                return;
            case 1:
                saveInteractive(library);
                break;
            case 2:
                saveAsInteractive(library);
                break;
            case 3:
                loadInteractive(library);
                break;
            default:
                break;
        }
        if (console::inputEnded()) {
            return;
        }
        console::pause();
    }
}

void mainMenu(Library& library) {
    printBanner();
    for (;;) {
        console::printTitle("主菜单");
        std::cout << "  1. 借阅人管理（学生 / 教师信息）\n"
                     "  2. 图书管理（图书信息）\n"
                     "  3. 借书 / 还书\n"
                     "  4. 查询与统计（含逾期清单）\n"
                     "  5. 参数设置（借期 / 罚款 / 最大借阅数量）\n"
                     "  6. 数据存档（保存 / 读取）\n"
                     "  7. 类与对象演示（构造函数 / 成员函数 / 多态）\n"
                     "  0. 退出系统\n";
        const int choice = console::readIntInRange("请选择: ", 0, 7);
        if (console::inputEnded()) {
            return;
        }
        switch (choice) {
            case 0:
                return;
            case 1:
                memberMenu(library);
                break;
            case 2:
                bookMenu(library);
                break;
            case 3:
                borrowMenu(library);
                break;
            case 4:
                queryMenu(library);
                break;
            case 5:
                policyMenu(library);
                break;
            case 6:
                dataMenu(library);
                break;
            case 7:
                runClassDemo();
                break;
            default:
                break;
        }
    }
}

}

int main(int argc, char* argv[]) {
    console::setupEncoding();

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--selftest") {
            return runSelfTest();
        }
        if (argument == "--demo") {
            runClassDemo();
            return 0;
        }
        if (argument == "--help" || argument == "-h" || argument == "/?") {
            printUsage();
            return 0;
        }
    }

    Library library;
    const std::string dataFile = Library::defaultDataFile();

    std::ifstream probe(dataFile.c_str());
    if (probe.good()) {
        probe.close();
        std::string message;
        if (library.loadFromFile(dataFile, message)) {
            std::cout << "  " << message << "\n";
        } else {
            std::cout << "  [提示] " << message << "\n";
        }
    } else {
        library.seedDemoData();
        std::cout << "  [提示] 未找到数据文件 " << dataFile
                  << "，已载入演示数据（可直接试用，退出时自动保存）。\n";
    }

    mainMenu(library);

    std::string message;
    if (library.saveToFile(dataFile, message)) {
        std::cout << "  " << message << "\n";
    } else {
        std::cout << "  [提示] " << message << "\n";
    }
    std::cout << "  感谢使用，再见！\n";
    return 0;
}
