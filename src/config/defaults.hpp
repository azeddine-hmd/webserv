#pragma once

#include "config_model.hpp"

namespace ws {
    namespace defaults {

        /*
         *  config defaults
         */

        char const* CONFIG_PATH = "config/default.conf";

        // error pages
        char const* PATH_ERROR_PAGE_200 = "error_pages/200.html";
        char const* PATH_ERROR_PAGE_400 = "error_pages/400.html";
        char const* PATH_ERROR_PAGE_403 = "error_pages/403.html";
        char const* PATH_ERROR_PAGE_404 = "error_pages/404.html";
        char const* PATH_ERROR_PAGE_405 = "error_pages/405.html";
        char const* PATH_ERROR_PAGE_406 = "error_pages/406.html";
        char const* PATH_ERROR_PAGE_413 = "error_pages/413.html";
        char const* PATH_ERROR_PAGE_500 = "error_pages/500.html";
        char const* PATH_ERROR_PAGE_501 = "error_pages/501.html";
        char const* PATH_ERROR_PAGE_502 = "error_pages/502.html";
        char const* PATH_ERROR_PAGE_503 = "error_pages/503.html";
        char const* PATH_ERROR_PAGE_520 = "error_pages/520.html";

        std::string ALLOWED_METHODS[] = {
                "GET",
                "POST",
                "PUT",
                "HEAD"
        };

        char const* INDEX = "index.html";

        char const* ROOT = "www";

        size_t UNLIMITED_BODY_SIZE = 0;

        size_t VALUES_LIMIT = 999;

        bool AUTOINDEX = false;

        std::pair<int, std::string> EMPTY_REDIRECT(-1, "");


    } // namespace defaults
} // namespace ws
