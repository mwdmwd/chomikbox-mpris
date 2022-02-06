//////////////////////////////////////////////////////////////////////////////
//
//  Detours Test Program (simple.cpp of simple.dll)
//
//  Microsoft Research Detours Package
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  This DLL will detour the Windows SleepEx API so that TimedSleep function
//  gets called instead.  TimedSleepEx records the before and after times, and
//  calls the real SleepEx API through the TrueSleepEx function pointer.
//
#include <stdio.h>
#include <windows.h>

#include "detours.h"

#include "mpris/interface.h"

static void *app;

static void(__fastcall *StartPlaying)(void *qObject) = (void __fastcall (*)(void *))0x004e0220;
static void(__fastcall *PauseInternal)(void *qObject) = (void __fastcall (*)(void *))0x004e0d40;

enum class PlayState
{
	Stopped,
	Playing,
	Buffering,
	Paused,
	Stopped2,
};

static PlayState GetPlayState(void *thiz)
{
	PlayState ***ppp = (PlayState ***)thiz;
	return ppp[87][2][10];
}

static void Play(void *thiz)
{
	void **qObject = (void **)thiz;
	static void(__thiscall * Play1)(void *) = (void __thiscall (*)(void *))0x005f0050;
	static void(__thiscall * Play2)(void *, int) = (void __thiscall (*)(void *, int))0x004ef7a0;
	static void(__thiscall * Play3)(void *) = (void __thiscall (*)(void *))0x004eb510;

	PlayState state = GetPlayState(thiz);
	printf("play: play state %d\n", state);
	switch(state)
	{
		case PlayState::Stopped:
			StartPlaying(thiz);
			break;
		case PlayState::Paused:
		case PlayState::Buffering:
		case PlayState::Stopped2:
			Play1(qObject[87]);
			Play2(&qObject[90], 1);
			// Play3(&qObject[90]);
			break;
		default:
			printf("Don't know how to play from state %d\n", state);
			break;
	}
}

static void Pause(void *thiz)
{
	printf("pause: play state %d\n", GetPlayState(thiz));
	if(GetPlayState(thiz) == PlayState::Playing)
	{
		PauseInternal(thiz);
	}
}

static DWORD(__thiscall *SetSongTimeLabel)(void *thiz, LONGLONG param) =
    (DWORD __thiscall(*)(void *, LONGLONG))0x004dfbf0;

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

static void(__fastcall *PlayPauseButtonClicked)(void *thiz) =
    (void __fastcall (*)(void *))0x004e0180;

static void PlayPause(void *app)
{
	PlayPauseButtonClicked(app);
}

struct Detour
{
	void **original;
	void *detour;
} detours[] = {
    {(void **)&SetSongTimeLabel, (void *)HK_SetSongTimeLabel},
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

	__builtin_unreachable();
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

//
///////////////////////////////////////////////////////////////// End of File.
