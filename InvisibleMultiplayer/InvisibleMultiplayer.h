#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "MacrosStructsEnums.h"

class InvisibleMultiplayer : public BakkesMod::Plugin::BakkesModPlugin
{
private:
    std::shared_ptr<std::string> SelectedPlayer;

public:
	void onLoad() override;
	void onUnload() override;

    //General
    ServerWrapper GetCurrentGameState();

    //Host
    void OnPlayerSelectionChanged();

    //Client
    void OnCarRespawn();
    void CompareMatchGUID();
    void OnGUIDChanged(const std::string& NewGUID);
    std::string GetChosenPlayerFromGUID(const std::string& NewGUID);
    void SetHiddenStatus(CarWrapper Car, bool bHide);
    void UnhideLocalCar();

    //UI
    void GenerateSettingsFile();
    std::string GetPlayerList();
};
