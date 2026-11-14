# MOVM stores to and MOVR loads from the data segment (addresses 1024 to 2047).
MOVI R1 1024       # base address of the data segment
MOVI R2 42
MOVI R3 -7
MOVM R2 R1 5       # MEM[1029] = 42
MOVM R3 R1 0       # MEM[1024] = -7
MOVR R4 R1 5       # R4 = MEM[1029]
MOVR R5 R1 0       # R5 = MEM[1024]
# expect: R1=1024 R2=42 R3=-7 R4=42 R5=-7 MEM[1024]=-7 MEM[1029]=42 PC=7 cycles=19
