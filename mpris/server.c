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

static char const *playbackStatusNames[] = {
    [PS_STOPPED] = "Stopped", [PS_PAUSED] = "Paused", [PS_PLAYING] = "Playing"};
static char const *repeatTypeNames[] = {
    [RPT_NONE] = "None", [RPT_TRACK] = "Track", [RPT_PLAYLIST] = "Playlist"};

static MediaPlayer2 *player;
static MediaPlayer2Player *playerPlayer;

extern ServerImports imports;
extern ServerCallbacks callbacks;

static struct
{
	char const *title;
	int64_t duration;
} track = {
    .duration = -1,
};

#define METHOD_HANDLER(iface, method)                                                              \
	static gboolean handle_##method(iface *interface, GDBusMethodInvocation *invocation)
#define METHOD_HANDLER_EX(iface, method, ...)                                                      \
	static gboolean handle_##method(iface *interface, GDBusMethodInvocation *invocation,           \
	                                __VA_ARGS__)

METHOD_HANDLER(MediaPlayer2, quit)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	imports.quit();

	media_player2_complete_quit(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, play)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	imports.play();

	media_player2_player_complete_play(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, pause)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	imports.pause();

	media_player2_player_complete_pause(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, play_pause)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	imports.play_pause();

	media_player2_player_complete_play_pause(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, stop)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	imports.stop();

	media_player2_player_complete_stop(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, next)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	imports.next();

	media_player2_player_complete_next(interface, invocation);
	return TRUE;
}

METHOD_HANDLER(MediaPlayer2Player, previous)
{
	printf("%s\n", __PRETTY_FUNCTION__);

	imports.prev();

	media_player2_player_complete_previous(interface, invocation);
	return TRUE;
}

METHOD_HANDLER_EX(MediaPlayer2Player, seek, gint64 offset)
{
	printf("%s %" PRId64 "\n", __PRETTY_FUNCTION__, offset);

	imports.seek(offset);

	media_player2_player_complete_seek(interface, invocation);
	return TRUE;
}

METHOD_HANDLER_EX(MediaPlayer2Player, set_position, gchar const *trackId, gint64 position)
{
	printf("%s %s %" PRId64 "\n", __PRETTY_FUNCTION__, trackId, position);

	imports.set_position(trackId, position);

	media_player2_player_complete_set_position(interface, invocation);
	return TRUE;
}

static void change_volume(MediaPlayer2Player *interface, GParamSpec *pspec)
{
	imports.set_volume(media_player2_player_get_volume(interface) * 100);
}

static void change_shuffle(MediaPlayer2Player *interface, GParamSpec *pspec)
{
	imports.set_shuffle(media_player2_player_get_shuffle(interface));
}

static void change_loop_status(MediaPlayer2Player *interface, GParamSpec *pspec)
{
	gchar const *loopStatus = media_player2_player_get_loop_status(interface);
	for(size_t i = 0; i < sizeof(repeatTypeNames) / sizeof(repeatTypeNames[0]); ++i)
	{
		if(!strcmp(loopStatus, repeatTypeNames[i]))
		{
			imports.set_repeat((RepeatType)i);
			break;
		}
	}
}

void name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	printf("name acquired, name='%s'\n", name);

	player = media_player2_skeleton_new();
	media_player2_set_supported_mime_types(player, supportedMimeTypes);
	media_player2_set_supported_uri_schemes(player, supportedUriSchemes);
	media_player2_set_identity(player, playerIdentity);
	media_player2_set_can_quit(player, TRUE);

#define SIGNAL(name) g_signal_connect(player, "handle-" #name, G_CALLBACK(handle_##name), NULL)
	SIGNAL(quit);
#undef SIGNAL

	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(player), connection, objectPath,
	                                 NULL);

	playerPlayer = media_player2_player_skeleton_new();
	media_player2_player_set_can_control(playerPlayer, TRUE);
	media_player2_player_set_can_seek(playerPlayer, TRUE);
	media_player2_player_set_can_go_next(playerPlayer, TRUE);     // FIXME
	media_player2_player_set_can_go_previous(playerPlayer, TRUE); // FIXME
	media_player2_player_set_can_pause(playerPlayer, TRUE);
	media_player2_player_set_can_play(playerPlayer, TRUE); // FIXME?
	media_player2_player_set_minimum_rate(playerPlayer, 1.0);
	media_player2_player_set_maximum_rate(playerPlayer, 1.0);
	media_player2_player_set_rate(playerPlayer, 1.0);
	media_player2_player_set_loop_status(playerPlayer, "None");
	media_player2_player_set_playback_status(playerPlayer, playbackStatusNames[PS_STOPPED]);

#define SIGNAL(name)                                                                               \
	g_signal_connect(playerPlayer, "handle-" #name, G_CALLBACK(handle_##name), NULL)
	SIGNAL(play);
	SIGNAL(pause);
	SIGNAL(play_pause);
	SIGNAL(stop);
	SIGNAL(seek);
	SIGNAL(set_position);
	SIGNAL(next);
	SIGNAL(previous);
#undef SIGNAL

#define NOTIFY(name)                                                                               \
	g_signal_connect(playerPlayer, "notify::" #name, G_CALLBACK(change_##name), NULL)
	NOTIFY(volume);
	NOTIFY(shuffle);
	// macro not used because of the `-`
	g_signal_connect(playerPlayer, "notify::loop-status", G_CALLBACK(change_loop_status), NULL);
#undef NOTIFY

	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(playerPlayer), connection,
	                                 objectPath, NULL);
}

void send_metadata(void)
{
	GVariantBuilder b;
	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(&b, "{sv}", "mpris:trackid", g_variant_new_object_path("/playlist/1"));
	g_variant_builder_add(&b, "{sv}", "xesam:title", g_variant_new_string(track.title));
	if(track.duration >= 0)
		g_variant_builder_add(&b, "{sv}", "mpris:length", g_variant_new_int64(track.duration));
	media_player2_player_set_metadata(playerPlayer, g_variant_builder_end(&b));
}

void title_changed(char const *title)
{
	free((char *)track.title);

	track.title = strdup(title);
	track.duration = -1;

	send_metadata();
}

void duration_changed(int64_t duration)
{
	track.duration = duration;
	send_metadata();
}

void position_changed(int64_t position)
{
	media_player2_player_set_position(playerPlayer, position);
}

void state_changed(PlayState state)
{
	media_player2_player_set_playback_status(playerPlayer, playbackStatusNames[state]);
}

void volume_changed(int volume)
{
	media_player2_player_set_volume(playerPlayer, volume / 100.0);
}

void shuffle_changed(bool shuffle)
{
	media_player2_player_set_shuffle(playerPlayer, shuffle);
}

void repeat_changed(RepeatType repeat)
{
	media_player2_player_set_loop_status(playerPlayer, repeatTypeNames[repeat]);
}

void mpris_server_run(void)
{
	callbacks = (ServerCallbacks){
	    .title_changed = title_changed,
	    .duration_changed = duration_changed,
	    .position_changed = position_changed,
	    .state_changed = state_changed,
	    .volume_changed = volume_changed,
	    .shuffle_changed = shuffle_changed,
	    .repeat_changed = repeat_changed,
	};

	guint ownerId =
	    g_bus_own_name(G_BUS_TYPE_SESSION, "org.mpris.MediaPlayer2.chomikbox",
	                   G_BUS_NAME_OWNER_FLAGS_NONE, NULL, name_acquired, NULL, NULL, NULL);

	GMainLoop *loop = g_main_loop_new(NULL, FALSE);

	printf("hi\n");
	g_main_loop_run(loop);
	printf("bye\n");

	g_bus_unown_name(ownerId);
}
