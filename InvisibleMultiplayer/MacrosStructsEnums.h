#pragma once

#define SHOULD_LOG 0
#if SHOULD_LOG == 1
    #define LOGC(x) cvarManager->log(x)
#else
    #define LOGC(x)
#endif

//Cvars and default value
#define CVAR_SELECTED_PLAYER "InvisibleMulti_SelectedPlayer"
#define NOTIFIER_REFRESH_LIST "InvisibleMulti_RefreshList"
#define DEFAULT_NONE "-- NONE --"

#define PLUGIN_DESCRIPTION "Invisible Multiplayer plugin"
#define GUID_INVISIBLE "[InvisiMulti]"
