
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

    FACTION_SUPER_ENEMY = 1620
};

enum UniqueNPC : uint32
{
    NPC_BURNING_SPIRIT = 9178,
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

    //GPT  Arachnos the Mindbender
    //phase 1
    SPELL_DEFILING_HORROR = 72435,
    SPELL_ENTANGLING_ROOTS = 339,
    SPELL_DRAIN_POWER = 44131, //stackable
    SPELL_SUMMON_DREADBONE_SKELETON = 29066,

    //phase 2
    SPELL_DESTRUCTIVE_BARRAGE = 48734,

    // Balinda
    SPELL_ICEBLOCK = 46604,

    SECOND = 1000   // Constant representing one second in milliseconds
};

enum UniqueEvents
{
    //phase 1
    EVENT_INNER_FIRE = 1,
    EVENT_DEFILING_HORROR,
    EVENT_ENTANGLING_ROOTS,
    EVENT_DRAIN_POWER,
    EVENT_SUMMON_DREADBONE_SKELETON,

    //phase 2
    EVENT_DESTRUCTIVE_BARRAGE,
    EVENT_DESTRUCTIVE_BARRAGE_STOP,
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
            _UpdatePowerStats();
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

                events.ScheduleEvent(SPELL_DRAIN_POWER, 1s);
            }
        }

        //!UnitAI.h
        //!Called before damage apply
        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damageType, SpellInfo const* /*spellInfo = nullptr*/) override
        {
            if (me->HealthBelowPctDamaged(50, damage) && events.IsInPhase(PHASE_ONE))
            {
                events.CancelEventGroup(1);
                events.SetPhase(PHASE_TWO);
                DoCast(SPELL_ICEBLOCK);
                _PhaseTwoSpellInit();
            }
        }

        //!CreatureAI.h
        void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
        {
            if (caster->ToCreature() == me)
                return;
            if (spellInfo->Id == SPELL_CURSE_OF_NAZJATAR ||
                spellInfo->Id == SPELL_CURSE_PAIN ||
                spellInfo->Id == SPELL_CURSE_OF_MENDING)
            {
                me->ApplySpellImmune(SPELL_CURSE_OF_NAZJATAR, 0, 162, true);
                me->ApplySpellImmune(SPELL_CURSE_PAIN, 0, 162, true);
                me->ApplySpellImmune(SPELL_CURSE_OF_MENDING, 0, 162, true);
                return;
            }
            if (me->GetFaction() == FACTION_FRIENDLY)
                if (spellInfo->CanDispelAura(sSpellMgr->GetSpellInfo(SPELL_CURSE_OF_NAZJATAR))
                    && spellInfo->CanDispelAura(sSpellMgr->GetSpellInfo(SPELL_CURSE_PAIN))
                    && spellInfo->CanDispelAura(sSpellMgr->GetSpellInfo(SPELL_CURSE_OF_MENDING)))
                    _IsCursesBroken = true;

            events.ScheduleEvent(EVENT_INNER_FIRE, 1s);
        }

        //!CreatureAI.h
        void JustEngagedWith(Unit* who) override
        {
            BossAI::JustEngagedWith(who);
            events.ScheduleEvent(EVENT_DEFILING_HORROR, 5s, 15s, 1, PHASE_ONE);
            events.ScheduleEvent(EVENT_ENTANGLING_ROOTS, 1s, 1, PHASE_ONE);
            events.ScheduleEvent(EVENT_DRAIN_POWER, 24s, 1, PHASE_ONE);
            events.ScheduleEvent(EVENT_SUMMON_DREADBONE_SKELETON, 3s, 1, PHASE_ONE);
        }

        //!CreatureAI.h
        void JustSummoned(Creature* summoned) override
        {
            BossAI::JustSummoned(summoned);
            /*summoned->AI()->AttackStart(SelectTarget(SelectTargetMethod::Random, 0, 50, true));
            summoned->SetFaction(me->GetFaction());*/
        }

        //!CreatureAI.h
        void SummonedCreatureDespawn(Creature* summoned) override
        {
            BossAI::SummonedCreatureDespawn(summoned);
            //
        }

        //!CreatureAI.h
        void JustDied(Unit* killer)
        {
            summons.DespawnAll();
            killer->CastSpell(killer, SPELL_BLIGHT_BOMB, false);
            /*if (!spell)
            {
                handler->PSendSysMessage(LANG_COMMAND_NOSPELLFOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
            if (!target)
            {
                handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }*/
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

        //!CreatureAI.h
        void JustReachedHome()
        {
            //me->SetStandState(UNIT_STAND_STATE_SLEEP);
            me->SetHealth(me->GetMaxHealth());
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
                    case EVENT_DEFILING_HORROR:
                        DoCastVictim(SPELL_DEFILING_HORROR);
                        events.ScheduleEvent(EVENT_DEFILING_HORROR, 5s, 15s, 1, PHASE_ONE);
                        break;
                    case EVENT_ENTANGLING_ROOTS:
                        DoCastVictim(SPELL_ENTANGLING_ROOTS);
                        events.ScheduleEvent(EVENT_ENTANGLING_ROOTS, 5s, 9s, 1, PHASE_ONE);
                        break;
                    case EVENT_DRAIN_POWER:
                        for (size_t i = 0; i < 20; i++)
                            DoCastVictim(SPELL_DRAIN_POWER);
                        events.ScheduleEvent(EVENT_DRAIN_POWER, 50s, 90s, 1, PHASE_ONE);
                        break;
                    case EVENT_SUMMON_DREADBONE_SKELETON:
                        if (summons.size() == 0)
                            for (uint32 i = 0; i < 7; ++i)
                                DoSpawnCreature(6388, frand(-9, 9), frand(-9, 9), 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60s);
                        events.ScheduleEvent(EVENT_SUMMON_DREADBONE_SKELETON, 30s, 1, PHASE_ONE);
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
                        me->SetEmoteState(EMOTE_ONESHOT_DANCE);
                        DoCastVictim(SPELL_DESTRUCTIVE_BARRAGE);    
                        events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE, 500ms);
                        break;
                    case EVENT_DESTRUCTIVE_BARRAGE_STOP:
                        DoCastVictim(SPELL_DESTRUCTIVE_BARRAGE);
                        events.CancelEvent(EVENT_DESTRUCTIVE_BARRAGE);
                        events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE, 10s);
                        events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE_STOP, 18s);
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
        std::unordered_map<ObjectGuid /*attackerGUID*/, Milliseconds /*combatTime*/> _combatTimer;
        bool _zero = urand(0, 100); // random uint value
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
            me->SetCreateMana(BASE_MANA);
            me->SetFullPower(POWER_MANA);
        }

        void _UpdatePowerStats()
        {
            me->UpdateAllStats();
            me->UpdateMaxPower(POWER_MANA);
        }

        void _PhaseTwoSpellInit()
        {
            events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE, 4s);
            events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE_STOP, 12s);
        }
    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_uniqueAI(creature);
    }
};

void AddSC_NPCUnique()
{
    new npc_unique();
}
