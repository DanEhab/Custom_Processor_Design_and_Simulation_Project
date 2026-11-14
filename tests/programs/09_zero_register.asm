# R0 can be a destination, but it always stays 0.
MOVI R0 5
MOVI R1 6
MOVI R9 0
ADD R0 R1 R1
ADD R2 R0 R1       # 0 + 6
# expect: R1=6 R2=6 PC=5 cycles=15
