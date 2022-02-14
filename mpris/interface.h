#ifndef MPRIS_SERVER_INTERFACE_H
#define MPRIS_SERVER_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
	IM_PLAY,
	IM_PAUSE,
	IM_PLAYPAUSE,
	IM_SEEK,
	IM_QUIT,
	IM_NEXT,
	IM_PREV,
	IM_SET_VOLUME,
	IM_IMPORT_COUNT, /* must be last */
} ServerImport;

typedef enum
{
	PS_STOPPED,
	PS_PLAYING,
	PS_PAUSED,
} PlayState;

typedef struct
{
	void (*title_changed)(char const *title);
	void (*duration_changed)(int64_t duration);
	void (*position_changed)(int64_t position);
	void (*state_changed)(PlayState state);
} ServerCallbacks;

typedef int (*mpris_server_import)(uint64_t arg);

void register_import(ServerImport index, mpris_server_import function);
void call_import(ServerImport index, uint64_t arg);

ServerCallbacks *get_callbacks(void);
void set_callbacks(ServerCallbacks *cbs);

#ifdef __cplusplus
}
#endif

#endif