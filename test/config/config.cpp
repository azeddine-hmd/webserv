#include "../test_utils.cpp"
#include "../../src/config.hpp"

/*
 * Test file for config class
 */

const char *paths[] = {
        "default.conf",
};

int     main(int argc, char **argv) {
    headerTest("Config");

    // path tests
    ws::Config config1;
    assertEqual(config1.getPath(), std::string(ws::Config::DEFAULT_CONFIG_PATH));
    ws::Config config2(paths[0]);
    assertEqual(config2.getPath(), std::string(paths[0]));

    return EXIT_SUCCESS;
}