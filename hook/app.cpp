#include <stdio.h>

#include "app.h"
#include "offsets.h"

struct QString;

// Detour targets
DWORD(__thiscall *SetSongTimeLabel)
(void *, LONGLONG) = (DWORD __thiscall(*)(void *thiz, LONGLONG millis))OFS_SET_SONG_TIME_LABEL;
void(__thiscall *PlayerWindowStateChanged)(void *, PlayState) =
    (void __thiscall (*)(void *, PlayState))OFS_PLAYER_WINDOW_STATE_CHANGED;
void(__thiscall *TrackFinished)(void *thiz) = (void(__thiscall *)(void *))OFS_TRACK_FINISHED;
void(__thiscall *TrackChanged)(void *thiz,
                               void *qUrl) = (void(__thiscall *)(void *, void *))OFS_TRACK_CHANGED;

// Imports to be resolved with GetProcAddress
static void(__thiscall *QAbstractSlider_SetValue)(void *thiz, int value);
static void(__thiscall *QUrl_fileName)(void *thiz, QString *outStr);
static void(__thiscall *QString_dtor)(QString *thiz);

// Other "imports"
static auto StartPlaying = (void(__thiscall *)(void *thiz))OFS_START_PLAYING;
static auto PauseInternal = (void __thiscall (*)(void *thiz))OFS_PAUSE_INTERNAL;
static auto PlayPauseButtonClicked = (void __thiscall (*)(void *thiz))OFS_PLAY_PAUSE_BUTTON_CLICKED;
static auto PlayInternal1 = (void __thiscall (*)(void *thiz))OFS_PLAY_1;
static auto PlayInternal2 = (void __thiscall (*)(void *thiz, int))OFS_PLAY_2;
static auto PlayInternal3 = (void __thiscall (*)(void *thiz))OFS_PLAY_3;
static auto NextPrev = (void __thiscall (*)(void *thiz, int previous))OFS_NEXT_PREV;

struct QString
{
	struct Data
	{
		int refCount;
		int alloc, size;
		wchar_t *data;
	} * data;

	~QString()
	{
		QString_dtor(this);
	}
};

int ResolveDynamicImports(void)
{
#define REQUIRE(x)                                                                                 \
	({                                                                                             \
		auto x_ = (x);                                                                             \
		if(!x_)                                                                                    \
			return 1;                                                                              \
		x_;                                                                                        \
	})

	HMODULE qtGui4 = REQUIRE(GetModuleHandle("qtgui4.dll"));
	QAbstractSlider_SetValue = (void(__thiscall *)(void *, int))REQUIRE(
	    GetProcAddress(qtGui4, "?setValue@QAbstractSlider@@QAEXH@Z"));

	HMODULE qtCore4 = REQUIRE(GetModuleHandle("qtcore4.dll"));
	QUrl_fileName = (void(__thiscall *)(void *, QString *))REQUIRE(
	    GetProcAddress(qtCore4, "?fileName@QUrl@@QBE?AVQString@@XZ"));
	QString_dtor =
	    (void(__thiscall *)(QString *))REQUIRE(GetProcAddress(qtCore4, "??1QString@@QAE@XZ"));

#undef REQUIRE
	return 0;
}

PlayState GetPlayState(void *thiz)
{
	PlayState ***ppp = (PlayState ***)thiz;
	return ppp[87][2][10];
}

void Play(void *thiz)
{
	void **qObject = (void **)thiz;

	PlayState state = GetPlayState(thiz);
	printf("play: play state %d\n", state);
	switch(state)
	{
		case PlayState::Stopped:
			StartPlaying(thiz);
			break;
		case PlayState::Paused:
		case PlayState::Buffering:
		case PlayState::UnsupportedFormat:
			PlayInternal1(qObject[87]);
			PlayInternal2(&qObject[90], 1);
			// PlayInternal3(&qObject[90]);
			break;
		default:
			printf("Don't know how to play from state %d\n", state);
			break;
	}
}

void Pause(void *thiz)
{
	printf("pause: play state %d\n", GetPlayState(thiz));
	if(GetPlayState(thiz) == PlayState::Playing)
	{
		PauseInternal(thiz);
	}
}

void PlayPause(void *thiz)
{
	PlayPauseButtonClicked(thiz);
}

void Next(void *thiz)
{
	NextPrev(thiz, 0);
}

void Prev(void *thiz)
{
	NextPrev(thiz, 1);
}

void SetVolume(void *thiz, int volume)
{
	void **qObject = (void **)thiz;
	QAbstractSlider_SetValue(qObject[58], volume);
}

std::string GetFileName(void *qUrl)
{
	QString fileName;
	QUrl_fileName(qUrl, &fileName);

	int ulen = WideCharToMultiByte(CP_UTF8, 0, fileName.data->data, fileName.data->size, nullptr, 0,
	                               nullptr, nullptr);

	if(!ulen)
	{
		printf("Getting string length failed: 0x%08x\n", GetLastError());
		return "";
	}

	std::string out(ulen, ' ');
	int ret = WideCharToMultiByte(CP_UTF8, 0, fileName.data->data, fileName.data->size,
	                              (char *)out.data(), ulen, nullptr, nullptr);
	if(!ret)
	{
		printf("String conversion failed: 0x%08x\n", GetLastError());
		return "";
	}

	return out;
}