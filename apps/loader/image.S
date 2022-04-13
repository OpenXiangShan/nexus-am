.section .rodata
.global image, image_end
.p2align 3
image:
.incbin ELF_PATH
image_end:
