#pragma once

#include <iostream>

namespace ws {

    class Config {
        std::string mPath;

    public:
        static char const* DEFAULT_CONFIG_PATH;

    private:

    public:
        Config(): mPath(DEFAULT_CONFIG_PATH) {

        }

        Config(std::string const& path): mPath(path) {

        }

        Config(Config const& other) {
            *this = other;
        }

        Config& operator=(Config const& rhs) {
            mPath = rhs.mPath;
            return *this;
        }

        std::string const& getPath() const {
            return mPath;
        }

        class FileNotExistException : std::exception {

        };
    };

    char const* ws::Config::DEFAULT_CONFIG_PATH = "config/default.conf";

    std::ostream& operator<<(std::ostream& out, ws::Config const& obj) {
        out << "[ path: " << obj.getPath() << "]" << std::endl;
        return out;
    }

}

