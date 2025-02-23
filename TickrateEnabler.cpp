//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//===========================================================================//

#include <stdio.h>
#include <sourcehook/sourcehook_impl.h>
#include <eiface.h>
#include <tier0/icommandline.h>

class TickrateEnabler: public IServerPluginCallbacks
{
public:
	TickrateEnabler() {}
	~TickrateEnabler() {}

	virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
	virtual void Unload(void);
	virtual void Pause(void) {}
	virtual void UnPause(void) {}
	virtual const char *GetPluginDescription(void)
	{
		return "Tickrate Enabler 0.5, updated, original by Didrole";
	}
	virtual void LevelInit(char const *pMapName) {}
	virtual void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) {}
	virtual void GameFrame(bool simulating) {}
	virtual void LevelShutdown(void) {}
	virtual void ClientActive(edict_t *pEntity) {}
	virtual void ClientDisconnect(edict_t *pEntity) {}
	virtual void ClientPutInServer(edict_t *pEntity, char const *playername) {}
	virtual void SetCommandClient(int index) {}
	virtual void ClientSettingsChanged(edict_t *pEdict) {}
	virtual PLUGIN_RESULT ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen)
	{
		return PLUGIN_CONTINUE;
	}
	virtual PLUGIN_RESULT ClientCommand(edict_t *pEntity, const CCommand &args)
	{
		return PLUGIN_CONTINUE;
	}
	virtual PLUGIN_RESULT NetworkIDValidated(const char *pszUserName, const char *pszNetworkID)
	{
		return PLUGIN_CONTINUE;
	}
	virtual void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue) {}
	virtual void OnEdictAllocated(edict_t *edict) {}
	virtual void OnEdictFreed(const edict_t *edict) {}
};

IServerGameDLL *server = nullptr;
SourceHook::Impl::CSourceHookImpl g_SourceHook;
SourceHook::ISourceHook *g_SHPtr = &g_SourceHook;
int g_PLID = 0;

TickrateEnabler g_TickrateEnabler;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(TickrateEnabler, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_TickrateEnabler);

SH_DECL_HOOK0(IServerGameDLL, GetTickInterval, const, 0, float);

float GetTickInterval()
{
#ifndef TICK_INTERVAL
	float tickinterval = DEFAULT_TICK_INTERVAL;

	if (CommandLine()->CheckParm("-tickrate"))
	{
		float tickrate = CommandLine()->ParmValue("-tickrate", 0.0f);
		if (tickrate > 10)
			tickinterval = 1.0f / tickrate;
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, tickinterval);
#else
	RETURN_META_VALUE(MRES_SUPERCEDE, 1.0f / TICK_INTERVAL);
#endif
}

bool TickrateEnabler::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	char version[64];
	for(int i = 10; i <= 25; ++i)
	{
		snprintf(version, sizeof(version), "ServerGameDLL%03d", i);
		if((server = reinterpret_cast<IServerGameDLL*>(gameServerFactory(version, nullptr))) != nullptr) break;
	}

	if (!server)
	{
		Warning("Failed to get a pointer on ServerGameDLL.\n");
		return false;
	}

	SH_ADD_HOOK(IServerGameDLL, GetTickInterval, server, SH_STATIC(GetTickInterval), false);

	return true;
}

void TickrateEnabler::Unload(void)
{
	SH_REMOVE_HOOK(IServerGameDLL, GetTickInterval, server, SH_STATIC(GetTickInterval), false);
}