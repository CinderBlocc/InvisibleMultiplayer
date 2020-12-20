#include "InvisibleMultiplayer.h"
#include <fstream>
#include <filesystem>

#define nl(x) SettingsFile << std::string(x) << '\n'
#define blank SettingsFile << '\n'
#define cv(x) std::string(x)

void InvisibleMultiplayer::GenerateSettingsFile()
{
    std::ofstream SettingsFile(gameWrapper->GetBakkesModPath() / "plugins" / "settings" / "InvisibleMultiplayer.set");

    nl("InvisibleMultiplayer");
    blank;
    nl("6|Invisible player|" + cv(CVAR_SELECTED_PLAYER) + "|" + GetPlayerList());
    nl("7|");
    nl("0|Refresh list|" + cv(NOTIFIER_REFRESH_LIST));

    SettingsFile.close();
    cvarManager->executeCommand("cl_settings_refreshplugins");
}

std::string InvisibleMultiplayer::GetPlayerList()
{
    //Fill list of eligible players
    ServerWrapper server = GetCurrentGameState();
    std::vector<PriWrapper> Players;
    if(!server.IsNull())
    {
        ArrayWrapper<PriWrapper> PlayerPRIs = server.GetPRIs();
        for(int i = 0; i < PlayerPRIs.Count(); ++i)
        {
            PriWrapper ThisPlayer = PlayerPRIs.Get(i);
            if(!ThisPlayer.IsNull())
            {
                Players.push_back(ThisPlayer);
            }
        }
    }

    //Construct player list string
    std::string OutputList = std::string(DEFAULT_NONE) + '@' + DEFAULT_NONE;
    for(auto& Player : Players)
    {
        std::string PlayerName = Player.GetPlayerName().IsNull() ? "Invalid Name" : Player.GetPlayerName().ToString();
        OutputList += '&' + PlayerName + '@' + std::to_string(Player.GetUniqueIdWrapper().GetUID());                    // CHANGE THIS TO WORK WITH EPIC PLAYERS LATER
    }

    return OutputList;
}
