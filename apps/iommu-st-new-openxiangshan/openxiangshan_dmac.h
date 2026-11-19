#ifndef __OPENXIANGSHAN_H__
#define __OPENXIANGSHAN_H__

int openxiangshan_dmac_transfer(void *src, void *dst, int size);
int openxiangshan_dmac_wait_for_complete(unsigned long timeout_cycles);

#endif
