#pragma once

#include "config/config.hpp"

namespace ws {

    class Application {
        Config* mConfig;

        Application( Application const& other);
        Application& operator=( Application const& rhs );
    public:
        Application(): mConfig(NULL) {

        }

        ~Application() {
            delete mConfig;
        }

        void run( Config* config ) {
            mConfig = config;
            std::cout << "===[ Application started ]===" << std::endl;
        }

    };

}

