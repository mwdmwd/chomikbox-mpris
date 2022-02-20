#include <stdio.h>

#include "app.h"
#include "offsets.h"

struct QString;
struct QVariant;

// Detour targets
DWORD(__thiscall *SetSongTimeLabel)
(void *, LONGLONG) = (DWORD __thiscall(*)(void *thiz, LONGLONG millis))OFS_SET_SONG_TIME_LABEL;
void(__thiscall *PlayerWindowStateChanged)(void *, AppPlayState) =
    (void __thiscall (*)(void *, AppPlayState))OFS_PLAYER_WINDOW_STATE_CHANGED;
void(__thiscall *TrackChanged)(void *thiz,
                               void *qUrl) = (void(__thiscall *)(void *, void *))OFS_TRACK_CHANGED;
void(__thiscall *QueryDuration)(void *thiz) = (void(__thiscall *)(void *))OFS_QUERY_DURATION;
void(__thiscall *Application_ctor)(void *thiz, int *argc, char **argv, uint8_t unk) =
    (void __thiscall (*)(void *, int *, char **, uint8_t))OFS_APPLICATION_CTOR;
void(__thiscall *BaseApplication_close)(void *thiz, uint32_t force, uint32_t exitCode) =
    (void __thiscall (*)(void *, uint32_t, uint32_t))OFS_BASEAPPLICATION_CLOSE;
void *(__thiscall *PlayerWindow_ctor)(void *thiz, uint32_t unk, uint32_t unk2) =
    (void *__thiscall (*)(void *, uint32_t, uint32_t))OFS_PLAYER_WINDOW_CTOR;
void(__thiscall *SetVolumePercentLabel)(void *thiz, int32_t volume) =
    (void(__thiscall *)(void *, int32_t))OFS_SET_VOLUME_PERCENT_LABEL;
void(__thiscall *CheckboxStateChanged)(void *thiz, bool checked) =
    (void(__thiscall *)(void *, bool))OFS_CHECKBOX_STATE_CHANGED;

// Imports to be resolved with GetProcAddress
static void(__thiscall *QAbstractSlider_SetValue)(void *thiz, int value);
static int(__thiscall *QAbstractSlider_value)(void *thiz);
static void(__thiscall *QUrl_fileName)(void *thiz, QString *outStr);
static void(__cdecl *QString_fromLatin1)(QString *outStr, char const *str, int len);
static void(__thiscall *QString_dtor)(QString *thiz);
static int (*gst_element_query_duration)(void *element, int *format, int64_t *duration);
static int (*gst_element_query_position)(void *element, int *format, int64_t *cur);
static void(__thiscall *QVariant_bool)(QVariant *thiz, bool value);
static void(__thiscall *QVariant_dtor)(QVariant *thiz);
static void(__thiscall *QAbstractButton_setChecked)(void *thiz, bool checked);

// Other "imports"
static auto StartPlaying = (void(__thiscall *)(void *thiz))OFS_START_PLAYING;
static auto PauseInternal = (void __thiscall (*)(void *thiz))OFS_PAUSE_INTERNAL;
static auto StopInternal = (void __thiscall (*)(void *thiz))OFS_STOP;
static auto PlayPauseButtonClicked = (void __thiscall (*)(void *thiz))OFS_PLAY_PAUSE_BUTTON_CLICKED;
static auto PlayInternal1 = (void __thiscall (*)(void *thiz))OFS_PLAY_1;
static auto PlayInternal2 = (void __thiscall (*)(void *thiz, int))OFS_PLAY_2;
static auto PlayInternal3 = (void __thiscall (*)(void *thiz, void *that))OFS_PLAY_3;
static auto NextPrev = (void __thiscall (*)(void *thiz, int previous))OFS_NEXT_PREV;
static auto SetPositionInternal = (void __thiscall (*)(void *thiz, int position))OFS_SET_POSITION;
static auto WriteConfigValue = (void __thiscall (*)(void *thiz, QString *section, QString *name,
                                                    QVariant *value))OFS_WRITE_CONFIG_VALUE;
static auto GetConfigBoolInternal = (bool __thiscall (*)(
    void *thiz, char const *section, char const *name, bool default_))OFS_GET_CONFIG_BOOL;
static auto SeedShuffle = (void __thiscall (*)(void *thiz, bool shuffle))OFS_SEED_SHUFFLE;

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

struct QVariant
{
	void *data[4]; // should be enough for everybody

	~QVariant()
	{
		QVariant_dtor(this);
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
	QAbstractSlider_value = (int(__thiscall *)(void *))REQUIRE(
	    GetProcAddress(qtGui4, "?value@QAbstractSlider@@QBEHXZ"));
	QAbstractButton_setChecked = (void(__thiscall *)(void *, bool))REQUIRE(
	    GetProcAddress(qtGui4, "?setChecked@QAbstractButton@@QAEX_N@Z"));

	HMODULE qtCore4 = REQUIRE(GetModuleHandle("qtcore4.dll"));
	QUrl_fileName = (void(__thiscall *)(void *, QString *))REQUIRE(
	    GetProcAddress(qtCore4, "?fileName@QUrl@@QBE?AVQString@@XZ"));
	QString_fromLatin1 = (void(__cdecl *)(QString *, char const *, int))REQUIRE(
	    GetProcAddress(qtCore4, "?fromLatin1@QString@@SA?AV1@PBDH@Z"));
	QString_dtor =
	    (void(__thiscall *)(QString *))REQUIRE(GetProcAddress(qtCore4, "??1QString@@QAE@XZ"));
	QVariant_bool = (void(__thiscall *)(QVariant *, bool))REQUIRE(
	    GetProcAddress(qtCore4, "??0QVariant@@QAE@_N@Z"));
	QVariant_dtor =
	    (void(__thiscall *)(QVariant *))REQUIRE(GetProcAddress(qtCore4, "??1QVariant@@QAE@XZ"));

	HMODULE gstreamer = REQUIRE(GetModuleHandle("libgstreamer-0.10.dll"));
	gst_element_query_duration = (int (*)(void *, int *, int64_t *))REQUIRE(
	    GetProcAddress(gstreamer, "gst_element_query_duration"));
	gst_element_query_position = (int (*)(void *, int *, int64_t *))REQUIRE(
	    GetProcAddress(gstreamer, "gst_element_query_position"));

#undef REQUIRE
	return 0;
}

static void *GetQGStreamer(void *playerWindow)
{
	return ((void **)playerWindow)[87];
}

static void *GetQGStreamerPrivate(void *playerWindow)
{
	return ((void **)GetQGStreamer(playerWindow))[2];
}

AppPlayState GetPlayState(void *player)
{
	AppPlayState *pp = (AppPlayState *)GetQGStreamerPrivate(player);
	return pp[10];
}

static void *GetGstElement(void *player)
{
	return ((void **)GetQGStreamerPrivate(player))[2];
}

void Play(void *player)
{
	void **qObject = (void **)player;

	AppPlayState state = GetPlayState(player);
	printf("play: play state %d\n", (int)state);
	switch(state)
	{
		case AppPlayState::Stopped:
			StartPlaying(player);
			break;
		case AppPlayState::Paused:
		case AppPlayState::Buffering:
		case AppPlayState::UnsupportedFormat:
			PlayInternal1(GetQGStreamer(player));
			PlayInternal2(&qObject[90], 1);
			PlayInternal3(&qObject[88], qObject[90]);
			break;
		default:
			printf("Don't know how to play from state %d\n", (int)state);
			break;
	}
}

void Pause(void *player)
{
	printf("pause: play state %d\n", (int)GetPlayState(player));
	if(GetPlayState(player) == AppPlayState::Playing)
	{
		PauseInternal(player);
	}
}

void PlayPause(void *player)
{
	PlayPauseButtonClicked(player);
}

void Stop(void *player)
{
	StopInternal(player);
}

void Next(void *player)
{
	NextPrev(player, 0);
}

void Prev(void *player)
{
	NextPrev(player, 1);
}

int GetVolume(void *player)
{
	void **qObject = (void **)player;
	return QAbstractSlider_value(qObject[58]);
}

void SetVolume(void *player, int volume)
{
	void **qObject = (void **)player;
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
		printf("Getting string length failed: 0x%08lx\n", GetLastError());
		return "";
	}

	std::string out(ulen, ' ');
	int ret = WideCharToMultiByte(CP_UTF8, 0, fileName.data->data, fileName.data->size,
	                              (char *)out.data(), ulen, nullptr, nullptr);
	if(!ret)
	{
		printf("String conversion failed: 0x%08lx\n", GetLastError());
		return "";
	}

	return out;
}

int64_t GetDuration(void *player)
{
	void *element = GetGstElement(player);
	int format = 3;
	int64_t duration;
	gst_element_query_duration(element, &format, &duration);
	return duration / 1000; // nanos to micros
}

void SetPosition(void *player, int64_t position)
{
	SetPositionInternal(GetQGStreamerPrivate(player), position / 1000000); // micros to seconds
}

static void *GetConfig()
{
	void **pConfig = (void **)OFS_CONFIG;
	void *config = *pConfig;

	return config ? config : pConfig;
}

static bool GetConfigBool(char const *section, char const *name, bool default_)
{
	return GetConfigBoolInternal(GetConfig(), section, name, default_);
}

static void SetConfigBool(char const *section, char const *name, bool value)
{
	QVariant variant;
	QString sectionStr, nameStr;

	QVariant_bool(&variant, value);
	QString_fromLatin1(&sectionStr, section, -1);
	QString_fromLatin1(&nameStr, name, -1);

	WriteConfigValue(GetConfig(), &sectionStr, &nameStr, &variant);
}

bool GetShuffle()
{
	return GetConfigBool("player", "shuffle", false);
}

void SetShuffle(void *player, bool shuffle)
{
	void **qObject = (void **)player;
	SetConfigBool("player", "shuffle", shuffle);
	SeedShuffle(qObject[88], shuffle);
	QAbstractButton_setChecked(qObject[64], shuffle);
}

bool GetRepeat()
{
	return GetConfigBool("player", "repeat", false);
}

void SetRepeat(void *player, bool repeat)
{
	void **qObject = (void **)player;
	SetConfigBool("player", "repeat", repeat);
	QAbstractButton_setChecked(qObject[63], repeat);
}

int64_t GetPosition(void *player)
{
	int format = 3;
	int64_t position;
	gst_element_query_position(GetGstElement(player), &format, &position);
	return position / 1000; // nanos to micros
}

void CloseApplication(void *application)
{
	BaseApplication_close(application, 0, 0);
}
