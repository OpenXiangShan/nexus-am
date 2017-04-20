import sys

INST = """ADD
LI
ADDU
ADDI
ADDIU
SUB
SUBU
AND
ANDI
OR
ORI
XOR
XORI
NOR
SLT
SLTU
SLTI
SLTIU
SLL
SLLV
SRL
SRLV
SRA
SRAV
CLO
SEH
SEB
CLZ
B
BEQ
BEQL
BEQZ
BGEZ
BGTZ
BLEZ
BLTZ
BNE
BNEZ
TEQ
TEQI
TGE
TGEI
TGEU
TGEIU
TLT
TLTI
TLTU
TLTIU
TNE
TNEI
J
JAL
JR
MOVE
MOVN
MOVZ
LUI
MUL
MULT
MULTU
DIV
DIVU
MFHI
MTHI
MFLO
MTLO
LB
LBU
LH
LHU
LW
SB
SH
SW
LWL
LWR
SWL
SWR
MFC0
MTC0
SYSCALL
ERET
NOP"""

INST = [i.strip() for i in INST.strip().split('\n')]

for line in sys.stdin:
    content = line.strip().split('\t')
    if len(content) >= 3:
        inst = content[2].upper()
        if inst not in INST:
            print "[INSTcheck] '{0}' is invalid.".format(inst)
