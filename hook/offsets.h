#ifndef HOOK_OFFSETS_H
#define HOOK_OFFSETS_H

#define EXE_BASE 0x00400000
#define OFS_START_PLAYING ((void *)(EXE_BASE + 0x000e0220))
#define OFS_PAUSE_INTERNAL ((void *)(EXE_BASE + 0x000e0d40))
#define OFS_PLAY_1 ((void *)(EXE_BASE + 0x001f0050))
#define OFS_PLAY_2 ((void *)(EXE_BASE + 0x000ef7a0))
#define OFS_PLAY_3 ((void *)(EXE_BASE + 0x000eb510))
#define OFS_STOP ((void *)(EXE_BASE + 0x000e0e30))
#define OFS_SET_SONG_TIME_LABEL ((void *)(EXE_BASE + 0x000dfbf0))
#define OFS_PLAYER_WINDOW_STATE_CHANGED ((void *)(EXE_BASE + 0x000e22d0))
#define OFS_PLAY_PAUSE_BUTTON_CLICKED ((void *)(EXE_BASE + 0x000e0180))
#define OFS_NEXT_PREV ((void *)(EXE_BASE + 0x000e1080))
#define OFS_TRACK_CHANGED ((void *)(EXE_BASE + 0x001f0310))
#define OFS_QUERY_DURATION ((void *)(EXE_BASE + 0x001f2050))
#define OFS_SET_POSITION ((void *)(EXE_BASE + 0x001f1e00))
#define OFS_APPLICATION_CTOR ((void *)(EXE_BASE + 0x00013a80))
#define OFS_BASEAPPLICATION_CLOSE ((void *)(EXE_BASE + 0x001a9af0))
#define OFS_PLAYER_WINDOW_CTOR ((void *)(EXE_BASE + 0x00d9450))
#define OFS_SET_VOLUME_PERCENT_LABEL ((void *)(EXE_BASE + 0x000e2090))

#endif
