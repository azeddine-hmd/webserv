#include <iostream>
#include "application.hpp"

void intercept(int sig) {
    exit(EXIT_SUCCESS);
}

int     main( int argc, char **argv ) {
    if (argc > 2) {
        std::cerr << "usage: ./webserv [<config-file>]" << std::endl;
        return EXIT_FAILURE;
    }

    signal(SIGINT, intercept);
    signal(SIGTERM, intercept);

    ws::Application app;

    return EXIT_SUCCESS;
}