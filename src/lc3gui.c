#include <ncurses.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "../include/lc3sim.h"
#include "../include/lc3gui.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Bad argument! Just give me a filename of a compiled assembly program.\n");
		return -EINVAL;
	}

	enable_udiv = 1;

	FILE* program;
	if (!(program = fopen(argv[1], "r")))
	{
		printf("Bad argument! File not found.\n");
		return -EINVAL;
	}

	build_symbol_table(argv[1]);

	pc = 0x3000;
	running = 1;
	read_program(program);

	int ch;
	initialize();

	while(ch != KEY_F(1))
	{
		ch = getch();
		switch (ch) {
		case KEY_F(2):
			reset_program(program);
			break;
		case KEY_F(3):
			dbgwin_state = 1;
			break;
		case KEY_F(4):
			break;
		case KEY_F(5):
			step_forward();
			break;
		case KEY_F(6):
			run_program();
			break;
		case KEY_F(7):
			memwin_state = (memwin_state == 2 ? 0 : 2);
			mem_cursor = mem_index;
			break;
		case KEY_F(8):
			dbgwin_state = 2;
			break;
		case KEY_UP:
			if (memwin_state == 2)
				mem_cursor--;
			break;
		case KEY_DOWN:
			if (memwin_state == 2)
				mem_cursor++;
			break;
		case KEY_NPAGE:
			if (memwin_state == 2)
				mem_cursor += (LINES-DEBUGWIN_HEIGHT);
			break;
		case KEY_PPAGE:
			if (memwin_state == 2)
				mem_cursor -= (LINES-DEBUGWIN_HEIGHT);
			break;
		case 0xA:
			if (memwin_state ==2)
				brk[(unsigned short)mem_cursor] ^= 1;
			break;
		}
		refreshall();
	}
quit:
	curs_set(1);
	endwin();
	return 0;
}

void build_symbol_table(const char* filename)
{
	int namelength = strlen(filename);
	char* symbolfile;

	if (!(symbolfile = malloc(namelength)))
	{
		printf("Malloc returned NULL! That's no good!\n");
		exit(-ENOMEM);
	}

	strncpy(symbolfile, filename, namelength-4);
	symbolfile[namelength-4] = '.';
	symbolfile[namelength-3] = 's';
	symbolfile[namelength-2] = 'y';
	symbolfile[namelength-1] = 'm';

	FILE* symbols;
	if (!(symbols = fopen(symbolfile, "r")))
	{
		printf("Couldn't find the symbol file!\n");
		exit(-ENOENT);
	}

	//printf("\n\n%s\n\n", symbolfile);
	//while(1);
	unsigned short address;
	char* symbol;
	while (!feof(symbols))
	{
		symbol = (char*)malloc(16);
		fscanf(symbols, "%4hx", &address);
		printf("%4hx ", address); 
		fscanf(symbols, "%s", symbol);
		printf("%s\n", symbol); 
		syms[address] = symbol;
	}

	free(symbolfile);
	fclose(symbols);
}
/*
char* getsym(unsigned short addr)
{
	return syms[addr] ? syms[addr] : "";
}
*/
void initialize()
{
	initscr();
	cbreak();
	curs_set(0);
	keypad(stdscr, TRUE);
	
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_GREEN);

	refresh();

	WINDOW* mem_win, *reg_win, *cns_win, *dbg_win;

	mem_win = create_win(LINES-DEBUGWIN_HEIGHT, COLS-REGWIN_WIDTH, 0, 0);
	reg_win = create_win(REGWIN_HEIGHT, REGWIN_WIDTH, 0, COLS-REGWIN_WIDTH);
	cns_win = create_win(LINES-REGWIN_HEIGHT-DEBUGWIN_HEIGHT, REGWIN_WIDTH, REGWIN_HEIGHT, COLS-REGWIN_WIDTH);
	dbg_win = create_win(DEBUGWIN_HEIGHT, COLS, LINES-DEBUGWIN_HEIGHT, 0);

	wprintw(mem_win, "Memory Viewer");
	wprintw(reg_win, "Registers");
	wprintw(cns_win, "Console");
	wprintw(dbg_win, "Debugging");
	
	dbgwin_state = 0;
	memwin_state = 0;
	cnswin_state = 0;

	console = (char*)malloc(CONSOLE_SIZE);
	cns_index = 0;
	cns_max = CONSOLE_SIZE;

	refreshall();
}

WINDOW* create_win(int height, int width, int y, int x)
{
	WINDOW* win;

	win = newwin(height, width, y, x);
	box(win, 0, 0);

	wrefresh(win);

	windows[windex] = win;
	windex++;

	return win;
}

void refreshall()
{
	int i;
	update_memwin();
	update_regwin();
	update_dbgwin();
	update_cnswin();
	for (i=0; i<windex; i++)
		wrefresh(windows[i]);
	refresh();
}


void update_memwin()
{
	int i;
	if (!memwin_state)
		mem_index = pc-1;
	else if (memwin_state == 2)
		mem_index = mem_cursor;
	for (i; i<LINES-DEBUGWIN_HEIGHT-WINDOW_PADDING*2; i++)
	{
		unsigned short addr = mem_index-(LINES-DEBUGWIN_HEIGHT-WINDOW_PADDING)/2+i;
		short curr = mem[(unsigned short)addr];
		char binstring[20];
		char disasmstr[35];
		hex_to_binstr(curr, binstring);
		disassemble_to_str(curr, disasmstr);
		if (mem_cursor == addr && memwin_state == 2)
			wattron(MEMWIN, COLOR_PAIR(1));
		else if (pc-1 == addr)
			wattron(MEMWIN, A_STANDOUT);
		int c;
		for (c=0; c<COLS-REGWIN_WIDTH-WINDOW_PADDING*2; c++)
			mvwprintw(MEMWIN, i+WINDOW_PADDING, c+WINDOW_PADDING, " ");
		mvwprintw(MEMWIN, i+WINDOW_PADDING, WINDOW_PADDING, "%c x%.4hx\t x%.4hx\t %.5d\t %s\t %s\t  %s", brk[addr] ? '@' : ' ', addr, curr, curr, binstring, disasmstr, syms[addr] ? syms[addr] : "");
		wattroff(MEMWIN, A_STANDOUT);
		wattroff(MEMWIN, COLOR_PAIR(1));
	}
	if (memwin_state != 2)
		memwin_state = 0;
}

void update_regwin()
{
	int i;
	for (i=0; i<4; i++)
		mvwprintw(REGWIN, i+WINDOW_PADDING, WINDOW_PADDING, "R%d: x%.4hx | R%d: x%.4hx", 2*i, regfile[2*i], 2*i+1, regfile[2*i+1]);
	mvwprintw(REGWIN, WINDOW_PADDING+6, WINDOW_PADDING, "PC: x%.4hx", pc);
	mvwprintw(REGWIN, WINDOW_PADDING+7, WINDOW_PADDING, "IR: x%.4hx", ir);
	mvwprintw(REGWIN, WINDOW_PADDING+8, WINDOW_PADDING, "CC: x%.4hx", cc);
	mvwprintw(REGWIN, WINDOW_PADDING+9, WINDOW_PADDING, "EX: %d", executions);
}

void update_dbgwin()
{
	int i, j;
	char goaddr[6];
	char realaddr[7];
	for (i=0; i<DEBUGWIN_HEIGHT-WINDOW_PADDING*2; i++)
		for (j=0; j<COLS-WINDOW_PADDING*2; j++)
			mvwprintw(DBGWIN, i+WINDOW_PADDING, j+WINDOW_PADDING, " ");
	switch (dbgwin_state) {
	case 0:
		mvwprintw(DBGWIN, WINDOW_PADDING, WINDOW_PADDING, "F5 - Step | F6 - Run | F7 - Memory Explorer | F1 - Exit");
		break;
	case 1:
		dbggetstrw(WINDOW_PADDING, WINDOW_PADDING, "Goto address: ", goaddr);
		realaddr[0] = '0';
		strcpy(&realaddr[1], goaddr);
		sscanf(realaddr, "%x", &mem_index);
		dbgwin_state = 0;
		if (!memwin_state)
			memwin_state = 1;
		else if (memwin_state == 2)
			mem_cursor = mem_index;
		refreshall();
		break;
	case 2:
		dbggetstrw(WINDOW_PADDING, WINDOW_PADDING, "New value: ", goaddr);
		realaddr[0] = '0';
		strcpy(&realaddr[1], goaddr);
		short a;
		sscanf(realaddr, "%x", &a);
		mem[mem_cursor] = a;
		dbgwin_state = 0;
		refreshall();
		break;
	}
}

void update_cnswin()
{
	int i, j;
	for (i = cns_index; i != (cns_index ? cns_index-1 : cns_length); i++)
	{
		j = i - cns_index;
		mvwaddch(CNSWIN, j/(REGWIN_WIDTH-2)+1, j%(REGWIN_WIDTH-2)+1, console[i]);
	}
}

void wait_for_key(int print)
{
	refreshall();
	char ch;
	while(!(ch=getch()));
	regfile[0] = (short)ch;
	if (print) send_to_console(ch);
}

void hex_to_binstr(short hex, char* buffer)
{
	char* strings[16] = { "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111" };

	int a = (hex & 0xF000) >> 12;
	int b = (hex & 0x0F00) >> 8;
	int c = (hex & 0x00F0) >> 4;
	int d = (hex & 0x000F) >> 0;

	sprintf(buffer, "%s %s %s %s", strings[a], strings[b], strings[c], strings[d]);
}

void dbggetstrw(int y, int x, const char* prompt, char* buffer)
{
	mvwprintw(DBGWIN, y, x, prompt);
	wrefresh(DBGWIN);
	move(LINES-DEBUGWIN_HEIGHT+y, x+strlen(prompt));

	curs_set(1);
	getstr(buffer);
	curs_set(0);
}
