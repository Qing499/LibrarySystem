#pragma once

#include <string>

#include "Config.h"

class Person {
public:
    virtual ~Person() = default;

    Person(std::string id, std::string name, std::string department, int maxBorrow);

    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& department() const { return department_; }
    int maxBorrow() const { return maxBorrow_; }
    int borrowedCount() const { return borrowedCount_; }
    int remainingQuota() const { return maxBorrow_ - borrowedCount_; }

    void setName(const std::string& name) { name_ = name; }
    void setDepartment(const std::string& department) { department_ = department; }
    void setMaxBorrow(int maxBorrow) { maxBorrow_ = maxBorrow; }
    void setBorrowedCount(int count) { borrowedCount_ = count; }

    virtual std::string roleName() const = 0;
    virtual std::string idLabel() const = 0;
    virtual std::string typeTag() const = 0;
    virtual int borrowDays(const BorrowPolicy& policy) const = 0;
    virtual double finePerDay(const BorrowPolicy& policy) const = 0;

    std::string toString() const;

protected:

    Person();

    std::string id_;
    std::string name_;
    std::string department_;
    int maxBorrow_ = 0;
    int borrowedCount_ = 0;
};
