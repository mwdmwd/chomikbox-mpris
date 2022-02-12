#ifndef HOOK_OFFSETS_H
#define HOOK_OFFSETS_H

#define EXE_BASE 0x00400000
#define OFS_START_PLAYING ((void *)(EXE_BASE + 0x000e0220))
#define OFS_PAUSE_INTERNAL ((void *)(EXE_BASE + 0x000e0d40))
#define OFS_PLAY_1 ((void *)(EXE_BASE + 0x001f0050))
#define OFS_PLAY_2 ((void *)(EXE_BASE + 0x000ef7a0))
#define OFS_PLAY_3 ((void *)(EXE_BASE + 0x000eb510))
#define OFS_SET_SONG_TIME_LABEL ((void *)(EXE_BASE + 0x000dfbf0))
#define OFS_PLAYER_WINDOW_STATE_CHANGED ((void *)(EXE_BASE + 0x000e22d0))
#define OFS_PLAY_PAUSE_BUTTON_CLICKED ((void *)(EXE_BASE + 0x000e0180))
#define OFS_NEXT_PREV ((void *)(EXE_BASE + 0x000e1080))
#define OFS_TRACK_FINISHED ((void *)(EXE_BASE + 0x000e1060))
#define OFS_TRACK_CHANGED ((void *)(EXE_BASE + 0x001f0310))

#endif