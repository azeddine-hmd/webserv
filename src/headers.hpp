#pragma once

#include <string>
#include "StatusCode.hpp"
#include <cstdarg>
#include <sys/time.h>

namespace ws {

    class Headers {
        std::string _Content;
        bool        _HaveSeparator;

        Headers();
    public:

        Headers( int status ) {
            _HaveSeparator = false;
            _Content += "HTTP/1.1 " + std::to_string(status) + " " + StatusCode::reasonPhrase(status) + "\r\n";
            _Content += "Date: " + getFormattedDate() +  "\r\n";
        }

        void add( std::string attribute, std::string value ) {
            _Content += attribute + ": " + value + "\r\n";
        }

        template<int n>
        void add( std::string attribute, ...) {
            int len = n - 1;
            va_list args;

            va_start(args, attribute);
            _Content += attribute + ": ";
            char const * value = va_arg(args, char const*);
            _Content += value;
            len--;
            while (len--) {
                char const * value = va_arg(args, char const*);
                _Content += "; " + std::string(value);
            }
            _Content += "\r\n";
            va_end(args);
        }

        void addBodySeparator() {
            _HaveSeparator = true;
            _Content += "\r\n";
        }

        std::string content() {
            if (!_HaveSeparator)
                throw std::error_condition();
            return _Content;
        }

    private:

        std::string getFormattedDate() {
            char buffer[30];
            struct timeval tv;

            bzero(buffer, 30);
            gettimeofday(&tv, NULL);
            strftime(buffer, 29, "%a, %d %b %Y %T %Z", gmtime(&tv.tv_sec));
            return (std::string(buffer));
        }

    };

} // namespace ws