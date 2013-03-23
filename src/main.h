/***************************************************************************
    ufoshooter - simple 2d shooter game
    Copyright (C) 2009 Stefan Zidar

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to Free Software 
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    
    Stefan Zidar
    software@steffmeister.at
****************************************************************************

****************************************************************************
* main.h - the main include file                                           *
****************************************************************************/

#include <stdlib.h>
#include <sys/stat.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

#define VERSION "0.5"

#define VIDEO_X 800
#define VIDEO_Y 600

#define DATA_DIR "data/"

#define MAX_ENEMIES 18

#define GAME_TIME 30

#define TICK_INTERVAL 15

#define MENU_ITEM_HEIGHT 25

Uint32 flags = SDL_HWSURFACE;

/* XPM */
static const char *cursor[] = {
  /* width height num_colors chars_per_pixel */
  "    32    32        3            1",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "             XXXXXX             ",
  "             X....X             ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "XX            X..X            XX",
  "X.XXXXXXXXXXXXX..XXXXXXXXXXXXX.X",
  "X..............................X",
  "X..............................X",
  "X.XXXXXXXXXXXXX..XXXXXXXXXXXXX.X",
  "XX            X..X            XX",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "              X..X              ",
  "             X....X             ",
  "             XXXXXX             ",
  "16,16"
};


