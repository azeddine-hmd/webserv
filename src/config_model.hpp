#pragma once

#include <iostream>
#include <map>
#include <vector>

namespace ws {

    struct LocationBlock {
        std::map <std::string, std::string> data;
        std::string path;
        bool allowed;
        std::vector <std::string> allowedMethods;
        std::string autoIndex;
        std::string uploadStore;
        std::string cgiPath;
        std::vector <std::string> cgiExit;
        std::string root;
        std::string returnCode;
        std::string returnPath;
        long maxBodySize;
        std::string indexFile;
    };

    struct ServerBlock {
        std::map <std::string, std::string> data;
        std::string ip;
        std::string port;
        std::string uploadStore;
        bool allowed; // to check if there is a key or not.
        std::vector <std::string> allowedMethods;
        std::string errorPagePath;
        std::string errorCode;
        std::map<int, std::string> errorPage;
        std::vector <std::string> serverName;
        std::string max_body_size;
        std::vector <LocationBlock> locations;
        int serverFd;
        sockaddr_in address;
        long maxBodySize; // in kb
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

} // namespace ws
