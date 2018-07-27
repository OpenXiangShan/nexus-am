.section .data
.global __fsimg_start, __fsimg_end
__fsimg_start:
.incbin "build/fsimg"
__fsimg_end:
