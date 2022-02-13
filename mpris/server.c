#include <gio/gio.h>
#include <inttypes.h>
#include <stdio.h>
#include <threads.h>

#include "interface.h"
#include "mpris.h"

static char const *objectPath = "/org/mpris/MediaPlayer2";
// Copied from VLC's org/mpris/MediaPlayer2 object
static gchar const *supportedMimeTypes[] = {"audio/mpeg",      "audio/x-mpeg", "audio/mp4",
                                            "application/ogg", "audio/wav",    "audio/x-wav",
                                            "audio/3gpp",      "audio/3gpp2",  "audio/x-matroska"};
static gchar const *supportedUriSchemes[] = {"file"};
static gchar const *playerIdentity = "ChomikBox";

static MediaPlayer2 *player;
static MediaPlayer2Player *playerPlayer;

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

METHOD_HANDLER(MediaPlayer2Player, next)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	call_import(IM_NEXT, 0);

	media_player2_player_complete_next(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, previous)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	call_import(IM_PREV, 0);

	media_player2_player_complete_previous(interface, invocation);
	return TRUE;
}

METHOD_HANDLER_EX(MediaPlayer2Player, seek, gint64 offset)
{
	printf("%s %" PRId64 "\n", __PRETTY_FUNCTION__, offset);

	call_import(IM_SEEK, offset);

	media_player2_player_complete_seek(interface, invocation);
	return TRUE;
}

static void change_volume(MediaPlayer2Player *interface, GParamSpec *pspec)
{
	call_import(IM_SET_VOLUME, media_player2_player_get_volume(interface) * 100);
}

void name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	printf("name acquired, name='%s'\n", name);

	player = media_player2_skeleton_new();
#define SIGNAL(name) g_signal_connect(player, "handle-" #name, G_CALLBACK(handle_##name), NULL)
	SIGNAL(quit);
#undef SIGNAL

	media_player2_set_supported_mime_types(player, supportedMimeTypes);
	media_player2_set_supported_uri_schemes(player, supportedUriSchemes);
	media_player2_set_identity(player, playerIdentity);
	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(player), connection, objectPath,
	                                 NULL);

	playerPlayer = media_player2_player_skeleton_new();
#define SIGNAL(name)                                                                               \
	g_signal_connect(playerPlayer, "handle-" #name, G_CALLBACK(handle_##name), NULL)
	SIGNAL(play);
	SIGNAL(pause);
	SIGNAL(play_pause);
	SIGNAL(seek);
	SIGNAL(next);
	SIGNAL(previous);
#undef SIGNAL

#define NOTIFY(name)                                                                               \
	g_signal_connect(playerPlayer, "notify::" #name, G_CALLBACK(change_##name), NULL)
	NOTIFY(volume);
#undef NOTIFY

	media_player2_player_set_can_control(playerPlayer, TRUE);
	media_player2_player_set_can_seek(playerPlayer, TRUE);
	media_player2_player_set_can_go_next(playerPlayer, TRUE);     // FIXME
	media_player2_player_set_can_go_previous(playerPlayer, TRUE); // FIXME
	media_player2_player_set_can_pause(playerPlayer, TRUE);
	media_player2_player_set_can_play(playerPlayer, TRUE); // FIXME?

	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(playerPlayer), connection,
	                                 objectPath, NULL);
}

void title_changed(char const *title)
{
	GVariantBuilder b;
	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(&b, "{sv}", "mpris:trackid", g_variant_new_object_path("/playlist/1"));
	g_variant_builder_add(&b, "{sv}", "xesam:title", g_variant_new_string(title));
	media_player2_player_set_metadata(playerPlayer, g_variant_builder_end(&b));
}

void mpris_server_run(void)
{
	set_callbacks(&(ServerCallbacks){
	    .title_changed = title_changed,
	});

	guint ownerId =
	    g_bus_own_name(G_BUS_TYPE_SESSION, "org.mpris.MediaPlayer2.chomikbox",
	                   G_BUS_NAME_OWNER_FLAGS_NONE, NULL, name_acquired, NULL, NULL, NULL);

	GMainLoop *loop = g_main_loop_new(NULL, FALSE);

	printf("hi\n");
	g_main_loop_run(loop);
	printf("bye\n");

	g_bus_unown_name(ownerId);
}
