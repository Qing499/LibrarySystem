#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Book.h"
#include "BorrowRecord.h"
#include "Config.h"
#include "Person.h"

struct OperationResult {
    bool ok = false;
    std::string message;
    double fine = 0.0;
    long long overdueDays = 0;
};

class Library {
public:

    Library();

    explicit Library(const BorrowPolicy& policy);

    const BorrowPolicy& policy() const { return policy_; }
    void setPolicy(const BorrowPolicy& policy) { policy_ = policy; }

    OperationResult addMember(const std::string& role,
                              const std::string& id,
                              const std::string& name,
                              const std::string& department,
                              int maxBorrow);
    OperationResult updateMember(const std::string& id,
                                 const std::string& newName,
                                 const std::string& newDepartment,
                                 int newMaxBorrow);
    OperationResult removeMember(const std::string& id);

    Person* findMember(const std::string& id);
    const Person* findMember(const std::string& id) const;

    std::vector<const Person*> searchMembers(const std::string& keyword,
                                             const std::string& role = std::string()) const;
    const std::vector<std::unique_ptr<Person>>& members() const { return members_; }

    OperationResult addBook(const Book& book);

    OperationResult updateBook(const std::string& barcode, const Book& values);
    OperationResult removeBook(const std::string& barcode);

    Book* findBook(const std::string& barcode);
    const Book* findBook(const std::string& barcode) const;
    std::vector<const Book*> searchBooks(const std::string& keyword) const;
    const std::vector<Book>& books() const { return books_; }

    OperationResult borrowBook(const std::string& memberId,
                               const std::string& barcode,
                               const Date& date);
    OperationResult returnBook(const std::string& memberId,
                               const std::string& barcode,
                               const Date& date);

    OperationResult returnBookByBarcode(const std::string& barcode, const Date& date);

    std::vector<const BorrowRecord*> recordsOfMember(const std::string& memberId) const;
    std::vector<const BorrowRecord*> activeRecordsOfMember(const std::string& memberId) const;
    std::vector<const BorrowRecord*> overdueRecords(const Date& reference) const;
    const std::vector<BorrowRecord>& records() const { return records_; }

    double outstandingFineOf(const std::string& memberId, const Date& reference) const;
    double totalOutstandingFine(const Date& reference) const;
    double totalCollectedFine() const;
    std::string statisticsReport(const Date& reference) const;

    bool saveToFile(const std::string& path, std::string& message) const;
    bool loadFromFile(const std::string& path, std::string& message);
    static std::string defaultDataFile();
    void seedDemoData();

    int refreshDerivedState();

private:
    BorrowRecord* findActiveRecord(const std::string& memberId, const std::string& barcode);
    const BorrowRecord* findActiveRecord(const std::string& memberId, const std::string& barcode) const;
    static bool validateBookFields(const Book& book, std::string& error);

    std::vector<std::unique_ptr<Person>> members_;
    std::vector<Book> books_;
    std::vector<BorrowRecord> records_;
    BorrowPolicy policy_;
};
