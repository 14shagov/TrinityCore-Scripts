#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "World.h"
#include "Chat.h"
#include "CombatAI.h"
#include "CreatureTextMgr.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "GridNotifiersImpl.h"
#include "PassiveAI.h"
#include "InstanceScript.h"

/*std::list<Player*> players;
Trinity::UnitAuraCheck check(false, 29726);
Trinity::PlayerListSearcher<Trinity::UnitAuraCheck> searcher(me, players, check);
Cell::VisitWorldObjects(me, searcher, 10.0f);
if (!players.empty())ChatHandler(players.front()->GetSession()).PSendSysMessage("spell %i", DoCast(me, 34812, false));//xdebug
*/

enum UniqueGossipOptions : uint32
{
    MENU_ID_UNIQUE_1 = 60102, // "Your presence here is an anomaly. Speak quickly before my patience wanes."
    NPC_TEXT_UNIQUE_1 = 60122, //
    MENU_ID_UNIQUE_2 = 60103, // "Peaceful intentions are often veiled. Convince me, mortal, why I should trust you."
    NPC_TEXT_UNIQUE_2 = 60123, //
    MENU_ID_UNIQUE_3 = 60104, // "In the tapestry of being, unravel this mystery: What speaks without a mouth and hears without ears?
    NPC_TEXT_UNIQUE_3 = 60104, // Know that if you get the answer wrong, you will pay with your life!"
    MENU_ID_UNIQUE_4 = 60105, // "A wise choice, traveler. Accept this artifact, and may it aid you on your journey."
    NPC_TEXT_UNIQUE_4 = 60105, //
    GOSSIP_OPTION_1 = 0,     // "Ghost"
    GOSSIP_OPTION_2 = 1,     // "River"
    GOSSIP_OPTION_3 = 2,     // "Mirror"
    GOSSIP_OPTION_4 = 3,     // "Shadow"
    GOSSIP_OPTION_5 = 4,     // "Echo"

    FACTION_SUPER_ENEMY = 54 //1620
};
enum UniqueNPC : uint32
{
    NPC_BURNING_SPIRIT = 9178,
    NPC_SHADOWY_MINION = 90007,
    NPC_ETHEREAL_ARACHNIDS = 90008,
    NPC_GREATER_WATER_ELEMENTAL = 25040
};

enum UniqueItem : uint32
{
    ITEM_PROTO_DRAKE_REINS = 44168
};

enum UniqueSpells : uint32
{
    SPELL_CURSE_OF_NAZJATAR = 34812,
    SPELL_CURSE_PAIN = 38048,
    SPELL_CURSE_OF_MENDING = 15730,
    SPELL_BLIGHT_BOMB = 48212,

    //phase 1
    SPELL_BLINDING_WEBS = 59365,
    SPELL_ENVELOPING_WEBS = 24110,
    SPELL_DRAIN_POWER = 44131, //stackable
    SPELL_SPIDER_WEB = 28434,

    //phase 2
    SPELL_DESTRUCTIVE_BARRAGE = 48734,
    SPELL_SHOCKWAVE = 25425, //todo !!!

    //phase 3
    SPELL_BRAIN_LINK = 63802,
    SPELL_ETHEREAL = 58548,
    SPELL_DIVINE_HYMN = 64843,
    SPELL_DEFILING_HORROR = 72435,
    SPELL_WEB_EXPLOSION = 52491,
    SPELL_WEB_GRAB = 53406,
    SPELL_WEB_SPRAY = 29484,

    // Balinda
    SPELL_ICEBLOCK = 46604,

    SECOND = 1000   // Constant representing one second in milliseconds
};

enum UniqueEvents
{
    //phase 1
    EVENT_INNER_FIRE = 1,
    EVENT_BLINDING_WEBS,
    EVENT_ENVELOPING_WEBS,
    EVENT_DRAIN_POWER,
    EVENT_SUMMON_SHADOWY_MINION,
    EVENT_SPIDER_WEB,

    //phase 2
    EVENT_DESTRUCTIVE_BARRAGE,
    EVENT_DESTRUCTIVE_BARRAGE_STOP,
    EVENT_SHOCKWAVE, //todo !!!
    EVENT_BRAIN_LINK, //todo !!!

    //phase 3
    EVENT_DEFILING_HORROR,
    EVENT_ETHEREAL,
    EVENT_ETHEREAL_STOP,
    EVENT_WEB_EXPLOSION,
    EVENT_WEB_GRAB,
    EVENT_WEB_SPRAY,
    EVENT_SUMMON_ETHEREAL_ARACHNIDS,
};

enum UniquePower
{
    BASE_MANA = 1000000
};

enum UniqueTexts
{
    SAY_AGGRO = 0,
    SAY_EVADE = 1,
    SAY_SALVATION = 2,
};

enum UniqueAction
{
    ACTION_START_FIGHT = -1
};

enum UniquePhases
{
    PHASE_INTRO = 1,
    PHASE_ONE,
    PHASE_TWO,
    PHASE_THREE,
};

class npc_unique : public CreatureScript
{
public:
    npc_unique() : CreatureScript("npc_unique") {}
    struct npc_uniqueAI : public BossAI
    {
        npc_uniqueAI(Creature* creature) : BossAI(creature, 1)
        {
            Initialize();
        }

        //!is used to initialize variables when creating a creature
        void Initialize()
        {
            _SetStartConfigure();
            _SetCurses();
            _IsCursesBroken = false;
        }

        //!is called every time an agro is removed.
        void Reset() override
        {
            Initialize();
            _Reset();
        }

        //!CreatureAI.h
        bool OnGossipHello(Player* player) override
        {
            InitGossipMenuFor(player, MENU_ID_UNIQUE_1);
            if (_IsCursesBroken)
                AddGossipItemFor(player, MENU_ID_UNIQUE_1, GOSSIP_OPTION_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); //todo change
            else
                AddGossipItemFor(player, MENU_ID_UNIQUE_1, GOSSIP_OPTION_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            SendGossipMenuFor(player, NPC_TEXT_UNIQUE_1, me->GetGUID());
            return true;
        }

        //!CreatureAI.h
        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);
            switch (action)
            {
            case GOSSIP_ACTION_INFO_DEF + 1: //
                AddGossipItemFor(player, MENU_ID_UNIQUE_2, GOSSIP_OPTION_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                SendGossipMenuFor(player, NPC_TEXT_UNIQUE_2, me->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2: //
                AddGossipItemFor(player, MENU_ID_UNIQUE_3, GOSSIP_OPTION_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                AddGossipItemFor(player, MENU_ID_UNIQUE_3, GOSSIP_OPTION_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                AddGossipItemFor(player, MENU_ID_UNIQUE_3, GOSSIP_OPTION_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                AddGossipItemFor(player, MENU_ID_UNIQUE_3, GOSSIP_OPTION_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                AddGossipItemFor(player, MENU_ID_UNIQUE_3, GOSSIP_OPTION_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
                SendGossipMenuFor(player, NPC_TEXT_UNIQUE_3, me->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 3: //
            case GOSSIP_ACTION_INFO_DEF + 4: //
            case GOSSIP_ACTION_INFO_DEF + 5: //
            case GOSSIP_ACTION_INFO_DEF + 6: //
                CloseGossipMenuFor(player);
                Talk(SAY_AGGRO, player); //!creature_txt.dbc
                DoAction(ACTION_START_FIGHT);
                break;
            case GOSSIP_ACTION_INFO_DEF + 7:
                player->AddItem(ITEM_PROTO_DRAKE_REINS, 1);
                SendGossipMenuFor(player, NPC_TEXT_UNIQUE_4, me->GetGUID());
                break;
            }
            return true;
        }

        void DoAction(int32 action) override
        {
            if (action == ACTION_START_FIGHT)
            {
                events.SetPhase(PHASE_ONE);
                me->SetImmuneToPC(false);
                me->SetFaction(FACTION_SUPER_ENEMY);
                //DoZoneInCombat();

               /* EntryCheckPredicate pred(NPC_ANTAGONIST);
                summons.DoAction(ACTION_ANTAGONIST_HOSTILE, pred);*/
            }
        }

        //!UnitAI.h
        //!Called before damage apply
        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damageType, SpellInfo const* /*spellInfo = nullptr*/) override
        {
            if (me->HealthBelowPctDamaged(66, damage) && events.IsInPhase(PHASE_ONE))
            {
                events.CancelEventGroup(1);
                events.SetPhase(PHASE_TWO);
                DoCastSelf(SPELL_ICEBLOCK);
                _PhaseTwoSpellInit();
            }
            if (me->HealthBelowPctDamaged(33, damage) && events.IsInPhase(PHASE_TWO))
            {
                events.CancelEventGroup(2);
                events.SetPhase(PHASE_THREE);
                me->AddAura(SPELL_ETHEREAL, me);
                DoCastSelf(SPELL_DIVINE_HYMN);
                summons.DespawnAll();
                _PhaseThreeSpellInit();
            }
        }

        //!CreatureAI.h
        void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
        {
            if (caster->ToCreature() == me)
                return;
            //todo immunity to curse
            events.ScheduleEvent(EVENT_INNER_FIRE, 1s);
        }

        //!CreatureAI.h
        void JustEngagedWith(Unit* who) override
        {
            BossAI::JustEngagedWith(who);
            //_PhaseOneSpellInit();
            _PhaseOneSpellInit();
        }

        //!CreatureAI.h
        void JustSummoned(Creature* summoned) override
        {
            BossAI::JustSummoned(summoned);
            if (me->GetVictim())ChatHandler(me->GetVictim()->ToUnit()->ToPlayer()->GetSession()).PSendSysMessage("JustSummoned %i", summons.size());//xdebug

            /*summoned->AI()->AttackStart(SelectTarget(SelectTargetMethod::Random, 0, 50, true));
            summoned->SetFaction(me->GetFaction());*/
        }

        //!CreatureAI.h
        void SummonedCreatureDespawn(Creature* summoned) override
        {
            BossAI::SummonedCreatureDespawn(summoned);
            if (me->GetVictim())ChatHandler(me->GetVictim()->ToUnit()->ToPlayer()->GetSession()).PSendSysMessage("SummonedCreatureDespawn %i", summons.size());//xdebug
        }

        //!CreatureAI.h
        void JustDied(Unit* killer)
        {
            summons.DespawnAll();
            killer->CastSpell(killer, SPELL_BLIGHT_BOMB, false);
        }

        bool CheckInRoom() override
        {
            if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 50)
            {
                EnterEvadeMode();
                Talk(1);
                return false;
            }
            return true;
        }

        // Called at World update tick
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || !CheckInRoom())  //!will check to see if the victim has updated
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                if (events.IsInPhase(PHASE_ONE))
                {
                    switch (eventId)
                    {
                    case EVENT_BLINDING_WEBS:
                        DoCastVictim(SPELL_BLINDING_WEBS);
                        events.ScheduleEvent(EVENT_BLINDING_WEBS, 5s, 15s, 1, PHASE_ONE);
                        break;
                    case EVENT_ENVELOPING_WEBS:
                        DoCastVictim(SPELL_ENVELOPING_WEBS);
                        events.ScheduleEvent(EVENT_ENVELOPING_WEBS, 5s, 9s, 1, PHASE_ONE);
                        break;
                    case EVENT_SPIDER_WEB:
                        DoCastVictim(SPELL_SPIDER_WEB);
                        events.ScheduleEvent(EVENT_SPIDER_WEB, 50s);
                        break;
                    case EVENT_DRAIN_POWER:
                        for (size_t i = 0; i < 20; i++)
                            DoCast(SelectTarget(SelectTargetMethod::Random, 0, 50, true), SPELL_DRAIN_POWER); //DoCastVictim(SPELL_DRAIN_POWER);
                        events.ScheduleEvent(EVENT_DRAIN_POWER, 50s, 90s, 1, PHASE_ONE);
                        break;
                    case EVENT_SUMMON_SHADOWY_MINION:
                        if (summons.size() < 7)
                            for (uint32 i = 0; i < 7; ++i)
                                DoSpawnCreature(NPC_SHADOWY_MINION, frand(-9, 9), frand(-9, 9), 1, 0, TEMPSUMMON_MANUAL_DESPAWN, 0s);
                        events.ScheduleEvent(EVENT_SUMMON_SHADOWY_MINION, 40s, 1, PHASE_ONE);
                        break;

                    default:
                        break;
                    }
                }

                if (events.IsInPhase(PHASE_TWO))
                {
                    switch (eventId)
                    {
                    case EVENT_DESTRUCTIVE_BARRAGE:
                        DoCastVictim(SPELL_DESTRUCTIVE_BARRAGE);
                        me->SetFacingTo(me->GetAbsoluteAngle(me->GetVictim()));
                        events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE, 500ms, 2, PHASE_TWO);
                        break;
                    case EVENT_DESTRUCTIVE_BARRAGE_STOP:
                        DoCastVictim(SPELL_DESTRUCTIVE_BARRAGE);
                        events.CancelEventGroup(2);
                        events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE, 10s, 2, PHASE_TWO);
                        events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE_STOP, 18s);
                        break;
                        /* case EVENT_BRAIN_LINK:
                             DoCastAOE(SPELL_BRAIN_LINK, { SPELLVALUE_MAX_TARGETS, 2 });
                             events.ScheduleEvent(EVENT_BRAIN_LINK, 50s, 0, PHASE_TWO);
                             break;*/
                    default:
                        break;
                    }
                }

                if (events.IsInPhase(PHASE_THREE))
                {
                    switch (eventId)
                    {
                        /*case EVENT_ETHEREAL:
                            me->AddAura(SPELL_ETHEREAL, me);
                            events.ScheduleEvent(EVENT_ETHEREAL, 45s);
                            break;*/
                    case EVENT_DEFILING_HORROR:
                        DoCastVictim(SPELL_DEFILING_HORROR);
                        events.ScheduleEvent(EVENT_DEFILING_HORROR, 5s, 15s, 1, PHASE_ONE);
                        break;
                    case EVENT_ETHEREAL_STOP:
                        me->RemoveAurasDueToSpell(SPELL_ETHEREAL);
                        events.ScheduleEvent(EVENT_ETHEREAL_STOP, 50s);
                        break;
                    case EVENT_WEB_EXPLOSION:
                        DoCastVictim(SPELL_WEB_EXPLOSION);
                        events.ScheduleEvent(EVENT_WEB_EXPLOSION, 26s);
                        break;
                    case EVENT_WEB_GRAB:
                        DoCastVictim(SPELL_WEB_GRAB);
                        events.ScheduleEvent(EVENT_WEB_GRAB, 45s);
                        break;
                    case EVENT_WEB_SPRAY:
                        DoCastVictim(SPELL_WEB_SPRAY);
                        events.ScheduleEvent(EVENT_WEB_SPRAY, 34s);
                        break;
                    case EVENT_SUMMON_ETHEREAL_ARACHNIDS:
                        if (summons.size() <= 1)
                            for (uint32 i = 0; i < 4; ++i)
                                DoSpawnCreature(NPC_ETHEREAL_ARACHNIDS, frand(-9, 9), frand(-9, 9), 1, 0, TEMPSUMMON_CORPSE_DESPAWN, 0s);
                        events.ScheduleEvent(EVENT_SUMMON_ETHEREAL_ARACHNIDS, 40s);
                        break;
                    default:
                        break;
                    }
                }
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            }
            DoMeleeAttackIfReady();
            //_scheduler.Update(diff);
        }

    private:
        TaskScheduler _scheduler;
        bool _IsCursesBroken;
        //inline static uint8 q = 0;

        void _SetCurses()
        {
            uint32 aura_dur_in_sec = 86400;
            Aura* a = me->AddAura(SPELL_CURSE_OF_NAZJATAR, me);
            a->SetDuration(aura_dur_in_sec * SECOND);
            a->SetMaxDuration(aura_dur_in_sec * SECOND);

            a = me->AddAura(SPELL_CURSE_OF_MENDING, me);
            a->SetDuration(aura_dur_in_sec * SECOND);
            a->SetMaxDuration(aura_dur_in_sec * SECOND);

            a = me->AddAura(SPELL_CURSE_PAIN, me);
            a->SetDuration(aura_dur_in_sec * SECOND);
            a->SetMaxDuration(aura_dur_in_sec * SECOND);
        }

        void _SetStartConfigure()
        {
            events.SetPhase(PHASE_INTRO);
            me->SetImmuneToPC(true);
            me->SetFaction(FACTION_FRIENDLY);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void _PhaseOneSpellInit()
        {
            events.ScheduleEvent(EVENT_SPIDER_WEB, 5000s, 1, PHASE_ONE);
            events.ScheduleEvent(EVENT_BLINDING_WEBS, 5000s, 15000s, 1, PHASE_ONE);
            events.ScheduleEvent(EVENT_DRAIN_POWER, 2400s, 1, PHASE_ONE);
            events.ScheduleEvent(EVENT_SUMMON_SHADOWY_MINION, 1s, 1, PHASE_ONE);
            events.ScheduleEvent(EVENT_ENVELOPING_WEBS, 5000s, 9000s, 1, PHASE_ONE);
        }
        void _PhaseTwoSpellInit()
        {
            events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE, 4s, 2, PHASE_TWO);
            events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE_STOP, 12s, 2, PHASE_TWO);
            events.ScheduleEvent(EVENT_BRAIN_LINK, 1s, 2s, 0, PHASE_TWO);
        }
        void _PhaseThreeSpellInit()
        {
            events.ScheduleEvent(EVENT_ETHEREAL_STOP, 8s);
            events.ScheduleEvent(EVENT_DEFILING_HORROR, 9s);
            events.ScheduleEvent(EVENT_WEB_EXPLOSION, 5s);
            events.ScheduleEvent(EVENT_WEB_GRAB, 2s);
            events.ScheduleEvent(EVENT_WEB_SPRAY, 6s);
            events.ScheduleEvent(EVENT_SUMMON_ETHEREAL_ARACHNIDS, 3s);
            //events.ScheduleEvent(EVENT_BRAIN_LINK, 1s, 2s, 0, PHASE_TWO);
        }
    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_uniqueAI(creature);
    }
};

enum ShadowyMinion
{
    SPELL_CONSUMPTION = 28874
};
class npc_shadowy_minion : public CreatureScript
{
public:
    npc_shadowy_minion() : CreatureScript("npc_shadowy_minion") {};
    struct npc_shadowy_minionAI : public PassiveAI
    {
        npc_shadowy_minionAI(Creature* creature) : PassiveAI(creature), _instance(creature->GetInstanceScript()) { }

        void InitializeAI() override
        {
            //me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE_2);
            me->DespawnOrUnsummon(15s);
            _SetCirclePath();
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damageType, SpellInfo const* /*spellInfo = nullptr*/) override
        {
            me->DealDamage(me, attacker, damage);
        }

        void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
        {
            if (caster->ToCreature() == me)
                return;
            me->CastSpell(caster, spellInfo->Id);
            _SetCirclePath();
        }

        void JustAppeared() override
        {
            _scheduler
                .Schedule(2s, [this](TaskContext /*task*/)
                {
                    DoCastSelf(SPELL_CONSUMPTION);
                })
                .Schedule(15s, [this](TaskContext /*task*/)
                {
                });
        }

        void UpdateAI(uint32 diff) override
        {
            _scheduler.Update(diff);
        }

    private:
        TaskScheduler _scheduler;
        InstanceScript* _instance;
        void _SetCirclePath()
        {
            float x, y, z;
            bool clockwise(urand(0, 1));
            me->GetPosition(x, y, z);
            me->GetMotionMaster()->MoveCirclePath(x, y, z, 8, clockwise, 40);
        }
    };
private:
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shadowy_minionAI(creature);
    }
};

class npc_ethereal_arachnids : public CreatureScript
{
public:
    npc_ethereal_arachnids() : CreatureScript("npc_ethereal_arachnids") {};
    struct npc_ethereal_arachnidsAI : public ScriptedAI
    {
        npc_ethereal_arachnidsAI(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()) {}

        void InitializeAI() override
        {
            me->AI()->AttackStart(SelectTarget(SelectTargetMethod::Random, 0, 50, true));
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript* _instance;
    };

private:
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ethereal_arachnidsAI(creature);
    }
};

void AddSC_NPCUnique()
{
    new npc_unique();
    new npc_shadowy_minion();
    new npc_ethereal_arachnids();
}
