#include "ConsoleUtils.h"

#include <cstdlib>
#include <iostream>
#include <string>

#include "Config.h"
#include "TextUtils.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

bool g_inputEnded = false;

}

namespace console {

void setupEncoding() {
#if defined(_WIN32)

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

bool inputEnded() {
    return g_inputEnded;
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();

    std::string line;
    if (!std::getline(std::cin, line)) {
        g_inputEnded = true;
        return std::string();
    }
    return text::trim(line);
}

std::string readNonEmpty(const std::string& prompt) {
    for (;;) {
        const std::string line = readLine(prompt);
        if (g_inputEnded) {
            return std::string();
        }
        if (!line.empty()) {
            return line;
        }
        std::cout << "  输入不能为空，请重新输入。\n";
    }
}

std::string readStringOrDefault(const std::string& prompt, const std::string& current) {
    const std::string line = readLine(prompt + "（当前: " + current + "）: ");
    if (g_inputEnded || line.empty()) {
        return current;
    }
    return line;
}

int readIntInRange(const std::string& prompt, int minValue, int maxValue) {
    for (;;) {
        const std::string line = readLine(prompt);
        if (g_inputEnded) {
            return minValue;
        }
        long long value = 0;
        if (text::parseInt(line, value) && value >= minValue && value <= maxValue) {
            return static_cast<int>(value);
        }
        std::cout << "  输入无效，请输入 " << minValue << " ~ " << maxValue << " 之间的整数。\n";
    }
}

int readIntOrDefault(const std::string& prompt, int current, int minValue, int maxValue) {
    for (;;) {
        const std::string line = readLine(prompt + "（回车保持不变 = " + std::to_string(current) + "）: ");
        if (g_inputEnded || line.empty()) {
            return current;
        }
        long long value = 0;
        if (text::parseInt(line, value) && value >= minValue && value <= maxValue) {
            return static_cast<int>(value);
        }
        std::cout << "  输入无效，请输入 " << minValue << " ~ " << maxValue << " 之间的整数。\n";
    }
}

double readDoubleOrDefault(const std::string& prompt, double current, double minValue, double maxValue) {
    for (;;) {
        const std::string line = readLine(prompt + "（回车保持不变 = " + formatMoney(current) + "）: ");
        if (g_inputEnded || line.empty()) {
            return current;
        }
        double value = 0.0;
        if (text::parseDouble(line, value) && value >= minValue && value <= maxValue) {
            return value;
        }
        std::cout << "  输入无效，请输入 " << formatMoney(minValue) << " ~ "
                  << formatMoney(maxValue) << " 之间的数。\n";
    }
}

Date readDateOrDefault(const std::string& prompt, const Date& current) {
    for (;;) {
        const std::string line = readLine(prompt + "（回车 = " + current.toString() + "）: ");
        if (g_inputEnded || line.empty()) {
            return current;
        }
        Date parsed;
        if (Date::tryParse(line, parsed)) {
            return parsed;
        }
        std::cout << "  日期格式无效，请使用 2026-09-20、2026/9/20 或 20260920。\n";
    }
}

bool readYesNo(const std::string& prompt, bool defaultYes) {
    for (;;) {
        const std::string line = readLine(prompt + (defaultYes ? " [Y/n]: " : " [y/N]: "));
        if (g_inputEnded || line.empty()) {
            return defaultYes;
        }
        const std::string lower = text::toLowerAscii(line);
        if (lower == "y" || lower == "yes") {
            return true;
        }
        if (lower == "n" || lower == "no") {
            return false;
        }
        std::cout << "  请输入 y 或 n。\n";
    }
}

void printSeparator(char character, int width) {
    std::cout << std::string(static_cast<std::size_t>(width > 0 ? width : 0), character) << "\n";
}

void printTitle(const std::string& title) {
    printSeparator('=');
    std::cout << title << "\n";
    printSeparator('=');
}

void pause() {
    readLine("按回车键继续...");
}

}
