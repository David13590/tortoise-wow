#pragma once
// Rolling per-player combat stats for the WobStatsController HTTP endpoint.
// Populated entirely from UnitScript/PlayerScript hooks (WobStatsScript.cpp) -
// no changes to core damage/death code paths are needed.
#include "Common.h"
#include <deque>
#include <mutex>
#include <unordered_map>

namespace HttpApi
{
    class WobStatsTracker
    {
    public:
        static WobStatsTracker& Instance()
        {
            static WobStatsTracker instance;
            return instance;
        }

        // Called for every point of damage a player deals (melee or spell).
        void RecordDamage(uint32 attackerLowGuid, uint32 damage);

        // Sum of damage dealt by this player in the trailing window, divided by
        // the window length. Returns 0 if the player hasn't hit anything recently.
        float GetDps(uint32 lowGuid);

        void MarkDied(uint32 lowGuid);
        void MarkWorldBossKilled(uint32 lowGuid);

        // "Consuming" a flag returns whether it fired within the last few
        // seconds and clears it, so a slow poller still catches a one-shot
        // event without it firing again on the next poll.
        bool ConsumeJustDied(uint32 lowGuid);
        bool ConsumeJustWon(uint32 lowGuid);

    private:
        static constexpr uint32 DPS_WINDOW_MS = 5000;
        static constexpr uint32 EVENT_FLAG_TTL_MS = 4000;

        std::mutex _mutex;
        std::unordered_map<uint32, std::deque<std::pair<uint32, uint32>>> _damageWindows; // lowGuid -> (tick, damage)
        std::unordered_map<uint32, uint32> _diedAtTick;
        std::unordered_map<uint32, uint32> _wonAtTick;
    };
}
