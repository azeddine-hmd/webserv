#pragma once

#include <iostream>
#include <map>
#include <vector>

namespace ws {

    class LocationBlock {
        std::map <std::string, std::vector<std::string> >   dataKeyValue;

    public:
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

    public:
        std::map<std::string, std::vector<std::string>> const& getDataKeyValue() const {
            return dataKeyValue;
        }

        void setDataKeyValue(std::map<std::string, std::vector<std::string>> const& dataKeyValue) {
            LocationBlock::dataKeyValue = dataKeyValue;
        }
    };

    class ServerBlock {
        std::map <std::string, std::vector<std::string> >   dataKeyValue;

    public:
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
        std::string                                         indexFile;

    public:
        std::map<std::string, std::vector<std::string>> const& getDataKeyValue() const {
            return dataKeyValue;
        }

        void setDataKeyValue(std::map<std::string, std::vector<std::string>> const& dataKeyValue) {
            ServerBlock::dataKeyValue = dataKeyValue;
        }
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
