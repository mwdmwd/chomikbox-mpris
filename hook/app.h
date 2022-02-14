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

AppPlayState GetPlayState(void *player);
void Play(void *player);
void Pause(void *player);
void PlayPause(void *player);
void Next(void *player);
void Prev(void *player);
void SetVolume(void *player, int volume);
int GetVolume(void *player);
std::string GetFileName(void *qUrl);
int64_t GetDuration(void *player);
void Seek(void *player, int32_t position);
void CloseApplication(void *application);

// Detour targets
extern DWORD(__thiscall *SetSongTimeLabel)(void *thiz, LONGLONG millis);
extern void(__thiscall *PlayerWindowStateChanged)(void *thiz, AppPlayState state);
extern void(__thiscall *TrackChanged)(void *thiz, void *qUrl);
extern void(__thiscall *QueryDuration)(void *thiz);
extern void(__thiscall *Application_ctor)(void *thiz, int *argc, char **argv, uint8_t unk);
extern void *(__thiscall *PlayerWindow_ctor)(void *thiz, uint32_t unk, uint32_t unk2);

#endif