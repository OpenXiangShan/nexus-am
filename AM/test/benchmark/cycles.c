extern volatile long ticks;
long cycles(){
	asm volatile("mfc0 %0,$9\n\t"::"g"(ticks));
	return ticks;
}
