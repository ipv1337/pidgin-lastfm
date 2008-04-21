/*
 * LastFM Plugin
 *
 * Copyright (C) 2008, James H. Nguyen <james at uci dot edu>
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

/* config.h may define PURPLE_PLUGINS; protect the definition here so that we
 * don't get complaints about redefinition when it's not necessary. */
#ifndef PURPLE_PLUGINS
# define PURPLE_PLUGINS
#endif

#include <stdlib.h>
#include <string.h>
#include <glib.h>

/* This will prevent compiler errors in some instances and is better explained in the
 * how-to documents on the wiki */
#ifndef G_GNUC_NULL_TERMINATED
# if __GNUC__ >= 4
#  define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
# else
#  define G_GNUC_NULL_TERMINATED
# endif
#endif

#include <account.h>
#include <status.h>
#include <savedstatuses.h>
#include <notify.h>
#include <plugin.h>
#include <version.h>
#include <debug.h>

void lastPlayedCB (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message);
void recentTracksCB (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message);

void setStatusRecentTrack (const char * str);
void setSavedStatusRecentTrack (const char * str);


PurpleUtilFetchUrlData *url_data = NULL;

/* we're adding this here and assigning it in plugin_load because we need
 * a valid plugin handle for our call to purple_notify_message() in the
 * plugin_action_test_cb() callback function */
PurplePlugin *lastfm_plugin = NULL;

PurpleSavedStatus *original_savedstatus = NULL;

/* This function is the callback for the plugin action we added.*/
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

	sscanf(url_text, "%256[^\n]", &mostRecent);
	//sprintf(mostRecent, "The last time: %d", &lastTime);
	
	purple_notify_message(lastfm_plugin, 
			PURPLE_NOTIFY_MSG_INFO, 
			"Displaying Last Played", 
			mostRecent, 
			NULL, NULL, NULL);

	setStatusRecentTrack(mostRecent);
}

static void
plugin_action_recentTracksCB (PurplePluginAction * action) {
	gchar *recentList;
	
	url_data = purple_util_fetch_url("http://ws.audioscrobbler.com/1.0/user/sentinael/recenttracks.txt", 
			TRUE, 
			NULL, 
			TRUE, 
			recentTracksCB, 
			(gpointer)recentList);
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
setStatusRecentTrack (const char * str) {
	GList *accounts = NULL, *head = NULL;
	PurpleAccount *account = NULL;
	PurpleStatus *status = NULL;
	
	head = accounts = purple_accounts_get_all_active ();
	while (accounts != NULL) {
		account = (PurpleAccount*)accounts->data;
		if (account != NULL) {
			status = purple_account_get_active_status (account);
			purple_account_set_status (account, purple_status_get_id(status), TRUE, "message", str, NULL);
		}
		accounts = accounts->next;
	}
	if (head != NULL) g_list_free(head);
}

void
setSavedStatusRecentTrack (const char * str) {
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

/* we tell libpurple in the PurplePluginInfo struct to call this function to
 * get a list of plugin actions to use for the plugin.  This function gives
 * libpurple that list of actions. */
static GList *
plugin_actions (PurplePlugin * plugin, gpointer context) {
	/* some C89 (a.k.a. ANSI C) compilers will warn if any variable declaration
	 * includes an initilization that calls a function.  To avoid that, we
	 * generally initialize our variables first with constant values like NULL
	 * or 0 and assign to them with function calls later */
	GList *list = NULL;
	//PurplePluginAction *action = NULL;

	/* The action gets created by specifying a name to show in the UI and a
	 * callback function to call. */
	//action = purple_plugin_action_new("LastFM Action Test", plugin_action_test_cb);

	/* libpurple requires a GList of plugin actions, even if there is only one
	 * action in the list.  We append the action to a GList here. */
	//list = g_list_append(list, action);
	list = g_list_append(list, purple_plugin_action_new("Last Played", plugin_action_lastPlayedCB));
	list = g_list_append(list, purple_plugin_action_new("Recent Tracks", plugin_action_recentTracksCB));

	/* Once the list is complete, we send it to libpurple. */
	return list;
}

static gboolean
plugin_load (PurplePlugin * plugin) {
	lastfm_plugin = plugin; /* assign this here so we have a valid handle later */
	return TRUE;
}

/* For specific notes on the meanings of each of these members, consult the C Plugin Howto
 * on the website. */
static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	NULL,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,

	"gtk-lastfm",
	"Last.FM",
	"1.0", /* This constant is defined in version.h, but you shouldn't use it for
		    your own plugins.  We use it here because it's our plugin. */

	"Last.FM Plugin",
	"Last.FM Plugin Description",
	"James H. Nguyen <james at uci dot edu>", /* correct author */
	"http://bluecentre.net",

	plugin_load,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,
	plugin_actions,		/* this tells libpurple the address of the function to call
				   to get the list of plugin actions. */
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin (PurplePlugin * plugin) {
}

PURPLE_INIT_PLUGIN(lastfm, init_plugin, info)
