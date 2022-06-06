#pragma once

#include <vector>
#include <string>
#include <cstdarg>

namespace ws {

    std::vector<std::string> split( std::string const& s, std::string const delim) {
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

    char* formatMessage(const char *fmt, ...) {
        va_list args;
        char*   output = NULL;

        va_start(args,fmt);
        vasprintf(&output, fmt, args);
        va_end(args);

        return output;
    }

}