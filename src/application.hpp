#include "config.hpp"

namespace ws {

    class Application {
        Config mConfig;

    public:
        Application(): mConfig()  {

        }

        Application( std::string const& path ): mConfig(path) {

        }

        Config const& getConfig() {
            return mConfig;
        }
    };

}