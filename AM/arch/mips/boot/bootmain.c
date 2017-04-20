extern int main();

void bootmain(){
	asm volatile("jal kentry");
	while(1);
}
