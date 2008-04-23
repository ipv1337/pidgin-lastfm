/*
 * Pidgin LastFM Plugin
 *
 * Copyright (C) 2008, James H. Nguyen <james dot nguyen at gmail dot com>
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

#include "lastfm.h"
#include "core.h"
#include "account.h"
#include "status.h"
#include "savedstatuses.h"
#include "notify.h"
#include "plugin.h"
#include "util.h"
#include "eventloop.h"
#include "version.h"
#include "debug.h"

/* 
 * we're adding this here and assigning it in plugin_load because we need
 * a valid plugin handle for our call to purple_notify_message() in the
 * plugin_action_test_cb() callback function
 */
PurplePlugin *lastfm_plugin = NULL;

PurpleSavedStatus *original_savedstatus = NULL;
PurpleUtilFetchUrlData *url_data = NULL;

gboolean display_song;
guint g_tid;

/* 
 * This function is the callback for the plugin action we added.
 */
static void
plugin_action_lastPlayedCB (PurplePluginAction * action) {
	gchar *recentList;
	
	url_data = purple_util_fetch_url("http://ws.audioscrobbler.com/1.0/user/sentinael/recenttracks.txt", 
			TRUE, 
			NULL, 
			TRUE, 
			lastPlayedCB, 
			(gpointer)recentList);
}

void 
lastPlayedCB (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message) {
	/* 1208276806,Airbourne – Let's Ride */
	gchar *recentList = (gchar *)user_data;
	gchar mostRecent[256 + 1];
	GString *statusMsg = NULL;

	sscanf(url_text, "%256[^\n]", &mostRecent);
	
	/*
	purple_notify_message(lastfm_plugin, 
			PURPLE_NOTIFY_MSG_INFO, 
			"Displaying Last Played", 
			mostRecent, 
			NULL, NULL, NULL);
	*/

	statusMsg = g_string_new("");
	g_string_append_printf(statusMsg, mostRecent);
	g_string_erase(statusMsg, (gssize)0, (gssize)11);
	setStatusRecentTrack(statusMsg);
	g_string_free(statusMsg, TRUE);
}

static void
plugin_action_recentTracksCB (PurplePluginAction * action) {
	gchar *recentList;
	
	url_data = purple_util_fetch_url("http://ws.audioscrobbler.com/1.0/user/sentinael/recenttracks.txt", 
			TRUE, 
			NULL, 
			TRUE, 
			recentTracksCB, 
			(gpointer)recentList); // passed to recentTracksCB as user_data
}

void 
recentTracksCB (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message) {
	/* 1208276806,Airbourne – Let's Ride */
	gchar *recentList = (gchar *)user_data;
	
	purple_notify_message(lastfm_plugin, 
			PURPLE_NOTIFY_MSG_INFO, 
			"Displaying Recent Tracks", 
			url_text, 
			NULL, NULL, NULL);
}

void
quitting_cb(void *data) {
	unset_status();
}

void
unset_status(void) {
	PurpleSavedStatus *current_savedstatus = purple_savedstatus_get_current();
	if (strcmp("LastFM", purple_savedstatus_get_title(current_savedstatus))) {
		original_savedstatus = current_savedstatus;
		//purple_debug_info(PLUGIN_ID, "unset_status: new original savedstatus %s %d\n", purple_savedstatus_get_title(original_savedstatus), purple_savedstatus_get_type(original_savedstatus));
		return;
	}
	if (!original_savedstatus) {
		original_savedstatus = purple_savedstatus_new(NULL, purple_savedstatus_get_type(current_savedstatus));
	}
	purple_savedstatus_activate(original_savedstatus);
	//purple_debug_info(PLUGIN_ID, "unsetted status\n");
}

void
lastfmFetchServiceCB (gpointer data) {
	url_data = purple_util_fetch_url("http://ws.audioscrobbler.com/1.0/user/sentinael/recenttracks.txt", 
			TRUE, 
			NULL, 
			TRUE, 
			lastPlayedCB, 
			NULL);
}

void
setStatusRecentTrack (GString *str) {
	GList *accounts = NULL, *head = NULL;
	PurpleAccount *account = NULL;
	PurpleStatus *status = NULL;
	
	head = accounts = purple_accounts_get_all_active ();
	while (accounts != NULL) {
		account = (PurpleAccount*)accounts->data;
		if (account != NULL) {
			status = purple_account_get_active_status (account);
			purple_account_set_status (account, purple_status_get_id(status), TRUE, "message", str->str, NULL);
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
	purple_savedstatus_set_message(savedstatus, str);
	purple_savedstatus_activate(savedstatus);
}

void
status_changed_cb(PurpleAccount *account, PurpleStatus *old, PurpleStatus *new, gpointer data) {
	PurpleSavedStatus *current_savedstatus = purple_savedstatus_get_current();
        
	//purple_debug_info(PLUGIN_ID, "got status changed: %s\n", purple_savedstatus_get_message(current_savedstatus));
	if (!display_song) return;
	if (strcmp("LastFM", purple_savedstatus_get_title(current_savedstatus))) {
		//XMMS_CALLBACK_SET(xmms2conn, xmmsc_playback_status, xmms2_status_cb, xmms2conn)
	}
}

void
toggle_display_status_cb(PurplePluginAction *action) {
	//if (!xmms2_connected) {
	//	xmms2_connected = xmms2_connect();
	//	return;
	//} else {
		display_song = !display_song;
		//purple_debug_info(PLUGIN_ID, "toggle_display_status_cb: changing %d\n", display_song);
		if (display_song) {
			//XMMS_CALLBACK_SET(xmms2conn, xmmsc_playback_status, xmms2_status_cb, xmms2conn)
		} else {
			unset_status();
		}
	//}
}


/* 
 * we tell libpurple in the PurplePluginInfo struct to call this function to
 * get a list of plugin actions to use for the plugin.  This function gives
 * libpurple that list of actions. 
 */
static GList *
plugin_actions (PurplePlugin * plugin, gpointer context) {
	/* some C89 (a.k.a. ANSI C) compilers will warn if any variable declaration
	 * includes an initilization that calls a function.  To avoid that, we
	 * generally initialize our variables first with constant values like NULL
	 * or 0 and assign to them with function calls later */
	GList *list = NULL;

	/* The action gets created by specifying a name to show in the UI and a
	 * callback function to call. */
	//PurplePluginAction *action = NULL;
	//action = purple_plugin_action_new("LastFM Action Test", plugin_action_test_cb);

	/* libpurple requires a GList of plugin actions, even if there is only one
	 * action in the list.  We append the action to a GList here. */
	//list = g_list_append(list, action);
	list = g_list_append(list, purple_plugin_action_new("Last Played", plugin_action_lastPlayedCB));
	list = g_list_append(list, purple_plugin_action_new("Recent Tracks", plugin_action_recentTracksCB));
	list = g_list_append(list, purple_plugin_action_new("Toggle displaying status", toggle_display_status_cb));
	
	/* Once the list is complete, we send it to libpurple. */
	return list;
}

static gboolean
plugin_load (PurplePlugin * plugin) {
    void *core_handle = purple_get_core();
    void *accounts_handle = purple_accounts_get_handle();

    //purple_signal_connect(core_handle, "quitting", plugin, PURPLE_CALLBACK(quitting_cb), NULL);
    //purple_signal_connect(accounts_handle, "account-status-changed", plugin, PURPLE_CALLBACK(status_changed_cb), NULL);
    
	/* assign this here so we have a valid handle later */
	lastfm_plugin = plugin;
    //original_savedstatus = purple_savedstatus_get_current();
    
    /* defaults to off on load */
    display_song = FALSE;

    g_tid = purple_timeout_add_seconds((guint)120, &lastfmFetchServiceCB, 0);
    //purple_timeout_remove(guint handle);
    
	return TRUE;
}

static gboolean
plugin_unload (PurplePlugin * plugin) {
	purple_timeout_remove(g_tid);
	
	return TRUE;
}

//--------------------------------------------------------------------

/*
static PidginPluginUiInfo ui_info = {
	pref_frame,
	0
};
*/

/* 
 * For specific notes on the meanings of each of these members, consult the C Plugin Howto
 * on the website.
 */
static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	NULL,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,

	PLUGIN_ID,
	"Last.FM",
	"1.0",

	"Last.FM Plugin",
	"Last.FM Plugin Description",
	"James H. Nguyen <james dot nguyen at gmail dot com>",
	"http://code.google.com/p/pidgin-lastfm",

	plugin_load,
	plugin_unload,
	NULL,

	NULL, // &ui_info,
	NULL,
	NULL,
	plugin_actions,
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin (PurplePlugin * plugin) {
}

PURPLE_INIT_PLUGIN(lastfm, init_plugin, info)
