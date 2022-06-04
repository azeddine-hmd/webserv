#pragma once

#include "config.hpp"

namespace ws {

    class Application {
        Config* mConfig;

        Application() {
            throw std::runtime_error("don't call default constructor, Idiot!");
        }

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