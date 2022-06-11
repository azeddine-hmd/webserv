#pragma once

namespace ws {
    namespace defaults {

        /*
         *  config defaults
         */

        char const* CONFIG_PATH = "config/default.conf";

        // error pages
        char const* PATH_ERROR_PAGE_204 = "resources/error_pages/204.html";
        char const* PATH_ERROR_PAGE_400 = "resources/error_pages/400.html";
        char const* PATH_ERROR_PAGE_403 = "resources/error_pages/403.html";
        char const* PATH_ERROR_PAGE_404 = "resources/error_pages/404.html";
        char const* PATH_ERROR_PAGE_413 = "resources/error_pages/413.html";
        char const* PATH_ERROR_PAGE_500 = "resources/error_pages/500.html";
        char const* PATH_ERROR_PAGE_502 = "resources/error_pages/502.html";
        char const* PATH_ERROR_PAGE_504 = "resources/error_pages/504.html";
        char const* PATH_ERROR_PAGE_505 = "resources/error_pages/505.html";

        char const *ALLOWED[] = {
                "GET",
                "POST"
        };

        char const* INDEX = "index.html";

        char const* ROOT = "www";

        char const* UPLOAD_STORE = "www/uploads";

        size_t UNLIMITED_BODY_SIZE = 0;

        size_t SERVER_NAMES_LIMIT = 10;

        size_t ALLOWED_METHOD_LIMIT = 3;

    } // namespace defaults
} // namespace ws