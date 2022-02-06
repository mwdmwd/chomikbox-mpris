#include <gio/gio.h>
#include <inttypes.h>
#include <stdio.h>
#include <threads.h>

#include "interface.h"
#include "mpris.h"

static char const *objectPath = "/org/mpris/MediaPlayer2";

#define METHOD_HANDLER(iface, method)                                                              \
	static gboolean handle_##method(iface *interface, GDBusMethodInvocation *invocation)
#define METHOD_HANDLER_EX(iface, method, ...)                                                      \
	static gboolean handle_##method(iface *interface, GDBusMethodInvocation *invocation,           \
	                                __VA_ARGS__)

METHOD_HANDLER(MediaPlayer2, quit)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	call_import(IM_QUIT, 0);

	media_player2_complete_quit(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, play)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	call_import(IM_PLAY, 0);

	media_player2_player_complete_play(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, pause)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	call_import(IM_PAUSE, 0);

	media_player2_player_complete_pause(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, play_pause)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	call_import(IM_PLAYPAUSE, 0);

	media_player2_player_complete_play_pause(interface, invocation);
	return TRUE;
}

METHOD_HANDLER_EX(MediaPlayer2Player, seek, gint64 offset)
{
	printf("%s %" PRId64 "\n", __PRETTY_FUNCTION__, offset);

	call_import(IM_SEEK, offset);

	media_player2_player_complete_seek(interface, invocation);
	return TRUE;
}

void name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	printf("name acquired, name='%s'\n", name);

	MediaPlayer2 *player = media_player2_skeleton_new();
#define SIGNAL(name) g_signal_connect(player, "handle-" #name, G_CALLBACK(handle_##name), NULL)
	SIGNAL(quit);
#undef SIGNAL
	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(player), connection, objectPath,
	                                 NULL);

	MediaPlayer2Player *playerPlayer = media_player2_player_skeleton_new();
#define SIGNAL(name)                                                                               \
	g_signal_connect(playerPlayer, "handle-" #name, G_CALLBACK(handle_##name), NULL)
	SIGNAL(play);
	SIGNAL(pause);
	SIGNAL(play_pause);
	SIGNAL(seek);
#undef SIGNAL
	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(playerPlayer), connection,
	                                 objectPath, NULL);
}

void mpris_server_run(void)
{
	guint ownerId =
	    g_bus_own_name(G_BUS_TYPE_SESSION, "org.mpris.MediaPlayer2.chomikbox",
	                   G_BUS_NAME_OWNER_FLAGS_NONE, NULL, name_acquired, NULL, NULL, NULL);

	GMainLoop *loop = g_main_loop_new(NULL, FALSE);

	printf("hi\n");
	g_main_loop_run(loop);
	printf("bye\n");

	g_bus_unown_name(ownerId);
}
