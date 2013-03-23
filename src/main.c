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
* main.c - the main source file                                            *
****************************************************************************/

#include "main.h"

SDL_Surface *screen;
SDL_Surface *background;
SDL_Surface *temp_surface;
SDL_Surface *bullet;
SDL_Surface *sky;
SDL_Surface *terrain_front;
SDL_Surface *terrain_middle;
SDL_Surface *ufo;
SDL_Surface *ufo_small;
SDL_Surface *ufo_smaller;

SDL_Surface *house_01;

TTF_Font *hud_font;
TTF_Font *hud_font_big;
SDL_Color hud_color = {255, 255, 255, 255};
SDL_Color red_color = {255,   0,   0, 255};
SDL_Color ylw_color = {255, 255,   0, 255};

SDL_Event event;


SDL_Rect bullet_r;
Uint32 transparent_color = 0;
SDL_Rect hud_r;
SDL_Rect terrain_front_r;
SDL_Rect terrain_middle_r;
SDL_Rect terrain_front_r_dest;
SDL_Rect terrain_middle_r_dest;

Mix_Chunk *sfx_fire = NULL;
Mix_Chunk *sfx_reload = NULL;
Mix_Chunk *sfx_click = NULL;

int time_left = GAME_TIME;
unsigned int score = 0;
unsigned int bullets_left = 6;

unsigned int bullets_counter = 0;

unsigned int fps = 0;

int enemy_array[MAX_ENEMIES*3*7];
int text_height, text_length;
unsigned int sprite_offset = 0;

// audio stuff
int audio_rate = 22050;
Uint16 audio_format = AUDIO_S16SYS;
int audio_channels = 2;
int audio_buffers = 512;

struct player_score {
	char name[20];
	unsigned int score;
};

struct scores {
	char magic[4];
	struct player_score player[10];
	char checksum[50];
};

struct config {
	char magic[4];
	unsigned int version;
	unsigned int input_grab_mouse;
	unsigned int video_fullscreen;
	unsigned int audio_music;
	unsigned int audio_sound;
};

struct scores high_score;
struct config settings;

void resetHighScore(void) {
	high_score.magic[0] = 'S';
	high_score.magic[1] = 'U';
	high_score.magic[2] = 'S';
	high_score.magic[3] = 'S';

	strcpy(high_score.player[0].name, "steff");
	high_score.player[0].score = 500;
	sprintf(high_score.player[1].name, "steff");
	high_score.player[1].score = 450;
	sprintf(high_score.player[2].name, "steff");
	high_score.player[2].score = 400;
	sprintf(high_score.player[3].name, "steff");
	high_score.player[3].score = 350;
	sprintf(high_score.player[4].name, "steff");
	high_score.player[4].score = 300;
	sprintf(high_score.player[5].name, "steff");
	high_score.player[5].score = 250;
	sprintf(high_score.player[6].name, "steff");
	high_score.player[6].score = 200;
	sprintf(high_score.player[7].name, "steff");
	high_score.player[7].score = 150;
	sprintf(high_score.player[8].name, "steff");
	high_score.player[8].score = 100;
	sprintf(high_score.player[9].name, "steff");
	high_score.player[9].score = 50;

	sprintf(high_score.checksum, "blablalblalb");
}

/* taken from libsdl.org / sdltutorials.com */
void DrawPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color)
{
    int bpp = surface->format->BytesPerPixel;
    if (SDL_MUSTLOCK(screen)) {
		if (SDL_LockSurface(surface) < 0) return;
	}
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
		case 1:
		    *p = color;
		break;

		case 2:
		    *(Uint16 *)p = color;
		break;

		case 3:
		    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
		        p[0] = (color >> 16) & 0xff;
		        p[1] = (color >> 8) & 0xff;
		        p[2] = color & 0xff;
		    } else {
		        p[0] = color & 0xff;
		        p[1] = (color >> 8) & 0xff;
		        p[2] = (color >> 16) & 0xff;
		    }
		break;

		case 4:
		    *(Uint32 *)p = color;
		break;
    }
    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(surface);
}

/* horizontal line */
void draw_hline(SDL_Surface* surface, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color) {
	Sint16 counter;
	/* if x1 is bigger than x2 we have to swap them */
	if (x1 > x2) {
		counter = x2;
		x2 = x1;
		x1 = counter;
	}
	//fprintf(stderr, "hline was called with x1 %d, x2 %d, y %d\n", x1, x2, y);
	counter = 0;
	while((x1+counter) <= x2) {
		DrawPixel(surface, x1+counter, y, color);
		counter++;
	}
}

/* vertical line */
void draw_vline(SDL_Surface* surface, Sint16 y1, Sint16 y2, Sint16 x, Uint32 color) {
	Sint16 counter;

	/* if y1 is bigger than y2 we have to swap them */
	if (y1 > y2) {
		counter = y2;
		y2 = y1;
		y1 = counter;
	}
	//fprintf(stderr, "vline was called with y1 %d, y2 %d, x %d\n", y1, y2, x);
	counter = 0;
	while((y1+counter) <= y2) {
		DrawPixel(surface, x, y1+counter, color);
		counter++;
	}
}

/* rectangle */
void draw_rect(SDL_Surface* surface, Sint16 x, Sint16 y, Sint16 width, Sint16 height, SDL_Color color) {
	Uint32 rcolor = SDL_MapRGB(surface->format, color.r, color.g, color.b);
	draw_hline(surface, x, x+width, y, rcolor);
	draw_hline(surface, x, x+width, y+height, rcolor);
	draw_vline(surface, y, y+height, x, rcolor);
	draw_vline(surface, y, y+height, x+width, rcolor);
}

/* taken from libsdl.org / sdltutorials.com */
Uint32 ReadPixel(SDL_Surface* surface, Sint16 x, Sint16 y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
		case 1:
		    return *p;
		break;

		case 2:
		    return *(Uint16 *)p;
		break;

		case 3:
		    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
		        return p[0] << 16 | p[1] << 8 | p[2];
		    } else {
		        return p[0] | p[1] << 8 | p[2] << 16;
		    }
		break;

		case 4:
		    return *(Uint32 *)p;
		break;

		default:
		    return 0;       /* shouldn't happen, but avoids warnings */
		break;
    }
}

/* taken from http://www.sdltutorials.com/sdl-scale-surface/ */
SDL_Surface *ScaleSurface(SDL_Surface *Surface, Uint16 Width, Uint16 Height)
{
    if(!Surface || !Width || !Height)
        return 0;

    SDL_Surface *_ret = SDL_CreateRGBSurface(Surface->flags, Width, Height, Surface->format->BitsPerPixel,
        Surface->format->Rmask, Surface->format->Gmask, Surface->format->Bmask, Surface->format->Amask);

    double  _stretch_factor_x = (((double)Width)  / ((double)Surface->w)),
        _stretch_factor_y = (((double)Height)  / ((double)Surface->h));

	Sint32 y, x, o_y, o_x;
    for(y = 0; y < Surface->h; y++)
        for(x = 0; x < Surface->w; x++)
            for(o_y = 0; o_y < _stretch_factor_y; ++o_y)
                for(o_x = 0; o_x < _stretch_factor_x; ++o_x)
                    DrawPixel(_ret, (Sint32)(_stretch_factor_x * x) + o_x,
                        (Sint32)(_stretch_factor_y * y) + o_y, ReadPixel(Surface, x, y));

    return _ret;
}

void resetSettings(void) {
	settings.magic[0] = 'S';
	settings.magic[1] = 'U';
	settings.magic[2] = 'S';
	settings.magic[3] = 'C';
	settings.input_grab_mouse = 0;
	settings.video_fullscreen = 0;
	settings.audio_music = 1;
	settings.audio_sound = 1;
}

void loadHighScore(void) {
	FILE *file_handler;
	char tempString[255];
	#ifndef WIN32
        sprintf(tempString, "%s/.ssw/ufoshooter.scores", getenv("HOME"));
	#else
        sprintf(tempString, "ufoshooter.scores");
    #endif
	file_handler = fopen(tempString, "r");
	if (file_handler != NULL) {
		fread(&high_score, sizeof(high_score), 1, file_handler);
		if ((high_score.magic[0] == 'S') && (high_score.magic[1] == 'U') && (high_score.magic[2] == 'S')  && (high_score.magic[3] == 'S')) {
			// TODO checksum
		} else {
			printf("*** scores file has wrong magic.\n");
			resetHighScore();
		}
		fclose(file_handler);
	} else {
		printf("*** could not open scores file, does not exist?\n");
	}
}

void loadSettings(void) {
	FILE *file_handler;
	char tempString[255];
	#ifndef WIN32
        sprintf(tempString, "%s/.ssw/ufoshooter.conf", getenv("HOME"));
    #else
        sprintf(tempString, "ufoshooter.conf");
    #endif
	file_handler = fopen(tempString, "r");
	if (file_handler != NULL) {
		fread(&settings, sizeof(settings), 1, file_handler);
		if ((settings.magic[0] == 'S') && (settings.magic[1] == 'U') && (settings.magic[2] == 'S')  && (settings.magic[3] == 'C')) {
			// TODO checksum
		} else {
			printf("*** settings file has wrong magic.\n");
			resetSettings();
		}
		fclose(file_handler);
	} else {
		printf("*** could not open settings file, does not exist?\n");
	}
}

void writeHighScore(void) {
	FILE *file_handler;
	struct stat st;
	char tempString[255];
	sprintf(tempString, "ufoshooter.scores");
	#ifndef WIN32
        sprintf(tempString, "%s/.ssw", getenv("HOME"));
     	if (stat(tempString, &st) != 0) {
            printf("*** dir does not exist\n");
            if (mkdir(tempString, S_IRWXU | S_IRWXG | S_IRWXO) == -1) printf("*** ERROR\n");
	    }
        sprintf(tempString, "%s/ufoshooter.scores", tempString);
    #endif

	file_handler = fopen(tempString, "w");
	if (file_handler != NULL) {
		fwrite(&high_score, sizeof(high_score), 1, file_handler);
		fclose(file_handler);
	} else {
		printf("*** could not WRITE scores file. opening failed\n");
	}
}

void writeSettings(void) {
	FILE *file_handler;
	struct stat st;
	char tempString[255];
	sprintf(tempString, "ufoshooter.conf");
	#ifndef WIN32
        sprintf(tempString, "%s/.ssw", getenv("HOME"));
        if (stat(tempString, &st) != 0) {
            printf("*** dir does not exist\n");
            if (mkdir(tempString, S_IRWXU | S_IRWXG | S_IRWXO) == -1) printf("*** ERROR\n");
        }
    	sprintf(tempString, "%s/ufoshooter.conf", tempString);
	#endif
	file_handler = fopen(tempString, "w");
	if (file_handler != NULL) {
		fwrite(&settings, sizeof(settings), 1, file_handler);
		fclose(file_handler);
	} else {
		printf("*** could not WRITE settings file. opening failed\n");
	}
}

void addRandomEnemy(void) {
	unsigned int counter_enemy = 0;
	unsigned int quit_loop = 0;
	unsigned int end_number = 0;

	unsigned int layer = rand()%3;


	// layer offset
	switch(layer) {
		case 0: // near
			counter_enemy = (MAX_ENEMIES/3)*2*7;
		break;
		case 1: // middle
			counter_enemy = (MAX_ENEMIES/3) * 7;
		break;
		default: // away
			counter_enemy = 0;
		break;
	}
	end_number = counter_enemy + ((MAX_ENEMIES/3)*7);

	while(!quit_loop) {
		if (enemy_array[counter_enemy] == 0) {
			enemy_array[counter_enemy] = 1;  // active


			enemy_array[counter_enemy+2] = rand()%400; // y

			enemy_array[counter_enemy+3] = rand()%4;  // direction: 3 = to the right moving down


			if (enemy_array[counter_enemy+3] < 2) {
				enemy_array[counter_enemy+1] = VIDEO_X;  // x
			} else {
				enemy_array[counter_enemy+1] = -128;  // x
			}


			enemy_array[counter_enemy+4] = 0;  // offset

			enemy_array[counter_enemy+5] = layer;  // layer, 0 ... 2

			enemy_array[counter_enemy+6] = rand()%4; // current tile 0 ... 3

			quit_loop = 1;
		}
		counter_enemy += 7;
		if (counter_enemy >= end_number) quit_loop = 1;
	}

}

Uint32 game_add_random_enemy(Uint32 interval, void *param) {
	addRandomEnemy();
	return interval;
}

void clear_enemy_arrays(void) {
	unsigned int counter_enemy = 0;
	while(counter_enemy < (MAX_ENEMIES*3*7)) {
		enemy_array[counter_enemy] = 0;
		counter_enemy++;
	}
}

void update_hud(void) {
	char tempstring[20];

	sprintf(tempstring, "%3d", time_left);

	if (time_left < 10) {
		temp_surface = TTF_RenderText_Blended(hud_font, tempstring, red_color);
	} else {
		temp_surface = TTF_RenderText_Blended(hud_font, tempstring, hud_color);
	}

	hud_r.x = 10;
	hud_r.w = temp_surface->w + 10;
	SDL_BlitSurface(background, &hud_r, screen, &hud_r);
	SDL_BlitSurface(temp_surface, NULL, screen, &hud_r);

	sprintf(tempstring, "%6d", score);
	temp_surface = TTF_RenderText_Blended(hud_font, tempstring, hud_color);
	hud_r.x = 100;
	hud_r.w = temp_surface->w;
	SDL_BlitSurface(background, &hud_r, screen, &hud_r);
	SDL_BlitSurface(temp_surface, NULL, screen, &hud_r);

	if (bullets_left > 0) {
		bullets_counter = 0;
		while(bullets_counter <= bullets_left) {
			bullet_r.x = VIDEO_X - (bullets_counter*32);
			SDL_BlitSurface(bullet, NULL, screen, &bullet_r);
			bullets_counter++;
		}
	}
}

static SDL_Cursor *init_system_cursor(const char *image[])
{
	int i, row, col;
	Uint8 data[4*32];
	Uint8 mask[4*32];
	int hot_x, hot_y;

	i = -1;
	for ( row=0; row<32; ++row ) {
		for ( col=0; col<32; ++col ) {
			if ( col % 8 ) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				++i;
				data[i] = mask[i] = 0;
			}
			switch (image[4+row][col]) {
				case 'X':
					data[i] |= 0x01;
					mask[i] |= 0x01;
				break;
				case '.':
					mask[i] |= 0x01;
				break;
				case ' ':
				break;
			}
		}
	}
	sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

Uint32 game_timer(Uint32 interval, void *param) {
	char tempstring[20];
	if (time_left > -1) time_left--;
	sprite_offset++;
	if (sprite_offset > 3) sprite_offset = 0;
	printf("=== FPS: %d\n", fps);
	sprintf(tempstring, "%d", fps);
	temp_surface = TTF_RenderText_Blended(hud_font, tempstring, hud_color);
	hud_r.x = 300;
	hud_r.w = temp_surface->w + 10;
	SDL_BlitSurface(background, &hud_r, screen, &hud_r);
	SDL_BlitSurface(temp_surface, NULL, screen, &hud_r);
	fps = 0;
	return interval;
}

void are_you_ready() {
	SDL_Surface * ready;
	SDL_Rect ready_r;

	unsigned int quit_loop = 0;

	ready = TTF_RenderText_Blended(hud_font, "Right-click if you're ready!", hud_color);
	TTF_SizeText(hud_font, "Right-click if you're ready!", &text_length, &text_height);
	ready_r.x = screen->w / 2 - text_length / 2;
	ready_r.y = screen->h / 2 - text_height / 2;
	ready_r.w = text_length;
	ready_r.h = text_height;

	SDL_BlitSurface(ready, NULL, screen, &ready_r);
	SDL_Flip(screen);
	SDL_FreeSurface(ready);
	while(!quit_loop) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
					case SDLK_ESCAPE:
						quit_loop = 1;
					break;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				switch(event.button.button) {
					case SDL_BUTTON_RIGHT:
						quit_loop = 1;
					break;
				}
			}
		}
		SDL_Delay(10);
	}
}

Uint32 TimeLeft(void) {
    static Uint32 next_time = 0;
    Uint32 now, ret = 0;

    now = SDL_GetTicks();
    if (next_time <= now) {
        next_time = now + TICK_INTERVAL;
        return(ret);
    }
    ret = next_time - now;
    return(ret);
}

unsigned int collider_pixel(unsigned int x, unsigned int y, SDL_Rect *rect) {
	unsigned int ret = 0;
	if ((y > rect->y) && (y < (rect->y+rect->h))) {
		if ((x > rect->x) && (x < (rect->x+rect->w))) {
			ret = 1;
		}
	}
	return ret;
}

void show_highscores(int highlight) {
	SDL_Surface *m_string;
	SDL_Surface *m_high_score;

	SDL_Rect m_string_r;
	SDL_Rect m_scores_r;
	SDL_Rect m_high_score_r;

	unsigned int counter = 0;

	char tempString[20];

	SDL_Color score_color = hud_color;

	m_high_score = TTF_RenderText_Blended(hud_font, "high score", hud_color);
	TTF_SizeText(hud_font, "high score", &text_length, &text_height);
	m_high_score_r.x = screen->w / 2 - text_length / 2;
	m_high_score_r.y = 120;

	SDL_BlitSurface(m_high_score, NULL, screen, &m_high_score_r);

	m_high_score = TTF_RenderText_Blended(hud_font, "right-click to continue", hud_color);
	TTF_SizeText(hud_font, "right-click to continue", &text_length, &text_height);
	m_high_score_r.x = screen->w / 2 - text_length / 2;
	m_high_score_r.y = 500;

	SDL_BlitSurface(m_high_score, NULL, screen, &m_high_score_r);

	m_string_r.x = 250;
	m_string_r.y = 150;

	m_scores_r.x = m_string_r.x + 300;
	m_scores_r.y = m_string_r.y;

	while(counter < 10) {
		if (counter == highlight) {
			score_color = ylw_color;
		} else {
			score_color = hud_color;
		}
		m_string = TTF_RenderText_Blended(hud_font, high_score.player[counter].name, score_color);
		TTF_SizeText(hud_font, high_score.player[counter].name, &text_length, &text_height);
		SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

		sprintf(tempString, "%d", high_score.player[counter].score);
		m_string = TTF_RenderText_Blended(hud_font, tempString, score_color);
		TTF_SizeText(hud_font, tempString, &text_length, &text_height);
		m_scores_r.x -= text_length;
		SDL_BlitSurface(m_string, NULL, screen, &m_scores_r);

		m_string_r.y += text_height;
		m_scores_r.x += text_length;
		m_scores_r.y += text_height;

		counter++;
		SDL_FreeSurface(m_string);
	}
	SDL_FreeSurface(m_high_score);
}



void game(void) {

	SDL_Surface * m_game_over;
	SDL_Surface * m_your_score;
	SDL_Surface * m_score;

	SDL_Rect ufo_tile_rect;
	SDL_Rect ufo_rect_dest;
	SDL_Rect m_game_over_r;
	SDL_Rect m_your_score_r;
	SDL_Rect m_score_r;

	unsigned int moving_right = 0;
	unsigned int moving_left = 0;
	unsigned int bullet_shooted = 0;
	unsigned int bullet_shooted_x = 0;
	unsigned int bullet_shooted_y = 0;
	unsigned int enemy_worker = 0;
	unsigned int enemy_counter = 0;

	SDL_TimerID game_timer_id;
	SDL_TimerID enemy_timer_id;

	unsigned int quit = 0;
	int counter = 0;
	int rank = -1;

	int enemy_shooted = -1;

	char tempString[20];
	
	/* bonus extra stuff */
	unsigned int house_01_status = 0;
	SDL_Rect house_01_src;
	SDL_Rect house_01_dest;
	SDL_Rect house_01_hit_1;
	SDL_Rect house_01_hit_2;
	
	house_01_src.x = 0;
	house_01_src.y = 0;
	house_01_src.h = 64;
	house_01_src.w = 64;
	house_01_dest.x = 200;
	house_01_dest.y = 265;
	house_01_dest.h = 64;	
	house_01_dest.w = 64;
	house_01_hit_1.x = 17+house_01_dest.x;
	house_01_hit_1.y = 18+house_01_dest.y;
	house_01_hit_1.h = 41;
	house_01_hit_1.w = 17;
	house_01_hit_2.x = 43+house_01_dest.x;
	house_01_hit_2.y = 46+house_01_dest.y;
	house_01_hit_2.h = 12;
	house_01_hit_2.w = 12;
	
	fps = 0;

	clear_enemy_arrays();

	bullet_r.y = 600 - 64;
	bullet_r.w = 32;
	bullet_r.h = 96;

	TTF_SizeText(hud_font, "Size", NULL, &text_height);

	hud_r.x = 10;
	hud_r.y = 595 - text_height;
	hud_r.w = 0;
	hud_r.h = text_height;

	ufo_tile_rect.x = 0;
	ufo_tile_rect.y = 0;
	ufo_tile_rect.w = 128;
	ufo_tile_rect.h = 96;

	ufo_rect_dest.x = 0;
	ufo_rect_dest.y = 0;
	ufo_rect_dest.w = ufo_tile_rect.w;
	ufo_rect_dest.h = ufo_tile_rect.h;

	bullets_left = 6;
	score = 0; //TODO delete later
	time_left = GAME_TIME;
	//time_left = 10;

/*	SDL_BlitSurface(sky, NULL, background, NULL);
	SDL_BlitSurface(terrain_middle, &terrain_middle_r, background, &terrain_middle_r_dest);
	SDL_BlitSurface(terrain_front, &terrain_front_r, background, &terrain_front_r_dest);*/
	SDL_BlitSurface(background, NULL, screen, NULL);


	update_hud();

	m_game_over = TTF_RenderText_Blended(hud_font, "Game Over!", hud_color);
	TTF_SizeText(hud_font, "Game Over!", &text_length, &text_height);
	m_game_over_r.x = screen->w / 2 - text_length / 2;
	m_game_over_r.y = 200;
	m_your_score = TTF_RenderText_Blended(hud_font, "your score", hud_color);
	TTF_SizeText(hud_font, "your score", &text_length, &text_height);
	m_your_score_r.x = screen->w / 2 - text_length / 2;
	m_your_score_r.y = 250;



	//SDL_Flip(screen);

	are_you_ready();

	game_timer_id = SDL_AddTimer(1000, game_timer, 0);
	enemy_timer_id = SDL_AddTimer(500, game_add_random_enemy, 0);


	while(!quit) {
		while (SDL_PollEvent(&event)) {
			if (time_left > -1) {
				if (event.type == SDL_KEYDOWN) {
					switch(event.key.keysym.sym) {
						case SDLK_ESCAPE:
							time_left = -1;
						break;
						case SDLK_LEFT:
							moving_left = 1;
						break;
						case SDLK_RIGHT:
							moving_right = 1;
						break;
/*						case SDLK_SPACE:
							addRandomEnemy();
						break;*/
					}
				} else if (event.type == SDL_KEYUP) {
					switch(event.key.keysym.sym) {
						case SDLK_LEFT:
							moving_left = 0;
						break;
						case SDLK_RIGHT:
							moving_right = 0;
						break;
					}
				} else if (event.type == SDL_MOUSEBUTTONDOWN) {
					switch(event.button.button) {
						case SDL_BUTTON_LEFT:
							if (bullets_left > 0) {
								bullets_left--;
								bullet_shooted = 1;
								bullet_shooted_x = event.button.x;
								bullet_shooted_y = event.button.y;
								Mix_PlayChannel(-1, sfx_fire, 0);
							}
						break;
						case SDL_BUTTON_RIGHT:
							if (bullets_left == 0) {
								bullets_left = 6;
								Mix_PlayChannel(-1, sfx_reload, 0);
							}
						break;
					}
				}
			} else {
				if (event.type == SDL_KEYDOWN) {
					switch(event.key.keysym.sym) {
						case SDLK_RETURN:
						case SDLK_SPACE:
						case SDLK_ESCAPE:
							if (time_left == -3) {
								quit = 1;
							} else if (time_left == -2) {
								time_left = -3;
							}
						break;
					}
				} else if (event.type == SDL_MOUSEBUTTONDOWN) {
					switch(event.button.button) {
						case SDL_BUTTON_RIGHT:
							if (time_left == -3) {
								quit = 1;
							} else if (time_left == -2) {
								time_left = -3;
							}
						break;
					}
				}
			}
		}

		/* rebuild screen */
		SDL_BlitSurface(background, NULL, screen, NULL);

		if (time_left > -1) {
			update_hud();
		}



		/* bonus extra stuff drawing stuff */
		switch(house_01_status) {	
			case 1:
				house_01_src.x = 64;
			break;
			case 2:
				house_01_src.x = 128;
			break;
			case 3:
				house_01_src.x = 192;
			break;
		}
		SDL_BlitSurface(house_01, &house_01_src, screen, &house_01_dest);
//		draw_rect(screen, house_01_hit_1.x, house_01_hit_1.y, house_01_hit_1.w, house_01_hit_1.h, red_color);

		/* enemy stuff */
		enemy_worker = 0;
		enemy_counter = 0;
		while(enemy_worker < (MAX_ENEMIES*3*7)) {
			if (enemy_array[enemy_worker] == 1) {

				ufo_rect_dest.x = enemy_array[enemy_worker+1];
				ufo_rect_dest.y = enemy_array[enemy_worker+2];

				if (ufo_rect_dest.x < 0) ufo_rect_dest.x = 0;

				switch(enemy_array[enemy_worker+5]) {
					case 0:
						ufo_tile_rect.w = 128;
						ufo_tile_rect.h = 96;
					break;
					case 1:
						ufo_tile_rect.w = 64;
						ufo_tile_rect.h = 48;
					break;
					case 2:
						ufo_tile_rect.w = 43;
						ufo_tile_rect.h = 32;
					break;
				}

				ufo_rect_dest.w = ufo_tile_rect.w;
				ufo_rect_dest.h = ufo_tile_rect.h;


				switch(enemy_array[enemy_worker+3]) {
					case 0:
						enemy_array[enemy_worker+1]--;  // x
						enemy_array[enemy_worker+4]--;  // y
						if (enemy_array[enemy_worker+4] <= 0) enemy_array[enemy_worker+3] = 1;
					break;
					case 1:
						enemy_array[enemy_worker+1]--;  // x
						enemy_array[enemy_worker+4]++;  // y
						if (enemy_array[enemy_worker+4] >= 20) enemy_array[enemy_worker+3] = 0;
					break;
					case 2:
						enemy_array[enemy_worker+1]++;  // x
						enemy_array[enemy_worker+4]--;  // y
						if (enemy_array[enemy_worker+4] <= 0) enemy_array[enemy_worker+3] = 3;
					break;
					case 3:
						enemy_array[enemy_worker+1]++;  // x
						enemy_array[enemy_worker+4]++;  // y
						if (enemy_array[enemy_worker+4] >= 20) enemy_array[enemy_worker+3] = 2;
					break;
				}
				if (((enemy_array[enemy_worker+3] > 1) && (enemy_array[enemy_worker+1] < VIDEO_X)) ||
					((enemy_array[enemy_worker+3] < 2) && (enemy_array[enemy_worker+1] > -128))) {

					ufo_rect_dest.x = enemy_array[enemy_worker+1];
					ufo_rect_dest.y = enemy_array[enemy_worker+2] + enemy_array[enemy_worker+4];
					//enemy_array[enemy_worker+6] = sprite_offset;
					//if (enemy_array[enemy_worker+6] > 3) enemy_array[enemy_worker+6] -= 4;
					ufo_tile_rect.x = enemy_array[enemy_worker+6]+sprite_offset;
					if (ufo_tile_rect.x > 3) ufo_tile_rect.x -= 4;
					ufo_tile_rect.x *= ufo_tile_rect.w;

					// distance
					switch(enemy_array[enemy_worker+5]) {
						case 0:
							SDL_BlitSurface(ufo, &ufo_tile_rect, screen, &ufo_rect_dest);
						break;
						case 1:
							SDL_BlitSurface(ufo_small, &ufo_tile_rect, screen, &ufo_rect_dest);
						break;
						case 2:
							SDL_BlitSurface(ufo_smaller, &ufo_tile_rect, screen, &ufo_rect_dest);
						break;
					}

					if (bullet_shooted) {
						if (collider_pixel(bullet_shooted_x, bullet_shooted_y, &ufo_rect_dest)) enemy_shooted = enemy_worker;
					}

				} else {
					printf("enemy # %d deleted\n", enemy_worker);
					enemy_array[enemy_worker] = 0;
				}
				enemy_counter++;

			/* ufo is exploding */
			} else if (enemy_array[enemy_worker] == 2) {


				ufo_rect_dest.x = enemy_array[enemy_worker+1];
				ufo_rect_dest.y = enemy_array[enemy_worker+2];

				if (ufo_rect_dest.x < 0) ufo_rect_dest.x = 0;

				switch(enemy_array[enemy_worker+5]) {
					case 0:
						ufo_tile_rect.w = 128;
						ufo_tile_rect.h = 96;
					break;
					case 1:
						ufo_tile_rect.w = 64;
						ufo_tile_rect.h = 48;
					break;
					case 2:
						ufo_tile_rect.w = 43;
						ufo_tile_rect.h = 32;
					break;
				}

				ufo_rect_dest.w = ufo_tile_rect.w;
				ufo_rect_dest.h = ufo_tile_rect.h;


				switch(enemy_array[enemy_worker+3]) {
					case 0:
					case 1:
						enemy_array[enemy_worker+1]--;  // x
					break;
					case 2:
					case 3:
						enemy_array[enemy_worker+1]++;  // x
					break;
				}
				enemy_array[enemy_worker+4]++; // y, going down
				
				ufo_rect_dest.x = enemy_array[enemy_worker+1];
				ufo_rect_dest.y = enemy_array[enemy_worker+2] + enemy_array[enemy_worker+4];
				//enemy_array[enemy_worker+6] = sprite_offset;
				//if (enemy_array[enemy_worker+6] > 3) enemy_array[enemy_worker+6] -= 4;
				ufo_tile_rect.x = enemy_array[enemy_worker+6];
//				if (ufo_tile_rect.x > 3) ufo_tile_rect.x -= 4;
				ufo_tile_rect.x *= ufo_tile_rect.w;

				// distance
				switch(enemy_array[enemy_worker+5]) {
					case 0:
						SDL_BlitSurface(ufo, &ufo_tile_rect, screen, &ufo_rect_dest);
					break;
					case 1:
						SDL_BlitSurface(ufo_small, &ufo_tile_rect, screen, &ufo_rect_dest);
					break;
					case 2:
						SDL_BlitSurface(ufo_smaller, &ufo_tile_rect, screen, &ufo_rect_dest);
					break;
				}
				
				printf("*** enemy # %d tile # %d\n", enemy_worker, enemy_array[enemy_worker+6]);
				
				enemy_array[enemy_worker+6]++;
				if (enemy_array[enemy_worker+6] > 7) enemy_array[enemy_worker] = 0;
				
			}
			enemy_worker += 7;
		}
		if (enemy_shooted > -1) {
			enemy_array[enemy_shooted] = 2; // set object to explode
			enemy_array[enemy_worker+6] = 4; // set to first explode tile
			switch(enemy_array[enemy_shooted+5]) {
				case 0: // near
					score = score + 5;
				break;
				case 1: // middle
					score = score + 10;
				break;
				case 2: // far
					score = score + 20;
				break;
			}
			printf("*** enemy # %d shooted\n", enemy_shooted);
			//enemy_shooted = -1;
		}
		//printf("+++ enemy counter: %d\n", enemy_counter);
		
		/* bonus extra stuff drawing stuff */
		if ((enemy_shooted == -1) && (bullet_shooted == 1)) {
			if (collider_pixel(bullet_shooted_x, bullet_shooted_y, &house_01_hit_1)) {
				if (house_01_status == 0) house_01_status = 2;
				if (house_01_status == 1) house_01_status = 3;
			}
			
			if (collider_pixel(bullet_shooted_x, bullet_shooted_y, &house_01_hit_2)) {
				if (house_01_status == 0) house_01_status = 1;
				if (house_01_status == 2) house_01_status = 3;
			}
		}
		
		enemy_shooted = -1;
		
		bullet_shooted = 0;
		if (time_left == -1) {
			sprintf(tempString, "%d", score);
			m_score = TTF_RenderText_Blended(hud_font_big, tempString, hud_color);
			TTF_SizeText(hud_font_big, tempString, &text_length, &text_height);
			m_score_r.x = screen->w / 2 - text_length / 2;
			m_score_r.y = 252 + text_height;

			SDL_RemoveTimer(enemy_timer_id);
			SDL_RemoveTimer(game_timer_id);
			// check if we are in the high score
			if (score > high_score.player[9].score) {
				// check at what rank we are
				counter = 9;
				while(counter >= 0) {
					if (score > high_score.player[counter].score) rank = counter;
					counter--;
				}
				printf("*** you ranked #%d\n", rank);

				if (rank < 9) {
/*					counter = 9;
					while(counter >= rank) {
						strcpy(high_score.player[counter].name, high_score.player[counter-1].name);
						high_score.player[counter].score = high_score.player[counter-1].score;
						counter--;
					}*/

					unsigned int rank_counter;
					rank_counter = 8;
					while(rank_counter > rank) {
						printf("rank_counter is %d\n", rank_counter);
						strcpy(high_score.player[rank_counter+1].name, high_score.player[rank_counter].name);
						high_score.player[rank_counter+1] = high_score.player[rank_counter];
						rank_counter--;
					}

				}
				#ifndef WIN32
                    sprintf(high_score.player[rank].name, "%s", getenv("USERNAME"));
                #else
                    sprintf(high_score.player[rank].name, "%s", getenv("USER"));
                #endif
				high_score.player[rank].score = score;
				//printf("*** you ranked #%d", rank);
			}
			time_left = -2;
		} else if (time_left == -2) {
			SDL_BlitSurface(m_game_over, NULL, screen, &m_game_over_r);
			SDL_BlitSurface(m_your_score, NULL, screen, &m_your_score_r);
			SDL_BlitSurface(m_score, NULL, screen, &m_score_r);
		} else if (time_left == -3) {
			show_highscores(rank);
		}
		fps++;
		SDL_Flip(screen);
		SDL_Delay(TimeLeft());

	}
//	SDL_RemoveTimer(enemy_timer_id);
}

void pressAnyKey(void) {
	unsigned int quit = 0;
	while(!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
					case SDLK_SPACE:
					case SDLK_RETURN:
					case SDLK_ESCAPE:
						quit = 1;
					break;

				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				switch(event.button.button) {
					case SDL_BUTTON_RIGHT:
						quit = 1;
					break;
				}
			}
		}
		SDL_Delay(20);
	}
}

void showAbout(void) {
	SDL_Surface *m_string;
	SDL_Rect m_string_r;
	char tempChar[20];

	m_string = TTF_RenderText_Blended(hud_font, "ufoshooter", hud_color);
	TTF_SizeText(hud_font, "ufoshooter", &text_length, &text_height);
	m_string_r.x = 100;
	m_string_r.y = 185;
	SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

	sprintf(tempChar, "version %s", VERSION);
	m_string = TTF_RenderText_Blended(hud_font, tempChar, hud_color);
	m_string_r.x = 120;
	m_string_r.y += 20;
	SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

	m_string = TTF_RenderText_Blended(hud_font, "created by steffmeister software", hud_color);
	m_string_r.x = 120;
	m_string_r.y += 20;
	SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

	m_string = TTF_RenderText_Blended(hud_font, "ufoshooter uses", hud_color);
	m_string_r.x = 100;
	m_string_r.y += 40;
	SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

	m_string = TTF_RenderText_Blended(hud_font, "SDL <libsdl.org>", hud_color);
	m_string_r.x = 120;
	m_string_r.y += 20;
	SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

	m_string = TTF_RenderText_Blended(hud_font, "SDL_gfx", hud_color);
	m_string_r.x = 120;
	m_string_r.y += 20;
	SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

	m_string = TTF_RenderText_Blended(hud_font, "SDL_image", hud_color);
	m_string_r.x = 120;
	m_string_r.y += 20;
	SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

	m_string = TTF_RenderText_Blended(hud_font, "SDL_ttf", hud_color);
	m_string_r.x = 120;
	m_string_r.y += 20;
	SDL_BlitSurface(m_string, NULL, screen, &m_string_r);

	SDL_Flip(screen);
	pressAnyKey();

	SDL_FreeSurface(m_string);
}

void options_menue(void) {
	SDL_Surface *m_fullscreen;
	SDL_Surface *m_sfx_on;
	SDL_Surface *m_sfx_off;
	SDL_Surface *m_mus_on;
	SDL_Surface *m_mus_off;
	SDL_Surface *m_back;

	SDL_Rect m_fullscreen_r;
	SDL_Rect m_sfx_on_r;
	SDL_Rect m_sfx_off_r;
	SDL_Rect m_mus_on_r;
	SDL_Rect m_mus_off_r;
	SDL_Rect m_back_r;

	unsigned int quit = 0;
	int selected = 1;
	int updateMenue = 0;

	SDL_BlitSurface(background, NULL, screen, NULL);

	m_fullscreen = TTF_RenderText_Blended(hud_font, "toggle fullscreen", hud_color);
	TTF_SizeText(hud_font, "toggle fullscreen", &text_length, &text_height);
	m_fullscreen_r.x = screen->w / 2 - text_length / 2;
	m_fullscreen_r.y = 150;
	SDL_BlitSurface(m_fullscreen, NULL, screen, &m_fullscreen_r);

	m_sfx_on = TTF_RenderText_Blended(hud_font, "sound effects are on", hud_color);
	TTF_SizeText(hud_font, "sound effects are on", &text_length, &text_height);
	m_sfx_on_r.x = screen->w / 2 - text_length / 2;
	m_sfx_on_r.y = m_fullscreen_r.y + 50;
	SDL_BlitSurface(m_sfx_on, NULL, screen, &m_sfx_on_r);

	m_sfx_off = TTF_RenderText_Blended(hud_font, "sound effects are off", hud_color);
	TTF_SizeText(hud_font, "sound effects are off", &text_length, &text_height);
	m_sfx_off_r.x = screen->w / 2 - text_length / 2;
	m_sfx_off_r.y = m_fullscreen_r.y + 50;
	SDL_BlitSurface(m_sfx_off, NULL, screen, &m_sfx_off_r);

	m_back = TTF_RenderText_Blended(hud_font, "back", hud_color);
	TTF_SizeText(hud_font, "back", &text_length, &text_height);
	m_back_r.x = screen->w / 2 - text_length / 2;
	m_back_r.y = m_sfx_on_r.y + 50;
	SDL_BlitSurface(m_back, NULL, screen, &m_back_r);

	//rectangleRGBA(screen, -1, m_back_r.y - 3, screen->w, m_back_r.y+22, 255, 255, 0, 255);

	updateMenue = 1;

	while(!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
					case SDLK_UP:
						selected--;
						if (selected < 0) selected = 2;
						updateMenue = 1;
					break;
					case SDLK_DOWN:
						selected++;
						if (selected > 2) selected = 0;
						updateMenue = 1;
					break;
					case SDLK_RETURN:
					case SDLK_SPACE:
						if (settings.audio_sound) Mix_PlayChannel(-1, sfx_click, 0);
						switch(selected) {
							case 0:
								flags ^= SDL_FULLSCREEN;
								if (settings.video_fullscreen) {settings.video_fullscreen = 0;} else {settings.video_fullscreen = 1;}
								screen = SDL_SetVideoMode(screen->w, screen->h, screen->format->BitsPerPixel, flags);
								updateMenue = 1;
							break;
							case 1:
								// sfx is enabled, turn them off now
								if (settings.audio_sound) {
									settings.audio_sound = 0;
								// sfx are disabled, turn them on now
								} else {
									// audio was not initialized yet
									if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
										if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
											if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
												fprintf(stderr, "Unable to initialize Mixer! Deactivating audio.\n");
												settings.audio_music = 0;
												settings.audio_sound = 0;
												SDL_QuitSubSystem(SDL_INIT_AUDIO);
											} else {
												settings.audio_sound = 1;
											}
										} else {
											fprintf(stderr, "Unable to initialize audio!.\n");
										}
									// we don't need to initialize it again
									} else {
										settings.audio_sound = 1;
									}
									
								}
								updateMenue = 1;
							break;
							default:
								quit = 1;
							break;
						}
					break;
					case SDLK_ESCAPE:
						if (settings.audio_sound) Mix_PlayChannel(-1, sfx_click, 0);
						quit = 1;
					break;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (settings.audio_sound) Mix_PlayChannel(-1, sfx_click, 0);
				switch(selected) {
					case 0:
						flags ^= SDL_FULLSCREEN;
						if (settings.video_fullscreen) {settings.video_fullscreen = 0;} else {settings.video_fullscreen = 1;}
						screen = SDL_SetVideoMode(screen->w, screen->h, screen->format->BitsPerPixel, flags);
						updateMenue = 1;
					break;
					case 1:
						// sfx is enabled, turn them off now
						if (settings.audio_sound) {
							settings.audio_sound = 0;
						// sfx are disabled, turn them on now
						} else {
							// audio was not initialized yet
							if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
								if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
									if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
										fprintf(stderr, "Unable to initialize Mixer! Deactivating audio.\n");
										settings.audio_music = 0;
										settings.audio_sound = 0;
										SDL_QuitSubSystem(SDL_INIT_AUDIO);
									} else {
										settings.audio_sound = 1;
									}
								} else {
									fprintf(stderr, "Unable to initialize audio!.\n");
								}
							// we don't need to initialize it again
							} else {
								settings.audio_sound = 1;
							}
							
						}
						updateMenue = 1;
					break;
					default:
						quit = 1;
					break;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				if (collider_pixel(event.motion.x, event.motion.y, &m_fullscreen_r)) {
					selected = 0;
					updateMenue = 1;
				} else if (collider_pixel(event.motion.x, event.motion.y, &m_sfx_on_r)) {
					selected = 1;
					updateMenue = 1;
				} else if (collider_pixel(event.motion.x, event.motion.y, &m_back_r)) {
					selected = 2;
					updateMenue = 1;
				}
			}
			if (updateMenue) {
				SDL_BlitSurface(background, NULL, screen, NULL);
				switch(selected) {
					case 0:
						//rectangleRGBA(screen, -1, m_fullscreen_r.y - 3, screen->w, m_fullscreen_r.y+22, 255, 255, 0, 255);
						draw_rect(screen, 0, m_fullscreen_r.y - 3, screen->w-1, MENU_ITEM_HEIGHT, ylw_color);
					break;
					case 1:
						draw_rect(screen, 0, m_sfx_on_r.y - 3, screen->w-1, MENU_ITEM_HEIGHT, ylw_color);
					break;
					case 2:
						//rectangleRGBA(screen, -1, m_back_r.y - 3, screen->w, m_back_r.y+22, 255, 255, 0, 255);
						draw_rect(screen, 0, m_back_r.y - 3, screen->w-1, MENU_ITEM_HEIGHT, ylw_color);
					break;
				}
				SDL_BlitSurface(m_fullscreen, NULL, screen, &m_fullscreen_r);
				if (settings.audio_sound) {
					SDL_BlitSurface(m_sfx_on, NULL, screen, &m_sfx_on_r);
				} else {
					SDL_BlitSurface(m_sfx_off, NULL, screen, &m_sfx_off_r);
				}
				SDL_BlitSurface(m_back, NULL, screen, &m_back_r);
				SDL_Flip(screen);
				updateMenue = 0;
			}
		}
		SDL_Delay(5);
	}
	SDL_FreeSurface(m_fullscreen);
	SDL_FreeSurface(m_back);
}

unsigned int startSelected(int selected) {
	switch(selected) {
		case 0:
			game();
		break;
		case 1:
			SDL_BlitSurface(background, NULL, screen, NULL);
			show_highscores(0);
			SDL_Flip(screen);
			pressAnyKey();
		break;
		case 2:
			options_menue();
		break;
		case 3:
			SDL_BlitSurface(background, NULL, screen, NULL);
			showAbout();
		break;
	}
	return 0;
}


int main(int argc, char *argv[]) {
	SDL_Surface *m_start;
	SDL_Surface *m_scores;
	SDL_Surface *m_options;
	SDL_Surface *m_about;
	SDL_Surface *m_quit;
	SDL_Surface *icon;

	SDL_Rect m_start_r;
	SDL_Rect m_scores_r;
	SDL_Rect m_options_r;
	SDL_Rect m_about_r;
	SDL_Rect m_quit_r;


	unsigned int quit = 0;

	int selected = 0;
	int updateMenue = 0;

	char data_string[255];


	resetSettings();
	loadSettings();

	/* SDL initialisieren: VIDEO Subsystem */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		/* Init schlug fehl, Fehlermeldung ausgeben und beenden */
		fprintf(stderr, "SDL init failed:  %s\n", SDL_GetError());
		exit(1);
	}
	/* Beim Beenden des Programms aufräumen */
	atexit(SDL_Quit);

	/* if sound is enabled */
	if (settings.audio_music || settings.audio_sound) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
			fprintf(stderr, "unable to initialize audio subsystem! Deactivating audio.\n");
			settings.audio_music = 0;
			settings.audio_sound = 0;
		} else {
			fprintf(stderr, "audio subsystem loaded...\n");
			if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
				fprintf(stderr, "Unable to initialize Mixer! Deactivating audio.\n");
				settings.audio_music = 0;
				settings.audio_sound = 0;
				SDL_QuitSubSystem(SDL_INIT_AUDIO);
			} else {
				fprintf(stderr, "mixer loaded...\n");
			}
		}


	}

	if (settings.video_fullscreen) {
		flags = flags | SDL_FULLSCREEN;
	}

	/* Grafik Surface initialisieren mit 800x600 Pixel und 16Bit Farben */
	screen = SDL_SetVideoMode(VIDEO_X, VIDEO_Y , 16, flags);
	/* Init schlug fehl wenn NULL, Fehlermeldung ausgeben und beenden */
	if (screen == NULL) {
		fprintf(stderr, "Unable to open window: %s\n", SDL_GetError());
		exit(1);
	}

	if (TTF_Init() == -1) {
		fprintf(stderr, "Unable to init sdl_ttf: %s\n", TTF_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("ufoShooter", "ufoShooter");

	SDL_SetCursor(init_system_cursor(cursor));


	background = SDL_CreateRGBSurface(SDL_SWSURFACE, 800, 600, 16, 0, 0, 0, 255);

	srand(time(NULL));

	resetHighScore();
	loadHighScore();

	sprintf(data_string, "%sicon.gif", DATA_DIR);
	icon = IMG_Load(data_string);
	SDL_WM_SetIcon(icon, NULL);


	sprintf(data_string, "%shimmel.gif", DATA_DIR);
	sky = IMG_Load(data_string);

	sprintf(data_string, "%slandschaft1.gif", DATA_DIR);
	temp_surface =  IMG_Load(data_string);
	terrain_front = SDL_DisplayFormat(temp_surface);
	SDL_FreeSurface(temp_surface);
	transparent_color = SDL_MapRGB(background->format, 0xFF, 0, 0xFF);
	SDL_SetColorKey(terrain_front, SDL_SRCCOLORKEY | SDL_RLEACCEL, transparent_color);

	sprintf(data_string, "%slandschaft2.gif", DATA_DIR);
	temp_surface = IMG_Load(data_string);
	terrain_middle = SDL_DisplayFormat(temp_surface);
	SDL_FreeSurface(temp_surface);
	transparent_color = SDL_MapRGB(terrain_middle->format, 0xFF, 0, 0xFF);
	SDL_SetColorKey(terrain_middle, SDL_SRCCOLORKEY | SDL_RLEACCEL, transparent_color);

	sprintf(data_string, "%sbullet.png", DATA_DIR);
	temp_surface = IMG_Load(data_string);
	bullet = SDL_DisplayFormat(temp_surface);
	SDL_FreeSurface(temp_surface);
	transparent_color = SDL_MapRGB(bullet->format, 0xFF, 0, 0xFF);
	SDL_SetColorKey(bullet, SDL_SRCCOLORKEY | SDL_RLEACCEL, transparent_color);

	sprintf(data_string, "%sufo.gif", DATA_DIR);
	temp_surface = IMG_Load(data_string);
	ufo = SDL_DisplayFormat(temp_surface);
	SDL_FreeSurface(temp_surface);
	transparent_color = SDL_MapRGB(ufo->format, 0xFF, 0, 0xFF);
	SDL_SetColorKey(ufo, SDL_SRCCOLORKEY | SDL_RLEACCEL, transparent_color);

	sprintf(data_string, "%shouse_01.gif", DATA_DIR);
	temp_surface = IMG_Load(data_string);
	house_01 = SDL_DisplayFormat(temp_surface);
	SDL_FreeSurface(temp_surface);
	transparent_color = SDL_MapRGB(house_01->format, 0xFF, 0, 0xFF);
	SDL_SetColorKey(house_01, SDL_SRCCOLORKEY | SDL_RLEACCEL, transparent_color);

	
	fprintf(stderr, "ufo original %dx%d\n", ufo->w, ufo->h);


	temp_surface = ScaleSurface(ufo, ufo->w/2, ufo->h/2);
	ufo_small = SDL_DisplayFormat(temp_surface);
	SDL_FreeSurface(temp_surface);
	transparent_color = SDL_MapRGB(ufo_small->format, 0xFF, 0, 0xFF);
	SDL_SetColorKey(ufo_small, SDL_SRCCOLORKEY | SDL_RLEACCEL, transparent_color);
	fprintf(stderr, "ufo_small %dx%d\n", ufo_small->w, ufo_small->h);

	temp_surface = ScaleSurface(ufo, ufo->w/3, ufo->h/3);
	ufo_smaller = SDL_DisplayFormat(temp_surface);
	SDL_FreeSurface(temp_surface);
	transparent_color = SDL_MapRGB(ufo_smaller->format, 0xFF, 0, 0xFF);
	SDL_SetColorKey(ufo_smaller, SDL_SRCCOLORKEY | SDL_RLEACCEL, transparent_color);
	fprintf(stderr, "ufo_smaller %dx%d\n", ufo_smaller->w, ufo_smaller->h);

	/*ufo_small = shrinkSurface(ufo, factorx_small, factory_small);
	ufo_smaller = shrinkSurface(ufo, (int)3, (int)3);*/

	sprintf(data_string, "%sdsshotgn.wav", DATA_DIR);
	sfx_fire = Mix_LoadWAV(data_string);
	if (sfx_fire == NULL) {
		fprintf(stderr, "unable to load %s\n", data_string);
	}

	sprintf(data_string, "%sdswpnup.wav", DATA_DIR);
	sfx_reload = Mix_LoadWAV(data_string);
	if (sfx_reload == NULL) {
		fprintf(stderr, "unable to load %s\n", data_string);
	}

	sprintf(data_string, "%sclick1.wav", DATA_DIR);
	sfx_click = Mix_LoadWAV(data_string);
	if (sfx_click == NULL) {
		fprintf(stderr, "unable to load %s\n", data_string);
	}

	terrain_front_r.x = 0;//terrain_front->w / 3;
	terrain_front_r.y = 0;
	terrain_front_r.w = 800;
	terrain_front_r.h = 600;
	terrain_middle_r.x = 0;//terrain_middle->w / 4;
	terrain_middle_r.y = 0;
	terrain_middle_r.w = 800;
	terrain_middle_r.h = 600;

	terrain_front_r_dest.x = 0;
	terrain_front_r_dest.y = 600 - terrain_front->h;
	terrain_front_r_dest.w = 800;
	terrain_front_r_dest.h = 600;
	terrain_middle_r_dest.x = 0;
	terrain_middle_r_dest.y = 600 - terrain_middle->h;
	terrain_middle_r_dest.w = 800;
	terrain_middle_r_dest.h = 600;

	SDL_BlitSurface(sky, NULL, background, NULL);
	SDL_BlitSurface(terrain_middle, &terrain_middle_r, background, &terrain_middle_r_dest);
	SDL_BlitSurface(terrain_front, &terrain_front_r, background, &terrain_front_r_dest);
	SDL_BlitSurface(background, NULL, screen, NULL);

	sprintf(data_string, "%sAction Man.ttf", DATA_DIR);


	hud_font = TTF_OpenFont(data_string, 28);
	if (hud_font == NULL) {
		fprintf(stderr, "Unable to open font: %s\n", TTF_GetError());
		exit(1);
	}

	hud_font_big = TTF_OpenFont(data_string, 48);

	m_start = TTF_RenderText_Blended(hud_font, "start", hud_color);
	TTF_SizeText(hud_font, "start", &text_length, &text_height);
	m_start_r.x = screen->w / 2 - text_length / 2;
	m_start_r.y = 150;
	SDL_BlitSurface(m_start, NULL, screen, &m_start_r);

	m_scores = TTF_RenderText_Blended(hud_font, "highscore", hud_color);
	TTF_SizeText(hud_font, "highscore", &text_length, &text_height);
	m_scores_r.x = screen->w / 2 - text_length / 2;
	m_scores_r.y = m_start_r.y + 50;
	SDL_BlitSurface(m_scores, NULL, screen, &m_scores_r);

	m_options = TTF_RenderText_Blended(hud_font, "options", hud_color);
	TTF_SizeText(hud_font, "options", &text_length, &text_height);
	m_options_r.x = screen->w / 2 - text_length / 2;
	m_options_r.y = m_scores_r.y + 50;
	SDL_BlitSurface(m_options, NULL, screen, &m_options_r);

	m_about = TTF_RenderText_Blended(hud_font, "about", hud_color);
	TTF_SizeText(hud_font, "about", &text_length, &text_height);
	m_about_r.x = screen->w / 2 - text_length / 2;
	m_about_r.y = m_options_r.y + 50;
	SDL_BlitSurface(m_about, NULL, screen, &m_about_r);

	m_quit = TTF_RenderText_Blended(hud_font, "quit", hud_color);
	TTF_SizeText(hud_font, "quit", &text_length, &text_height);
	m_quit_r.x = screen->w / 2 - text_length / 2;
	m_quit_r.y = m_about_r.y + 50;
	SDL_BlitSurface(m_quit, NULL, screen, &m_quit_r);


//	SDL_Flip(screen);

	updateMenue = 1;
	selected = 0;
	//rectangleRGBA(screen, -1, m_start_r.y - 3, screen->w, m_start_r.y+22, 255, 255, 0, 255);

	while(!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
					case SDLK_UP:
						selected--;
						if (selected < 0) selected = 4;
						updateMenue = 1;
					break;
					case SDLK_DOWN:
						selected++;
						if (selected > 4) selected = 0;
						updateMenue = 1;
					break;
					case SDLK_RETURN:
					case SDLK_SPACE:
						if (selected < 4) {
							if (settings.audio_sound) Mix_PlayChannel(-1, sfx_click, 0);
							startSelected(selected);
							updateMenue = 1;
						} else {
							if (settings.audio_sound) Mix_PlayChannel(-1, sfx_click, 0);
							quit = 1;
						}
					break;
					case SDLK_ESCAPE:
						if (settings.audio_sound) Mix_PlayChannel(-1, sfx_click, 0);
						quit = 1;
					break;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (selected < 4) {
					if (settings.audio_sound) Mix_PlayChannel(-1, sfx_click, 0);
					startSelected(selected);
					updateMenue = 1;
				} else {
					if (settings.audio_sound) Mix_PlayChannel(-1, sfx_click, 0);
					quit = 1;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				if (collider_pixel(event.motion.x, event.motion.y, &m_start_r)) {
					selected = 0;
					updateMenue = 1;
				} else if (collider_pixel(event.motion.x, event.motion.y, &m_scores_r)) {
					selected = 1;
					updateMenue = 1;
				} else if (collider_pixel(event.motion.x, event.motion.y, &m_options_r)) {
					selected = 2;
					updateMenue = 1;
				} else if (collider_pixel(event.motion.x, event.motion.y, &m_about_r)) {
					selected = 3;
					updateMenue = 1;
				} else if (collider_pixel(event.motion.x, event.motion.y, &m_quit_r)) {
					selected = 4;
					updateMenue = 1;
				}
			}
			if (updateMenue) {
				SDL_BlitSurface(background, NULL, screen, NULL);
				switch(selected) {
					case 0:
						//rectangleRGBA(screen, -1, m_start_r.y - 3, screen->w, m_start_r.y+22, 255, 255, 0, 255);
						draw_rect(screen, 0, m_start_r.y - 3, screen->w-1, MENU_ITEM_HEIGHT, ylw_color);
					break;
					case 1:
						//rectangleRGBA(screen, -1, m_scores_r.y - 3, screen->w, m_scores_r.y+22, 255, 255, 0, 255);
						draw_rect(screen, 0, m_scores_r.y - 3, screen->w-1, MENU_ITEM_HEIGHT, ylw_color);
					break;
					case 2:
						//rectangleRGBA(screen, -1, m_options_r.y - 3, screen->w, m_options_r.y+22, 255, 255, 0, 255);
						draw_rect(screen, 0, m_options_r.y - 3, screen->w-1, MENU_ITEM_HEIGHT, ylw_color);
					break;
					case 3:
						//rectangleRGBA(screen, -1, m_about_r.y - 3, screen->w, m_about_r.y+22, 255, 255, 0, 255);
						draw_rect(screen, 0, m_about_r.y - 3, screen->w-1, MENU_ITEM_HEIGHT, ylw_color);
					break;
					case 4:
						//rectangleRGBA(screen, -1, m_quit_r.y - 3, screen->w, m_quit_r.y+22, 255, 255, 0, 255);
						draw_rect(screen, 0, m_quit_r.y - 3, screen->w-1, MENU_ITEM_HEIGHT, ylw_color);
					break;
				}
				SDL_BlitSurface(m_start, NULL, screen, &m_start_r);
				SDL_BlitSurface(m_scores, NULL, screen, &m_scores_r);
				SDL_BlitSurface(m_options, NULL, screen, &m_options_r);
				SDL_BlitSurface(m_about, NULL, screen, &m_about_r);
				SDL_BlitSurface(m_quit, NULL, screen, &m_quit_r);
				SDL_Flip(screen);
				updateMenue = 0;
			}
		}
		SDL_Delay(5);
	}

	writeHighScore();
	writeSettings();

	if ((settings.audio_sound) || (settings.audio_music)) {
		Mix_FreeChunk(sfx_fire);
		Mix_FreeChunk(sfx_reload);
		Mix_CloseAudio();
	}

	fprintf(stderr, "freeing... screen...");
	SDL_FreeSurface(screen);
	fprintf(stderr, "background...");
	SDL_FreeSurface(background);
	fprintf(stderr, "terrain_middle...");
	SDL_FreeSurface(terrain_middle);
	fprintf(stderr, "terrain_fron...");
	SDL_FreeSurface(terrain_front);
	fprintf(stderr, "bullet...");
	SDL_FreeSurface(bullet);
	fprintf(stderr, "sky...");
	SDL_FreeSurface(sky);
	fprintf(stderr, "house_01...");
	SDL_FreeSurface(house_01);
	fprintf(stderr, "ufo...");
	SDL_FreeSurface(ufo);
	fprintf(stderr, "ufo_small...");
	SDL_FreeSurface(ufo_small);
	fprintf(stderr, "ufo_smaller...");
	SDL_FreeSurface(ufo_smaller);
	fprintf(stderr, "icon...");
	SDL_FreeSurface(icon);
	fprintf(stderr, "m_start...");
	SDL_FreeSurface(m_start);
	fprintf(stderr, "m_scores...");
	SDL_FreeSurface(m_scores);
	fprintf(stderr, "m_options...");
	SDL_FreeSurface(m_options);
	fprintf(stderr, "about...");
	SDL_FreeSurface(m_about);
	fprintf(stderr, "m_quit.\n");
	SDL_FreeSurface(m_quit);
	return 0;
}


