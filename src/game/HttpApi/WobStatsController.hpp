#pragma once
#include "httplib.h"
#include "HttpApi/BaseController.hpp"

namespace HttpApi
{
    void GetWobStatsAction(const httplib::Request& req, httplib::Response& resp);

    // Read-only endpoint for external tools to poll a
    // single character's live HP%, combat state and DPS instead of relying on
    // WeakAuras/Details + screen OCR. Reuses the existing HttpApi server/port,
    // so it inherits that server's default loopback-only bind and requires its
    // own API key (HttpApi.WobStatsKey) on top of that.
    class WobStatsController final : public BaseController
    {
    public:
        explicit WobStatsController(std::string key);

        void RegisterCommands(httplib::Server* server) override
        {
            RegisterEndpoint<HttpMethod::Get>("/wob/stats", &GetWobStatsAction);
        }
    };
}
