.orig x40
	.fill x4000
.end

.orig x3000
	AND R0, R0, 0
	AND R1, R1, 0
	AND R2, R2, 0

	ADD R0, R0, 1
	ADD R1, R1, -1
	ADD R2, R2, 3

M7 
	ADD R2, R2, R1
	BRP M7

	LD R3, STUFF
	LDR R4, R3, 0
	
	TRAP x40
	GETC

	HALT

STUFF .fill OTHERSTUFF
OTHERSTUFF .fill x1337
.end

.orig x4000
	ADD R6, R6, 13
	RET
.end
