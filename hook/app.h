#ifndef HOOK_APP_H
#define HOOK_APP_H

#include <windows.h>

enum class PlayState
{
	Stopped,
	Playing,
	Buffering,
	Paused,
	UnsupportedFormat,
};

PlayState GetPlayState(void *thiz);
void Play(void *thiz);
void Pause(void *thiz);
void PlayPause(void *thiz);

// Detour targets
extern DWORD(__thiscall *SetSongTimeLabel)(void *thiz, LONGLONG millis);
extern void(__thiscall *PlayerWindowStateChanged)(void *thiz, PlayState state);

#endif