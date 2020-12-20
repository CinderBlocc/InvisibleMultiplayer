#include "InvisibleMultiplayer.h"
#include "bakkesmod\wrappers\includes.h"

BAKKESMOD_PLUGIN(InvisibleMultiplayer, PLUGIN_DESCRIPTION, "1.0", PLUGINTYPE_FREEPLAY)

void InvisibleMultiplayer::onLoad()
{
	using namespace std::placeholders;

    //Hook event to compare current GUID to previous GUID every tick
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", std::bind(&InvisibleMultiplayer::CompareMatchGUID, this));

    //Hook event to rehide car when it has respawned if it is supposed to be invisible
    //There might be a better event, but this one seems to happen late enough that it works fine
	gameWrapper->HookEventPost("Function TAGame.Car_TA.EventTeamPaintChanged", std::bind(&InvisibleMultiplayer::OnCarRespawn, this));

    //Create cvar for player list dropdown menu
    SelectedPlayer = std::make_shared<std::string>("");
	CVarWrapper CvarSelectedPlayer = cvarManager->registerCvar(CVAR_SELECTED_PLAYER, DEFAULT_NONE, "Player chosen to be invisible");
    CvarSelectedPlayer.bindTo(SelectedPlayer);
    CvarSelectedPlayer.addOnValueChanged(std::bind(&InvisibleMultiplayer::OnPlayerSelectionChanged, this));
	
    //Command to refresh the UI and generate a new dropdown list menu
    cvarManager->registerNotifier(NOTIFIER_REFRESH_LIST, [this](std::vector<std::string> params)
    {
        GenerateSettingsFile();
    }, "Regenerates settings file with new player list", PERMISSION_ALL);
    
    //Always generate a new dropdown on first load
    GenerateSettingsFile();
}
void InvisibleMultiplayer::onUnload() {}

ServerWrapper InvisibleMultiplayer::GetCurrentGameState()
{
    if(gameWrapper->IsInReplay())
        return gameWrapper->GetGameEventAsReplay().memory_address;
    else if(gameWrapper->IsInOnlineGame())
        return gameWrapper->GetOnlineGame();
    else
        return gameWrapper->GetGameEventAsServer();
}


// HOST //
void InvisibleMultiplayer::OnPlayerSelectionChanged()
{
    //In LAN matches, the host is considered offline
    if(gameWrapper->IsInOnlineGame()) { return; }

    ServerWrapper server = GetCurrentGameState();
    if(server.IsNull()) { return; }

    //Notify all clients about the new invisible player
    std::string NewGUID = GUID_INVISIBLE + *SelectedPlayer;
    server.SetMatchGUID(NewGUID);

    //Force the match GUID comparison to immediately apply changes locally
    CompareMatchGUID();
}


// CLIENT //
void InvisibleMultiplayer::OnCarRespawn()
{
    ServerWrapper server = GetCurrentGameState();
    if(server.IsNull()) { return; }

    //Use OnGUIDChanged to check if the newly respawned car matches the intended invisibility target
    OnGUIDChanged(server.GetMatchGUID());
}

void InvisibleMultiplayer::CompareMatchGUID()
{
    //Create static variable to persistently store known GUID
    static std::string PreviousGUID;

    CarWrapper localCar = gameWrapper->GetLocalCar();
    if(!localCar.IsNull())
    {
        SetHiddenStatus(localCar, false);
    }

    ServerWrapper server = GetCurrentGameState();
    if(server.IsNull()) { return; }

    std::string CurrentGUID = server.GetMatchGUID();

    //Cache current GUID and attempt to change invisibility target
    if(PreviousGUID != CurrentGUID)
    {
        PreviousGUID = CurrentGUID;
        OnGUIDChanged(CurrentGUID);
    }
}

void InvisibleMultiplayer::OnGUIDChanged(const std::string& NewGUID)
{
    ServerWrapper server = GetCurrentGameState();
    if(server.IsNull()) { return; }

    //Parse the GUID to get the intended invisibility target
    std::string ChosenPlayerStr = GetChosenPlayerFromGUID(NewGUID);

    //Find a player with the matching ID and
    //unhide all players in preparation for hiding a new one
    ArrayWrapper<PriWrapper> Players = server.GetPRIs();
    for(int i = 0; i < Players.Count(); ++i)
    {
        PriWrapper Player = Players.Get(i);
        if(Player.IsNull()) { continue; }

        //Unhide player's car in case they aren't the target
        SetHiddenStatus(Player.GetCar(), false);

        //Check if this player is the intended invisibility target
        //Skip the local player since they are always supposed to be visible
        unsigned long long PlayerID = Player.GetUniqueIdWrapper().GetUID();         //CHANGE THESE TO WORK WITH EPIC PLAYERS LATER
        unsigned long long LocalID = gameWrapper->GetUniqueID().GetUID();

        if(PlayerID != LocalID)
        {
            //Chosen player is not local player. Attempt to hide
            if(std::to_string(PlayerID) == ChosenPlayerStr)
            {
                SetHiddenStatus(Player.GetCar(), true);
            }
        }
        else
        {
            //Chosen player was local player
            //Host hiding local player will replicate, so need to unhide after a tick
            gameWrapper->SetTimeout(std::bind(&InvisibleMultiplayer::UnhideLocalCar, this), 0.001f);
        }
    }
}

std::string InvisibleMultiplayer::GetChosenPlayerFromGUID(const std::string& NewGUID)
{
    //Check if new GUID is intended for this plugin
    static const std::string PluginIdentifier = GUID_INVISIBLE;
    if(NewGUID.substr(0, PluginIdentifier.size()) != PluginIdentifier) { return DEFAULT_NONE; }

    //Erase the plugin identifier
    std::string ChosenPlayerStr = NewGUID;
    ChosenPlayerStr.erase(0, PluginIdentifier.size());

    return ChosenPlayerStr;
}

void InvisibleMultiplayer::SetHiddenStatus(CarWrapper Car, bool bHide)
{
    if(Car.IsNull()) { return; }

    std::string hidestatus = bHide ? "hiding" : "unhiding";

    Car.SetHidden2(bHide);
    Car.SetbHiddenSelf(bHide);
}

void InvisibleMultiplayer::UnhideLocalCar()
{
    SetHiddenStatus(gameWrapper->GetLocalCar(), false);
}
