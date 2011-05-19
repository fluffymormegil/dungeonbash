// event.hh - event list for Martin's Dungeon Bash
// 
// Copyright 2011 Martin Read
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>

#ifndef event_hh
#define event_hh

enum Event_type
{
    // Statgains
    Event_player_gain_agility,
    Event_player_gain_body,
    // Statdrains
    Event_player_lose_agility,
    Event_player_lose_body,
    // Deaths
    Event_player_death_agility,
    Event_player_death_body
};

#define Total_event_types (1 + Event_player_death_body)

#endif

// event.hh
