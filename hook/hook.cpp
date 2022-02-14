#include <stdio.h>
#include <windows.h>

#include "detours.h"
#include "mpris/interface.h"

#include "app.h"

static void *app;
static ServerCallbacks *callbacks;
static ServerImports imports;

DWORD __thiscall HK_SetSongTimeLabel(void *thiz, LONGLONG millis)
{
	if(!app)
	{
		printf("FIXME?: saving app from time label hook\n");
		app = thiz;
	}

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

static void __thiscall HK_TrackFinished(void *thiz)
{
	printf("track finished\n");
	return TrackFinished(thiz);
}

static void __thiscall HK_TrackChanged(void *thiz, void *qUrl)
{
	std::string name = GetFileName(qUrl);
	callbacks->title_changed(name.c_str());
	return TrackChanged(thiz, qUrl);
}

static void __thiscall HK_QueryDuration(void *thiz)
{
	callbacks->duration_changed(GetDuration(thiz) / 1000); // nanos to micros
	QueryDuration(thiz);
}

struct Detour
{
	void **original;
	void *detour;
} detours[] = {
#define DETOUR(name)                                                                               \
	{                                                                                              \
		(void **)&name, (void *)HK_##name                                                          \
	}
    DETOUR(SetSongTimeLabel), DETOUR(PlayerWindowStateChanged),
    DETOUR(TrackFinished),    DETOUR(TrackChanged),
    DETOUR(QueryDuration),
#undef DETOUR
};

void EnsureAppHandleAndCallWithApp(void (*function)(void *))
{
	if(!app)
	{
		printf("WARNING: no app handle\n");
		return;
	}

	function(app);
}

DWORD WINAPI MprisServerThread(LPVOID lpParameter)
{
	void (*mpris_server_run)(void) = (void (*)(void))lpParameter;
	mpris_server_run();
	return 0;
}

DWORD WINAPI QuitThreadProc(LPVOID lpParameter)
{
	Sleep(500);                               // Allow some time for the MPRIS response
	TerminateProcess(GetCurrentProcess(), 0); // FIXME: call the player's quit method instead
	return 0UL;
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

		imports.play = []() { EnsureAppHandleAndCallWithApp(Play); };
		imports.pause = []() { EnsureAppHandleAndCallWithApp(Pause); };
		imports.play_pause = []() { EnsureAppHandleAndCallWithApp(PlayPause); };
		imports.quit = []()
		{
			// FIXME use proper quit
			CreateThread(nullptr, 0, QuitThreadProc, nullptr, 0, nullptr);
		};
		imports.next = []() { Next(app); };
		imports.prev = []() { Prev(app); };
		imports.set_volume = [](int32_t volume) { SetVolume(app, volume); };

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
