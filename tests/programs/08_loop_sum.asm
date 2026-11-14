# Sum 1 + 2 + 3 + 4 with a loop. JEQ R0 R0 -5 is always taken and branches backwards.
MOVI R1 0          # 0: i
MOVI R2 4          # 1: n
MOVI R4 1          # 2: step
MOVI R3 0          # 3: sum
ADD R1 R1 R4       # 4: loop: i = i + 1
MOVI R9 0          # 5: gap (no forwarding)
ADD R3 R3 R1       # 6: sum = sum + i
JEQ R1 R2 1        # 7: if i == n, leave the loop (go to 9)
JEQ R0 R0 -5       # 8: back to 4 (8 + 1 - 5)
MOVM R3 R0 1024    # 9: MEM[1024] = sum
# expect: R1=4 R2=4 R3=10 R4=1 MEM[1024]=10 PC=10 cycles=61
