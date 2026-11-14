# A JEQ that is not taken lets the pipeline continue normally.
MOVI R1 3
MOVI R2 4
MOVI R9 0
JEQ R1 R2 1        # R1 != R2, no jump
MOVI R5 99
MOVI R6 7
# expect: R1=3 R2=4 R5=99 R6=7 PC=6 cycles=17
