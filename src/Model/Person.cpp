#include "Person.h"

#include <sstream>
#include <utility>

Person::Person()
    : id_("未设置"),
      name_("未命名"),
      department_("未填写"),
      maxBorrow_(0),
      borrowedCount_(0) {}

Person::Person(std::string id, std::string name, std::string department, int maxBorrow)
    : id_(std::move(id)),
      name_(std::move(name)),
      department_(std::move(department)),
      maxBorrow_(maxBorrow),
      borrowedCount_(0) {}

std::string Person::toString() const {
    std::ostringstream out;
    out << '[' << roleName() << "] " << idLabel() << ": " << id_
        << " | 姓名: " << name_
        << " | 院系: " << department_
        << " | 最大可借: " << maxBorrow_
        << " | 当前在借: " << borrowedCount_
        << " | 剩余额度: " << remainingQuota();
    return out.str();
}
