#include "WobStatsController.hpp"

#include "HttpApi/Authorizers/ApiKeyAuthorizer.hpp"
#include "WobStatsTracker.hpp"
#include "ObjectAccessor.h"
#include "Objects/Player.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace httplib;

namespace HttpApi
{
    WobStatsController::WobStatsController(std::string key)
    {
        _authorizer = std::make_unique<ApiKeyAuthorizer>(key);
    }

    void GetWobStatsAction(const Request& req, Response& resp)
    {
        std::string name = req.get_param_value("name");

        // FindPlayerByName normalizes internally, but guard length/emptiness
        // before we even touch it - it's the only untrusted input this
        // endpoint accepts.
        if (name.empty() || name.length() > 32)
        {
            resp.status = 400;
            resp.set_content("Bad name.", "text/plain");
            return;
        }

        rapidjson::Document doc;
        doc.SetObject();
        auto& alloc = doc.GetAllocator();

        Player* player = ObjectAccessor::FindPlayerByName(name.c_str());
        if (!player)
        {
            doc.AddMember("online", false, alloc);
        }
        else
        {
            uint32 lowGuid = player->GetGUIDLow();
            uint32 maxHealth = player->GetMaxHealth();
            float hpPercent = maxHealth ? (100.0f * float(player->GetHealth()) / float(maxHealth)) : 0.0f;

            doc.AddMember("online", true, alloc);
            doc.AddMember("alive", player->IsAlive(), alloc);
            doc.AddMember("hpPercent", hpPercent, alloc);
            doc.AddMember("inCombat", player->IsInCombat(), alloc);
            doc.AddMember("dps", WobStatsTracker::Instance().GetDps(lowGuid), alloc);
            doc.AddMember("justDied", WobStatsTracker::Instance().ConsumeJustDied(lowGuid), alloc);
            doc.AddMember("justWon", WobStatsTracker::Instance().ConsumeJustWon(lowGuid), alloc);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        resp.set_content(buffer.GetString(), "application/json");
    }
}
