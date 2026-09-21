#pragma once

#include "Person.h"

class Student : public Person {
public:

    Student();

    Student(const std::string& id, const std::string& name, const std::string& department);

    Student(const std::string& id, const std::string& name, const std::string& department, int maxBorrow);

    ~Student() override = default;

    std::string roleName() const override;
    std::string idLabel() const override;
    std::string typeTag() const override;
    int borrowDays(const BorrowPolicy& policy) const override;
    double finePerDay(const BorrowPolicy& policy) const override;
};
