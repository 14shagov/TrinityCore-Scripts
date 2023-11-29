//h@itmpZL2JiH
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Creature.h"
#include "GossipDef.h"
#include "WorldSession.h"
#include "Chat.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "TemplateNPC.h"
#include "Item.h"
#include "npc_dev.h"
#include "World.h"
#include "ObjectMgr.h"

void snpc_dev::ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry)
{
    if (!item)
        return;
    if (!bonusEntry || bonusEntry == 0)
        return;
    player->ApplyEnchantment(item, slot, false);
    item->SetEnchantment(slot, bonusEntry, 0, 0);
    player->ApplyEnchantment(item, slot, true);
}

void snpc_dev::AddCustomQuest(Player* player, uint32 quest_id, Creature* creature)
{
    Quest const* quest = sObjectMgr->GetQuestTemplate(quest_id);
    if (!quest)
    {
        ChatHandler(player->GetSession()).PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, quest_id);
        ChatHandler(player->GetSession()).SetSentErrorMessage(true); 
        return;
    }
    if (player->CanAddQuest(quest, true))
        player->AddQuestAndCheckCompletion(quest, creature);
    return;
}

class npc_dev : public CreatureScript
{
public:
    npc_dev() : CreatureScript("npc_dev_s") {}
    static npc_dev* instance()
    {
        static npc_dev* instance = new npc_dev();
        return instance;
    }

    struct npc_devAI : public ScriptedAI
    {
        npc_devAI(Creature* creature) : ScriptedAI(creature) {}

        void Initialize()
        {
            me->SetUnitFlag(UNIT_FLAG_RENAME);
        }

        bool OnGossipHello(Player* player) override
        {
            if (!player || !me)
                return true;
            ClearGossipMenuFor(player);

            AddGossipItemFor(player, GOSSIP_ICON_TABARD,"|TInterface\\icons\\inv_ore_gold_01:35|t|r Give me gold",
                GOSSIP_SENDER_MAIN, AC_GOLD);
            AddGossipItemFor(player, GOSSIP_ICON_TABARD,"|TInterface\\icons\\spell_holy_mindvision:35|t|r Give me effect",
                GOSSIP_SENDER_MAIN, AC_EFFECT);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|cff00ff00|TInterface\\icons\\Spell_ChargeNegative:30|t|r Destroy my equipped gear",
                GOSSIP_SENDER_MAIN, AC_DESTROY_GEAR);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|cff00ff00|TInterface\\icons\\Spell_ChargeNegative:30|t|r Enchantmen",
                GOSSIP_SENDER_MAIN, AC_ENCHANT);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\trade_blacksmithing:30|t|r Trainer and Vendor",
                GOSSIP_SENDER_MAIN, AC_TRAINER_VENDOR_MENU);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\inv_misc_questionmark:30|t|r Quests",
                GOSSIP_SENDER_MAIN, AC_QUEST_START);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\achievement_bg_defendxtowers_av:35|t|r Cities",
                GOSSIP_SENDER_MAIN, AC_CITIES_MENU);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\achievement_arena_2v2_3:35|t|r Arena",
                GOSSIP_SENDER_MAIN, AC_ARENA_MENU);

            AddGossipItemFor(player, GOSSIP_ICON_TABARD, "[Newermind]", GOSSIP_SENDER_MAIN, AC_CLOSE_GOSSIP_MENU);

            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }

        void SendMainActionMenu(Player* player, uint32 action)
        {
            //ChatHandler(player->GetSession()).PSendSysMessage("actiondebug6 %u", action); //debug
            switch (action)
            {
            case AC_CLOSE_GOSSIP_MENU: //close gossip menu
            {
                CloseGossipMenuFor(player);
                break;
            }
            case AC_CITIES_MENU: //Open Cities menu
            {
                if (player->GetTeam() == Team::ALLIANCE)
                {
                    //for alliance
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\spell_arcane_portalstormwind:35|t|r Teleport to Stormwind",
                        GOSSIP_SENDER_TELEPORT, AC_STORMWIND_TELE);
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\inv_misc_bone_skull_02:35|t|r Teleport to Stormwind Prison",
                        GOSSIP_SENDER_TELEPORT, AC_STORMWIND_PRISON_TELE);
                    CloseGossipMenuFor(player);
                }
                else
                {
                    //for horde
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\spell_arcane_portalorgrimmar:35|t|r Teleport to Orgrimmar",
                        GOSSIP_SENDER_TELEPORT, AC_ORGRIMMAR_TELE);
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\achievement_boss_lichking:35|t|r Teleport to Ymirheim",
                        GOSSIP_SENDER_TELEPORT, AC_YMIHEIM_TELE);
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\ability_mount_raptor:35|t|r Teleport to Raptor Grounds",
                        GOSSIP_SENDER_TELEPORT, AC_RAPTOR_GROUNDS_TELE);
                    CloseGossipMenuFor(player);
                }

                AddGossipItemFor(player, GOSSIP_ICON_TABARD,"|TInterface\\icons\\achievement_bg_returnxflags_def_wsg:35|t|r [Back]",
                    GOSSIP_SENDER_MAIN, AC_REFRESH_MENU);
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "[Newermind]", GOSSIP_SENDER_MAIN, AC_CLOSE_GOSSIP_MENU);
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                break;
            }
            case AC_ARENA_MENU: //Open Arena menu
            {
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,"|TInterface\\icons\\inv_helm_mask_zulgurub_d_01:35|t|r Teleport to Gurubashi",
                    GOSSIP_SENDER_TELEPORT, AC_GURUBASHI_TELE);
                AddGossipItemFor(player, GOSSIP_ICON_TABARD,"|TInterface\\icons\\achievement_bg_returnxflags_def_wsg:35|t|r [Back]",
                    GOSSIP_SENDER_MAIN, AC_REFRESH_MENU);
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "[|cffFF0000Newermindr|r]", GOSSIP_SENDER_MAIN, AC_CLOSE_GOSSIP_MENU);
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                break;
            }
            case AC_REFRESH_MENU: //refresh main gossip menu
            {
                CloseGossipMenuFor(player);
                OnGossipHello(player);
                break;
            }            
            case AC_GOLD: //give gold
            {
                player->ModifyMoney(1000 * GOLD);
                CloseGossipMenuFor(player);
                OnGossipHello(player);
                break;
            }
            case AC_EFFECT: //give effect
            {
                player->RemoveAllAuras();
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(50490);
                Spell* custom_spell = new Spell(player, spellInfo, TRIGGERED_NONE);

                if (!player)
                    ChatHandler(player->GetSession()).PSendSysMessage("!player");
                if (!spellInfo)
                    ChatHandler(player->GetSession()).PSendSysMessage("!spellInfo");

                //player->AddAura(50490, player);
                player->AddAura(spellInfo, MAX_EFFECT_MASK, player);
                sTemplateNpcMgr->LearnPlateMailSpells(player);
                player->LearnSpell(49497, false);
                player->SetSkill(293, 1, 100, 150);
                CloseGossipMenuFor(player);
                OnGossipHello(player);
                //player->GetSession()->SendNotification("You got effect1"); //debug
                break;
            }
            case AC_DESTROY_GEAR: //Destroy gear
            {
                for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
                {
                    if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    {
                        if (haveItemEquipped)
                        {
                            player->DestroyItemCount(haveItemEquipped->GetEntry(), 1, true, true);
                            if (haveItemEquipped->IsInWorld())
                            {
                                haveItemEquipped->RemoveFromWorld();
                                haveItemEquipped->DestroyForPlayer(player);
                            }
                            haveItemEquipped->SetSlot(NULL_SLOT);
                            //ChatHandler(player->GetSession()).PSendSysMessage("debug6 %u", haveItemEquipped->GetSlot()); //debug
                            //haveItemEquipped->SetGuidValue(ITEM_FIELD_CONTAINED, ObjectGuid::Empty); //crash game
                            //ChatHandler(player->GetSession()).PSendSysMessage("debug6 %s", haveItemEquipped->GetGuidValue(ITEM_FIELD_CONTAINED).ToString()); //debug
                            //haveItemEquipped->SetState(ITEM_REMOVED, player);        //crash game
                            //ChatHandler(player->GetSession()).PSendSysMessage("debug6 %u", haveItemEquipped->GetState()); //debug
                        }
                    }
                }
                player->SendEquipmentSetList();
                player->GetSession()->SendAreaTriggerMessage("Your equipped gear has been destroyed.");
                CloseGossipMenuFor(player);
                break;
            }
            case AC_ENCHANT: //enchant
            {
                PERM_ENCHANTMENT_SLOT;
                uint32 bonusEntry = 3796;

                Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, 0);
                sDevMgr->ApplyBonus(player, item, PERM_ENCHANTMENT_SLOT, bonusEntry);
                sDevMgr->ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT, 3621);
                sDevMgr->ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT_2, 3520);
                sDevMgr->ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT_3, 0);
                sDevMgr->ApplyBonus(player, item, BONUS_ENCHANTMENT_SLOT, 3352);
                sDevMgr->ApplyBonus(player, item, PRISMATIC_ENCHANTMENT_SLOT, 0);
                player->GetSession()->SendAreaTriggerMessage("Your equipped gear has been enchanted.");
                CloseGossipMenuFor(player);
                break;
            }
            case AC_TRAINER_VENDOR_MENU: //trainer and vendor
            {
                //gossipMenuItemID responsible for the OptionID in gossip_menu_option table
                AddGossipItemFor(player, 0, 1, GOSSIP_SENDER_TRAINER_VENDOR, AC_VENDOR); //vendor
                AddGossipItemFor(player, 0, 2, GOSSIP_SENDER_TRAINER_VENDOR, AC_CUSTOM_COSSIP_MENU); //gossip menu id test
                AddGossipItemFor(player, 0, 3, GOSSIP_SENDER_TRAINER_VENDOR, AC_TRAINER); //trainer             
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());  
                break;
            }            
            case AC_QUEST_START: //quest
            {
                if (!me->IsQuestGiver())
                {
                    break;
                    CloseGossipMenuFor(player);
                }
                uint32 entry = 90010;                
                if (!player->IsActiveQuest(entry))
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "Quest 90010", GOSSIP_SENDER_QUEST, AC_QUEST_0);
                entry = 90001;
                if (!player->IsActiveQuest(entry))
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "Quest 90001", GOSSIP_SENDER_QUEST, AC_QUEST_1);
                entry = 900021;
                if (!player->IsActiveQuest(entry))
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "Quest 90002", GOSSIP_SENDER_QUEST, AC_QUEST_2);
                entry = 90003;
                if (!player->IsActiveQuest(entry))
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "Quest 90003", GOSSIP_SENDER_QUEST, AC_QUEST_3);
                entry = 90005;
                if (!player->IsActiveQuest(entry))
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "Quest 90005", GOSSIP_SENDER_QUEST, AC_QUEST_5);
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());     
                break;
            }
            }
        }

        //GOSSIP_SENDER_TRAINER_VENDOR
        void SendVendorTrainerActionMenu(Player* player, uint32 action) 
        {
            switch (action)
            {
            case AC_TRAINER: //trainer
            {
                if (me->IsTrainer())
                    player->GetSession()->SendTrainerList(me);
                break;
            }
            case AC_VENDOR: //vendor
            {
                if (me->IsVendor())
                    player->GetSession()->SendListInventory(me->GetGUID());
                break;
            }
            case AC_CUSTOM_COSSIP_MENU: //gossip menu id test
            {
                AddGossipItemFor(player, TRAINER_BOX, GOSSIP_OPTION_3, GOSSIP_SENDER_TRAINER_VENDOR, AC_TRAINER_VENDOR_MENU);
                AddGossipItemFor(player, TRAINER_BOX, GOSSIP_OPTION_4, GOSSIP_SENDER_TRAINER_VENDOR, AC_TRAINER_VENDOR_MENU);
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                break;
            }
            default:
                break;
            }
           
        }

        //GOSSIP_SENDER_QUEST
        void SendTeleportActionMenu(Player* player, uint32 action) 
        {
            //ChatHandler(player->GetSession()).PSendSysMessage("actiondebug6 %u", action); //debug
            switch (action)
            {
            case AC_STORMWIND_TELE: //Stormwind teleport
            {
                player->TeleportTo(0, -8833.38f, 628.628f, 94.0066f, 1.06535);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_STORMWIND_PRISON_TELE: //StormwindPrison teleport
            {
                player->TeleportTo(35, -1.33456f, 57.3787f, -27.5507f, 1.56687);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_ORGRIMMAR_TELE: //Orgrimmar teleport
            {
                player->TeleportTo(1, 1629.36f, -4373.39f, 31.2564f, 3.54839);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_RAPTOR_GROUNDS_TELE: //Raptor Grounds teleport
            {
                player->TeleportTo(1, -2023.09f, -3220.48f, 90.5983f, 1.07774);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_YMIHEIM_TELE: //Ymirheim teleport
            {
                player->TeleportTo(571, 8495.49f, 2657.95f, 944.031, 1.07774);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_GURUBASHI_TELE: //gurubashi arena teleport
            {
                player->TeleportTo(0, -13277.4f, 127.372f, 26.1416f, 1.1878);
                CloseGossipMenuFor(player);
                break;
            }
            default:
                break;
            }
        }

        //GOSSIP_SENDER_QUEST
        void SendQuestActionMenu(Player* player, uint32 action)
        {
            //ChatHandler(player->GetSession()).PSendSysMessage("debug6 %u", action); //debug
            switch (action)
            {
            case AC_QUEST_0:
            {
                //player->SetQuestSlot(0, questid);
                //player->SetQuestStatus(questid, QUEST_STATUS_INCOMPLETE);
                uint32 entry = 90010;
                sDevMgr->AddCustomQuest(player, entry, me);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_QUEST_1:
            {
                uint32 entry = 90001;
                sDevMgr->AddCustomQuest(player, entry, me);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_QUEST_2:
            {
                uint32 entry = 90002;
                sDevMgr->AddCustomQuest(player, entry, me);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_QUEST_3:
            {
                uint32 entry = 90003;
                sDevMgr->AddCustomQuest(player, entry, me);
                CloseGossipMenuFor(player);
                break;
            }
            case AC_QUEST_5:
            {
                uint32 entry = 90005;
                sDevMgr->AddCustomQuest(player, entry, me);
                CloseGossipMenuFor(player);
                break;
            }
            default:
                break;
            }
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const sender = player->PlayerTalkClass->GetGossipOptionSender(gossipListId);
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            if (!player || !me)
            {
                return true;
            }
            player->PlayerTalkClass->ClearMenus(); //can also be CloseGossipMenuFor(player);
            switch (sender)
            {
            case GOSSIP_SENDER_MAIN:
                SendMainActionMenu(player, action);
                break;
            case GOSSIP_SENDER_QUEST:
                SendQuestActionMenu(player, action);
                break;
            case GOSSIP_SENDER_TELEPORT:
                SendTeleportActionMenu(player, action);
                break;
            case GOSSIP_SENDER_TRAINER_VENDOR:
                SendVendorTrainerActionMenu(player, action);
                break;
            default:
                break;
            }
            return true;
        }
    };
    CreatureAI* GetAI(Creature* creature)const override
    {
        return new npc_devAI(creature);
    }
};
void AddSC_npc_dev() {
    new npc_dev();
}
