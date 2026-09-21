#include "Book.h"

#include <sstream>

#include "TextUtils.h"

Book::Book()
    : barcode_(),
      isbn_(),
      title_(),
      author_(),
      publisher_(),
      available_(true) {}

Book::Book(const std::string& barcode,
           const std::string& isbn,
           const std::string& title,
           const std::string& author,
           const std::string& publisher,
           bool available)
    : barcode_(barcode),
      isbn_(isbn),
      title_(title),
      author_(author),
      publisher_(publisher),
      available_(available) {}

bool Book::matches(const std::string& keyword) const {
    const std::string key = text::trim(keyword);
    if (key.empty()) {
        return true;
    }
    return text::containsIgnoreCaseAscii(barcode_, key) ||
           text::containsIgnoreCaseAscii(isbn_, key) ||
           text::containsIgnoreCaseAscii(title_, key) ||
           text::containsIgnoreCaseAscii(author_, key) ||
           text::containsIgnoreCaseAscii(publisher_, key);
}

std::string Book::toString() const {
    std::ostringstream out;
    out << "条码号: " << barcode_
        << " | ISBN: " << isbn_
        << " | 书名: " << title_
        << " | 作者: " << author_
        << " | 出版社: " << publisher_
        << " | 状态: " << availabilityText();
    return out.str();
}
