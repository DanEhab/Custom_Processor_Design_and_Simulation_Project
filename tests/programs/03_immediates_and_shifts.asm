# XORI with signed immediates, logical shifts, and the 18-bit immediate range.
MOVI R1 -8
MOVI R2 12
XORI R3 R1 -1      # ~(-8) = 7
XORI R4 R2 10      # 1100 ^ 1010 = 0110 = 6
LSL R5 R1 2        # -8 << 2 = -32
LSR R6 R1 28       # logical: 0xFFFFFFF8 >>> 28 = 15
LSR R7 R2 2        # 12 >>> 2 = 3
MOVI R8 131071     # largest 18-bit immediate
MOVI R9 -131072    # smallest 18-bit immediate
# expect: R1=-8 R2=12 R3=7 R4=6 R5=-32 R6=15 R7=3 R8=131071 R9=-131072 PC=9 cycles=23
