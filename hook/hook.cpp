#include <stdio.h>
#include <windows.h>

#include "detours.h"
#include "mpris/interface.h"

#include "app.h"

static void *app, *playerWindow;
static ServerCallbacks *callbacks;
static ServerImports imports;

DWORD __thiscall HK_SetSongTimeLabel(void *thiz, LONGLONG millis)
{
	callbacks->position_changed(millis * 1000); // millis to micros
	return SetSongTimeLabel(thiz, millis);
}

static void __thiscall HK_PlayerWindowStateChanged(void *thiz, AppPlayState state)
{
	callbacks->state_changed(state == AppPlayState::Playing  ? PS_PLAYING
	                         : state == AppPlayState::Paused ? PS_PAUSED
	                                                         : PS_STOPPED);
	return PlayerWindowStateChanged(thiz, state);
}

static void __thiscall HK_TrackChanged(void *thiz, void *qUrl)
{
	std::string name = GetFileName(qUrl);
	callbacks->title_changed(name.c_str());
	return TrackChanged(thiz, qUrl);
}

static void __thiscall HK_QueryDuration(void *thiz)
{
	callbacks->duration_changed(GetDuration(playerWindow) / 1000); // nanos to micros
	QueryDuration(thiz);
}

static void __thiscall HK_Application_ctor(void *thiz, int *argc, char **argv, uint8_t unk)
{
	app = thiz;
	Application_ctor(thiz, argc, argv, unk);
}

static void *__thiscall HK_PlayerWindow_ctor(void *thiz, uint32_t unk, uint32_t unk2)
{
	playerWindow = thiz;

	void *ret = PlayerWindow_ctor(thiz, unk, unk2);
	callbacks->volume_changed(GetVolume(playerWindow));
	return ret;
}

static void __thiscall HK_SetVolumePercentLabel(void *thiz, int32_t volume)
{
	SetVolumePercentLabel(thiz, volume);
	callbacks->volume_changed(volume);
}

struct
{
	void **original;
	void *detour;
} detours[] = {
#define DETOUR(name)                                                                               \
	{                                                                                              \
		(void **)&name, (void *)HK_##name                                                          \
	}
    DETOUR(SetSongTimeLabel),      DETOUR(PlayerWindowStateChanged), DETOUR(TrackChanged),
    DETOUR(QueryDuration),         DETOUR(Application_ctor),         DETOUR(PlayerWindow_ctor),
    DETOUR(SetVolumePercentLabel),
#undef DETOUR
};

DWORD WINAPI MprisServerThread(LPVOID lpParameter)
{
	void (*mpris_server_run)(void) = (void (*)(void))lpParameter;
	mpris_server_run();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	LONG error;

	if(DetourIsHelperProcess())
	{
		return TRUE;
	}

	if(dwReason == DLL_PROCESS_ATTACH)
	{
		DetourRestoreAfterWith();

		if(ResolveDynamicImports())
		{
			MessageBox(nullptr, "Error resolving dynamic imports", "Error", MB_ICONHAND);
			abort();
		}

		printf("Attaching detours\n");
		fflush(stdout);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		for(auto const &detour : detours)
		{
			DetourAttach(detour.original, detour.detour);
		}

		if(DetourTransactionCommit() != NO_ERROR)
		{
			MessageBox(nullptr, "Error installing detours", "Error", MB_ICONHAND);
			abort();
		}

		HMODULE mprisServer = LoadLibrary("mpris-server.dll");
		if(!mprisServer)
		{
			MessageBox(nullptr, "Error loading mpris-server.dll", "Error", MB_ICONHAND);
			abort();
		}

		void *mpsr = (void *)GetProcAddress(mprisServer, "mpris_server_run");
		if(!mpsr)
		{
			MessageBox(nullptr, "Error getting procedure", "Error", MB_ICONHAND);
			abort();
		}

		ServerCallbacks *(*get_callbacks)(void) =
		    (ServerCallbacks * (*)(void)) GetProcAddress(mprisServer, "get_callbacks");
		callbacks = get_callbacks();

		imports.play = []() { Play(playerWindow); };
		imports.pause = []() { Pause(playerWindow); };
		imports.play_pause = []() { PlayPause(playerWindow); };
		imports.stop = []() { Stop(playerWindow); };
		imports.quit = []() { CloseApplication(app); };
		imports.next = []() { Next(playerWindow); };
		imports.prev = []() { Prev(playerWindow); };
		imports.set_volume = [](int32_t volume) { SetVolume(playerWindow, volume); };
		imports.seek = [](int64_t offset) {

		};
		imports.set_position = [](char const *trackId, int64_t position)
		{
			SetPosition(playerWindow, position / 1000000); // micros to seconds
		};

		void (*set_imports)(ServerImports * imports) =
		    (void (*)(ServerImports *))GetProcAddress(mprisServer, "set_imports");
		set_imports(&imports);

		CreateThread(nullptr, 0, MprisServerThread, mpsr, 0, nullptr);
	}
	else if(dwReason == DLL_PROCESS_DETACH)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		for(auto const &detour : detours)
		{
			DetourDetach(detour.original, detour.detour);
		}

		error = DetourTransactionCommit();

		printf("Removed detours (result=%ld)\n", error);
		fflush(stdout);
	}
	return TRUE;
}
