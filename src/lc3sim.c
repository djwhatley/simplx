/**
 * @file		lc3sim.c
 * @author	Daniel Whatley
 * @date		May, 2013
 * @brief		C file for lc3 simulator
 *
 * This file contains the code to simulate the LC-3 computer.
 * Note that this is a simulator, not an emulator; it is only required to simulate the output, not the actual internal workings of the system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lc3sim.h"

static FILE* file;

/**
 * @name 	Set Condition Code
 * @param [short] writeval Value to write to the CC (negative, zero, or positive)
 */
void setcc(short writeval)
{
	if (writeval < 0)
		cc = -1;
	else if (writeval == 0)
		cc = 0;
	else
		cc = 1;
}

/**
 * @name 	Compare NZP
 * @param [char] nzp An NZP condition from a branch instruction
 * @retval 1	condition code met specified NZP condition
 * @retval 0	condition code did not meet specified NZP condition
 */
char comparenzp(char nzp)
{
	if (nzp == 7)									// nzp (always true)
		return 1;
	else if (nzp == 6 && cc <= 0) // nz
		return 1;
	else if (nzp == 5 && cc != 0)	// np
		return 1;
	else if (nzp == 4 && cc < 0)	// n
		return 1;
	else if (nzp == 3 && cc >= 0)	// zp
		return 1;
	else if (nzp == 2 && cc == 0)	// z
		return 1;
	else if (nzp == 1 && cc > 0)	// p
		return 1;
	return 0;
}

/**
 * @name 	Sign Extend
 * @brief Extends a value to 16 bits
 * @param [short] value The value to be sign-extended
 * @param [char] 	bits 	The number of bits in the value (starting from the right) that should be extended
 * @retval The given value extended to 16 bits
 */
short signext(short value, char bits)
{
	short mask1 = 1 << bits;							// Make a mask to check the highest bit in the specified range
	short mask2 = (short) 0xFFFF << bits;	// Make a mask to set the high bits of the return value if negative
	short ret = 0;
	if ((short)value & mask1)							// If the highest bit in rhe range is 1, then it's a negative number
		ret = (short) mask2 | value;				// Use the second mask to extend the sign bit
	else
		ret = (short) ret | value;					// Otherwise just return the larger number.
	return ret;
}

/**
 * @name 	Fetch Instruction
 * @brief Fetches the next instruction to be executed
 * @retval The next instruction
 */
short fetch_instruction()
{
	ir = mem[pc];
	pc++;
	return ir;
}

/**
 * @name 	Decode Instruction
 * @brief Fills in the various values from a raw instruction
 * @param [lc3inst_t*] instruction 	A pointer to the instruction struct to be filled
 * @param [short] raw_inst	The raw instruction value as a 16-bit number
 */
void decode_instruction(lc3inst_t* instruction, short raw_inst)
{
	instruction->opcode = (raw_inst & OPCODE_MASK) >> OPCODE_SHFT;
	instruction->nzpbits = (raw_inst & NZP_MASK) >> NZP_SHFT;
	instruction->destreg = (raw_inst & DEST_MASK) >> DEST_SHFT;
	instruction->src1reg = (raw_inst & SRC1_MASK) >> SRC1_SHFT;
	instruction->src2reg = (raw_inst & SRC2_MASK) >> SRC2_SHFT;
	instruction->imm5 = signext((raw_inst & IMM5_MASK) >> IMM5_SHFT, 4);
	instruction->pcoffset9 = signext((raw_inst & PC9_MASK) >> PC9_SHFT, 8);
	instruction->pcoffset11 = signext((raw_inst & PC11_MASK) >> PC11_SHFT, 10);
	instruction->offset6 = signext((raw_inst & OFF6_MASK) >> OFF6_SHFT, 5);
	instruction->trapvect = (raw_inst & TRAP_MASK) >> TRAP_SHFT;
	instruction->jsrr_flag = !((raw_inst & JSRR_MASK) >> JSRR_SHFT);
	instruction->imm5_flag = (raw_inst & IMMF_MASK) >> IMMF_SHFT;
}

/** name	Execute Instruction
 * @brief Executes an Lc-3 instruction
 * @param [lc3inst_t*] instruction	A pointer to the instruction to be executed
 */
void execute_instruction(lc3inst_t* instruction)
{
	short old_pc;
	short old_reg0;
	switch (instruction->opcode) {
	// Branch
	case BR:
		if (comparenzp(instruction->nzpbits))
			pc += instruction->pcoffset9;
		break;
	// Add
	case ADD:
		if (instruction->imm5_flag)
			regfile[instruction->destreg] = regfile[instruction->src1reg] + instruction->imm5;
		else
			regfile[instruction->destreg] = regfile[instruction->src1reg] + regfile[instruction->src2reg];
		setcc(regfile[instruction->destreg]);
		break;
	// Load
	case LD:
		regfile[instruction->destreg] = mem[pc+instruction->pcoffset9];
		setcc(regfile[instruction->destreg]);
		break;
	// Store
	case ST:
		mem[pc+instruction->pcoffset9] = regfile[instruction->destreg];
		break;
	// Jump to Subroutine
	case JSR:
		old_pc = pc;
		if (instruction->jsrr_flag)
			pc = regfile[instruction->src1reg];
		else
			pc = pc+instruction->pcoffset11;
		regfile[7] = old_pc;
		break;
	// And
	case AND:
		if (instruction->imm5_flag)
			regfile[instruction->destreg] = regfile[instruction->src1reg] & instruction->imm5;
		else
			regfile[instruction->destreg] = regfile[instruction->src1reg] & regfile[instruction->src2reg];
		setcc(regfile[instruction->destreg]);
		break;
	// Load Register
	case LDR:
		regfile[instruction->destreg] = mem[regfile[instruction->src1reg] + instruction->offset6];
		setcc(regfile[instruction->destreg]);
		break;
	// Store Register
	case STR:
		mem[regfile[instruction->src1reg] + instruction->offset6] = regfile[instruction->destreg];
		break;
	// Return from Interrupt
	case RTI:
		break;
	// Not
	case NOT:
		regfile[instruction->destreg] = ~regfile[instruction->src1reg];
		setcc(regfile[instruction->destreg]);
		break;
	// Load Indirect
	case LDI:
		regfile[instruction->destreg] = mem[mem[pc+instruction->pcoffset9]];
		setcc(regfile[instruction->destreg]);
		break;
	// Store Indirect
	case STI:
		mem[mem[pc+instruction->pcoffset9]] = regfile[instruction->destreg];
		break;
	// Jump
	case JMP:
		old_pc = pc;
		pc = regfile[instruction->src1reg];
		regfile[7] = old_pc;
		break;
	// Load Effective Address
	case LEA:
		regfile[instruction->destreg] = pc + instruction->pcoffset9;
		setcc(regfile[instruction->destreg]);
		break;
	// Trap
	case TRAP:
		switch (instruction->trapvect) {
		// GETC
		case 0x20:
			wait_for_key(0);
			break;
		// OUT
		case 0x21:
			send_to_console((char)regfile[0]);
			break;
		// PUTS
		case 0x22:
			old_reg0 = regfile[0];
			while (mem[regfile[0]])
			{
				send_to_console((char)mem[regfile[0]]);
				regfile[0]++;
			}
			regfile[0] = old_reg0;
			break;
		// IN
		case 0x23:
			wait_for_key(1);
			break;
		// HALT
		case 0x25:
			halted = 1;
			running = 0;
			break;
		// UDIV
		case 0x80:
			if (enable_udiv)
			{
				if (!regfile[1])
					break;
				unsigned short temp = regfile[0]/regfile[1];
				regfile[1] = regfile[0]%regfile[1];
				regfile[0] = temp;
			}
			break;
		// Generic trap
		default:
			old_pc = pc;
			pc = mem[instruction->trapvect];
			regfile[7] = old_pc;
			break;
		}
		break;
	}
	executions++;
}

/**
 * @name 	Send to Console
 * @brief Sends a character to the console to be printed
 * @param [const char] c The character to be sent to the console
 */
void send_to_console(const char c)
{
	static int cindex = 0;
	console[cindex] = c;
	cindex++;
	if (cns_length < cns_max)		// Update the length of the text in the console, if it isn't at max capacity
		cns_length++;
	if (cindex-1 >= cns_length)	// Make sure the console index isn't greater than the console size
		cindex %= cns_length;
}

/**
 * @name 	Read Program
 * @brief Reads an assembled LC-3 program from a file
 * @param [FILE*] program Pointer to the assembled LC-3 program as an opened FILE
 */
void read_program(FILE* program)
{
	unsigned short a = 1;	// beautiful variable names
	unsigned short address;
	unsigned char num = 0;
	unsigned short b;	// exquisite

	rewind(program); // Be kind, rewind!

	// This loop reads the format of the object file into two 
	while(a != 0xffff)
	{
		a = ((fgetc(program) << 8) | fgetc(program));
		if (!num) // The first number is actually an address, not an instruction.
		{
			address = a;	// Set the address
			a = ((fgetc(program) << 8) | fgetc(program));	// The second number is how many instructions are next
			num = a;	// Set the number of following instructions
			b = address + num;	// How to derive the addresses of the other things in the else block
		} else {
			mem[b-num] = a;	// Derives the address of the instruction
			num--;	// One fewer instruction to process before the next block (or the end)
		}
	}

	// Set up some console stuff
	cns_index = 0;
	cns_length = 0;

	// Prefetch and decode the first instruction so the PC and IR values accurately reflect the current state of the machine
	next = fetch_instruction();
	decode_instruction(&next_inst, next);
}

void reset_program(FILE* program)
{
	pc = 0x3000;
	running = 1;
	halted = 0;
	executions = 0;
	read_program(program);
	
	int i;
	for(i=0; i<8; i++)
		regfile[i] = 0;

	cc = 0;
}

void run_program()
{
	running = 1;
	while (running && !halted)
	{
		if (brk[pc-1] == 1)
		{
			running = 0;
			brk[pc-1] = 2;
			break;
		}
		step_forward();
		if (brk[pc-1] == 2)
			brk[pc-1] = 1;
	}
}

void step_forward()
{
	if (halted)
		return;
	
	execute_instruction(&next_inst);
	if (halted) return;
	next = fetch_instruction();
	decode_instruction(&next_inst, next);

}

void set_breakpoint(unsigned short address)
{
	brk[address] = 1;
}

void unset_breakpoint(unsigned short address)
{
	brk[address] = 0;
}

void disassemble_to_str(short instruction, char* buffer)
{
	lc3inst_t inst;
	decode_instruction(&inst, instruction);
	char* nzpstrings[8] = { "", "p", "z", "zp", "n", "np", "nz", "nzp" };

	switch(inst.opcode) {
	case BR:
		if (!inst.nzpbits)
			sprintf(buffer, "NOP");
		else
			sprintf(buffer, "BR%s #%d", nzpstrings[inst.nzpbits], inst.pcoffset9);
		break;
	case ADD:
		if (inst.imm5_flag)
			sprintf(buffer, "ADD R%d, R%d, #%d", inst.destreg, inst.src1reg, inst.imm5);
		else
			sprintf(buffer, "ADD R%d, R%d, R%d", inst.destreg, inst.src1reg, inst.src2reg);
		break;
	case LD:
		sprintf(buffer, "LD R%d, #%d", inst.destreg, inst.pcoffset9);
		break;
	case ST:
		sprintf(buffer, "ST R%d, #%d", inst.destreg, inst.pcoffset9);
		break;
	case JSR:
		sprintf(buffer, "JSR #%d", inst.pcoffset11);
		break;
	case AND:
		if (inst.imm5_flag)
			sprintf(buffer, "AND R%d, R%d, #%d", inst.destreg, inst.src1reg, inst.imm5);
		else
			sprintf(buffer, "AND R%d, R%d, R%d", inst.destreg, inst.src1reg, inst.src2reg);
		break;
	case LDR:
		sprintf(buffer, "LDR R%d, R%d, #%d", inst.destreg, inst.src1reg, inst.offset6);
		break;
	case STR:
		sprintf(buffer, "STR R%d, R%d, #%d", inst.destreg, inst.src1reg, inst.offset6);
		break;
	case RTI:
		break;
	case NOT:
		sprintf(buffer, "NOT R%d, R%d", inst.destreg, inst.src1reg);
		break;
	case LDI:
		sprintf(buffer, "LDI R%d, #%d", inst.destreg, inst.pcoffset9);
		break;
	case STI:
		sprintf(buffer, "STI R%d, #%d", inst.destreg, inst.pcoffset9);
		break;
	case JMP:
		sprintf(buffer, "JMP R%d", inst.src1reg);
		break;
	case LEA:
		sprintf(buffer, "LEA R%d, #%d", inst.destreg, inst.pcoffset9);
		break;
	case TRAP:
		sprintf(buffer, "TRAP x%.2hx", inst.trapvect);
		break;
	}
}
