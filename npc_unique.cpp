//h@itmpZL2JiH
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


/*std::list<Player*> players;
Trinity::UnitAuraCheck check(false, 29726);
Trinity::PlayerListSearcher<Trinity::UnitAuraCheck> searcher(me, players, check);
Cell::VisitWorldObjects(me, searcher, 10.0f);
if (!players.empty())ChatHandler(players.front()->GetSession()).PSendSysMessage("spell %i", DoCast(me, 34812, false));//xdebug
*/

//todo check command .npc set

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
    SPELL_BLAZING_SPEED = 31643,
    SPELL_COLD_SNAP = 11958,
    SPELL_FOCUS_MAGIC = 54648,
    SPELL_INCANTERS_ABSORPTION = 44413,
    SPELL_BLIGHT_BOMB = 48212,
    SPELL_POWER_WORLD_FORTITUDE = 1243,
    SPELL_INNER_FIRE = 588,

    // Priest
    SPELL_PRIEST_BLESSED_RECOVERY_R1 = 27813,
    SPELL_PRIEST_DIVINE_AEGIS = 47753,
    SPELL_PRIEST_EMPOWERED_RENEW = 63544,
    SPELL_PRIEST_GLYPH_OF_CIRCLE_OF_HEALING = 55675,
    SPELL_PRIEST_GLYPH_OF_LIGHTWELL = 55673,
    SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL = 56161,
    SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL = 48153,
    SPELL_PRIEST_ITEM_EFFICIENCY = 37595,

    // Balinda
    SPELL_ARCANE_EXPLOSION = 46608,
    SPELL_CONE_OF_COLD = 38384,
    SPELL_FIREBALL = 46988,
    SPELL_FROSTBOLT = 46987,
    SPELL_SUMMON_WATER_ELEMENTAL = 45067,
    SPELL_ICEBLOCK = 46604,

    SECOND = 1000   // Constant representing one second in milliseconds
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
    // Balinda
    ACTION_BUFF_YELL = -30001 // shared from Battleground
};

enum UniqueEvents
{
    EVENT_INNER_FIRE = 1,

    // Balinda
    EVENT_ARCANE_EXPLOSION,
    EVENT_FIREBOLT,
    EVENT_FROSTBOLT,
    EVENT_SUMMON_WATER_ELEMENTAL,
    EVENT_CHECK_RESET,          // Checks if Balinda or the Water Elemental are outside of building.

    // Ambtassa flamelash
    EVENT_SUMMON_SPIRITS
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
            /*WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_SPAWN_TIME_SECS);
            stmt->setUInt32(0, spawnTime);
            stmt->setUInt32(1, creature->GetSpawnId());
            WorldDatabase.Execute(stmt);*/ //Update in DB (cs_nps.cpp)
            /*if (CreatureTemplate const* cinfo = creature->GetCreatureTemplate())
                const_cast<CreatureTemplate*>(cinfo)->faction = factionId;*/ //Update in memory (cs_nps.cpp)
        }

        //!is used to initialize variables when creating a creature
        void Initialize()
        {
            _SetCurses();
            _WaterElementalGUID.Clear();
            _IsCursesBroken = false;
            _HasCastIceblock = false;
        }

        //!is called every time an agro is removed.
        void Reset() override
        {
            Initialize();
            _events.Reset();
            summons.DespawnAll();
        }

        //!CreatureAI.h
        void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
        {
            if (caster->ToCreature() == me)
                return;
            if (spellInfo->Id == SPELL_CURSE_OF_NAZJATAR ||
                spellInfo->Id == SPELL_CURSE_PAIN ||
                spellInfo->Id == SPELL_CURSE_OF_MENDING)
                return;
            if (spellInfo->CanDispelAura(sSpellMgr->GetSpellInfo(SPELL_CURSE_OF_NAZJATAR))
                && spellInfo->CanDispelAura(sSpellMgr->GetSpellInfo(SPELL_CURSE_PAIN))
                && spellInfo->CanDispelAura(sSpellMgr->GetSpellInfo(SPELL_CURSE_OF_MENDING)))
            {
                me->RemoveAura(SPELL_CURSE_OF_NAZJATAR);
                me->RemoveAura(SPELL_CURSE_PAIN);
                me->RemoveAura(SPELL_CURSE_OF_MENDING);
                _IsCursesBroken = true;
            }

            _events.ScheduleEvent(EVENT_INNER_FIRE, 1s);
        }

        //!CreatureAI.h
        void ReceiveEmote(Player* player, uint32 emote) override
        {
            if (emote == TEXT_EMOTE_FROWN) {
                me->AddAura(SPELL_BLAZING_SPEED, me);
            }
            if (emote == TEXT_EMOTE_GASP) {
                me->AddAura(SPELL_PRIEST_BLESSED_RECOVERY_R1, me);
            }
            if (emote == TEXT_EMOTE_GAZE) {
                me->AddAura(SPELL_COLD_SNAP, me);
            }
            if (emote == TEXT_EMOTE_GIGGLE)
            {
                me->AddAura(SPELL_FOCUS_MAGIC, me);
            }
            if (emote == TEXT_EMOTE_GROAN)
            {
                me->AddAura(SPELL_INCANTERS_ABSORPTION, me);
            }
            if (emote == TEXT_EMOTE_GROVEL) {
                DoCastSelf(SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL, me);
            }
            ChatHandler(player->GetSession()).PSendSysMessage("20719");//xdebug
        }

        
        //!CreatureAI.h
        void JustEnteredCombat(Unit* who) override
        {
            //me->SetStandState(UNIT_STAND_STATE_STAND);
            //me->CastSpell(who, SPELL_POWER_WORLD_FORTITUDE, true);
        }

        //!CreatureAI.h
        void JustEngagedWith(Unit* who) override
        {
            BossAI::JustEngagedWith(who);
            _events.ScheduleEvent(EVENT_ARCANE_EXPLOSION, 5s, 15s);
            _events.ScheduleEvent(EVENT_FIREBOLT, 1s);
            _events.ScheduleEvent(EVENT_SUMMON_SPIRITS, 24s);
            _events.ScheduleEvent(EVENT_SUMMON_WATER_ELEMENTAL, 3s);
        }

        //!CreatureAI.h
        void JustSummoned(Creature* summoned) override
        {
            summoned->AI()->AttackStart(SelectTarget(SelectTargetMethod::Random, 0, 50, true));
            summoned->SetFaction(me->GetFaction());
            if (NPC_GREATER_WATER_ELEMENTAL == summoned->GetEntry())
                _WaterElementalGUID = summoned->GetGUID();
            summons.Summon(summoned);
        }

        //!CreatureAI.h
        void SummonedCreatureDespawn(Creature* summoned) override
        {
            summons.Despawn(summoned);
        }

        //!CreatureAI.h
        void JustDied(Unit* killer)
        {
            summons.DespawnAll();
            killer->SetUnitFlag(UNIT_FLAG_SILENCED);
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

        //!CreatureAI.h
        void KilledUnit(Unit* victim)
        {
            /*_UpdatePowerStats();
            me->GetHealthGain(3);*/
        }

        //!ScriptedCreature.h
        void DoAction(int32 actionId) override
        {
            if (actionId == ACTION_BUFF_YELL)
                Talk(0);
        }

        bool CheckInRoom() override
        {
            if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 50)
            {
                EnterEvadeMode();
                Talk(1);
                return false;
            }
            if (Creature* elemental = ObjectAccessor::GetCreature(*me, _WaterElementalGUID))
                if (elemental->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 50)
                    elemental->AI()->EnterEvadeMode();
            _events.ScheduleEvent(EVENT_CHECK_RESET, 5s);

            return true;
        }

        //!UnitAI.h
        //!Called before damage apply
        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damageType, SpellInfo const* /*spellInfo = nullptr*/) override
        {
            /* damage = 0;*/
            if (me->HealthBelowPctDamaged(40, damage) && !_HasCastIceblock)
            {
                DoCast(SPELL_ICEBLOCK);
                _HasCastIceblock = true;
            }
        }

        //!UnitAI.h
        void JustExitedCombat() override
        {
        }

        //!CreatureAI.h
        void JustReachedHome()
        {
            //me->SetStandState(UNIT_STAND_STATE_SLEEP);
            me->SetHealth(me->GetMaxHealth());
        }

        //!CreatureAI.h
        void IsSummonedBy(WorldObject* summoner) override
        {
            //xexample
            /*if (summoner->GetTypeId() == TYPEID_PLAYER)
            {
                summonerGUID = summoner->GetGUID();
            }*/
            //todo own spell to summoning
        }

        //!CreatureAI.h
        void MovementInform(uint32 /*type*/, uint32 id) override
        {
            //xexample
            /*if (id == MOVEID_CHASE)
                _nextAction = EVENT_DO_JUMP;
            else if (id == MOVEID_JUMP)
                _nextAction = EVENT_DO_FACING;*/
        }

        //!CreatureAI.h
        void JustAppeared() override
        {
            //_scheduler.Schedule(2s, [this](TaskContext /*task*/)
            //    {
            //        DoCastSelf(28874);
            //    });

            _SetStartConfigure();
            _UpdatePowerStats();
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
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetImmuneToPC(false);
                me->SetFaction(FACTION_SUPER_ENEMY);
                break;
            case GOSSIP_ACTION_INFO_DEF + 7:
                SendGossipMenuFor(player, NPC_TEXT_UNIQUE_4, me->GetGUID());
                ChatHandler(player->GetSession()).PSendSysMessage("Team %u", player->GetTeam());//xdebug
                player->AddItem(ITEM_PROTO_DRAKE_REINS, 1);
                break;
            }

            return true;
        }

        // Called at World update tick
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || !CheckInRoom())  //!will check to see if the victim has updated
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ARCANE_EXPLOSION:
                    DoCastVictim(SPELL_ARCANE_EXPLOSION);
                    _events.ScheduleEvent(EVENT_ARCANE_EXPLOSION, 5s, 15s);
                    break;
                case EVENT_FIREBOLT:
                    DoCastVictim(SPELL_FIREBALL);
                    _events.ScheduleEvent(EVENT_FIREBOLT, 5s, 9s);
                    break;
                case EVENT_INNER_FIRE:
                    for (size_t i = 0; i < 2; i++)
                        me->CastSpell(me, SPELL_INNER_FIRE, true);
                    break;
                case EVENT_SUMMON_SPIRITS:
                    if (summons.size() < 5)
                        for (uint32 i = 0; i < 4; ++i)
                            DoSpawnCreature(9178, frand(-9, 9), frand(-9, 9), 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60s);
                    _events.ScheduleEvent(EVENT_SUMMON_SPIRITS, 30s);
                    break;
                case EVENT_SUMMON_WATER_ELEMENTAL:
                    if (summons.size() == 0)
                        DoCast(SPELL_SUMMON_WATER_ELEMENTAL);
                    _events.ScheduleEvent(EVENT_SUMMON_WATER_ELEMENTAL, 25s);
                    break;
                default:
                    break;
                }
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            }
            DoMeleeAttackIfReady();

            //_scheduler.Update(diff);
        }

    private:
        std::unordered_map<ObjectGuid /*attackerGUID*/, Milliseconds /*combatTime*/> _combatTimer;
        bool _zero = urand(0, 100) == 0; // random uint value
        bool _isJustEnteredCombat = false;
        TaskScheduler _scheduler;
        EventMap _events;
        ObjectGuid _WaterElementalGUID;
        ObjectGuidMap _WaterElementalGUIDMap;
        bool _HasCastIceblock;
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
            me->SetImmuneToPC(true);
            me->SetFaction(FACTION_FRIENDLY);
            me->SetCreateMana(BASE_MANA);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ALLOW_CHEAT_SPELLS); //???
            me->SetFullPower(POWER_MANA);
        }

        void _UpdatePowerStats()
        {
            me->UpdateAllStats();
            me->UpdateMaxPower(POWER_MANA);
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
