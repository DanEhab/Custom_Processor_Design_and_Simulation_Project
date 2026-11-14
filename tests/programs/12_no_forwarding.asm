# There is no forwarding: an instruction right after another one reads the old register value.
MOVI R1 5
ADD R2 R1 R1       # R1 is not written back yet, so R2 = 0 + 0
MOVI R9 0
ADD R3 R1 R1       # one instruction later the value is there: 5 + 5
# expect: R1=5 R2=0 R3=10 PC=4 cycles=13
