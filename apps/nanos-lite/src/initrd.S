.section .data
.global ramdisk_start, ramdisk_end
ramdisk_start:
.incbin "ramdisk.img"
ramdisk_end:
