// Self-registering hooks that feed WobStatsTracker. No core files are
// modified: UNITHOOK_ON_DAMAGE and PLAYERHOOK_ON_PLAYER_JUST_DIED already
// fire from Unit::DealDamage / Unit::Kill for every registered script.
#include "ScriptObjects.h"
#include "Objects/Player.h"
#include "Objects/Creature.h"
#include "WobStatsTracker.hpp"

namespace HttpApi
{
    void EnsureWobStatsScriptsLinked()
    {
    }

    class WobStatsUnitScript : public UnitScript
    {
    public:
        WobStatsUnitScript()
            : UnitScript("WobStatsUnitScript", { UNITHOOK_ON_DAMAGE, UNITHOOK_ON_UNIT_DEATH })
        {}

        void OnDamage(Unit* attacker, Unit* /*victim*/, uint32& damage) override
        {
            if (!attacker || !damage)
                return;

            if (Player* player = attacker->ToPlayer())
                WobStatsTracker::Instance().RecordDamage(player->GetGUIDLow(), damage);
        }

        void OnUnitDeath(Unit* unit, Unit* /*killer*/) override
        {
            if (!unit || !unit->IsCreature())
                return;

            Creature* creature = unit->ToCreature();
            if (!creature->IsWorldBoss())
                return;

            std::list<Player*> nearbyPlayers;
            unit->GetAlivePlayerListInRange(unit, nearbyPlayers, 100.0f);
            for (Player* player : nearbyPlayers)
                WobStatsTracker::Instance().MarkWorldBossKilled(player->GetGUIDLow());
        }
    };

    class WobStatsPlayerScript : public PlayerScript
    {
    public:
        WobStatsPlayerScript()
            : PlayerScript("WobStatsPlayerScript", { PLAYERHOOK_ON_PLAYER_JUST_DIED })
        {}

        void OnPlayerJustDied(Player* player) override
        {
            if (player)
                WobStatsTracker::Instance().MarkDied(player->GetGUIDLow());
        }
    };

    static WobStatsUnitScript s_wobStatsUnitScript;
    static WobStatsPlayerScript s_wobStatsPlayerScript;
}
