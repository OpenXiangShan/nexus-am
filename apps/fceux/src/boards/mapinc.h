#include "../types.h"
#include "../utils/memory.h"
#include "../x6502.h"
#include "../fceu.h"
#include "../ppu.h"
#include "../sound.h"
#include "../state.h"
#include "../cart.h"
#include "../unif.h"
#include <stdio.h>
#include <string.h>

void FCEU_CheatAddRAM(int s, uint32 A, uint8 *p);
