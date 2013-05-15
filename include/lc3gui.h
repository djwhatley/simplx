#ifndef LC3GUI_H
#define LC3GUI_H

#include <ncurses.h>

#define NUM_WINDOWS 4
#define REGWIN_WIDTH 25
#define REGWIN_HEIGHT 14
#define DEBUGWIN_HEIGHT 5
#define WINDOW_PADDING 2

#define MEMWIN windows[0]
#define REGWIN windows[1]
#define CNSWIN windows[2]
#define DBGWIN windows[3]

static WINDOW* windows[NUM_WINDOWS];
static int windex;
static int mem_index;

WINDOW* create_win(int height, int width, int starty, int startx);
void destroy_win(WINDOW* local_win);
void initialize();
void refreshall();

void update_memwin();
void update_regwin();

void hex_to_binstr(short hex, char* buffer);

#endif
