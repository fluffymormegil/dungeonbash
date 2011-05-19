// display-ncw.cc - template source file for Martin's Dungeon Bash
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

#define display_ncw_cc
#include "dunbash.hh"
#include "tiles.hh"
#include <ncurses.h>

short ncw_color_pairs[Total_dbash_colours] = 
{
    DBCLR_L_GREY, DBCLR_D_GREY,
    DBCLR_RED, DBCLR_BLUE, DBCLR_GREEN, DBCLR_PURPLE, DBCLR_BROWN, DBCLR_CYAN,
    DBCLR_L_GREY,
    DBCLR_RED, DBCLR_BLUE, DBCLR_GREEN, DBCLR_PURPLE, DBCLR_BROWN, DBCLR_CYAN
};

attr_t ncw_color_attrs[Total_dbash_colours] =
{
    0, A_BOLD,
    0, 0, 0, 0, 0, 0,
    A_BOLD,
    A_BOLD, A_BOLD, A_BOLD, A_BOLD, A_BOLD, A_BOLD
};

std::vector<cchar_t> tiles[Total_tile_layers];
int tiles_back_buffer[Total_tile_layers][MAX_DUN_HEIGHT][MAX_DUN_WIDTH];

void register_tile(Tile_layer layer, unsigned idx, const wchar_t *characters,
                   const attr_t attrs, short color_pair, void *opts)
{
    if (tiles[layer].size() <= idx)
    {
        tiles[layer].resize(idx);
    }
    setcchar(&(tiles[layer][idx]), characters, attrs, color_pair, opts);
}

// It happens to be the case that the early initalization is just about
// the only thing a strongly ncursesw program has in common with an ncurses
// program. However, since part of the exercise here is to completely
// re-examine the setup code, I'm not too worried about that :)
int display_init_ncw(void)
{
    initscr();
    return 0;
}

// display-ncw.cc
