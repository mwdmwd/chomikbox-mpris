#include <stdio.h>

#include "app.h"
#include "offsets.h"

struct QString;

// Detour targets
DWORD(__thiscall *SetSongTimeLabel)
(void *, LONGLONG) = (DWORD __thiscall(*)(void *thiz, LONGLONG millis))OFS_SET_SONG_TIME_LABEL;
void(__thiscall *PlayerWindowStateChanged)(void *, AppPlayState) =
    (void __thiscall (*)(void *, AppPlayState))OFS_PLAYER_WINDOW_STATE_CHANGED;
void(__thiscall *TrackChanged)(void *thiz,
                               void *qUrl) = (void(__thiscall *)(void *, void *))OFS_TRACK_CHANGED;
void(__thiscall *QueryDuration)(void *thiz) = (void(__thiscall *)(void *))OFS_QUERY_DURATION;

// Imports to be resolved with GetProcAddress
static void(__thiscall *QAbstractSlider_SetValue)(void *thiz, int value);
static void(__thiscall *QUrl_fileName)(void *thiz, QString *outStr);
static void(__thiscall *QString_dtor)(QString *thiz);
static int (*gst_element_query_duration)(void *element, int *format, int64_t *duration);

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

	HMODULE gstreamer = REQUIRE(GetModuleHandle("libgstreamer-0.10.dll"));
	gst_element_query_duration = (int (*)(void *, int *, int64_t *))REQUIRE(
	    GetProcAddress(gstreamer, "gst_element_query_duration"));

#undef REQUIRE
	return 0;
}

void *GetQGStreamer(void *playerWindow)
{
	return ((void **)playerWindow)[87];
}

void *GetQGStreamerPrivate(void *playerWindow)
{
	return ((void **)GetQGStreamer(playerWindow))[2];
}

AppPlayState GetPlayState(void *thiz)
{
	AppPlayState *pp = (AppPlayState *)GetQGStreamerPrivate(thiz);
	return pp[10];
}

void *GetGstElement(void *qGStreamerPrivate)
{
	return ((void **)qGStreamerPrivate)[2];
}

void Play(void *thiz)
{
	void **qObject = (void **)thiz;

	AppPlayState state = GetPlayState(thiz);
	printf("play: play state %d\n", state);
	switch(state)
	{
		case AppPlayState::Stopped:
			StartPlaying(thiz);
			break;
		case AppPlayState::Paused:
		case AppPlayState::Buffering:
		case AppPlayState::UnsupportedFormat:
			PlayInternal1(GetQGStreamer(thiz));
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
	if(GetPlayState(thiz) == AppPlayState::Playing)
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

int64_t GetDuration(void *qGStreamerPrivate)
{
	void *element = GetGstElement(qGStreamerPrivate);
	int format = 3;
	int64_t duration;
	gst_element_query_duration(element, &format, &duration);
	return duration;
}
