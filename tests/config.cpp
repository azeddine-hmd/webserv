#include "test_utils.cpp"
#include "../src/config/config.hpp"

/*
 * Test file for config class
 */

int     main(int argc, char **argv) {
    headerTest("Config");

    // path tests
    {
        ws::Config config_1;
        assertEqual(config_1.path, std::string(ws::Config::DEFAULT_CONFIG_PATH));

        ws::Config config_2("config/default.conf");
        assertEqual(config_2.path, std::string("config/default.conf"));

        ws::Config config_3(ws::Config::DEFAULT_CONFIG_PATH);
        assertNotEqual(config_3.path, std::string("wrong_path/"));

        // testing FileNotOpened exception
        try {
            ws::Config config_4("config_not_exit/some_path/random_file");
        } catch (ws::Config::PathException& e) {
            test_passed();
        } catch (std::exception& e) {
            test_failed();
        }
    }
    separator();
    // server block
    {
        // empty config
        try {
            ws::Config config_1("config/empty_config.conf");
        } catch (ws::Config::ParsingException& e) {
            test_passed();
        } catch (std::exception& e) {
            test_failed();
        }

    }


    return EXIT_SUCCESS;
}