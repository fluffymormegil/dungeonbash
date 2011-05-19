// event.cc - event processor for Martin's Dungeon Bash
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

#define event_cc

#include "dunbash.hh"
#include "event.hh"
#include "messages.hh"

void playback_agility_death(void *event_data)
{
    print_msg(0, "%s", event_messages[Event_player_death_agility]);
}

void playback_body_death(void *event_data)
{
    print_msg(0, "%s", event_messages[Event_player_death_body]);
}

void playback_agility_lose(void *event_data)
{
    status_updated = true;
    print_msg(0, "%s", event_messages[Event_player_lose_agility]);
}

void playback_body_lose(void *event_data)
{
    status_updated = true;
    print_msg(0, "%s", event_messages[Event_player_lose_body]);
}

void playback_agility_gain(void *event_data)
{
    unsigned char *ag = (unsigned char *) event_data;
    bool loud = ag[3];
    status_updated = true;
    if (loud)
    {
        print_msg(0, "%s", event_messages[Event_player_gain_agility]);
    }
    display_update();
}

void playback_body_gain(void *event_data)
{
    unsigned char *bo = (unsigned char *) event_data;
    bool loud = bo[3];
    status_updated = true;
    if (loud)
    {
        print_msg(0, "%s", event_messages[Event_player_gain_body]);
    }
    display_update();
}

void notify_event(uint32_t event_type, void *event_data)
{
    // Objective: create a magic rabbit hole which removes all direct print_msg
    // calls from the game logic. This is the first step in creating the
    // long-term goal of client-engine separation.
    switch (event_type)
    {
    case Event_player_gain_agility:
        playback_agility_gain(event_data);
        break;
    case Event_player_gain_body:
        playback_body_gain(event_data);
        break;
    case Event_player_death_agility:
        playback_agility_death(event_data);
        break;
    case Event_player_death_body:
        playback_body_death(event_data);
        break;
    case Event_player_lose_agility:
        playback_agility_lose(event_data);
        break;
    case Event_player_lose_body:
        playback_body_lose(event_data);
        break;
    }
}

// event.cc
