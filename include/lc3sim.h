#ifndef LC3SIM_H
#define LC3SIM_H

#include <stdio.h>

#define OPCODE_MASK 0xF000
#define NZP_MASK 0x0E00
#define DEST_MASK 0x0E00
#define SRC1_MASK 0x01C0
#define SRC2_MASK 0x0007
#define IMM5_MASK 0x001F
#define PC9_MASK 0x01FF
#define PC11_MASK 0x07FF
#define OFF6_MASK 0x003F
#define TRAP_MASK 0x00FF
#define JSRR_MASK 0x0800
#define IMMF_MASK 0x0020

#define OPCODE_SHFT 12
#define NZP_SHFT 9
#define DEST_SHFT 9
#define SRC1_SHFT 6
#define SRC2_SHFT 0
#define IMM5_SHFT 0
#define PC9_SHFT 0
#define PC11_SHFT 0
#define OFF6_SHFT 0
#define TRAP_SHFT 0
#define JSRR_SHFT 11
#define IMMF_SHFT 5

#define KBSR mem[0xFE00]
#define KBDR mem[0xFE02]
#define DSR mem[0xFE04]
#define DDR mem[0xFE06]
#define MCR mem[0xFFFE]

typedef enum {BR, ADD, LD, ST, JSR, AND, LDR, STR, RTI, NOT, LDI, STI, JMP, LOLFENDERCODE, LEA, TRAP} opcode_t;

typedef struct {
	opcode_t opcode;
	char nzpbits;
	char destreg;
	char src1reg;
	char src2reg;
	short imm5;
	short pcoffset9;
	short pcoffset11;
	short offset6;
	short trapvect;
	char jsrr_flag;
	char imm5_flag;
} lc3inst_t;

unsigned short regfile[8];
unsigned short pc;
unsigned short ir;
short cc;
unsigned short mem[65536];
unsigned char brk[65536];
unsigned char* console;
unsigned int executions;
int cns_index;
int cns_length;
int cns_max;

int running;
int halted;
int first;
short next;
lc3inst_t next_inst;

int enable_udiv;

short get_instruction();
void decode_instruction(lc3inst_t* instruction, short raw_inst);
void execute_instruction(lc3inst_t* instruction);
void setcc(short writeval);
char comparenzp(char nzp);
short signext(char value, char bits);
void show_register_contents();
void send_to_console(char c);
void read_program(FILE* program);

void run_program();
void step_forward();
void set_breakpoint(unsigned short address);
void unset_breakpoint(unsigned short address);
void reset_program(FILE* program);

void disassemble_to_str(short inst, char* buffer);

#endif
