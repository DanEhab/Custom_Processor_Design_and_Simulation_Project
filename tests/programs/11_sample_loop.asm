# The sample program from instructions.txt: add 2 to R3 until R1 reaches R2, then store R3.
MOVI R1 0
MOVI R2 4
MOVI R3 6
MOVI R4 2
MOVI R5 1
MOVI R30 0
MOVI R31 0
ADD R3 R3 R4       # 7: loop
ADD R1 R1 R5
MOVI R30 0
MOVI R30 0
JEQ R1 R2 2        # 11: if R1 == R2, go to 14
JMP 7
MOVI R10 9         # never runs
MOVM R3 R2 0       # 14: MEM[R2] = R3
# expect: R1=4 R2=4 R3=14 R4=2 R5=1 MEM[4]=14 PC=15 cycles=75
