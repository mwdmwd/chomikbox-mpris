#ifndef HOOK_APP_H
#define HOOK_APP_H

#include <string>
#include <windows.h>

enum class PlayState
{
	Stopped,
	Playing,
	Buffering,
	Paused,
	UnsupportedFormat,
};

int ResolveDynamicImports(void);

PlayState GetPlayState(void *thiz);
void Play(void *thiz);
void Pause(void *thiz);
void PlayPause(void *thiz);
void Next(void *thiz);
void Prev(void *thiz);
void SetVolume(void *thiz, int volume);
std::string GetFileName(void *qUrl);

// Detour targets
extern DWORD(__thiscall *SetSongTimeLabel)(void *thiz, LONGLONG millis);
extern void(__thiscall *PlayerWindowStateChanged)(void *thiz, PlayState state);
extern void(__thiscall *TrackFinished)(void *thiz);
extern void(__thiscall *TrackChanged)(void *thiz, void *qUrl);

#endif