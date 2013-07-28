#ifndef LC3GUI_H
#define LC3GUI_H

#include <ncurses.h>

#define NUM_WINDOWS 4
#define REGWIN_WIDTH 25
#define REGWIN_HEIGHT 14
#define DEBUGWIN_HEIGHT 6
#define CONSOLE_SIZE (LINES-DEBUGWIN_HEIGHT-REGWIN_HEIGHT-3)*(REGWIN_WIDTH-2)
#define WINDOW_PADDING 2

#define MEMWIN windows[0]
#define REGWIN windows[1]
#define CNSWIN windows[2]
#define DBGWIN windows[3]

static WINDOW* windows[NUM_WINDOWS];
static int windex;
static unsigned short mem_index;
static unsigned short mem_cursor;
static int memwin_state;
static int dbgwin_state;
static int cnswin_state;
static int key_wait;

void build_symbol_table(const char* filename);
//char* getsym(unsigned short addr);

WINDOW* create_win(int height, int width, int starty, int startx);
void destroy_win(WINDOW* local_win);
void initialize();
void refreshall();

void update_memwin();
void update_regwin();
void update_dbgwin();
void update_cnswin();

void wait_for_key(int print);

void hex_to_binstr(short hex, char* buffer);
void dbggetstrw(int y, int x, const char* prompt, char* buffer);

#endif
