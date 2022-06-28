#pragma once

#include <vector>
#include <string>
#include <cstdarg>

# define COLORS_RED_BOLD "\033[1;031m"
# define COLORS_DEFAULT "\033[0;0m"

namespace ws {

    std::vector<std::string> split( std::string const& s, std::string const delim ) {
        std::vector<std::string> result;
        size_t index = s.find(delim);
        size_t position = 0;
        std::string str;
        while (index != (size_t)-1) {
            str = s.substr(position, index - position);
            if (str.length())
                result.push_back(str);
            position = index + delim.length();
            index = s.find(delim, index + 1);
        }
        str = s.substr(position, s.length() - position);
        if (str != delim && !str.empty())
            result.push_back(str);
        return (result);
    }

    char* formatMessage( const char *fmt, ... ) {
        va_list args;
        char*   output = NULL;

        va_start(args,fmt);
        vasprintf(&output, fmt, args);
        va_end(args);

        return output;
    }

    void printError( size_t len, char const* err, ... ) {
        va_list args;


        va_start(args, err);
        std::cerr << COLORS_RED_BOLD << err;
        len--;
        while (len--) {
            char const* s = va_arg(args, char const*);
            std::cerr << s;
        }

        std::cerr << COLORS_DEFAULT << std::endl;
        va_end(args);
    }

    bool isNumber(std::string const& str) {
        for (size_t i = 0; i < str.size(); i++) {
            if (!std::isdigit(str[i])) {
                return false;
            }
        }

        return true;
    }

    std::string reverseString(std::string const& str) {
        std::string reversed;

        for (int64_t i = str.size() - 1; i >= 0; i--) {
            reversed += str[i];
        }

        return reversed;
    }

    std::string getNextLine(std::string& buffer)
    {
        int delimPos = buffer.find("\r\n");
        std::string ret = buffer.substr(0, delimPos);
        buffer = buffer.substr(delimPos + 2, buffer.size() - 1);
        std::cout << ret << std::endl;
        return ret;
    }
}