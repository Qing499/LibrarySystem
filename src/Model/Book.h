#pragma once

#include <string>

class Book {
public:

    Book();

    Book(const std::string& barcode,
         const std::string& isbn,
         const std::string& title,
         const std::string& author,
         const std::string& publisher,
         bool available = true);

    const std::string& barcode() const { return barcode_; }
    const std::string& isbn() const { return isbn_; }
    const std::string& title() const { return title_; }
    const std::string& author() const { return author_; }
    const std::string& publisher() const { return publisher_; }
    bool available() const { return available_; }

    void setBarcode(const std::string& barcode) { barcode_ = barcode; }
    void setIsbn(const std::string& isbn) { isbn_ = isbn; }
    void setTitle(const std::string& title) { title_ = title; }
    void setAuthor(const std::string& author) { author_ = author; }
    void setPublisher(const std::string& publisher) { publisher_ = publisher; }
    void setAvailable(bool available) { available_ = available; }

    std::string availabilityText() const { return available_ ? "可借" : "已借出"; }

    bool matches(const std::string& keyword) const;

    std::string toString() const;

private:
    std::string barcode_;
    std::string isbn_;
    std::string title_;
    std::string author_;
    std::string publisher_;
    bool available_ = true;
};
