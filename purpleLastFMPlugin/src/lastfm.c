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

gboolean display_song;
guint g_tid;

void 
lastPlayedCB (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message) {
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
recentTracksCB (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message) {
	/* 1208276806,Airbourne – Let's Ride */
	//gchar *recentList = (gchar *)user_data;
	
	purple_notify_message(lastfm_plugin, 
			PURPLE_NOTIFY_MSG_INFO, 
			"Displaying Recent Tracks", 
			url_text, 
			NULL, NULL, NULL);
}

void
lastfmFetchServiceCB (gpointer data) {
	purple_util_fetch_url("http://ws.audioscrobbler.com/1.0/user/sentinael/recenttracks.txt", 
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
			purple_account_set_status(account, purple_status_get_id(status), TRUE, "message", str->str, NULL);
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

static gboolean
plugin_load (PurplePlugin *plugin) {
    //void *core_handle = purple_get_core();
    //void *accounts_handle = purple_accounts_get_handle();

	/* assign this here so we have a valid handle later */
	lastfm_plugin = plugin;
    
    /* defaults to off on load */
    display_song = FALSE;

    g_tid = purple_timeout_add_seconds(120, (GSourceFunc)lastfmFetchServiceCB, NULL);
    //purple_timeout_remove(guint handle);
    
	return TRUE;
}

static gboolean
plugin_unload (PurplePlugin *plugin) {
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
	PLUGIN_NAME,
	PLUGIN_VER,
	"Last.FM Plugin",
	"Last.FM Plugin Description",
	PLUGIN_AUTHOR,
	PLUGIN_HOME_URL,
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
init_plugin (PurplePlugin *plugin) {
}

PURPLE_INIT_PLUGIN(lastfm, init_plugin, info)
