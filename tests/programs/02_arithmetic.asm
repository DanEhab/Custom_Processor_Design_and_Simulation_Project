# ADD, SUB, MUL and AND with a negative operand.
MOVI R1 7
MOVI R2 -3
MOVI R9 0          # gap, so R2 is written back before it is read
ADD R3 R1 R2       # 7 + (-3) = 4
SUB R4 R1 R2       # 7 - (-3) = 10
MUL R5 R1 R2       # 7 * (-3) = -21
AND R6 R1 R2       # 0111 & ...1101 = 0101 = 5
# expect: R1=7 R2=-3 R3=4 R4=10 R5=-21 R6=5 PC=7 cycles=19
