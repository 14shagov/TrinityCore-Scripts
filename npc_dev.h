#pragma once
#ifndef NPC_DEV_H
#define NPC_DEV_H
#include "Player.h"
#include "Define.h"

enum CustomGossipAction //AC - action
{
    //!main custom ac
    AC_CLOSE_GOSSIP_MENU                    = 0,
    AC_CITIES_MENU                          = 2,
    AC_ARENA_MENU                           = 3,
    AC_REFRESH_MENU                         = 9,    
    AC_GOLD                                 = 30,
    AC_EFFECT                               = 31,
    AC_DESTROY_GEAR                         = 32,
    AC_ENCHANT                              = 33,
    AC_TRAINER_VENDOR_MENU                  = 34,
    AC_QUEST_START                          = 38,

    //!quest ac (actions for certain quests)
    AC_QUEST_0                              = 0,
    AC_QUEST_1                              = 1,
    AC_QUEST_2                              = 2,
    AC_QUEST_3                              = 3,
    AC_QUEST_4                              = 4,
    AC_QUEST_5                              = 5,
    AC_QUEST_6                              = 6,
    AC_MAX_QUEST,

    //!tele ac
    //alliance tele (0 - 9)
    AC_STORMWIND_TELE                       = 0,
    AC_STORMWIND_PRISON_TELE                = 1,
    //horde tele (10 - 19)
    AC_ORGRIMMAR_TELE                       = 10,
    AC_RAPTOR_GROUNDS_TELE                  = 11,
    AC_YMIHEIM_TELE                         = 12,
    //arena tele (20 - ...)
    AC_GURUBASHI_TELE                       = 20,

    //!trianer vendor ac
    AC_VENDOR                               = 1,
    AC_CUSTOM_COSSIP_MENU                   = 2,
    AC_TRAINER                              = 3
};

enum CustomGossipSender 
{
    GOSSIP_SENDER_QUEST                          = 2,
    GOSSIP_SENDER_TELEPORT                       = 3,
    GOSSIP_SENDER_TRAINER_VENDOR                 = 4
};

enum CustomGossipMenu
{
    TRAINER_BOX = 60000,
    GOSSIP_OPTION_1 = 0,
    GOSSIP_OPTION_2 = 1,
    GOSSIP_OPTION_3 = 2,
    GOSSIP_OPTION_4 = 3
};

class snpc_dev
{
public:
    static snpc_dev* instance()
    {
        static snpc_dev* instance = new snpc_dev();
        return instance;
    }
    void ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry);
    void AddCustomQuest(Player* player, uint32 quest_id, Creature* creature);
};
#define sDevMgr snpc_dev::instance()
#endif
