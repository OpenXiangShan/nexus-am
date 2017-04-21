#include <am.h>
#include <npc.h>
#include <arch.h>

u32 GetCount(){
	u32 tick = 0;
	asm volatile("mfc0 %0, $9\n\t":"=r"(tick));
	return tick;
}

void SetCompare(u32 compare){
	asm volatile("mtc0 $11, %0\n\t"::"r"(compare));
}

void _asye_init(){
	SetCompare(100);
}

void _listen(_RegSet* (*l)(int ex, _RegSet *regs)){
}

_RegSet *_make(_Area kstack, void *entry){
	_RegSet *r = (void *)0;
	return r;
}

void _trap(){
}

void _idle(){
}

void _idisable(){
}

void _ienable(){
}

int _istatus(){
	return 0;
}

void irq_handle(struct TrapFrame *tf){
//TODO:handle
	u32 arg = 0;
	asm volatile("add %0,$4,$zero\n\t":"=r"(arg));
	tf = (void *)arg;
	u32 intr = 0;
	asm volatile("add %0,$5,$zero\n\t":"=r"(intr));
	switch(intr){
		case 0x80://time interrupt
			{
				u32 count= GetCount();
				_putc('g');
				SetCompare(count + 100);
			}break;
		default:_halt(0);
	}
	asm volatile(
	"addi $k1,$k1,8 \n\t"
	"jr $k1         \n\t"
	"nop		\n\t");
}
