#include "Teacher.h"

Teacher::Teacher()
    : Person("未设置", "未命名", "未填写", BorrowPolicy().teacherMaxBorrow) {}

Teacher::Teacher(const std::string& id, const std::string& name, const std::string& department)
    : Person(id, name, department, BorrowPolicy().teacherMaxBorrow) {}

Teacher::Teacher(const std::string& id, const std::string& name, const std::string& department, int maxBorrow)
    : Person(id, name, department, maxBorrow) {}

std::string Teacher::roleName() const {
    return "教师";
}

std::string Teacher::idLabel() const {
    return "工号";
}

std::string Teacher::typeTag() const {
    return "TEACHER";
}

int Teacher::borrowDays(const BorrowPolicy& policy) const {
    return policy.teacherBorrowDays;
}

double Teacher::finePerDay(const BorrowPolicy& policy) const {
    return policy.teacherFinePerDay;
}
