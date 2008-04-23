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

#include <glib.h>

#include "plugin.h"
#include "util.h"

#include "lastfm.h"
#include "actions.h"

/* 
 * This function is the callback for the plugin action we added.
 */
void
plugin_action_lastPlayedCB (PurplePluginAction *action) {
	gchar *recentList;
	
	purple_util_fetch_url("http://ws.audioscrobbler.com/1.0/user/sentinael/recenttracks.txt", 
			TRUE, 
			NULL, 
			TRUE, 
			lastPlayedCB, 
			(gpointer)recentList); // passed to lastPlayedCB as user_data
}

void
plugin_action_recentTracksCB (PurplePluginAction *action) {
	purple_util_fetch_url("http://ws.audioscrobbler.com/1.0/user/sentinael/recenttracks.txt", 
			TRUE, 
			NULL, 
			TRUE, 
			recentTracksCB, 
			NULL);
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
	list = g_list_append(list, purple_plugin_action_new("Last Played", plugin_action_lastPlayedCB));
	list = g_list_append(list, purple_plugin_action_new("Recent Tracks", plugin_action_recentTracksCB));
	
	/* Once the list is complete, we send it to libpurple. */
	return list;
}
