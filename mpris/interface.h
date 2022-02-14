#ifndef MPRIS_SERVER_INTERFACE_H
#define MPRIS_SERVER_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
	void (*play)(void);
	void (*pause)(void);
	void (*play_pause)(void);
	void (*seek)(int64_t position);
	void (*quit)(void);
	void (*next)(void);
	void (*prev)(void);
	void (*set_volume)(int32_t volume);
} ServerImports;

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

void set_imports(ServerImports *imps);
ServerCallbacks *get_callbacks(void);

#ifdef __cplusplus
}
#endif

#endif