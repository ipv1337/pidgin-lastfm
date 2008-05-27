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

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#include "plugin.h"

void plugin_action_cbLastPlayed (PurplePluginAction *action);
void plugin_action_cbRecentTracks (PurplePluginAction *action);
GList *plugin_actions (PurplePlugin *plugin, gpointer context);

#endif // _ACTIONS_H_
