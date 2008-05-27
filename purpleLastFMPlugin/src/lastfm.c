/*
 * Pidgin LastFM Plugin
 *
 * Copyright (C) 2008, James H. Nguyen <james.nguyen+pidgin@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* 
 * config.h may define PURPLE_PLUGINS; protect the definition here so that we
 * don't get complaints about redefinition when it's not necessary.
 */
#ifndef PURPLE_PLUGINS
# define PURPLE_PLUGINS
#endif

#include <stdlib.h>
#include <string.h>
#include <glib.h>

/* 
 * This will prevent compiler errors in some instances and is better explained in the
 * how-to documents on the wiki
 */
#ifndef G_GNUC_NULL_TERMINATED
# if __GNUC__ >= 4
#  define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
# else
#  define G_GNUC_NULL_TERMINATED
# endif
#endif

#include "core.h"
#include "account.h"
#include "status.h"
#include "savedstatuses.h"
#include "notify.h"
#include "plugin.h"
#include "pluginpref.h"
#include "prefs.h"
#include "version.h"
#include "util.h"
#include "eventloop.h"
#include "debug.h"

#include "lastfm.h"
#include "actions.h"

/* 
 * we're adding this here and assigning it in plugin_load because we need
 * a valid plugin handle for our call to purple_notify_message() in the
 * plugin_action_test_cb() callback function
 */
PurplePlugin *lastfm_plugin = NULL;

PurpleSavedStatus *original_savedstatus = NULL;
//PurpleUtilFetchUrlData *url_data = NULL;

GString *audioscrobblerUrl;
gboolean display_song;
guint g_tid;

//--------------------------------------------------------------------

void 
cbLastPlayed (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message) {
	/* 1208276806,Airbourne – Let's Ride */
	//gchar *recentList = (gchar *)user_data;
	gchar mostRecent[256 + 1];
	GString *statusMsg = NULL;

	sscanf(url_text, "%256[^\n]", &mostRecent);
	
	statusMsg = g_string_new("");
	g_string_append_printf(statusMsg, mostRecent);
	g_string_erase(statusMsg, (gssize)0, (gssize)11);
	setStatusRecentTrack(statusMsg);
	g_string_free(statusMsg, TRUE);
}

void 
cbRecentTracks (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message) {
	/* 1208276806,Airbourne – Let's Ride */
	//gchar *recentList = (gchar *)user_data;
	
	purple_notify_message(lastfm_plugin, 
			PURPLE_NOTIFY_MSG_INFO, 
			"Displaying Recent Tracks", 
			url_text, 
			NULL, NULL, NULL);
}

void
cbLastfmFetchService (gpointer data) {
	purple_util_fetch_url(audioscrobblerUrl->str, 
			TRUE, 
			NULL, 
			TRUE, 
			cbLastPlayed, 
			NULL);
}

//--------------------------------------------------------------------

void
setStatusRecentTrack (GString *recentTrack) {
	GList *accounts = NULL, *head = NULL;
	PurpleAccount *account = NULL;
	PurpleStatus *status = NULL;
	
	head = accounts = purple_accounts_get_all_active();
	while (accounts != NULL) {
		account = (PurpleAccount*)accounts->data;
		if (account != NULL) {
			status = purple_account_get_active_status(account);
			purple_account_set_status(account, purple_status_get_id(status), TRUE, "message", recentTrack->str, NULL);
		}
		accounts = accounts->next;
	}
	if (head != NULL) g_list_free(head);
}

void
setSavedStatusRecentTrack (GString *str) {
	PurpleSavedStatus *savedstatus = NULL, *current_savedstatus = purple_savedstatus_get_current();
	
	if (!(savedstatus = purple_savedstatus_find("LastFM"))) {
		savedstatus = purple_savedstatus_new("LastFM", purple_savedstatus_get_type(current_savedstatus));
	}
	if (strcmp("LastFM", purple_savedstatus_get_title(current_savedstatus))) {
		original_savedstatus = current_savedstatus;
		purple_savedstatus_set_type(savedstatus, purple_savedstatus_get_type(current_savedstatus));
		//purple_debug_info(PLUGIN_ID, "new original savedstatus %s %d\n", purple_savedstatus_get_title(original_savedstatus), purple_savedstatus_get_type(original_savedstatus));
	}
	//purple_debug_info(PLUGIN_ID, "previous savedstatus %s %d\n", purple_savedstatus_get_title(current_savedstatus), purple_savedstatus_get_type(current_savedstatus));
	purple_savedstatus_set_message(savedstatus, str->str);
	purple_savedstatus_activate(savedstatus);
}

//--------------------------------------------------------------------

static gboolean
plugin_load (PurplePlugin *plugin) {
    //void *core_handle = purple_get_core();
    //void *accounts_handle = purple_accounts_get_handle();
	gchar *username = (gchar *)purple_prefs_get_string(PREF_USERNAME);
	int interval = purple_prefs_get_int(PREF_INTERVAL);

	audioscrobblerUrl = g_string_new(LASTFM_AUDIOSCROBBLER);
	g_string_append_printf(audioscrobblerUrl, username);
	g_string_append_printf(audioscrobblerUrl, LASTFM_RECENTTRACKS);

	/* assign this here so we have a valid handle later */
	lastfm_plugin = plugin;
    
    /* defaults to off on load */
    display_song = FALSE;

    g_tid = purple_timeout_add_seconds(interval, (GSourceFunc)cbLastfmFetchService, NULL);
    
	return TRUE;
}

static gboolean
plugin_unload (PurplePlugin *plugin) {
	purple_timeout_remove(g_tid);
	g_string_free(audioscrobblerUrl, TRUE);
	
	return TRUE;
}

//--------------------------------------------------------------------

static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin) {
	PurplePluginPrefFrame *frame;
	PurplePluginPref *ppref;

	frame = purple_plugin_pref_frame_new();

	ppref = purple_plugin_pref_new_with_label("Last.FM Configurations");
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label("/plugins/core/lastfm/string_username", "username:");
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label("/plugins/core/lastfm/int_interval", "interval [s]:");
	purple_plugin_pref_set_bounds(ppref, 0, 600);
	purple_plugin_pref_frame_add(frame, ppref);

	/*
	ppref = purple_plugin_pref_new_with_name_and_label("/plugins/core/pluginpref_example/int_choice", "integer choice");
	purple_plugin_pref_set_type(ppref, PURPLE_PLUGIN_PREF_CHOICE);
	purple_plugin_pref_add_choice(ppref, "One", GINT_TO_POINTER(1));
	purple_plugin_pref_add_choice(ppref, "Two", GINT_TO_POINTER(2));
	purple_plugin_pref_add_choice(ppref, "Four", GINT_TO_POINTER(4));
	purple_plugin_pref_add_choice(ppref, "Eight", GINT_TO_POINTER(8));
	purple_plugin_pref_add_choice(ppref, "Sixteen", GINT_TO_POINTER(16));
	purple_plugin_pref_add_choice(ppref, "Thirty Two", GINT_TO_POINTER(32));
	purple_plugin_pref_add_choice(ppref, "Sixty Four", GINT_TO_POINTER(64));
	purple_plugin_pref_add_choice(ppref, "One Hundred Twenty Eight", GINT_TO_POINTER(128));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_label("string");
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label("/plugins/core/pluginpref_example/string", "string pref");
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label("/plugins/core/pluginpref_example/masked_string", "masked string");
	purple_plugin_pref_set_masked(ppref, TRUE);
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label("/plugins/core/pluginpref_example/max_string", "string pref\n(max length of 16)");
	purple_plugin_pref_set_max_length(ppref, 16);
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label("/plugins/core/pluginpref_example/string_choice", "string choice");
	purple_plugin_pref_set_type(ppref, PURPLE_PLUGIN_PREF_CHOICE);
	purple_plugin_pref_add_choice(ppref, "red", "red");
	purple_plugin_pref_add_choice(ppref, "orange", "orange");
	purple_plugin_pref_add_choice(ppref, "yellow", "yellow");
	purple_plugin_pref_add_choice(ppref, "green", "green");
	purple_plugin_pref_add_choice(ppref, "blue", "blue");
	purple_plugin_pref_add_choice(ppref, "purple", "purple");
	purple_plugin_pref_frame_add(frame, ppref);
	*/

	return frame;
}

/*
static PidginPluginUiInfo ui_info = {
	pref_frame,
	0
};
*/

static PurplePluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,   /* page_num (Reserved) */
	NULL, /* frame (Reserved) */
	/* Padding */
	NULL,
	NULL,
	NULL,
	NULL
};

/* 
 * For specific notes on the meanings of each of these members, consult the C Plugin Howto
 * on the website.
 */
static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	NULL, // PIDGIN_PLUGIN_TYPE
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,
	
	PLUGIN_ID,
	PLUGIN_NAME,
	PLUGIN_VER,
	"Last.FM Plugin",
	"Last.FM Plugin Description",
	PLUGIN_AUTHOR,
	PLUGIN_HOME_URL,
	
	plugin_load, // load
	plugin_unload, // unload
	NULL, // destroy
	
	NULL, // ui_info,
	NULL, // extra_info
	&prefs_info, // prefs_info
	plugin_actions,
	
	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin (PurplePlugin *plugin) {
	purple_prefs_add_none(PREFS_NAMESPACE);
	purple_prefs_add_string(PREF_USERNAME, "sentinael");
	purple_prefs_add_int(PREF_INTERVAL, 120);
	//purple_prefs_add_string(PREF_FORMAT, "%r: %t by %p on %a (%d)");
	//purple_prefs_add_string(PREF_OFF, "");
	//purple_prefs_add_string(PREF_PAUSED, "%r: Paused");
	//purple_prefs_add_int(PREF_PAUSED, 0);
	//purple_prefs_add_int(PREF_PLAYER, -1);
	//purple_prefs_add_bool(PREF_DISABLED, FALSE);
	//purple_prefs_add_bool(PREF_LOG, FALSE);
	//purple_prefs_add_bool(PREF_FILTER_ENABLE, FALSE);
	//purple_prefs_add_string(PREF_FILTER, filter_get_default());
	//purple_prefs_add_string(PREF_MASK, "*");

	// Player specific defaults
	//purple_prefs_add_string(PREF_XMMS_SEP, "|");
	//purple_prefs_add_string(PREF_MPD_HOSTNAME, "localhost");
	//purple_prefs_add_string(PREF_MPD_PASSWORD, "");
	//purple_prefs_add_string(PREF_MPD_PORT, "6600");
}

PURPLE_INIT_PLUGIN(lastfm, init_plugin, info)
