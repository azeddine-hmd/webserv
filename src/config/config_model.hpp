#pragma once

#include <iostream>
#include <map>
#include <vector>

namespace ws {

    typedef enum HttpMethods {
        GET,
        POST,
        DELETE
    } HttpMethods;

    class LocationBlock {
        std::map <std::string, std::vector<std::string> >   dataKeyValue;

    public:
        std::string                                         path;
        std::string                                         root;
        std::vector<HttpMethods>                            allowedMethods;
        bool                                                autoindex;
        std::string                                         uploadStore;
        std::pair<int, std::string>                         redirect;
        std::string                                         index;

        //TODO: what data cgi needs?
        std::string                                         cgiPath;
        //std::vector<std::string>                            cgiExit;

    public:
        std::map<std::string, std::vector<std::string> > const& getDataKeyValue() const {
            return dataKeyValue;
        }

        void setDataKeyValue(std::map<std::string, std::vector<std::string> > const& dataKeyValue) {
            LocationBlock::dataKeyValue = dataKeyValue;
        }

        LocationBlock& operator=( LocationBlock const& rhs ) {
            if (this != &rhs) {
                path = rhs.path;
                root = rhs.root;
                allowedMethods = rhs.allowedMethods;
                autoindex = rhs.autoindex;
                uploadStore = rhs.uploadStore;
                redirect = rhs.redirect;
            }

            return *this;
        }
    };

    class ServerBlock {
        std::map <std::string, std::vector<std::string> >   dataKeyValue;

    public:
        std::vector<LocationBlock>                          locations;
        std::string                                         host;
        uint16_t                                            port;
        std::vector<std::string>                            serverNames;
        std::vector<std::string>                            hosts;
        std::map<int, std::string>                          errorPages;
        std::string                                         root;
        std::string                                         uploadStore;
        size_t                                              maxBodySize;
        std::vector<HttpMethods>                            allowedMethods;
        std::string                                         indexFile;
        bool                                                autoindex;

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

        ServerBlock& operator=( ServerBlock const& rhs ) {
            if (this != &rhs) {
                locations = rhs.locations;
                host = rhs.host;
                port = rhs.port;
                serverNames = rhs.serverNames;
                errorPages = rhs.errorPages;
                root = rhs.root;
                uploadStore = rhs.uploadStore;
                maxBodySize = rhs.maxBodySize;
                allowedMethods = rhs.allowedMethods;
                indexFile = rhs.indexFile;
                autoindex = rhs.autoindex;
            }

            return *this;
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
                "cgi_pass",
                "cgi_ext",
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
