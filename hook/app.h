#ifndef HOOK_APP_H
#define HOOK_APP_H

#include <stdint.h>
#include <string>
#include <windows.h>

enum class AppPlayState
{
	Stopped,
	Playing,
	Buffering,
	Paused,
	UnsupportedFormat,
};

int ResolveDynamicImports(void);

AppPlayState GetPlayState(void *thiz);
void Play(void *thiz);
void Pause(void *thiz);
void PlayPause(void *thiz);
void Next(void *thiz);
void Prev(void *thiz);
void SetVolume(void *thiz, int volume);
std::string GetFileName(void *qUrl);
int64_t GetDuration(void *thiz);

// Detour targets
extern DWORD(__thiscall *SetSongTimeLabel)(void *thiz, LONGLONG millis);
extern void(__thiscall *PlayerWindowStateChanged)(void *thiz, AppPlayState state);
extern void(__thiscall *TrackFinished)(void *thiz);
extern void(__thiscall *TrackChanged)(void *thiz, void *qUrl);
extern void(__thiscall *QueryDuration)(void *thiz);

#endif