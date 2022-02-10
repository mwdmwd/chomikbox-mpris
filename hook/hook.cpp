#include <stdio.h>
#include <windows.h>

#include "detours.h"
#include "mpris/interface.h"

#include "app.h"

static void *app;

DWORD __thiscall HK_SetSongTimeLabel(void *thiz, LONGLONG millis)
{
	if(!app)
	{
		printf("FIXME?: saving app from time label hook\n");
		app = thiz;
	}

	LONGLONG seconds = millis / 1000;
	printf("%08x %016llx %02lld:%02lld\n", thiz, millis, seconds / 60, seconds % 60);

	return SetSongTimeLabel(thiz, millis);
}

static void __thiscall HK_PlayerWindowStateChanged(void *thiz, PlayState state)
{
	printf("player window state changed to %d this=%08x\n", state, thiz);
	return PlayerWindowStateChanged(thiz, state);
}

static void __thiscall HK_TrackFinished(void *thiz)
{
	printf("track finished\n");
	return TrackFinished(thiz);
}

struct Detour
{
	void **original;
	void *detour;
} detours[] = {
#define DETOUR(name) {(void **)&name, (void *)HK_##name}
    DETOUR(SetSongTimeLabel),
    DETOUR(PlayerWindowStateChanged),
    DETOUR(TrackFinished),
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

		void (*register_import_fixme)(ServerImport, int (*)(uint64_t)) =
		    (void (*)(ServerImport, int (*)(uint64_t)))GetProcAddress(mprisServer,
		                                                              "register_import");

		register_import_fixme(IM_PLAY,
		                      [](uint64_t)
		                      {
			                      EnsureAppHandleAndCallWithApp(Play);
			                      return 0;
		                      });

		register_import_fixme(IM_PAUSE,
		                      [](uint64_t)
		                      {
			                      EnsureAppHandleAndCallWithApp(Pause);
			                      return 0;
		                      });

		register_import_fixme(IM_PLAYPAUSE,
		                      [](uint64_t)
		                      {
			                      EnsureAppHandleAndCallWithApp(PlayPause);
			                      return 0;
		                      });

		register_import_fixme(IM_QUIT,
		                      [](uint64_t)
		                      {
			                      // FIXME use proper quit
			                      CreateThread(nullptr, 0, QuitThreadProc, nullptr, 0, nullptr);
			                      return 0;
		                      });

		register_import_fixme(IM_NEXT,
		                      [](uint64_t)
		                      {
			                      Next(app);
			                      return 0;
		                      });

		register_import_fixme(IM_PREV,
		                      [](uint64_t)
		                      {
			                      Prev(app);
			                      return 0;
		                      });

		register_import_fixme(IM_SET_VOLUME,
		                      [](uint64_t volume)
		                      {
			                      SetVolume(app, volume);
			                      return 0;
		                      });

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
