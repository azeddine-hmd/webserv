#pragma once

#include "server.hpp"
#include "request.hpp"
#include "responseBuilder.hpp"

namespace ws {

    class Call {
    public:
        Request mReq;
        ResponseBuilder* mRes;
    };

}