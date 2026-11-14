# The last instruction must run exactly once (the submitted version kept repeating it).
MOVI R2 2
MOVI R9 0
ADD R1 R1 R2
# expect: R1=2 R2=2 PC=3 cycles=11
