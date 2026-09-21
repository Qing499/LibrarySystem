#pragma once

#include <string>

#include "Date.h"

namespace console {

void setupEncoding();

bool inputEnded();

std::string readLine(const std::string& prompt);

std::string readNonEmpty(const std::string& prompt);

std::string readStringOrDefault(const std::string& prompt, const std::string& current);

int readIntInRange(const std::string& prompt, int minValue, int maxValue);

int readIntOrDefault(const std::string& prompt, int current, int minValue, int maxValue);

double readDoubleOrDefault(const std::string& prompt, double current, double minValue, double maxValue);

Date readDateOrDefault(const std::string& prompt, const Date& current);

bool readYesNo(const std::string& prompt, bool defaultYes);

void printSeparator(char character = '-', int width = 78);

void printTitle(const std::string& title);

void pause();

}
