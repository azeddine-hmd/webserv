#pragma once

#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <stack>
#include <cstring>

#include "config_model.hpp"
#include "utils.hpp"

namespace ws {

    class Config {
        typedef std::vector<std::string>::iterator                              LineIter;

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
        Config( std::string const& path ): path(path) {
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

        std::vector<ServerBlock> parsingServerBlocks( std::string const& path ) const {
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
            fs.close();

            if (data.size() < MINIMUM_CONFIG_SIZE)
                throw ParsingException(formatMessage("file config too small"));

            // removing unnecessary information from data string for easy parsing later
            data = removeAllComments(data); // also whitespaces at end of line
            data = removeAllDuplicateEmptyLines(data);
            data = removeSpacesBeforeBrackets(data);
            data = removingWhitespacesAtBeginningOfLine(data);
            replaceAllTabsWithSpaces(data);

            try { checkingConfigSyntaxError(data); } catch (ParsingException& e) { throw e; }

            std::vector<ServerBlock> serversBlocks = getServersBlocks(data);
            //TODO: implement fillDataInStruct(serversBlocks)

            return serversBlocks;
        }

        std::vector<ServerBlock> getServersBlocks(std::string const& data ) const {
            std::vector<ServerBlock> serversBlocks;
            std::vector<std::string> serversBlocksData = getServersBlocksData(data);
            try { checkingEmptyServerBlock(serversBlocksData); } catch (ParsingException& e) { throw e; }

            for (size_t i = 0; i < serversBlocksData.size(); i++) {
                ServerBlock serverBlock;
                std::map<std::string, std::vector<std::string> > dataKeyValue = getServerBlockKeyValue(serversBlocksData[i]);
                serverBlock.dataKeyValue = dataKeyValue;
                serversBlocks.push_back(serverBlock);
            }

            return serversBlocks;
        }

        std::map<std::string, std::vector<std::string> > getServerBlockKeyValue( std::string const& serverBlockData ) const {
            std::map<std::string, std::vector<std::string> > dataKeyValue;
            std::stack<char> matches;
            char brackets[] = {'{', '}'};

            size_t i = 0;
            while (i < serverBlockData.size()) {
                std::string line;
                bool matchesDeferPop = false;

                // get line from string
                size_t j = i;
                while (j < serverBlockData.size() && serverBlockData[j] != '\n') {
                    line += serverBlockData[j];
                    j++;
                }
                i = ++j;
                std::string key = getKeyInLine(line);
                std::vector<std::string> values = getValuesInLine(line);

                // checking key
                if (key.empty()) {
                    continue;
                } else if (key == "{" && !values.empty()) {
                    throw ParsingException(formatMessage("found key after brackets"));
                } else if (key == "{" && values.empty()) {
                    matches.push(brackets[0]);
                    continue;
                } else if (key == "}" && values.empty()) {
                    matches.pop();
                    continue;
                } else if (key != "{"
                    && key != "}"
                    && (values.empty() || values.front() == "{" || values.front() == "}")
                ) {
                    throw ParsingException(formatMessage("key `%s` lacks value", key.c_str()));
                }

                // checking values
                if (!values.empty() && values.back() == "}") {
                    values.pop_back();
                    matchesDeferPop = true;
                } else if (!values.empty() && values.back() == "{") {
                    values.pop_back();
                    matches.push(brackets[0]);
                }
                for (size_t i = 0; i < values.size(); i++) {
                    if (i > 0 && i < values.size() - 1 && (values[i] == "{" || values[i] == "}") ) {
                        throw ParsingException(formatMessage("found brackets between values"));
                    }
                }

                std::pair<std::string, std::vector<std::string> > pair = std::make_pair(key, values);
                if ( matches.empty()
                     && key != "location"
                     && !dataKeyValue.insert(pair).second
                ) {
                    throw ParsingException(formatMessage("duplicate keyword `%s` were found", pair.first.c_str()));
                }

                if (matchesDeferPop) {
                    matches.pop();
                }
            }

            return dataKeyValue;
        }

        std::vector<std::string> getServersBlocksData( std::string const& data ) const {
            std::vector<std::string> serversBlocksData;

            for (size_t i = 0; data[i]; i++) {
                int64_t start = findBeginOfServerBlockData(data, i);
                if (start != -1) {
                    int64_t end = findEndOfServerBlockData(data, start);
                    serversBlocksData.push_back(data.substr(start, end - start  + 1));
                }
            }

            return serversBlocksData;
        }

        void checkingEmptyServerBlock(std::vector<std::string>& serversBlocksData) const {
            for (LineIter line = serversBlocksData.begin(); line != serversBlocksData.end(); line++) {
                std::string const& serverBlock = *line;
                bool isEmpty = true;
                for (size_t i = 0; i < serverBlock.size(); i++) {
                    if (serverBlock[i] != ' ' && serverBlock[i] != '\t' && serverBlock[i] != '\n')
                        isEmpty = false;
                }
                if (isEmpty)
                    throw ParsingException(formatMessage("found empty server block"));
            }
        }

        int64_t findEndOfServerBlockData( std::string const& data, size_t start ) const {
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

        int64_t findBeginOfServerBlockData( std::string const& data, size_t start ) const {
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

        void replaceAllTabsWithSpaces(std::string& data) const {
            for (size_t i = 0; i < data.size(); i++) {
                if (data[i] == '\t')
                    data[i] = ' ';
            }
        }

        std::string removeSpacesBeforeBrackets( std::string const& data ) const {
            std::vector<char> newData(data.begin(), data.end());


            for (int64_t i = static_cast<int64_t>(newData.size()) - 3; i >= 0; i--) {

                if (newData[i + 1] == '{' || newData[i + 1] == '}') {

                    while ( i - 1 >= 0
                            && (newData[i - 1] == ' ' || newData[i] == '\t' || newData[i] == '\n' )
                            && (newData[i] == ' ' || newData[i] == '\t')
                    ) {
                        newData.erase(newData.begin() + i);
                        i--;
                    }

                }

                if (i == 0 && newData[i] == ' ') {
                    newData.erase(newData.begin());
                }

                // avoiding overflowing size_t to max side after iteration ends
                if (i == 0)
                    break;
            }

            return std::string(newData.begin(), newData.end() - 1);
        }

        std::string removeAllDuplicateEmptyLines( std::string const& data ) const {
            std::vector<char> newData(data.begin(), data.end());
            newData.push_back('\0');

            for (size_t i = 0; newData[i]; i++) {
                if (newData[i + 1] && newData[i] == '\n' && newData[i + 1] == '\n') {
                    newData.erase(  newData.begin() + static_cast<int64_t>(i) );
                    i--;
                }
            }

            std::string result = std::string(newData.begin(), newData.end() - 1);

            return result;
        }

        // symbols for starting comments are ';' and '#'.
        // How it work? by adding character after character from old data to a newer one but this time skipping
        // characters that considered part of comments.
        std::string removeAllComments( std::string& data ) const {
            std::string newData;

            size_t i = 0;
            size_t dataLen = data.length();
            // iterator from beginning until the end
            while (i < dataLen) {
                // skip characters
                if (isAtEndOfLine(data, i)) {
                    while ( i < dataLen && data[i] != '\n' ) {
                        i++;
                    }
                }
                newData += data[i];
                i++;
            }
            // remove whitespaces from last line
            i = newData.size() - 1;
            while (i > 0 && ( newData[i] == ' ' || newData[i] == '\t')) {
                newData.pop_back();
                if ( i == 0 && (newData[i] == ' ' || newData[i] == '\t') ) {
                    break;
                }
            }
            return newData;
        }

        std::string removingWhitespacesAtBeginningOfLine( std::string const& data ) const {
            std::vector<std::string> newDataLines;
            std::vector<std::string> dataLines = split(data, "\n");


            for (LineIter lineIter = dataLines.begin(); lineIter != dataLines.end(); lineIter++) {
                std::string const &line = *lineIter;

                size_t i = 0;
                while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
                    i++;
                std::string newLine;
                while (i < line.size()) {
                    newLine += line[i];
                    i++;
                }
                newDataLines.push_back(newLine);
            }

            // joining all lines together to form a single string
            std::string newData;
            for (LineIter lineIter = newDataLines.begin(); lineIter != newDataLines.end(); lineIter++) {
                newData += *lineIter;
                newData += "\n";
            }

            return newData;
        }

        // 'End' in function name means last important character in a line or file
        bool isAtEndOfLine( std::string& data, size_t i ) const {
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

        void checkingConfigSyntaxError( std::string const& data ) const {
            try { checkingBrackets(data); } catch (ParsingException& e) { throw e; }

            // checking valid keys
            std::vector<std::string> dataLines = split(data, "\n");
            for (LineIter it = dataLines.begin(); it != dataLines.end(); it++) {
                std::string key = getKeyInLine(*it);
                if (!isValidKey(key)) {
                    throw ParsingException(formatMessage("Unknown keyword `%s`", key.c_str()));
                }
            }

        }

        bool isValidKey( std::string const& key ) const {
            bool isEqual = false;
            for (size_t i = 0; i < parse::keywordsLen; i++) {
                if (key == parse::validKeys[i])
                    isEqual = true;
            }

            return isEqual;
        }

        void checkingBrackets( std::string const& data ) const {
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
                        throw ParsingException(formatMessage("missing close bracket on line..., who needs line number just fix it yourself and considered as eyes exercise"));
                    }
                    matches.pop();
                }
            }
            if (!matches.empty())
                throw ParsingException(formatMessage("missing open bracket on line..., who needs line number just count them yourself and considered as eyes exercise"));
        }

        std::vector<std::string> getValuesInLine( std::string const& line ) const {
            size_t i = 0;
            while ( i < line.size() && !isWhitespace(line[i]) )
                i++;
            while (i < line.size() && isWhitespace(line[i]) )
                i++;
            std::string valueData(line.begin() + i, line.end());

            std::vector<std::string> values = split(valueData, " ");

            return values;
        }

        std::string getKeyInLine(std::string const& lineData ) const {
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

        bool isWhitespace(char c) const {
            if (c == ' ' || c == '\t')
                return true;
            return false;
        }

    public:
        class PathException : public std::exception {
        public:
            virtual char const* what() const throw() {
                return strerror(errno);
            }
        }; // class PathException

        /*
         * free msg (char*) before quitting
         */
        class ParsingException : public std::exception {
        public:
            char* msg;

            /*
             * message (char*) that holds exception description, should be allocated via malloc(3)
             */
            explicit ParsingException( char *message ): msg(message) {

            }

            virtual char const* what() const throw() {
                if (!msg)
                    return "";
                else
                    return msg;
            }
        }; // class ParsingException

    }; // class Config

    char const* ws::Config::DEFAULT_CONFIG_PATH = "config/default.conf";
    size_t      ws::Config::MINIMUM_CONFIG_SIZE = 10;

} // namespace ws