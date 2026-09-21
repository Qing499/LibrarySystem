#include "SelfTest.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "Book.h"
#include "ConsoleUtils.h"
#include "Date.h"
#include "Library.h"
#include "Student.h"
#include "Teacher.h"
#include "TextUtils.h"

namespace {

int g_passed = 0;
int g_failed = 0;

void check(bool condition, const std::string& name) {
    if (condition) {
        ++g_passed;
        std::cout << "  [通过] " << name << "\n";
    } else {
        ++g_failed;
        std::cout << "  [失败] " << name << "\n";
    }
}

bool nearlyEqual(double left, double right) {
    return std::fabs(left - right) < 1e-6;
}

void checkDate(const std::string& text, int year, int month, int day, const std::string& name) {
    Date parsed;
    const bool ok = Date::tryParse(text, parsed) &&
                    parsed.year() == year && parsed.month() == month && parsed.day() == day;
    check(ok, name);
}

void testDate() {
    std::cout << "\n[1] 日期类测试\n";

    checkDate("2026-09-20", 2026, 9, 20, "解析 YYYY-MM-DD");
    checkDate("2026/9/20", 2026, 9, 20, "解析 YYYY/M/D");
    checkDate("2026.9.20", 2026, 9, 20, "解析 YYYY.M.D");
    checkDate("20260920", 2026, 9, 20, "解析 YYYYMMDD");
    checkDate(" 2026-9-5 ", 2026, 9, 5, "忽略首尾空格");

    Date parsed;
    check(!Date::tryParse("2026-02-30", parsed), "拒绝非法日期 2026-02-30");
    check(!Date::tryParse("2026-13-01", parsed), "拒绝非法月份 2026-13-01");
    check(!Date::tryParse("2023-02-29", parsed), "拒绝非闰年 2023-02-29");
    check(!Date::tryParse("abc", parsed), "拒绝非数字内容");
    check(!Date::tryParse("", parsed), "拒绝空字符串");
    check(Date::isLeapYear(2024) && Date::isLeapYear(2000) && !Date::isLeapYear(1900),
          "闰年判断");
    check(!Date().isValid(), "默认日期为无效状态");
    check(Date(2026, 9, 5).toString() == "2026-09-05", "格式化补零 YYYY-MM-DD");

    check(Date(2026, 1, 1).addDays(30) == Date(2026, 1, 31), "日期加天数（跨月）");
    check(Date(2025, 12, 31).addDays(1) == Date(2026, 1, 1), "日期加天数（跨年）");
    check(Date(2024, 3, 1).addDays(-2) == Date(2024, 2, 28), "日期减天数（闰年二月）");
    check(Date(2026, 1, 1).daysSince(Date(2025, 12, 31)) == 1, "相差天数（跨年）");
    check(Date(2024, 3, 1).daysSince(Date(2024, 2, 28)) == 2, "相差天数（闰年）");
    check(Date(2026, 3, 1).addDays(30) == Date(2026, 3, 31), "学生借期 30 天推算应还日期");

    check(text::trim("  ab  ") == "ab", "text::trim");
    long long value = 0;
    check(text::parseInt("12", value) && value == 12, "text::parseInt 正常");
    check(!text::parseInt("12a", value), "text::parseInt 拒绝混合内容");
    check(!text::parseInt("", value), "text::parseInt 拒绝空串");
}

void testMemberAndBookManagement() {
    std::cout << "\n[2] 学生 / 教师 / 图书信息管理测试\n";

    Library library;
    check(library.addMember("STUDENT", "S001", "张三", "计算机科学与技术学院", 2).ok,
          "添加学生");
    check(library.addMember("STUDENT", "S002", "李四", "计算机科学与技术学院", 1).ok,
          "添加学生（第二个）");
    check(library.addMember("TEACHER", "T001", "王老师", "信息工程学院", 5).ok,
          "添加教师");
    check(!library.addMember("STUDENT", "S001", "张三三", "其他学院", 3).ok,
          "拒绝重复学号");
    check(!library.addMember("GUEST", "G001", "游客", "外单位", 1).ok,
          "拒绝非法角色");
    check(!library.addMember("STUDENT", "S003", "", "计算机学院", 2).ok,
          "拒绝空姓名");
    check(!library.addMember("STUDENT", "S003", "赵六", "计算机学院", 0).ok,
          "拒绝非法最大借阅数量");

    const Person* student = library.findMember("S001");
    check(student != nullptr && student->roleName() == "学生" && student->idLabel() == "学号",
          "多态：学生角色名与编号名称");
    check(student != nullptr && student->borrowDays(library.policy()) == 30 &&
                                 nearlyEqual(student->finePerDay(library.policy()), 0.20),
          "多态：学生借期 30 天、罚款 0.20 元/天");

    const Person* teacher = library.findMember("T001");
    check(teacher != nullptr && teacher->roleName() == "教师" && teacher->idLabel() == "工号",
          "多态：教师角色名与编号名称");
    check(teacher != nullptr && teacher->borrowDays(library.policy()) == 60 &&
                                 nearlyEqual(teacher->finePerDay(library.policy()), 0.10),
          "多态：教师借期 60 天、罚款 0.10 元/天");

    check(library.updateMember("S001", "张三丰", "信息工程学院", 3).ok, "修改借阅人信息");
    check(library.findMember("S001")->name() == "张三丰" &&
          library.findMember("S001")->department() == "信息工程学院" &&
          library.findMember("S001")->maxBorrow() == 3,
          "修改后的信息正确");
    check(!library.updateMember("S999", "无名", "无", 1).ok, "修改不存在的借阅人应失败");
    check(!library.updateMember("S001", "张三丰", "信息工程学院", 0).ok,
          "修改时非法最大借阅数量应失败");

    check(library.addBook(Book("B001", "978-7-111-40701-0", "C++ Primer", "Lippman", "机械工业出版社")).ok,
          "添加图书");
    check(library.addBook(Book("B002", "978-7-302-52056-7", "数据结构", "邓俊辉", "清华大学出版社")).ok,
          "添加图书（第二本）");
    check(library.addBook(Book("B003", "978-7-115-44663-3", "算法", "Sedgewick", "人民邮电出版社")).ok,
          "添加图书（第三本）");
    check(!library.addBook(Book("B001", "X", "重复条码", "作者", "出版社")).ok, "拒绝重复条码号");
    check(!library.addBook(Book("B004", "978-7-000-00000-0", "", "作者", "出版社")).ok, "拒绝空书名");
    check(!library.addBook(Book("B004", "978-7-000-00000-0", "有书名", "", "出版社")).ok, "拒绝空作者");

    check(library.updateBook("B003", Book("B003", "978-7-115-44663-4", "算法（第4版）", "Sedgewick", "人民邮电出版社")).ok,
          "修改图书信息");
    check(library.findBook("B003")->title() == "算法（第4版）" &&
          library.findBook("B003")->isbn() == "978-7-115-44663-4",
          "修改后的图书信息正确");
    check(!library.updateBook("B999", Book("B999", "I", "T", "A", "P")).ok, "修改不存在的图书应失败");

    check(library.searchBooks("C++").size() == 1, "按关键字查询图书（书名）");
    check(library.searchBooks("清华大学").size() == 1, "按关键字查询图书（出版社）");
    check(library.searchBooks("978").size() == 3, "按关键字查询图书（ISBN）");
    check(library.searchMembers("计算机", "STUDENT").size() == 1, "按关键字 + 角色查询借阅人");
    check(library.searchMembers("", "TEACHER").size() == 1, "列出全部教师");

    check(library.updateMember("S001", "张三丰", "信息工程学院", 2).ok, "恢复学生最大借阅数量");
}

void testBorrowAndReturn() {
    std::cout << "\n[3] 借书 / 还书 / 逾期罚款测试\n";

    Library library;
    library.addMember("STUDENT", "S001", "张三", "计算机科学与技术学院", 2);
    library.addMember("STUDENT", "S002", "李四", "计算机科学与技术学院", 1);
    library.addMember("TEACHER", "T001", "王老师", "信息工程学院", 5);
    library.addBook(Book("B001", "ISBN-1", "C++ Primer", "Lippman", "机械工业出版社"));
    library.addBook(Book("B002", "ISBN-2", "数据结构", "邓俊辉", "清华大学出版社"));
    library.addBook(Book("B003", "ISBN-3", "算法", "Sedgewick", "人民邮电出版社"));

    const Date day1(2026, 3, 1);
    const Date dueExpected(2026, 3, 31);

    OperationResult result = library.borrowBook("S001", "B001", day1);
    check(result.ok, "学生借书成功");
    check(!library.findBook("B001")->available(), "借出后图书状态为“已借出”");
    check(library.findMember("S001")->borrowedCount() == 1, "借出后在借数量 +1");
    std::vector<const BorrowRecord*> active = library.activeRecordsOfMember("S001");
    check(active.size() == 1 && active[0]->dueDate() == dueExpected,
          "应还日期 = 借出日期 + 30 天");

    check(!library.borrowBook("S001", "B001", Date(2026, 3, 2)).ok, "同一本书不能重复借阅");
    check(!library.borrowBook("S999", "B003", day1).ok, "不存在的借阅人不能借书");
    check(!library.borrowBook("S001", "B999", day1).ok, "不存在的图书不能借阅");
    check(!library.borrowBook("S001", "B003", Date()) .ok, "非法借书日期应失败");

    check(library.borrowBook("S001", "B002", day1).ok, "借第二本书成功");
    result = library.borrowBook("S001", "B003", day1);
    check(!result.ok && result.message.find("最大借阅数量") != std::string::npos,
          "超过最大借阅数量时拒绝借书");
    active = library.activeRecordsOfMember("S001");
    check(active.size() == 2, "当前在借 2 本（达到该学生最大借阅数量）");

    library.findBook("B003")->setAvailable(false);
    library.findMember("S002")->setBorrowedCount(5);
    check(library.refreshDerivedState() >= 2, "refreshDerivedState 检测到不一致");
    check(library.findBook("B003")->available() && library.findMember("S002")->borrowedCount() == 0,
          "refreshDerivedState 按借阅记录修复状态");

    result = library.returnBook("S001", "B001", dueExpected);
    check(result.ok && result.overdueDays == 0 && nearlyEqual(result.fine, 0.0),
          "应还日期当天归还不罚款");
    check(library.findBook("B001")->available(), "归还后图书状态恢复“可借”");
    check(library.findMember("S001")->borrowedCount() == 1, "归还后在借数量 -1");

    result = library.borrowBook("S001", "B003", Date(2026, 4, 1));
    check(!result.ok && result.message.find("逾期未还") != std::string::npos,
          "存在逾期未还图书时拒绝借书");

    result = library.returnBook("S001", "B002", Date(2026, 4, 6));
    check(result.ok && result.overdueDays == 6, "逾期天数计算正确（6 天）");
    check(nearlyEqual(result.fine, 1.20), "学生逾期罚款 = 6 × 0.20 = 1.20 元");
    check(library.findMember("S001")->borrowedCount() == 0, "全部归还后在借数量为 0");

    result = library.borrowBook("S001", "B003", Date(2026, 4, 6));
    check(result.ok, "归还后再借成功");
    active = library.activeRecordsOfMember("S001");
    check(active.size() == 1 && active[0]->dueDate() == Date(2026, 5, 6),
          "再次借书的应还日期 = 借出日期 + 30 天");

    check(library.overdueRecords(Date(2026, 5, 6)).empty(), "未到期时逾期清单为空");
    check(library.overdueRecords(Date(2026, 5, 7)).size() == 1, "逾期清单能查出逾期记录");
    check(nearlyEqual(library.outstandingFineOf("S001", Date(2026, 5, 7)), 0.20),
          "未归还图书的预计罚款 = 1 × 0.20 = 0.20 元");
    check(nearlyEqual(library.totalOutstandingFine(Date(2026, 5, 7)), 0.20),
          "全馆未结罚款合计 = 0.20 元");

    result = library.returnBook("S001", "B003", Date(2026, 5, 10));
    check(result.ok && result.overdueDays == 4 && nearlyEqual(result.fine, 0.80),
          "逾期 4 天罚款 = 4 × 0.20 = 0.80 元");
    check(library.findMember("S001")->borrowedCount() == 0, "再次归还后在借数量为 0");
    check(library.borrowBook("S001", "B001", Date(2026, 5, 10)).ok, "逾期还清后可以继续借书");

    result = library.borrowBook("T001", "B002", Date(2026, 5, 10));
    check(result.ok, "教师借书成功");
    active = library.activeRecordsOfMember("T001");
    check(active.size() == 1 && active[0]->dueDate() == Date(2026, 7, 9),
          "教师应还日期 = 借出日期 + 60 天");
    result = library.returnBook("T001", "B002", Date(2026, 7, 15));
    check(result.ok && result.overdueDays == 6 && nearlyEqual(result.fine, 0.60),
          "教师逾期罚款 = 6 × 0.10 = 0.60 元");

    check(!library.returnBook("T001", "B002", Date(2026, 7, 16)).ok, "重复归还应失败");
    check(!library.returnBook("S001", "B001", Date(2026, 5, 1)).ok,
          "归还日期早于借出日期应失败");
    check(!library.returnBookByBarcode("B002", Date(2026, 7, 16)).ok,
          "归还未借出的图书应失败");

    check(library.recordsOfMember("S001").size() == 4, "查询某借阅人的借阅历史条数");
    check(nearlyEqual(library.totalCollectedFine(), 2.60), "已收罚款合计 = 2.60 元");
    check(library.statisticsReport(Date(2026, 7, 16)).find("统计基准日期") != std::string::npos,
          "统计报表生成");

    check(!library.removeBook("B001").ok, "图书未归还时不能删除");
    check(!library.removeMember("S001").ok, "有未归还图书时不能删除借阅人");
    check(library.returnBookByBarcode("B001", Date(2026, 6, 9)).ok, "按条码号归还成功");
    check(nearlyEqual(library.outstandingFineOf("S001", Date(2026, 7, 16)), 0.0),
          "还清后无未结罚款");
    check(library.overdueRecords(Date(2026, 7, 16)).empty(), "全部归还后逾期清单为空");

    const std::string tempFile = "selftest_data.tmp";
    std::string message;
    check(library.saveToFile(tempFile, message), "保存数据到文件");

    Library reloaded;
    check(reloaded.loadFromFile(tempFile, message), "从文件读取数据");
    check(reloaded.members().size() == library.members().size(), "读取后借阅人数量一致");
    check(reloaded.books().size() == library.books().size(), "读取后图书数量一致");
    check(reloaded.records().size() == library.records().size(), "读取后借阅记录数量一致");
    check(nearlyEqual(reloaded.totalCollectedFine(), library.totalCollectedFine()),
          "读取后罚款金额一致");
    check(reloaded.findMember("S001") != nullptr && reloaded.findBook("B001") != nullptr,
          "读取后能按编号查询到借阅人与图书");
    check(reloaded.findMember("T001") != nullptr &&
          nearlyEqual(reloaded.findMember("T001")->finePerDay(reloaded.policy()), 0.10),
          "读取后教师罚款规则保留");

    Library untouched;
    untouched.addMember("STUDENT", "X001", "测试", "测试学院", 1);
    check(!untouched.loadFromFile("no_such_file_should_not_exist.txt", message),
          "读取不存在的文件应失败");
    check(untouched.members().size() == 1, "读取失败时不破坏原有数据");

    std::remove(tempFile.c_str());

    BorrowPolicy policy = library.policy();
    policy.studentBorrowDays = 15;
    policy.studentFinePerDay = 0.50;
    library.setPolicy(policy);
    check(library.borrowBook("S002", "B003", Date(2026, 8, 1)).ok, "调整参数后借书成功");
    active = library.activeRecordsOfMember("S002");
    check(active.size() == 1 && active[0]->dueDate() == Date(2026, 8, 16),
          "调整借期为 15 天后应还日期随之变化");
    result = library.returnBook("S002", "B003", Date(2026, 8, 18));
    check(result.ok && result.overdueDays == 2 && nearlyEqual(result.fine, 1.00),
          "调整罚款单价后罚款 = 2 × 0.50 = 1.00 元");

    check(library.removeBook("B003").ok, "图书归还后可以删除");
    check(library.removeMember("S002").ok, "图书还清后可以删除借阅人");
    check(library.findMember("S002") == nullptr && library.findBook("B003") == nullptr,
          "删除后查询不到对应数据");
}

void testConstructors() {
    std::cout << "\n[4] 默认构造函数 / 重载构造函数 / 多态测试\n";

    Student defaultStudent;
    check(defaultStudent.id() == "未设置" && defaultStudent.name() == "未命名" &&
          defaultStudent.department() == "未填写" && defaultStudent.maxBorrow() == 5 &&
          defaultStudent.borrowedCount() == 0,
          "Student 默认构造函数把数据成员初始化为确定的值");
    check(defaultStudent.roleName() == "学生" && defaultStudent.idLabel() == "学号",
          "默认构造的学生对象可以正常调用成员函数");

    Teacher defaultTeacher;
    check(defaultTeacher.id() == "未设置" && defaultTeacher.maxBorrow() == 10 &&
          defaultTeacher.borrowedCount() == 0,
          "Teacher 默认构造函数把数据成员初始化为确定的值");

    Book defaultBook;
    check(defaultBook.barcode().empty() && defaultBook.isbn().empty() && defaultBook.title().empty() &&
          defaultBook.available(),
          "Book 默认构造函数：文本成员为空串、状态为可借");

    Date defaultDate;
    check(!defaultDate.isValid() && defaultDate.toString() == "-",
          "Date 默认构造函数构造空日期");

    BorrowRecord defaultRecord;
    check(defaultRecord.borrowerId().empty() && defaultRecord.barcode().empty() &&
          !defaultRecord.returned() && defaultRecord.overdueDays() == 0 &&
          nearlyEqual(defaultRecord.fine(), 0.0),
          "BorrowRecord 默认构造函数把数据成员初始化为确定的值");

    Library defaultLibrary;
    check(defaultLibrary.members().empty() && defaultLibrary.books().empty() &&
          defaultLibrary.records().empty() && defaultLibrary.policy().studentBorrowDays == 30,
          "Library 默认构造函数：容器为空、规则取默认值");

    Person* person = &defaultStudent;
    check(person->roleName() == "学生" && person->idLabel() == "学号" &&
          person->borrowDays(BorrowPolicy()) == 30 && person->maxBorrow() == 5,
          "基类指针调用虚函数（学生对象）");
    person = &defaultTeacher;
    check(person->roleName() == "教师" && person->idLabel() == "工号" &&
          person->borrowDays(BorrowPolicy()) == 60,
          "基类指针调用虚函数（教师对象）");

    Student briefStudent("S100", "测试学生", "计算机科学与技术学院");
    check(briefStudent.maxBorrow() == 5 && briefStudent.id() == "S100",
          "重载构造函数：学生三参数版本使用默认最大借阅数量");

    Teacher briefTeacher("T200", "测试教师", "信息工程学院");
    check(briefTeacher.maxBorrow() == 10,
          "重载构造函数：教师三参数版本使用默认最大借阅数量");

    Student fullStudent("S101", "完整学生", "信息工程学院", 3);
    check(fullStudent.maxBorrow() == 3 && fullStudent.department() == "信息工程学院",
          "重载构造函数：学生四参数版本按参数初始化");

    Book availableBook("BC-9001", "978-7-000-00000-1", "测试图书", "测试作者", "测试出版社");
    check(availableBook.available() && availableBook.title() == "测试图书",
          "重载构造函数：图书默认构造为可借");

    Book borrowedBook("BC-9002", "978-7-000-00000-2", "测试图书二", "测试作者", "测试出版社", false);
    check(!borrowedBook.available(), "重载构造函数：图书可以构造为已借出");

    BorrowRecord record("S100", "BC-9001", Date(2026, 9, 1), Date(2026, 10, 1));
    check(!record.returned() && record.dueDate() == Date(2026, 10, 1) &&
          record.overdueDaysAsOf(Date(2026, 10, 6)) == 5,
          "重载构造函数：借阅记录按参数构造并能判断逾期");

    BorrowPolicy customPolicy;
    customPolicy.studentBorrowDays = 7;
    customPolicy.studentFinePerDay = 0.50;
    Library customLibrary(customPolicy);
    check(customLibrary.policy().studentBorrowDays == 7 &&
          nearlyEqual(customLibrary.policy().studentFinePerDay, 0.50),
          "重载构造函数：Library 使用自定义借阅规则");
    customLibrary.addMember("STUDENT", "S300", "演示学生", "计算机科学与技术学院", 2);
    customLibrary.addBook(Book("BC-9003", "978-7-000-00000-3", "演示图书", "演示作者", "演示出版社"));
    const OperationResult customBorrow = customLibrary.borrowBook("S300", "BC-9003", Date(2026, 9, 1));
    const std::vector<const BorrowRecord*> customActive = customLibrary.activeRecordsOfMember("S300");
    check(customBorrow.ok && customActive.size() == 1 &&
          customActive[0]->dueDate() == Date(2026, 9, 8),
          "自定义规则生效：应还日期 = 借出日期 + 7 天");
    const OperationResult customReturn = customLibrary.returnBook("S300", "BC-9003", Date(2026, 9, 11));
    check(customReturn.ok && customReturn.overdueDays == 3 && nearlyEqual(customReturn.fine, 1.50),
          "自定义规则生效：逾期 3 天罚款 = 3 × 0.50 = 1.50 元");
}

}

int runSelfTest() {
    console::setupEncoding();

    std::cout << "==============================================\n";
    std::cout << " 图书馆借阅管理系统 · 自检程序\n";
    std::cout << "==============================================\n";

    testDate();
    testMemberAndBookManagement();
    testBorrowAndReturn();
    testConstructors();

    std::cout << "\n----------------------------------------------\n";
    std::cout << " 自检结束：通过 " << g_passed << " 项，失败 " << g_failed << " 项\n";
    std::cout << "----------------------------------------------\n";

    if (g_failed == 0) {
        std::cout << "结果：全部通过。\n";
        return 0;
    }
    std::cout << "结果：存在失败项，请检查代码。\n";
    return 1;
}
