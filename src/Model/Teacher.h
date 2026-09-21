#pragma once

#include "Person.h"

class Teacher : public Person {
public:

    Teacher();

    Teacher(const std::string& id, const std::string& name, const std::string& department);

    Teacher(const std::string& id, const std::string& name, const std::string& department, int maxBorrow);

    ~Teacher() override = default;

    std::string roleName() const override;
    std::string idLabel() const override;
    std::string typeTag() const override;
    int borrowDays(const BorrowPolicy& policy) const override;
    double finePerDay(const BorrowPolicy& policy) const override;
};
