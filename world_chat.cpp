#include "RBAC.h"               
#include "Chat.h"
#include "ChatCommand.h"
#include "ChatPackets.h"
#include "World.h"
#include "WorldSession.h"
#include "Player.h"
#include "SocialMgr.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "SmartEnum.h"


std::string GetRankName(uint8 rankID)
{
    switch (rankID)
    {
    case 0:
        return "Player";
    case 1:
        return "Moderator";
    case 2:
        return "Game Master";
    case 3:
        return "Administrator";
    default:
        return "Server";
    }
}

class WorldChat : public CommandScript
{
public:
    WorldChat() : CommandScript("world_chat") {}

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> commandTable =
        {
            {"chat", HandleWorldChat, LANG_LFG_STATE_NONE, rbac::RBAC_PERM_COMMAND_CHAT, Trinity::ChatCommands::Console::No},
            {"world", HandleNameAnnounceCommand1, LANG_LFG_STATE_NONE, rbac::RBAC_PERM_COMMAND_CHAT, Trinity::ChatCommands::Console::Yes},
            {"powerup", HandlePowerUpBuffCommand, LANG_COMMAND_POWERUP_STATE, rbac::RBAC_PERM_COMMAND_GM_CHAT, Trinity::ChatCommands::Console::No},
            {"speedup", HandleSpeedUpBuffCommand, LANG_COMMAND_SPEEDUP_STATE, rbac::RBAC_ROLE_GAMEMASTER, Trinity::ChatCommands::Console::No},
            {"clear", HandleClearBPCommand, LANG_COMMAND_CLEAR_BP, rbac::RBAC_ROLE_GAMEMASTER, Trinity::ChatCommands::Console::No},
        };

        return commandTable;
    }

    static bool HandleSpeedUpBuffCommand(ChatHandler* handler, float speed_value)
    {
        Player* target = handler->getSelectedPlayer(); //
        if (!target) // if no one is selected, put the buff on yourself.
            target = handler->GetSession()->GetPlayer();

        //!rbac::RBAC_ROLE_GAMEMASTER replaces all this
        /*uint8 playerRank = target->GetSession()->GetSecurity();
        if (playerRank < 3)
        {
            handler->PSendSysMessage(LANG_CMD_INVALID, "speedup");
            return false;
        }*/

        target->SetSpeed(MOVE_RUN, speed_value);
        return true;
    }

    static bool HandlePowerUpBuffCommand(ChatHandler* handler, float power)
    {
        Player* target = handler->getSelectedPlayer();
        if (!target) // if no one is selected, put the buff on yourself.
            target = handler->GetSession()->GetPlayer();
        target->SetAttackPower(power * 10001);
        return true;
    }

    static bool HandleNameAnnounceCommand1(ChatHandler* handler, std::string message)
    {
        if (message.empty())
            return false;
        std::string name("Console");
        if (WorldSession* session = handler->GetSession())
            name = session->GetPlayer()->GetName();
        sWorld->SendWorldText(LANG_ANNOUNCE_COLOR, name.c_str(), message.data());
        return true;
    }

    static bool HandleWorldChat(ChatHandler* handler, std::string args)
    {
        //todo fix loop
        std::string msg = "";

        Player* player = handler->GetPlayer();

        if (!player || !handler)
            return false;

        uint8 playerRank = player->GetSession()->GetSecurity();
        msg += player->GetName();
        msg += "[|cffff0000" + GetRankName(playerRank) + "|r] " + ": ";
        msg += args;

        //if (args.data() == "yes") //! don't work
        //{
        //    msg = "no";
        //    sWorld->SendServerMessage(ServerMessageType::SERVER_MSG_STRING, msg);
        //    return true;
        //}
        //sWorld->SendServerMessage(ServerMessageType::SERVER_MSG_STRING, msg.c_str());

        for (SessionMap::const_iterator itr = sWorld->GetAllSessions().begin(); itr != sWorld->GetAllSessions().end(); ++itr)
        {
            if (!itr->second)
                continue;
            Player* plr = itr->second->GetPlayer();

            if (plr->GetSocial()->HasIgnore(player->GetGUID()) && player->GetSession()->GetSecurity() == 0)
            {
                msg = ""; //empty the message, it will not send to the player
            }
            if (msg != "")
            {
                //sWorld->SendServerMessage(ServerMessageType::SERVER_MSG_STRING, msg.c_str());
                handler->PSendSysMessage("%s", msg.c_str());
            }
        }
        return true;
    }

    static bool HandleClearBPCommand(ChatHandler* handler)
    {
        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
            return false;

        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        {
            if (Item* haveItemEquipped = target->GetItemByPos(INVENTORY_SLOT_BAG_0, i)) //target->GetItemByPos(i);
            {
                if (haveItemEquipped)
                {
                    uint32 item_count = haveItemEquipped->GetCount();

                    target->DestroyItemCount(haveItemEquipped->GetEntry(), item_count, true, true);
                    if (haveItemEquipped->IsInWorld())
                    {
                        haveItemEquipped->RemoveFromWorld();
                        haveItemEquipped->DestroyForPlayer(target);
                    }
                    haveItemEquipped->SetSlot(NULL_SLOT);
                }
            }
        }
        ChatHandler(target->GetSession()).PSendSysMessage(LANG_COMMAND_CLEAR_BP); //LANG
        return true;
    }
};
void AddSC_WorldChat()
{
    new WorldChat();
}
