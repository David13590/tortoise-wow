#include "WobStatsTracker.hpp"
#include "Timer.h"

namespace HttpApi
{
    void WobStatsTracker::RecordDamage(uint32 attackerLowGuid, uint32 damage)
    {
        if (!attackerLowGuid || !damage)
            return;

        uint32 now = WorldTimer::getMSTime();

        std::lock_guard<std::mutex> lock(_mutex);
        auto& window = _damageWindows[attackerLowGuid];
        window.emplace_back(now, damage);

        while (!window.empty() && WorldTimer::getMSTimeDiff(window.front().first, now) > DPS_WINDOW_MS)
            window.pop_front();
    }

    float WobStatsTracker::GetDps(uint32 lowGuid)
    {
        uint32 now = WorldTimer::getMSTime();

        std::lock_guard<std::mutex> lock(_mutex);
        auto itr = _damageWindows.find(lowGuid);
        if (itr == _damageWindows.end())
            return 0.0f;

        auto& window = itr->second;
        while (!window.empty() && WorldTimer::getMSTimeDiff(window.front().first, now) > DPS_WINDOW_MS)
            window.pop_front();

        if (window.empty())
            return 0.0f;

        uint64 total = 0;
        for (auto const& entry : window)
            total += entry.second;

        return float(total) / (DPS_WINDOW_MS / 1000.0f);
    }

    void WobStatsTracker::MarkDied(uint32 lowGuid)
    {
        if (!lowGuid)
            return;

        std::lock_guard<std::mutex> lock(_mutex);
        _diedAtTick[lowGuid] = WorldTimer::getMSTime();
    }

    void WobStatsTracker::MarkWorldBossKilled(uint32 lowGuid)
    {
        if (!lowGuid)
            return;

        std::lock_guard<std::mutex> lock(_mutex);
        _wonAtTick[lowGuid] = WorldTimer::getMSTime();
    }

    bool WobStatsTracker::ConsumeJustDied(uint32 lowGuid)
    {
        uint32 now = WorldTimer::getMSTime();

        std::lock_guard<std::mutex> lock(_mutex);
        auto itr = _diedAtTick.find(lowGuid);
        if (itr == _diedAtTick.end())
            return false;

        bool recent = WorldTimer::getMSTimeDiff(itr->second, now) <= EVENT_FLAG_TTL_MS;
        _diedAtTick.erase(itr);
        return recent;
    }

    bool WobStatsTracker::ConsumeJustWon(uint32 lowGuid)
    {
        uint32 now = WorldTimer::getMSTime();

        std::lock_guard<std::mutex> lock(_mutex);
        auto itr = _wonAtTick.find(lowGuid);
        if (itr == _wonAtTick.end())
            return false;

        bool recent = WorldTimer::getMSTimeDiff(itr->second, now) <= EVENT_FLAG_TTL_MS;
        _wonAtTick.erase(itr);
        return recent;
    }
}
