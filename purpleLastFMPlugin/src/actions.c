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

#include <glib.h>

#include "plugin.h"
#include "util.h"

#include "lastfm.h"
#include "actions.h"

/* 
 * This function is the callback for the plugin action we added.
 */
void
plugin_action_cbLastPlayed (PurplePluginAction *action) {
	GString *url = NULL;
	gchar *recentList = NULL;
	gchar *username = purple_prefs_get_string(PREF_USERNAME);
	
	url = g_string_new(LASTFM_AUDIOSCROBBLER);
	g_string_append_printf(url, username);
	g_string_append_printf(url, LASTFM_RECENTTRACKS);
	
	purple_util_fetch_url(url->str, 
			TRUE, 
			NULL, 
			TRUE, 
			cbLastPlayed, 
			(gpointer)recentList); // passed to cbLastPlayed as user_data
	
	g_string_free(url, TRUE);
}

void
plugin_action_cbRecentTracks (PurplePluginAction *action) {
	GString *url = NULL;
	gchar *username = purple_prefs_get_string(PREF_USERNAME);

	url = g_string_new(LASTFM_AUDIOSCROBBLER);
	g_string_append_printf(url, username);
	g_string_append_printf(url, LASTFM_RECENTTRACKS);

	purple_util_fetch_url(url->str, 
			TRUE, 
			NULL, 
			TRUE, 
			cbRecentTracks, 
			NULL);

	g_string_free(url, TRUE);
}

/* 
 * we tell libpurple in the PurplePluginInfo struct to call this function to
 * get a list of plugin actions to use for the plugin.  This function gives
 * libpurple that list of actions. 
 */
GList *
plugin_actions (PurplePlugin *plugin, gpointer context) {
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
	list = g_list_append(list, purple_plugin_action_new("Last Played", plugin_action_cbLastPlayed));
	list = g_list_append(list, purple_plugin_action_new("Recent Tracks", plugin_action_cbRecentTracks));
	
	/* Once the list is complete, we send it to libpurple. */
	return list;
}
