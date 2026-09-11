.align 3
.global cbo_cache_flush
.type cbo_cache_flush, @function
cbo_cache_flush:
	/* cbo.flush (a0), emitted explicitly for toolchains without Zicbom asm. */
	fence w, w
	.word 0x0025200f
	fence rw, rw
	ret
