void trap(){
//you can trap now
	trap_frame tf;
	asm volatile("sw zero,%0\n\t"::"r"(tf.regs0_25[0]));
	asm volatile("sw at,%0\n\t"::"r"(tf.regs0_25[1]));
	asm volatile("sw v0,%0\n\t"::"r"(tf.regs0_25[2]));
	asm volatile("sw v1,%0\n\t"::"r"(tf.regs0_25[3]));
	asm volatile("sw a0,%0\n\t"::"r"(tf.regs0_25[4]));
	asm volatile("sw a1,%0\n\t"::"r"(tf.regs0_25[5]));
	asm volatile("sw a2,%0\n\t"::"r"(tf.regs0_25[6]));
	asm volatile("sw a3,%0\n\t"::"r"(tf.regs0_25[7]));
	asm volatile("sw t0,%0\n\t"::"r"(tf.regs0_25[8]));
	asm volatile("sw t1,%0\n\t"::"r"(tf.regs0_25[9]));
	asm volatile("sw t2,%0\n\t"::"r"(tf.regs0_25[10]));
	asm volatile("sw t3,%0\n\t"::"r"(tf.regs0_25[11]));
	asm volatile("sw t4,%0\n\t"::"r"(tf.regs0_25[12]));
	asm volatile("sw t5,%0\n\t"::"r"(tf.regs0_25[13]));
	asm volatile("sw t6,%0\n\t"::"r"(tf.regs0_25[14]));
	asm volatile("sw t7,%0\n\t"::"r"(tf.regs0_25[15]));
	asm volatile("sw s0,%0\n\t"::"r"(tf.regs0_25[16]));
	asm volatile("sw s1,%0\n\t"::"r"(tf.regs0_25[17]));
	asm volatile("sw s2,%0\n\t"::"r"(tf.regs0_25[18]));
	asm volatile("sw s3,%0\n\t"::"r"(tf.regs0_25[19]));
	asm volatile("sw s4,%0\n\t"::"r"(tf.regs0_25[20]));
	asm volatile("sw s5,%0\n\t"::"r"(tf.regs0_25[21]));
	asm volatile("sw s6,%0\n\t"::"r"(tf.regs0_25[22]));
	asm volatile("sw s7,%0\n\t"::"r"(tf.regs0_25[23]));
	asm volatile("sw t8,%0\n\t"::"r"(tf.regs0_25[24]));
	asm volatile("sw t9,%0\n\t"::"r"(tf.regs0_25[25]));
	asm volatile("mfc0 k0,epc");
	asm volatile("sw k0,%0\n\t"::"r"(tf.cp0.Epc));
	asm volatile("mfc0 k0,cause");
	asm volatile("sw k0,%0\n\t"::"r"(tf.cp0.Cause));
	asm volatile("sw gp,%0\n\t"::"r"(tf.regs28_32[0]));
	asm volatile("sw sp,%0\n\t"::"r"(tf.regs28_32[1]));
	asm volatile("sw fp,%0\n\t"::"r"(tf.regs28_32[2]));
	asm volatile("sw ra,%0\n\t"::"r"(tf.regs28_32[3]));
}
