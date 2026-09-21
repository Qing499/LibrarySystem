#include "ClassDemo.h"

#include <iostream>
#include <string>

#include "Book.h"
#include "BorrowRecord.h"
#include "Config.h"
#include "ConsoleUtils.h"
#include "Date.h"
#include "Library.h"
#include "Person.h"
#include "Student.h"
#include "Teacher.h"

namespace {

void section(const std::string& title) {
    std::cout << "\n" << title << "\n";
}

std::string yesNo(bool value) {
    return value ? "是" : "否";
}

}

void runClassDemo() {
    console::printTitle("类与对象演示（构造函数 · 对象 · 成员函数 · 多态）");
    std::cout << "  说明：本演示使用的都是本函数内的局部对象，不会影响系统中的实际数据。\n";

    const BorrowPolicy policy = BorrowPolicy();

    section("【一】默认构造函数：把数据成员初始化为确定的值");

    Student defaultStudent;
    Teacher defaultTeacher;
    Book defaultBook;
    Date defaultDate;
    BorrowRecord defaultRecord;
    Library defaultLibrary;

    std::cout << "  Student      s1 : " << defaultStudent.idLabel() << "=\"" << defaultStudent.id()
              << "\"  姓名=\"" << defaultStudent.name()
              << "\"  院系=\"" << defaultStudent.department()
              << "\"  最大可借=" << defaultStudent.maxBorrow()
              << "  当前在借=" << defaultStudent.borrowedCount() << "\n";

    std::cout << "  Teacher      t1 : " << defaultTeacher.idLabel() << "=\"" << defaultTeacher.id()
              << "\"  姓名=\"" << defaultTeacher.name()
              << "\"  院系=\"" << defaultTeacher.department()
              << "\"  最大可借=" << defaultTeacher.maxBorrow()
              << "  当前在借=" << defaultTeacher.borrowedCount() << "\n";

    std::cout << "  Book         b1 : 条码号=\"" << defaultBook.barcode()
              << "\"  ISBN=\"" << defaultBook.isbn()
              << "\"  书名=\"" << defaultBook.title()
              << "\"  作者=\"" << defaultBook.author()
              << "\"  出版社=\"" << defaultBook.publisher()
              << "\"  状态=" << defaultBook.availabilityText() << "\n";

    std::cout << "  Date         d1 : \"" << defaultDate.toString()
              << "\"  isValid()=" << yesNo(defaultDate.isValid())
              << "（空日期，用于表示“尚未归还”等未知日期）\n";

    std::cout << "  BorrowRecord r1 : 借阅人=\"" << defaultRecord.borrowerId()
              << "\"  条码号=\"" << defaultRecord.barcode()
              << "\"  已归还=" << yesNo(defaultRecord.returned())
              << "  逾期=" << defaultRecord.overdueDays()
              << " 天  罚款=" << formatMoney(defaultRecord.fine()) << " 元\n";

    std::cout << "  Library      lib: 借阅人 " << defaultLibrary.members().size()
              << " 人，图书 " << defaultLibrary.books().size()
              << " 种，借阅记录 " << defaultLibrary.records().size()
              << " 条；默认规则：学生 " << defaultLibrary.policy().studentMaxBorrow
              << " 本 / " << defaultLibrary.policy().studentBorrowDays
              << " 天 / " << formatMoney(defaultLibrary.policy().studentFinePerDay) << " 元每天\n";

    section("【二】重载构造函数：以相应参数构造需要的对象");

    Student student("20210001", "张三", "计算机科学与技术学院", 3);
    Student briefStudent("20210002", "李四", "信息工程学院");
    Teacher teacher("T1001", "赵老师", "计算机科学与技术学院", 10);
    Book book("BC-0001", "978-7-111-40701-0", "C++ Primer（第5版）",
              "Stanley B. Lippman", "机械工业出版社");
    Book borrowedBook("BC-0002", "978-7-302-52056-7", "数据结构（C++语言版）",
                      "邓俊辉", "清华大学出版社", false);
    Date borrowDate(2026, 9, 1);
    Date dueDate(2026, 10, 1);
    BorrowRecord record("20210001", "BC-0001", borrowDate, dueDate);

    std::cout << "  Student  student      (4 参) -> " << student.toString() << "\n";
    std::cout << "  Student  briefStudent (3 参) -> " << briefStudent.toString() << "\n";
    std::cout << "  Teacher  teacher      (4 参) -> " << teacher.toString() << "\n";
    std::cout << "  Book     book         (5 参) -> " << book.toString() << "\n";
    std::cout << "  Book     borrowedBook (6 参) -> " << borrowedBook.toString() << "\n";
    std::cout << "  Date     borrowDate   (3 参) -> " << borrowDate.toString() << "\n";
    std::cout << "  BorrowRecord record   (4 参) -> " << record.toString() << "\n";

    section("【三】对象调用成员函数");

    student.setName("张三丰");
    student.setDepartment("信息工程学院");
    student.setMaxBorrow(4);
    std::cout << "  修改后 student.toString()          -> " << student.toString() << "\n";
    std::cout << "  student.borrowDays(policy)         -> " << student.borrowDays(policy) << " 天\n";
    std::cout << "  student.finePerDay(policy)         -> " << formatMoney(student.finePerDay(policy))
              << " 元每天\n";
    std::cout << "  student.remainingQuota()           -> " << student.remainingQuota() << " 本\n";
    std::cout << "  teacher.borrowDays(policy)         -> " << teacher.borrowDays(policy) << " 天\n";
    std::cout << "  book.matches(\"primer\")             -> " << yesNo(book.matches("primer"))
              << "（忽略大小写匹配书名）\n";
    std::cout << "  book.matches(\"机械工业\")           -> " << yesNo(book.matches("机械工业"))
              << "（匹配出版社）\n";
    std::cout << "  borrowedBook.availabilityText()    -> " << borrowedBook.availabilityText() << "\n";
    std::cout << "  borrowDate.addDays(30)             -> " << borrowDate.addDays(30).toString() << "\n";
    std::cout << "  borrowDate.daysSince(2026-08-20)   -> " << borrowDate.daysSince(Date(2026, 8, 20))
              << " 天（两个日期相差的天数）\n";
    std::cout << "  record.overdueDaysAsOf(2026-10-05) -> "
              << record.overdueDaysAsOf(Date(2026, 10, 5)) << " 天";
    std::cout << "（应还 " << record.dueDate().toString() << "，已超过应还日期）\n";

    section("【四】Library 对象调用成员函数完成借书 / 还书");

    Library demoLibrary;
    const OperationResult addMemberResult =
        demoLibrary.addMember("STUDENT", "20210001", "张三", "计算机科学与技术学院",
                              demoLibrary.policy().studentMaxBorrow);
    std::cout << "  addMember -> " << (addMemberResult.ok ? "[成功] " : "[失败] ")
              << addMemberResult.message << "\n";

    const OperationResult addBookResult =
        demoLibrary.addBook(Book("BC-0001", "978-7-111-40701-0", "C++ Primer（第5版）",
                                 "Stanley B. Lippman", "机械工业出版社"));
    std::cout << "  addBook   -> " << (addBookResult.ok ? "[成功] " : "[失败] ")
              << addBookResult.message << "\n";

    const OperationResult borrowResult = demoLibrary.borrowBook("20210001", "BC-0001", Date(2026, 9, 1));
    std::cout << "  borrowBook-> " << (borrowResult.ok ? "[成功] " : "[失败] ")
              << borrowResult.message << "\n";

    const OperationResult returnResult = demoLibrary.returnBook("20210001", "BC-0001", Date(2026, 10, 8));
    std::cout << "  returnBook-> " << (returnResult.ok ? "[成功] " : "[失败] ")
              << returnResult.message << "\n";
    std::cout << "  该图书馆已收罚款合计 -> " << formatMoney(demoLibrary.totalCollectedFine()) << " 元\n";
    std::cout << "  统计信息 ->\n" << demoLibrary.statisticsReport(Date(2026, 10, 8)) << "\n";

    section("【五】多态：基类指针 Person* 调用虚函数");

    Person* person = &student;
    std::cout << "  Person* 指向 Student -> roleName=" << person->roleName()
              << "  idLabel=" << person->idLabel()
              << "  借期=" << person->borrowDays(policy) << " 天"
              << "  罚款=" << formatMoney(person->finePerDay(policy)) << " 元每天\n";
    person = &teacher;
    std::cout << "  Person* 指向 Teacher -> roleName=" << person->roleName()
              << "  idLabel=" << person->idLabel()
              << "  借期=" << person->borrowDays(policy) << " 天"
              << "  罚款=" << formatMoney(person->finePerDay(policy)) << " 元每天\n";
    std::cout << "  person->toString()   -> " << person->toString() << "\n";

    section("【六】重载构造函数：自定义借阅规则构造 Library");

    BorrowPolicy customPolicy;
    customPolicy.studentBorrowDays = 7;
    customPolicy.studentFinePerDay = 0.50;
    Library customLibrary(customPolicy);

    std::cout << "  customLibrary.policy() -> 学生借期 " << customLibrary.policy().studentBorrowDays
              << " 天，逾期罚款 " << formatMoney(customLibrary.policy().studentFinePerDay)
              << " 元每天\n";

    customLibrary.addMember("STUDENT", "S900", "演示学生", "计算机科学与技术学院",
                            customLibrary.policy().studentMaxBorrow);
    customLibrary.addBook(Book("BC-9001", "978-7-000-00000-1", "演示图书",
                               "演示作者", "演示出版社"));
    const OperationResult customBorrow = customLibrary.borrowBook("S900", "BC-9001", Date(2026, 9, 1));
    std::cout << "  borrowBook -> " << (customBorrow.ok ? "[成功] " : "[失败] ")
              << customBorrow.message << "\n";
    const OperationResult customReturn = customLibrary.returnBook("S900", "BC-9001", Date(2026, 9, 9));
    std::cout << "  returnBook -> " << (customReturn.ok ? "[成功] " : "[失败] ")
              << customReturn.message << "\n";

    std::cout << "\n演示结束：覆盖了“类的设计与访问控制、对象的定义、默认构造函数、"
                 "重载构造函数、成员函数的调用以及运行时多态”。\n";
}
