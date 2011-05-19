// messages.cc - message texts for Martin's Dungeon Bash
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

#define messages_cc

#include "dunbash.hh"
#include "messages.hh"

// For any message where the English text is absolutely fixed, we are going
// to have (FOR NOW) that message as a static string, and a pointer to it
// which can be reassigned. These strings will be arranged in an array

static const char agil_death_msg_en[] = "Convulsing uncontrollably, you choke on your tongue.";
static const char agil_gain_msg_en[] = "You feel more agile!";
static const char agil_lose_msg_en[] = "You feel clumsy!";
static const char body_death_msg_en[] = "Your heart grows too weak to beat.";
static const char body_gain_msg_en[] = "You feel stronger!";
static const char body_lose_msg_en[] = "You feel weaker!";

const char *event_messages[Total_event_types] =
{
    agil_gain_msg_en,
    body_gain_msg_en,
    agil_lose_msg_en,
    body_lose_msg_en,
    agil_death_msg_en,
    body_death_msg_en,
};

// messages.cc
