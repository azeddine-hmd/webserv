#include <iostream>
#include "config.hpp"
#include "application.hpp"

void intercept(int sig) {
    (void)sig;
    //TODO: close all files descriptors
    exit(EXIT_SUCCESS);
}

int     main( int argc, char **argv ) {
    if (argc > 2) {
        std::cerr << "usage: ./webserv [<config-file>]" << std::endl;
        return EXIT_FAILURE;
    }
    signal(SIGINT, intercept);
    signal(SIGTERM, intercept);

    // create config
    ws::Config * config;
    try {
        config = (argc == 1) ? new ws::Config() : new ws::Config(argv[1]);
    } catch (ws::Config::PathException& e) {
        std::cerr << "PathException: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (ws::Config::ParsingException& e) {
        std::cerr << "ParsingException: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // run application
    ws::Application app(config);

    return EXIT_SUCCESS;
}