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

#ifndef _LASTFM_H_
#define _LASTFM_H_

#include <assert.h>
#include <glib.h>

#include "prefs.h"
#include "plugin.h"
#include "account.h"

#include "actions.h"

#define PLUGIN_ID "core-gtk-pidgin-lastfm"
#define PLUGIN_NAME "Last.FM"
#define PLUGIN_VER "1.1.25"
#define PLUGIN_AUTHOR "James H. Nguyen <james.nguyen+pidgin@gmail.com>"
#define PLUGIN_HOME_URL "http://code.google.com/p/pidgin-lastfm"

#define STATUS_OFF 0
#define STATUS_PAUSED 1
#define STATUS_NORMAL 2

#define LASTFM_AUDIOSCROBBLER "http://ws.audioscrobbler.com/1.0/user/"
#define LASTFM_RECENTTRACKS "/recenttracks.txt"

#define PREFS_NAMESPACE "/plugins/core/lastfm"
#define PREF_USERNAME PREFS_NAMESPACE "/string_username"
#define PREF_INTERVAL PREFS_NAMESPACE "/int_interval"

// Global preferences
//static const char *PREF_NAMESPACE = "/plugins/core/lastfm";
//static const char *PREF_USERNAME = "/plugins/core/lastfm/string_username";
//static const char *PREF_INTERVAL = "/plugins/core/lastfm/int_interval";
//static const char *PREF_DISABLED = "/plugins/core/lastfm/bool_disabled";
//static const char *PREF_LOG = "/plugins/core/lastfm/bool_log";
//static const char *PREF_FORMAT = "/plugins/core/lastfm/string_format";
//static const char *PREF_PAUSED = "/plugins/core/lastfm/string_paused";
//static const char *PREF_OFF = "/plugins/core/lastfm/string_off";

// Player specific preferences (should these go somewhere else?)
//static const char *PREF_MPD_HOSTNAME = "/plugins/core/musictracker/string_mpd_hostname";
//static const char *PREF_MPD_PORT = "/plugins/core/musictracker/string_mpd_port";
//static const char *PREF_MPD_PASSWORD = "/plugins/core/musictracker/string_mpd_password";

//GtkWidget* pref_frame(PurplePlugin *plugin);

// function definitions
void cbLastPlayed (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message);
void cbRecentTracks (PurpleUtilFetchUrlData *url_data, gpointer user_data, const gchar *url_text, gsize len, const gchar *error_message);
void cbLastfmFetchService (gpointer data);

void setStatusRecentTrack (GString *str);
void setSavedStatusRecentTrack (GString *str);

#endif // _LASTFM_H_
