# JMP sets the PC to an address; the instruction fetched behind it is flushed.
MOVI R1 1
JMP 3
MOVI R1 99         # skipped
MOVI R2 2
# expect: R1=1 R2=2 PC=4 cycles=13
