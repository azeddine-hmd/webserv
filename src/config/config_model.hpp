#pragma once

#include <iostream>
#include <map>
#include <vector>

namespace ws {

    class LocationBlock {
        std::map <std::string, std::vector<std::string> >   dataKeyValue;

    public:
        std::string                                         path;

        // general
        std::string                                         root;
        std::vector<std::string>                            allowedMethods;
        bool                                                autoindex;
        std::string                                         index;
        std::string                                         uploadStore;

        // redirection
        std::pair<int, std::string>                         redirect;

        // cgi
        std::string                                         cgiPath;


    public:
        std::map<std::string, std::vector<std::string> > const& getDataKeyValue() const {
            return dataKeyValue;
        }

        void setDataKeyValue(std::map<std::string, std::vector<std::string> > const& dataKeyValue) {
            LocationBlock::dataKeyValue = dataKeyValue;
        }
    };

    class ServerBlock {
        std::map <std::string, std::vector<std::string> >   dataKeyValue;

    public:
        std::string                                         host;
        uint16_t                                            port;
        std::vector<std::string>                            hosts;

        std::vector<std::string>                            serverNames;
        std::map<int, std::string>                          errorPages;
        size_t                                              maxBodySize;

        std::vector<LocationBlock>                          locations;

    public:
        std::map<std::string, std::vector<std::string> > const& getDataKeyValue() const {
            return dataKeyValue;
        }

        void setDataKeyValue(std::map<std::string, std::vector<std::string> > const& dataKeyValue) {
            ServerBlock::dataKeyValue = dataKeyValue;
        }

        void setHosts() {
            std::string portString = std::to_string(port);
            for (size_t i = 0; i < serverNames.size(); i++) {
                hosts.push_back(serverNames[i] + ":" + portString);
            }
        }
    };

    namespace parse {

        std::string validKeys[] = {
                "listen",
                "server_names",
                "error_page",
                "client_max_body_size",
                "location",
                "allow",
                "autoindex",
                "upload_store",
                "cgi_path",
                "index",
                "return",
                "root",
                "server",
                "error_page_204",
                "error_page_400",
                "error_page_403",
                "error_page_404",
                "error_page_413",
                "error_page_500",
                "error_page_502",
                "error_page_504",
                "error_page_505",
                "}",
                "{",
                "",
        };

        size_t keywordsLen = sizeof(parse::validKeys)/sizeof(*(parse::validKeys));

    } // namespace parse

} // namespace ws
