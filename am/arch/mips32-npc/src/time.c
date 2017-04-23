#include <am.h>
#include <npc.h>

u32 GetCount(){
	u32 tick = 0;
	asm volatile("mfc0 %0, $9\n\t":"=r"(tick));
	return tick;
}

void SetCompare(u32 compare){
	asm volatile("mtc0 %0, $11\n\t"::"r"(compare));
}

ulong npc_time = 0;

ulong _uptime() {
	return npc_time ++;
}

ulong _cycles(){
	return 0;
}
