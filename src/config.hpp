#pragma once

#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include "config_model.hpp"
#include "utils.hpp"

namespace ws {

    class Config {
        typedef std::vector<std::string>::iterator      LineIter;
        typedef std::vector<char>::iterator             CharIter;

    public:
        std::string                 path;
        std::vector<ServerBlock>    serverBlocks;

        static char const*          DEFAULT_CONFIG_PATH;
        static size_t               MINIMUM_CONFIG_SIZE;


        /*
         * throw Exception:
         *      - PathException
         *      - ParsingException
         */
        Config(): path(DEFAULT_CONFIG_PATH) {
            try {
                init();
            } catch (PathException& e) {
                throw e;
            } catch (ParsingException& e) {
                throw e;
            }
        }

        /*
         * throw Exception:
         *      - PathException
         *      - ParsingException
         */
        Config(std::string const& path): path(path) {
            try {
                init();
            } catch (PathException& e) {
                throw e;
            } catch (ParsingException& e) {
                throw e;
            }
        }

    private:

        void init() {
            try {
                serverBlocks = parsingServerBlocks(path);
            } catch (PathException& e) {
                throw e;
            } catch (ParsingException& e) {
                throw e;
            }

            if (serverBlocks.empty()) {
               throw ParsingException(formatMessage("no server block were found"));
            }
        }

//TODO: remove it
//        std::vector<Server_block> parsse_the_config_file(std::string path){
//            std::string data, line;
//            Server_block block;
//            std::ifstream path_stream(path.c_str());
//            while (getline(path_stream, line)){
//                data += line;
//                data += "\n";
//            }
//            data = remove_all_comment(data);
//            check_error_in_file(data);
//            std::vector<Server_block> all_server_block = get_all_server_blocks(data);
//            fill_data_in_struct(all_server_block);
//            return (all_server_block);
//        }

        std::vector<ServerBlock> parsingServerBlocks(std::string const& path) const {
            std::ifstream fs(path.c_str());
            if (!fs) {
                throw PathException();
            }

            // read config file into string
            std::string line, data;
            while (getline(fs, line)) {
                data += line;
                data += "\n";
            }

            if (data.size() < MINIMUM_CONFIG_SIZE)
                throw ParsingException(formatMessage("file config too small"));

            // removing unnecessary information from data string for easy parsing later
            data = removeAllComments(data);
            data = removeAllDuplicateEmptyLines(data);
            data = removeSpacesBeforeBrackets(data);

            try {
                checkingConfigSyntaxError(data);
            } catch (ParsingException& e) {
                throw e;
            }

            std::vector<ServerBlock> serverBlock = getServersBlock(data);

            fs.close();
            return std::vector<ServerBlock>();
        }

        std::vector<ServerBlock> getServersBlock(std::string const& data) const {
            std::vector<std::string> serversBlockData = getServersBlockData(data);
            //TODO: continue

            return std::vector<ServerBlock>();
        }

        std::vector<std::string> getServersBlockData(std::string const& data) const {
            std::vector<std::string> serversBlockData;

            for (size_t i = 0; data[i]; i++) {
                int64_t start = findBeginOfServerBlockData(data, i);
                if (start != -1) {
                    int64_t end = findEndOfServerBlockData(data, start);
                    serversBlockData.push_back(data.substr(start, end - start  + 1));
                }
            }

            return serversBlockData;
        }

        int64_t findEndOfServerBlockData(std::string const& data, size_t start) const {
            char brackets[] = {'{', '}'};
            std::stack<char> matches;

            for (size_t i = start + 1; i < data.length(); i++) {
                if (data[i] == brackets[0]) {
                    matches.push(brackets[0]);
                } else if (data[i] == brackets[1]) {
                    if (matches.empty()) {
                        return static_cast<int64_t>(i) - 1;
                    }
                    matches.pop();
                }
            }

            return -1;
        }

        int64_t findBeginOfServerBlockData(std::string const& data, size_t start) const {
            std::string const& keyword = "server";
            size_t i = 0;
            while ( data[i] && keyword[i] == data[start + i] ) {
                i++;
            }

            if (i == 6) {
                i += start;
                if ( i  + 2 < data.length() && (data[i] == ' ' || data[i] == '\n') ){
                    if (data[i + 1] == '{')
                        return static_cast<int64_t>(i) + 2;
                }
            }

            return -1;
        }

        std::string removeSpacesBeforeBrackets(std::string const& data) const {
            std::vector<char> newData(data.begin(), data.end());


            for (size_t i = newData.size() - 3; i >= 0; i--) {

                if (newData[i + 1] == '{' || newData[i + 1] == '}') {

                    while ( i - 1 >= 0
                            && (newData[i - 1] == ' ' || newData[i] == '\t' || newData[i] == '\n' )
                            && (newData[i] == ' ' || newData[i] == '\t')
                    ) {
                        newData.erase(newData.begin() + i);
                        i--;
                    }
                    //TODO: fix extra space or tab after newline and before bracket
                }

                if (i == 0 && newData[i] == ' ') {
                    newData.erase(newData.begin());
                }

                // avoiding overflowing size_t to max side after iteration ends
                if (i == 0)
                    break;
            }

            std::string result = std::string(newData.begin(), newData.end());
            std::cout << "'" << result << "'" << std::endl; //TODO: remove it
            return result;
        }

        std::string removeAllDuplicateEmptyLines(std::string const& data) const {
            std::vector<char> newData(data.begin(), data.end());
            newData.push_back('\0');

            for (size_t i = 0; newData[i]; i++) {
                if (newData[i + 1] && newData[i] == '\n' && newData[i + 1] == '\n') {
                    newData.erase(  newData.begin() + static_cast<size_t>(i) );
                    i--;
                }
            }

            return std::string(newData.begin(), newData.end());
        }

        // symbols for starting comments are ';' and '#'.
        // How it work? by adding character after character from old data to a newer one but this time skipping
        // characters that considered part of comments.
        std::string removeAllComments(std::string& data) const {
            std::string newData;

            size_t i = 0;
            // iterator from beginning until the end
            while (i < data.length()) {
                // skip characters
                if (isAtEndOfLine(data, i)) {
                    while (i < data.length() && data[i] != '\n') {
                        i++;
                    }
                }
                newData += data[i];
                i++;
            }
            // iterator from the end until beginning
            i = newData.length() - 1;
            while (i >= 0 && ( newData[i] == ' ' || newData[i] == '\t')) {
                newData.pop_back();
            }
            return newData;
        }

        // 'End' in function name means last important character in a line or file
        bool isAtEndOfLine(std::string& data, size_t i) const {
            if (data[i] == '#' || data[i] == ';') {
                return true;
            } else if (data[i] == ' ') {
                // skip whitespaces
                while (i < data.length() && data[i] == ' ') {
                    i++;
                }
                if (i == data.length() || data[i] == '\n' || data[i] == ';' || data[i] == '#')
                    return true;
            }

            return false;
        }

        void checkingConfigSyntaxError(std::string const& data) const {
            try {
                checkingBrackets(data);
            } catch (ParsingException& e) {
                throw e;
            }

            // checking valid keys
            std::vector<std::string> dataLines = split(data, "\n");
            for (LineIter it = dataLines.begin(); it != dataLines.end(); it++) {
                std::string key = getKey(*it);
                if (!isValidKey(key)) {
                    throw ParsingException(formatMessage("Unknown keyword `%s`", key.c_str()));
                }
            }

        }

        bool isValidKey(std::string const& key) const {
            bool isEqual = false;
            for (size_t i = 0; i < parse::keywordsLen; i++) {
                if (key == parse::validKeys[i])
                    isEqual = true;
            }

            return isEqual;
        }

        void checkingBrackets(std::string const& data) const {
            char                brackets[] = {'{', '}'};
            size_t              line = 1;
            std::stack<char>    matches;

            for (size_t i = 0; i < data.length(); i++) {
                if (data[i] == '\n')
                    line++;
                if (data[i] == brackets[0]) {
                    matches.push(brackets[0]);
                } else if (data[i] == brackets[1]) {
                    if (matches.empty()) {
                        throw ParsingException(formatMessage("missing close bracket"));
                    }
                    matches.pop();
                }
            }
            if (!matches.empty())
                throw ParsingException(formatMessage("missing open bracket"));
        }

        std::string getKey(std::string const& lineData) const {
            std::string key;
            size_t i = 0;
            while ( lineData[i] && (lineData[i] == ' ' || lineData[i] == '\t') )
                i++;
            while ( lineData[i] && (lineData[i] != ' ' && lineData[i] != '\t') ) {
                key += lineData[i];
                i++;
            }

            return key;
        }

    public:
        class PathException : public std::exception {
        public:
            virtual char const* what() const throw() {
                return strerror(errno);
            }
        };

        /*
         * free msg (char*) before quitting
         */
        class ParsingException : public std::exception {
        public:
            char* msg;

            /*
             * message (char*) that holds exception description, should be allocated via malloc(3)
             */
            explicit ParsingException(char *message): msg(message) {

            }

            virtual char const* what() const throw() {
                if (!msg)
                    return "";
                else
                    return msg;
            }
        };

    };

    char const* ws::Config::DEFAULT_CONFIG_PATH = "config/default.conf";
    size_t      ws::Config::MINIMUM_CONFIG_SIZE = 10;

} // namespace ws