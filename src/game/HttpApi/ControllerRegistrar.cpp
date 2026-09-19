#include "TestController.hpp"
#include "TransferController.hpp"
#include "WobStatsController.hpp"
#include "Config.hpp"

namespace HttpApi
{
    void RegisterControllers()
    {
        new TestController();
        new TransferController(sConfig.GetStringDefault("HttpApi.TransferKey", "Gheor"));
        new WobStatsController(sConfig.GetStringDefault("HttpApi.WobStatsKey", "changeme"));
    }
}

