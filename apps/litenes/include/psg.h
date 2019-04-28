// #define psg_io_read(...) 0xFF
// #define psg_io_write(...) /**/

#include "common.h"

#ifndef PSG_H
#define PSG_H

byte psg_io_read(word address);
void psg_io_write(word address, byte data);

#endif
