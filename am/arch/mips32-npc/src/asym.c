#include <am.h>
#include <npc.h>
#include <arch.h>

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
