#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>

namespace ws {

    struct LocationBlock {
        std::map<std::string, std::string>      data;
        std::string 						    path;
        bool 								    allowed;
        std::vector<std::string> 			    allowedMethods;
        std::string							    autoIndex;
        std::string 						    uploadStore;
        std::string 						    cgiPath;
        std::vector<std::string> 			    cgiExit;
        std::string 						    root;
        std::string 						    returnCode;
        std::string 						    returnPath;
        long								    maxBodySize;
        std::string							    indexFile;
    };

    struct ServerBlock {
        std::map<std::string, std::string>      data;
        std::string 						    ip;
        std::string 						    port;
        std::string 						    uploadStore;
        bool 								    allowed; // to check if there is a key or not.
        std::vector<std::string> 			    allowedMethods;
        std::string 						    errorPagePath;
        std::string 						    errorCode;
        std::map<int, std::string>			    errorPage;
        std::vector<std::string> 			    serverName;
        std::string							    max_body_size;
        std::vector<LocationBlock> 		        locations;
        int 								    serverFd;
        sockaddr_in 					        address;
        long								    maxBodySize; // in kb
    };

    namespace parse {
        std::string keywords[] = {
                "listen",
                "server_name",
                "error_page",
                "client_max_body_size",
                "location",
                "allow",
                "autoindex",
                "upload_store",
                "cgi_pass",
                "cgi_ext",
                "index",
                "return",
                "root",
                "server",
                "}",
                "{",
                ""
        };
    }

    struct Config {
        std::string                 path;
        std::vector<ServerBlock>    serverBlocks;

        static char const* DEFAULT_CONFIG_PATH;

    private:

        /*
         * Exception:
         *      - FileNotOpened
         *      - NoServerBlockFound
         *      - ParsingException
         */
        void init() {
            try {
                serverBlocks = parsingServerBlocks(path);
            } catch (FileNotOpened& e) {
                throw e;
            } catch (ParsingException& e) {
                throw e;
            }

            if (serverBlocks.empty()) {
                throw NoServerBlockFound();
            }
        }

        /*
         * Exception:
         *      - FileNotOpened
         *      - NoServerBlockFound
         *      - ParsingException
         */
        Config(): path(DEFAULT_CONFIG_PATH) {
            try {
                init();
            } catch (FileNotOpened& e) {
                throw e;
            } catch (NoServerBlockFound& e) {
                throw e;
            } catch (ParsingException& e) {
                throw e;
            }
        }

        /*
         * Exception:
         *      - FileNotOpened
         *      - NoServerBlockFound
         *      - ParsingException
         */
        Config(std::string const& path): path(path) {
            try {
                init();
            } catch (FileNotOpened& e) {
                throw e;
            } catch (NoServerBlockFound& e) {
                throw e;
            } catch (ParsingException& e) {
                throw e;
            }
        }

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

        /*
         * Exception:
         *      - ParsingException
         */
        std::vector<ServerBlock> parsingServerBlocks(std::string const& path) const {
            std::ifstream fs(path.c_str());
            if (!fs) {
                throw FileNotOpened();
            }

            std::string data, line;
            while (getline(fs, line)) {
                data += line;
                data += "\n";
            }

            data = removeAllComments(data);

            try {
                checkingConfigErrors(data);
            } catch (ParsingException& e) {
                throw e;
            }

            fs.close();
            return std::vector<ServerBlock>();
        }

        // symbols for starting comments are ';' and '#'.
        // How it work? by adding character after character from old data to a newer one
        // and skipping entire comment characters.
        std::string removeAllComments(std::string& data) const {
            std::string newData;

            size_t i = 0;
            // iterator from beginning until the end
            while (i < data.length()) {
                // skip characters
                if (isAtEnd(data, i)) {
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

        bool isAtEnd(std::string& data, size_t i) const {
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

//        void check_error_in_file(std::string data){
//            check_brackets(data);
//            std::string valide_keyword[] = {"listen", "server_name", "error_page", "client_max_body_size", "location", "allow", "autoindex", "upload_store", "cgi_pass", "cgi_ext", "index", "return", "root", "server", "}", "{", "" };
//            int count_of_keys = 17;
//            std::vector<std::string> check = split(data, "\n");
//            std::vector<std::string>::iterator it_begin(check.begin());
//            std::vector<std::string>::iterator it_end(check.end());
//            std::string element;
//            --it_begin;
//            while(++it_begin != it_end){
//                element = *it_begin;
//                std::string key = get_key_from_line(element);
//                for (int i = 0; i < count_of_keys + 1; i++)
//                {
//                    if (i == count_of_keys){
//                        throw "Found keyword not allowed";
//                    }
//                    if (valide_keyword[i] == key)
//                        break ;
//                }
//            }
//        }

        /*
         * Exception:
         *      - ParsingException
         */
        void checkingConfigErrors(std::string const& data) const {
            try {
                checkingBrackets(data);
            } catch (ParsingException& e) {
                throw e;
            }
        }

        /*
         * Exception:
         *      - ParsingException
         */
        void checkingBrackets(std::string const& data) const {
            char brackets[] = {'{', '}'};
            std::stack<char> matches;

            for (size_t i = 0; i < data.length(); i++) {
                if (data[i] == brackets[0]) {
                    matches.push(brackets[0]);
                } else if (data[i] == brackets[1]) {
                    if (matches.size() > 0) {
                        throw ParsingException("missing close bracket");
                    }
                    matches.pop();
                }
            }
            if (matches.size() != 0)
                throw ParsingException("missing open bracket");
        }


        class FileNotOpened : public std::exception {
        public:
            virtual char const* what() const throw() {
                return strerror(errno);
            }
        };

        class NoServerBlockFound : public std::exception {
        public:
            virtual char const* what() const throw() {
                return "No server block have been found";
            }
        };

        class ParsingException : public std::exception {
            const char* msg;
        public:
            ParsingException(const char *message): msg(message) {}
            virtual char const* what() const throw() {
                if (!msg)
                    return "";
                else
                    return msg;
            }
        };

    };

    char const* ws::Config::DEFAULT_CONFIG_PATH = "config/default.conf";

} // namespace ws