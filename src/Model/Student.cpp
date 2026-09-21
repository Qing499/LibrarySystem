#include "Student.h"

Student::Student()
    : Person("未设置", "未命名", "未填写", BorrowPolicy().studentMaxBorrow) {}

Student::Student(const std::string& id, const std::string& name, const std::string& department)
    : Person(id, name, department, BorrowPolicy().studentMaxBorrow) {}

Student::Student(const std::string& id, const std::string& name, const std::string& department, int maxBorrow)
    : Person(id, name, department, maxBorrow) {}

std::string Student::roleName() const {
    return "学生";
}

std::string Student::idLabel() const {
    return "学号";
}

std::string Student::typeTag() const {
    return "STUDENT";
}

int Student::borrowDays(const BorrowPolicy& policy) const {
    return policy.studentBorrowDays;
}

double Student::finePerDay(const BorrowPolicy& policy) const {
    return policy.studentFinePerDay;
}
