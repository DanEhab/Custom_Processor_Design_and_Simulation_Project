# A taken JEQ skips the next instruction, which was already fetched and gets flushed.
MOVI R1 3
MOVI R2 3
MOVI R9 0
JEQ R1 R2 1        # R1 == R2, so PC = 3 + 1 + 1 = 5
MOVI R5 99         # skipped
MOVI R6 7
# expect: R1=3 R2=3 R6=7 PC=6 cycles=17
