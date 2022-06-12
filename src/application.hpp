#pragma once

#include "config.hpp"

namespace ws {

    class Application {
        Config* mConfig;

        Application();
        Application( Application const& other);
        Application& operator=( Application const& rhs );
    public:
        Application( Config* config ): mConfig(config) {

        }

        ~Application() {
            delete mConfig;
        }

        Config const& getConfig() {
            return *mConfig;
        }

    };

}