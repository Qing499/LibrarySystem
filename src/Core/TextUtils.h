#pragma once

#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>

namespace text {

inline std::string trim(const std::string& value) {
    std::size_t begin = 0;
    std::size_t end = value.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return value.substr(begin, end - begin);
}

inline std::string toLowerAscii(const std::string& value) {
    std::string result = value;
    for (std::size_t i = 0; i < result.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(result[i]);
        if (ch < 0x80) {
            result[i] = static_cast<char>(std::tolower(ch));
        }
    }
    return result;
}

inline bool containsIgnoreCaseAscii(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) {
        return true;
    }
    return toLowerAscii(haystack).find(toLowerAscii(needle)) != std::string::npos;
}

inline bool equalsIgnoreCaseAscii(const std::string& left, const std::string& right) {
    return toLowerAscii(left) == toLowerAscii(right);
}

inline std::vector<std::string> split(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == delimiter) {
            parts.push_back(current);
            current.clear();
        } else {
            current.push_back(value[i]);
        }
    }
    parts.push_back(current);
    return parts;
}

inline std::string join(const std::vector<std::string>& parts, char delimiter) {
    std::string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            result.push_back(delimiter);
        }
        result += parts[i];
    }
    return result;
}

inline std::string sanitizeField(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (std::size_t i = 0; i < value.size(); ++i) {
        const char ch = value[i];
        if (ch == '|') {
            result.push_back('/');
        } else if (ch == '\n' || ch == '\r') {
            result.push_back(' ');
        } else {
            result.push_back(ch);
        }
    }
    return result;
}

inline bool parseInt(const std::string& value, long long& result) {
    const std::string trimmed = trim(value);
    if (trimmed.empty()) {
        return false;
    }
    errno = 0;
    const char* begin = trimmed.c_str();
    char* end = nullptr;
    const long long parsed = std::strtoll(begin, &end, 10);
    if (end == begin || end == nullptr || *end != '\0' || errno == ERANGE) {
        return false;
    }
    result = parsed;
    return true;
}

inline bool parseDouble(const std::string& value, double& result) {
    const std::string trimmed = trim(value);
    if (trimmed.empty()) {
        return false;
    }
    errno = 0;
    const char* begin = trimmed.c_str();
    char* end = nullptr;
    const double parsed = std::strtod(begin, &end);
    if (end == begin || end == nullptr || *end != '\0' || errno == ERANGE) {
        return false;
    }
    result = parsed;
    return true;
}

}
