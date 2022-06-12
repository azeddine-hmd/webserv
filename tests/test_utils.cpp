#pragma once

#include <iostream>

# define COLORS_DEFAULT "\033[0;0m"
# define COLORS_BLACK "\033[0;30m"
# define COLORS_RED "\033[031m"
# define COLORS_RED_BOLD "\033[1;031m"
# define COLORS_GREEN "\033[0;32m"
# define COLORS_YELLOW "\033[0;33m"
# define COLORS_BLUE "\033[0;34m"
# define COLORS_PURPLE "\033[0;35m"
# define COLORS_CYAN "\033[0;36m"
# define COLORS_CYAN_BOLD "\033[1;36m"
# define COLORS_WHITE "\033[0;36m"
# define COLORS_WHITE_BOLD "\033[1;36m"

void valid_output(std::string const& message) {
    std::cout << COLORS_GREEN << message << COLORS_DEFAULT << std::endl;
}

void failed_output(std::string const& message) {
    std::cout << COLORS_RED << message << COLORS_DEFAULT << std::endl;
}

void test_passed() {
    valid_output("Test: PASSED!");
}

void test_failed() {
    failed_output("Test: FAILED!");
}

void headerTest(std::string const& subject) {
    std::cout << COLORS_CYAN_BOLD << "=========[ Testing: " << subject << " ]========="  << COLORS_DEFAULT << std::endl;
}

void separator() {
    std::cout << COLORS_CYAN << "=========" << COLORS_DEFAULT << std::endl;
}

// conditional functions

template<typename T>
void assertEqual(T const& a, T const& b) {
    if (a == b) {
        test_passed();
    } else {
        test_failed();
    }
}

template<typename T>
void assertNotEqual(T const& a, T const& b) {
    if (a != b) {
        test_passed();
    } else {
        test_failed();
    }
}
