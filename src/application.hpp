#pragma once

#include "config.hpp"

namespace ws {

    class Application {
        Config mConfig;

        Application() {
            throw std::runtime_error("Don't call default constructor, Idiot!");
        }

    public:
        Application( Config const& config ): mConfig(config) {

        }

        Config const& getConfig() {
            return mConfig;
        }

    };

}