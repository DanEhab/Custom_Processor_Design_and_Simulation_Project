# Lower case, commas, tabs, blank lines and comments are accepted.

movi r1, 5    ; a comment after an instruction
	MOVI	R2	7
# expect: R1=5 R2=7 PC=2 cycles=9
