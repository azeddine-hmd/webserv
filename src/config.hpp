#pragma once

#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <stack>
#include <cstring>

#include "config_model.hpp"
#include "defaults.hpp"
#include "utils.hpp"

namespace ws {

    class Config {
        typedef std::vector<std::string>::iterator                            LineIter;
        typedef std::map<std::string, std::vector<std::string> >              MapKeyValue;
        typedef std::map<std::string, std::vector<std::string> >::iterator    MapKeyValueIter;

    public:
        std::string                 path;
        std::vector<ServerBlock>    serverBlocks;

        static char const*          DEFAULT_CONFIG_PATH;
        static size_t               MINIMUM_CONFIG_SIZE;


        /*
         *  throw Exception:
         *      - PathException
         *      - ParsingException
         */
        Config( std::string const& path ): path(path) {
            serverBlocks = parsingServerBlocks(path);
            if (serverBlocks.empty()) {
                throw ParsingException(formatMessage("no server block were found"));
            }
        }

    private:
        std::vector<ServerBlock> parsingServerBlocks( std::string const& path ) const {
            std::ifstream fs(path.c_str());
            if (!fs) {
                throw PathException();
            }

            // read config file into string (i call it `data`)
            std::string line, data;
            while (getline(fs, line)) {
                data += line;
                data += "\n";
            }
            fs.close();


            // removing unnecessary information from data string for easy parsing later
            data = removeAllComments(data); // also, whitespaces at end of line
            data = removeAllDuplicateEmptyLines(data);
            data = removeSpacesBeforeBrackets(data);
            data = removingWhitespacesAtBeginningOfLine(data);
            replaceAllTabsWithSpaces(data);

            checkingConfigSyntaxError(data);

            std::vector<ServerBlock> serversBlocks = getServersBlocks(data);
            process(serversBlocks);

            return serversBlocks;
        }

        std::vector<ServerBlock> getServersBlocks(std::string const& data ) const {
            std::vector<ServerBlock> serversBlocks;

            std::vector<std::string> serversBlocksData = getServersBlocksData(data);
            checkingEmptyBlock(serversBlocksData);

            for (size_t i = 0; i < serversBlocksData.size(); i++) {
                ServerBlock serverBlock;

                // parsing server block
                std::map<std::string, std::vector<std::string> > dataKeyValue = getServerBlockKeyValue(serversBlocksData[i]);
                serverBlock.setDataKeyValue(dataKeyValue);

                std::vector<std::string> locationBlockData = getLocationBlocksData(serversBlocksData[i]);
                checkingEmptyBlock(locationBlockData);
                for (size_t j = 0; j < locationBlockData.size(); j++) {
                    LocationBlock locationBlock;
                    std::map<std::string, std::vector<std::string> > locationDataKeyValue = getLocationBlockKeyValue(locationBlockData[i]);
                    locationBlock.setDataKeyValue(locationDataKeyValue);
                    serverBlock.locations.push_back(locationBlock);
                }

                serversBlocks.push_back(serverBlock);
            }

            return serversBlocks;
        }

        std::map<std::string, std::vector<std::string> > getLocationBlockKeyValue(std::string const& locationBlockData) const {
            std::map<std::string, std::vector<std::string> > dataKeyValue;

            size_t i = 0;
            while (i < locationBlockData.size()) {
                std::string line;

                // get line from string
                size_t j = i;
                while (j < locationBlockData.size() && locationBlockData[j] != '\n') {
                    line += locationBlockData[j];
                    j++;
                }
                i = ++j;
                std::string key = getKeyInLine(line);
                std::vector<std::string> values = getValuesInLine(line);

                if (key.empty()) {
                    continue;
                } else if (values.empty()) {
                    throw ParsingException(formatMessage("key `%s` lacks value in location block", key.c_str()));
                }

                // checking values

                std::pair<std::string, std::vector<std::string> > pair = std::make_pair(key, values);
                if (!dataKeyValue.insert(pair).second) {
                    throw ParsingException(formatMessage("duplicate keyword `%s` were found in location block", pair.first.c_str()));
                }
            }

            return dataKeyValue;
        }

        std::vector<std::string> getLocationBlocksData(std::string const& data) const {
            std::vector<std::string> locationBlocksData;

            for (size_t i = 0; data[i]; i++) {

                int64_t start = findFirstIndexOfLocationBlock(data, i);
                if (start != -1) {
                    int64_t end = findLastIndexOfLocationBlock(data, start);
                    if (end == -1) {
                        throw ParsingException(formatMessage("there was an error in location block"));
                    }
                    locationBlocksData.push_back(data.substr(start, end - start  + 1));
                }

            }

            return locationBlocksData;
        }

        int64_t findFirstIndexOfLocationBlock( std::string const& data, size_t start ) const {
            std::string const& keyword = "location ";

            size_t i = 0;
            while ( data[i] && keyword[i] == data[start + i] ) {
                i++;
            }

            if (i == 9) {
                i += start;
                while (i < data.size() && data[i] != '{' ) {
                    i++;
                }
                return i + 1;
            }

            return -1;
        }

        int64_t findLastIndexOfLocationBlock( std::string const& data, size_t start ) const {
            for (size_t end = start; end < data.size(); end++) {
                if (data[end] == '{') {
                    throw ParsingException(formatMessage("found block inside location block"));
                }

                if (end + 1 < data.size() && data[end] == '}') {
                    return static_cast<int64_t>(end) - 1;
                }
            }

            return -1;
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
                    throw ParsingException(formatMessage("key `%s` lacks value in server block", key.c_str()));
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
                    throw ParsingException(formatMessage("duplicate keyword `%s` were found in server block", pair.first.c_str()));
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

                int64_t start = findStartingIndexOfServerBlock(data, i);
                if (start != -1) {
                    int64_t end = findLastIndexOfBlock(data, start);
                    if (end == -1) {
                        throw ParsingException(formatMessage("There was an error inside server block"));
                    }
                    serversBlocksData.push_back(data.substr(start, end - start  + 1));
                }

            }

            return serversBlocksData;
        }

        void checkingEmptyBlock(std::vector<std::string>& blockData) const {
            for (LineIter line = blockData.begin(); line != blockData.end(); line++) {
                std::string const& blockLine = *line;
                bool isEmpty = true;
                for (size_t i = 0; i < blockLine.size(); i++) {
                    if (blockLine[i] != ' ' && blockLine[i] != '\t' && blockLine[i] != '\n')
                        isEmpty = false;
                }
                if (isEmpty)
                    throw ParsingException(formatMessage("found empty block"));
            }
        }

        int64_t findLastIndexOfBlock( std::string const& data, size_t start ) const {
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

        int64_t findStartingIndexOfServerBlock( std::string const& data, size_t start ) const {
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
            checkingBrackets(data);

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
                        throw ParsingException(formatMessage("missing close bracket"));
                    }
                    matches.pop();
                }
            }
            if (!matches.empty())
                throw ParsingException(formatMessage("missing open bracket"));
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

        /*
         *  processing server and location key values
         */

        void process(std::vector<ServerBlock>& serversBlock) const {
            for (size_t i = 0; i< serversBlock.size(); i++) {
                processServerBlock(serversBlock[i]);
            }
        }

        void processServerBlock(ServerBlock& sb) const {
            MapKeyValue kv = sb.getDataKeyValue();

            // host and port
            std::pair<std::string, uint16_t> hostAndPort = getHostAndPort(kv);
            sb.host = hostAndPort.first;
            sb.port = hostAndPort.second;
            sb.serverNames = getServerNames(kv);
            sb.errorPages = getErrorPages(kv);
            sb.root = getRoot(kv);
            sb.maxBodySize = getMaxClientSize(kv);
        }

        MapKeyValueIter getKeyIter(MapKeyValue& kv, std::string const& keyword, size_t nvalues, bool mustFound = false) const {
            MapKeyValueIter iter = kv.find(keyword);
            if (mustFound && iter == kv.end()) {
                throw ParsingException(formatMessage("`%s` keyword not found", keyword.c_str()));
            } else if (iter != kv.end() && (*iter).second.size() > nvalues) {
                throw ParsingException(formatMessage("found more than one value for keyword `%s`", keyword.c_str()));
            }

            return iter;
        }

        std::pair<std::string, uint16_t> getHostAndPort(MapKeyValue& kv) const {
            MapKeyValueIter iter = getKeyIter(kv, "listen", 1, true);

            std::vector<std::string> hostAndPort = split((*iter).second.front(), ":");
            if (hostAndPort.size() < 2) {
                throw ParsingException(formatMessage("port not found"));
            }

            std::string host = hostAndPort.front();

            // validating port
            std::string portString = hostAndPort.back();
            uint64_t port;
            if (isNumber(portString)) {
                throw ParsingException(formatMessage("couldn't parse port number"));
            }
            try {
                port = stoul(portString);
            } catch (std::invalid_argument& e) {
                throw ParsingException(formatMessage("couldn't parse port number"));
            }
            if (port > std::numeric_limits<uint16_t>::max()) {
                throw ParsingException(formatMessage("port range should be from 0 to 65535"));
            }


            return std::make_pair(host, static_cast<uint16_t>(port));
        }

        std::vector<std::string> getServerNames(MapKeyValue& kv) const {
            MapKeyValueIter iter = getKeyIter(kv, "server_names", defaults::SERVER_NAMES_LIMIT);
            if (iter == kv.end()) {
                return std::vector<std::string>();
            } else {
                return (*iter).second;
            }
        }

        std::map<int, std::string> getErrorPages(MapKeyValue& kv) const {
            std::map<int, std::string> errorPages;
            std::pair<int, std::string> errorPair;

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_204", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(204, defaults::PATH_ERROR_PAGE_204);
                } else {
                    errorPair = std::make_pair(204, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_400", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(400, defaults::PATH_ERROR_PAGE_400);
                } else {
                    errorPair = std::make_pair(400, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_403", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(403, defaults::PATH_ERROR_PAGE_403);
                } else {
                    errorPair = std::make_pair(403, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_404", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(404, defaults::PATH_ERROR_PAGE_404);
                } else {
                    errorPair = std::make_pair(404, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_413", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(413, defaults::PATH_ERROR_PAGE_413);
                } else {
                    errorPair = std::make_pair(413, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_500", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(500, defaults::PATH_ERROR_PAGE_500);
                } else {
                    errorPair = std::make_pair(500, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_502", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(502, defaults::PATH_ERROR_PAGE_502);
                } else {
                    errorPair = std::make_pair(502, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_504", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(504, defaults::PATH_ERROR_PAGE_504);
                } else {
                    errorPair = std::make_pair(504, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            {
                MapKeyValueIter iter = getKeyIter(kv, "error_page_505", 1);
                if (iter == kv.end()) {
                    errorPair = std::make_pair(505, defaults::PATH_ERROR_PAGE_505);
                } else {
                    errorPair = std::make_pair(505, (*iter).second.front());
                }
                errorPages.insert(errorPair);
            }

            return errorPages;
        }

        std::string getRoot(MapKeyValue& kv) const {
            std::string root;

            MapKeyValueIter iter = getKeyIter(kv, "root", 1);
            if (iter == kv.end()) {
                root = defaults::ROOT;
            } else {
                root = (*iter).second.front();
            }

            return root;
        }

        size_t  getMaxBodySize(MapKeyValue& kv) const {
            MapKeyValueIter iter = getKeyIter(kv, "client_max_body_size", 1);
            size_t maxBodySize = defaults::UNLIMITED_BODY_SIZE;

            //TODO: continue
            if (isNumber((*iter).second.front())) {
                std::stoi((*iter).second.front());
            }


            return maxBodySize;
        }

//      std::vector<std::string> getAllowedMethods(MapKeyValue& kv) const {
//          MapKeyValueIter iter = getKeyIter(kv, "allow", defaults::ALLOWED_METHOD_LIMIT, true);
//      }


        /*
         * Exceptions
         */
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

} // namespace ws