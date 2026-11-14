# The example from the project description: 7 instructions take 7 + (7 - 1) * 2 = 19 cycles.
MOVI R1 5
MOVI R2 3
MOVI R3 7
MOVI R4 1
MOVI R5 2
MOVI R6 4
MOVI R7 9
# expect: R1=5 R2=3 R3=7 R4=1 R5=2 R6=4 R7=9 PC=7 cycles=19
