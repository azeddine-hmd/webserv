#pragma once

#include <iostream>
#include <map>
#include <vector>

namespace ws {

    struct LocationBlock {
        std::map <std::string, std::vector<std::string> >   dataKeyValue;
        std::string                                         path;
        std::vector <std::string>                           allowedMethods;
        std::string                                         autoIndex;
        std::string                                         uploadStore;
        std::string                                         cgiPath;
        std::vector <std::string>                           cgiExit;
        std::string                                         root;
        std::string                                         returnCode;
        std::string                                         returnPath;
        size_t                                              maxBodySize; // in KB
        std::string                                         indexFile;
    };

    struct ServerBlock {
        std::map <std::string, std::vector<std::string> >   dataKeyValue;
        std::string                                         host;
        std::string                                         port;
        std::string                                         uploadStore;
        std::vector<std::string>                            allowedMethods;
        std::string                                         errorPagePath;
        std::string                                         errorCode;
        std::map<int, std::string>                          errorPage;
        std::vector <std::string>                           serverName;
        std::string                                         maxBodySize;
        std::vector<LocationBlock>                          locations;
    };

    namespace parse {

        std::string validKeys[] = {
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

        size_t keywordsLen = sizeof(parse::validKeys)/sizeof(*(parse::validKeys));

    } // namespace parse

} // namespace ws
