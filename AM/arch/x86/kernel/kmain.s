
kmain:     file format elf32-i386


Disassembly of section .text:

00100000 <init_seg>:
  100000:	55                   	push   %ebp
  100001:	89 e5                	mov    %esp,%ebp
  100003:	b9 a0 25 10 00       	mov    $0x1025a0,%ecx
  100008:	66 c7 05 a8 25 10 00 	movw   $0xffff,0x1025a8
  10000f:	ff ff 
  100011:	66 c7 05 aa 25 10 00 	movw   $0x0,0x1025aa
  100018:	00 00 
  10001a:	c6 05 ac 25 10 00 00 	movb   $0x0,0x1025ac
  100021:	c6 05 ad 25 10 00 9a 	movb   $0x9a,0x1025ad
  100028:	c6 05 ae 25 10 00 cf 	movb   $0xcf,0x1025ae
  10002f:	c6 05 af 25 10 00 00 	movb   $0x0,0x1025af
  100036:	66 c7 05 b0 25 10 00 	movw   $0xffff,0x1025b0
  10003d:	ff ff 
  10003f:	66 c7 05 b2 25 10 00 	movw   $0x0,0x1025b2
  100046:	00 00 
  100048:	c6 05 b4 25 10 00 00 	movb   $0x0,0x1025b4
  10004f:	c6 05 b5 25 10 00 92 	movb   $0x92,0x1025b5
  100056:	c6 05 b6 25 10 00 cf 	movb   $0xcf,0x1025b6
  10005d:	c6 05 b7 25 10 00 00 	movb   $0x0,0x1025b7
  100064:	a1 20 0f 10 00       	mov    0x100f20,%eax
  100069:	8b 15 24 0f 10 00    	mov    0x100f24,%edx
  10006f:	a3 b8 25 10 00       	mov    %eax,0x1025b8
  100074:	89 15 bc 25 10 00    	mov    %edx,0x1025bc
  10007a:	a1 28 0f 10 00       	mov    0x100f28,%eax
  10007f:	8b 15 2c 0f 10 00    	mov    0x100f2c,%edx
  100085:	a3 c0 25 10 00       	mov    %eax,0x1025c0
  10008a:	89 15 c4 25 10 00    	mov    %edx,0x1025c4
  100090:	66 c7 05 c8 25 10 00 	movw   $0x63,0x1025c8
  100097:	63 00 
  100099:	b8 e0 25 10 00       	mov    $0x1025e0,%eax
  10009e:	66 a3 ca 25 10 00    	mov    %ax,0x1025ca
  1000a4:	89 c2                	mov    %eax,%edx
  1000a6:	c1 ea 10             	shr    $0x10,%edx
  1000a9:	88 15 cc 25 10 00    	mov    %dl,0x1025cc
  1000af:	c6 05 cd 25 10 00 89 	movb   $0x89,0x1025cd
  1000b6:	c6 05 ce 25 10 00 40 	movb   $0x40,0x1025ce
  1000bd:	c1 e8 18             	shr    $0x18,%eax
  1000c0:	a2 cf 25 10 00       	mov    %al,0x1025cf
  1000c5:	66 c7 05 60 25 10 00 	movw   $0x37,0x102560
  1000cc:	37 00 
  1000ce:	66 89 0d 62 25 10 00 	mov    %cx,0x102562
  1000d5:	c1 e9 10             	shr    $0x10,%ecx
  1000d8:	66 89 0d 64 25 10 00 	mov    %cx,0x102564
  1000df:	b8 60 25 10 00       	mov    $0x102560,%eax
  1000e4:	0f 01 10             	lgdtl  (%eax)
  1000e7:	c7 05 e4 25 10 00 00 	movl   $0x800000,0x1025e4
  1000ee:	00 80 00 
  1000f1:	c7 05 e8 25 10 00 10 	movl   $0x10,0x1025e8
  1000f8:	00 00 00 
  1000fb:	b8 28 00 00 00       	mov    $0x28,%eax
  100100:	0f 00 d8             	ltr    %ax
  100103:	b8 10 00 00 00       	mov    $0x10,%eax
  100108:	8e c0                	mov    %eax,%es
  10010a:	8e d8                	mov    %eax,%ds
  10010c:	8e d0                	mov    %eax,%ss
  10010e:	31 c0                	xor    %eax,%eax
  100110:	0f 00 d0             	lldt   %ax
  100113:	5d                   	pop    %ebp
  100114:	c3                   	ret    
  100115:	66 90                	xchg   %ax,%ax
  100117:	90                   	nop

00100118 <write>:
  100118:	55                   	push   %ebp
  100119:	89 e5                	mov    %esp,%ebp
  10011b:	56                   	push   %esi
  10011c:	53                   	push   %ebx
  10011d:	8b 75 0c             	mov    0xc(%ebp),%esi
  100120:	8b 45 10             	mov    0x10(%ebp),%eax
  100123:	8b 5d 14             	mov    0x14(%ebp),%ebx
  100126:	8b 55 08             	mov    0x8(%ebp),%edx
  100129:	4a                   	dec    %edx
  10012a:	83 fa 01             	cmp    $0x1,%edx
  10012d:	77 2d                	ja     10015c <write+0x44>
  10012f:	8d 1c 9b             	lea    (%ebx,%ebx,4),%ebx
  100132:	c1 e3 04             	shl    $0x4,%ebx
  100135:	03 5d 18             	add    0x18(%ebp),%ebx
  100138:	01 db                	add    %ebx,%ebx
  10013a:	31 d2                	xor    %edx,%edx
  10013c:	85 c0                	test   %eax,%eax
  10013e:	7e 16                	jle    100156 <write+0x3e>
  100140:	8a 8c 16 00 00 20 00 	mov    0x200000(%esi,%edx,1),%cl
  100147:	b5 0b                	mov    $0xb,%ch
  100149:	66 89 8c 53 00 80 0b 	mov    %cx,0xb8000(%ebx,%edx,2)
  100150:	00 
  100151:	42                   	inc    %edx
  100152:	39 d0                	cmp    %edx,%eax
  100154:	75 ea                	jne    100140 <write+0x28>
  100156:	5b                   	pop    %ebx
  100157:	5e                   	pop    %esi
  100158:	5d                   	pop    %ebp
  100159:	c3                   	ret    
  10015a:	66 90                	xchg   %ax,%ax
  10015c:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
  100161:	5b                   	pop    %ebx
  100162:	5e                   	pop    %esi
  100163:	5d                   	pop    %ebp
  100164:	c3                   	ret    
  100165:	66 90                	xchg   %ax,%ax
  100167:	90                   	nop

00100168 <kentry>:
  100168:	55                   	push   %ebp
  100169:	89 e5                	mov    %esp,%ebp
  10016b:	53                   	push   %ebx
  10016c:	83 ec 10             	sub    $0x10,%esp
  10016f:	68 30 0f 10 00       	push   $0x100f30
  100174:	e8 a3 0a 00 00       	call   100c1c <printk>
  100179:	e8 ca 05 00 00       	call   100748 <init_serial>
  10017e:	c7 04 24 b8 0f 10 00 	movl   $0x100fb8,(%esp)
  100185:	e8 92 0a 00 00       	call   100c1c <printk>
  10018a:	e8 b9 00 00 00       	call   100248 <init_idt>
  10018f:	c7 04 24 4b 0f 10 00 	movl   $0x100f4b,(%esp)
  100196:	e8 81 0a 00 00       	call   100c1c <printk>
  10019b:	e8 50 05 00 00       	call   1006f0 <init_intr>
  1001a0:	c7 04 24 dc 0f 10 00 	movl   $0x100fdc,(%esp)
  1001a7:	e8 70 0a 00 00       	call   100c1c <printk>
  1001ac:	e8 4f fe ff ff       	call   100000 <init_seg>
  1001b1:	c7 04 24 00 10 10 00 	movl   $0x101000,(%esp)
  1001b8:	e8 5f 0a 00 00       	call   100c1c <printk>
  1001bd:	e8 22 0b 00 00       	call   100ce4 <init_vga>
  1001c2:	c7 04 24 69 0f 10 00 	movl   $0x100f69,(%esp)
  1001c9:	e8 4e 0a 00 00       	call   100c1c <printk>
  1001ce:	c7 04 24 87 0f 10 00 	movl   $0x100f87,(%esp)
  1001d5:	e8 42 0a 00 00       	call   100c1c <printk>
  1001da:	e8 15 0c 00 00       	call   100df4 <load_umain>
  1001df:	89 c3                	mov    %eax,%ebx
  1001e1:	c7 04 24 24 10 10 00 	movl   $0x101024,(%esp)
  1001e8:	e8 2f 0a 00 00       	call   100c1c <printk>
  1001ed:	b8 23 00 00 00       	mov    $0x23,%eax
  1001f2:	8e c0                	mov    %eax,%es
  1001f4:	8e d8                	mov    %eax,%ds
  1001f6:	6a 23                	push   $0x23
  1001f8:	68 00 00 80 00       	push   $0x800000
  1001fd:	6a 02                	push   $0x2
  1001ff:	6a 1b                	push   $0x1b
  100201:	53                   	push   %ebx
  100202:	cf                   	iret   
  100203:	c7 04 24 99 0f 10 00 	movl   $0x100f99,(%esp)
  10020a:	e8 0d 0a 00 00       	call   100c1c <printk>
  10020f:	58                   	pop    %eax
  100210:	5a                   	pop    %edx
  100211:	6a 19                	push   $0x19
  100213:	68 b1 0f 10 00       	push   $0x100fb1
  100218:	e8 2f 0a 00 00       	call   100c4c <abort>
  10021d:	83 c4 10             	add    $0x10,%esp
  100220:	8b 5d fc             	mov    -0x4(%ebp),%ebx
  100223:	c9                   	leave  
  100224:	c3                   	ret    
  100225:	8d 76 00             	lea    0x0(%esi),%esi

00100228 <enter_user_space>:
  100228:	55                   	push   %ebp
  100229:	89 e5                	mov    %esp,%ebp
  10022b:	b8 23 00 00 00       	mov    $0x23,%eax
  100230:	8e c0                	mov    %eax,%es
  100232:	8e d8                	mov    %eax,%ds
  100234:	6a 23                	push   $0x23
  100236:	68 00 00 80 00       	push   $0x800000
  10023b:	6a 02                	push   $0x2
  10023d:	6a 1b                	push   $0x1b
  10023f:	ff 75 08             	pushl  0x8(%ebp)
  100242:	cf                   	iret   
  100243:	5d                   	pop    %ebp
  100244:	c3                   	ret    
  100245:	66 90                	xchg   %ax,%ax
  100247:	90                   	nop

00100248 <init_idt>:
  100248:	55                   	push   %ebp
  100249:	89 e5                	mov    %esp,%ebp
  10024b:	53                   	push   %ebx
  10024c:	ba fc 0e 10 00       	mov    $0x100efc,%edx
  100251:	89 d3                	mov    %edx,%ebx
  100253:	c1 ea 10             	shr    $0x10,%edx
  100256:	b9 60 2e 10 00       	mov    $0x102e60,%ecx
  10025b:	b8 60 26 10 00       	mov    $0x102660,%eax
  100260:	66 89 18             	mov    %bx,(%eax)
  100263:	66 c7 40 02 08 00    	movw   $0x8,0x2(%eax)
  100269:	c6 40 04 00          	movb   $0x0,0x4(%eax)
  10026d:	c6 40 05 8f          	movb   $0x8f,0x5(%eax)
  100271:	66 89 50 06          	mov    %dx,0x6(%eax)
  100275:	83 c0 08             	add    $0x8,%eax
  100278:	39 c1                	cmp    %eax,%ecx
  10027a:	75 e4                	jne    100260 <init_idt+0x18>
  10027c:	b8 a4 0e 10 00       	mov    $0x100ea4,%eax
  100281:	66 a3 60 26 10 00    	mov    %ax,0x102660
  100287:	66 c7 05 62 26 10 00 	movw   $0x8,0x102662
  10028e:	08 00 
  100290:	c6 05 64 26 10 00 00 	movb   $0x0,0x102664
  100297:	c6 05 65 26 10 00 8f 	movb   $0x8f,0x102665
  10029e:	c1 e8 10             	shr    $0x10,%eax
  1002a1:	66 a3 66 26 10 00    	mov    %ax,0x102666
  1002a7:	b8 a8 0e 10 00       	mov    $0x100ea8,%eax
  1002ac:	66 a3 68 26 10 00    	mov    %ax,0x102668
  1002b2:	66 c7 05 6a 26 10 00 	movw   $0x8,0x10266a
  1002b9:	08 00 
  1002bb:	c6 05 6c 26 10 00 00 	movb   $0x0,0x10266c
  1002c2:	c6 05 6d 26 10 00 8f 	movb   $0x8f,0x10266d
  1002c9:	c1 e8 10             	shr    $0x10,%eax
  1002cc:	66 a3 6e 26 10 00    	mov    %ax,0x10266e
  1002d2:	b8 ac 0e 10 00       	mov    $0x100eac,%eax
  1002d7:	66 a3 70 26 10 00    	mov    %ax,0x102670
  1002dd:	66 c7 05 72 26 10 00 	movw   $0x8,0x102672
  1002e4:	08 00 
  1002e6:	c6 05 74 26 10 00 00 	movb   $0x0,0x102674
  1002ed:	c6 05 75 26 10 00 8f 	movb   $0x8f,0x102675
  1002f4:	c1 e8 10             	shr    $0x10,%eax
  1002f7:	66 a3 76 26 10 00    	mov    %ax,0x102676
  1002fd:	b8 b0 0e 10 00       	mov    $0x100eb0,%eax
  100302:	66 a3 78 26 10 00    	mov    %ax,0x102678
  100308:	66 c7 05 7a 26 10 00 	movw   $0x8,0x10267a
  10030f:	08 00 
  100311:	c6 05 7c 26 10 00 00 	movb   $0x0,0x10267c
  100318:	c6 05 7d 26 10 00 8f 	movb   $0x8f,0x10267d
  10031f:	c1 e8 10             	shr    $0x10,%eax
  100322:	66 a3 7e 26 10 00    	mov    %ax,0x10267e
  100328:	b8 b4 0e 10 00       	mov    $0x100eb4,%eax
  10032d:	66 a3 80 26 10 00    	mov    %ax,0x102680
  100333:	66 c7 05 82 26 10 00 	movw   $0x8,0x102682
  10033a:	08 00 
  10033c:	c6 05 84 26 10 00 00 	movb   $0x0,0x102684
  100343:	c6 05 85 26 10 00 8f 	movb   $0x8f,0x102685
  10034a:	c1 e8 10             	shr    $0x10,%eax
  10034d:	66 a3 86 26 10 00    	mov    %ax,0x102686
  100353:	b8 b8 0e 10 00       	mov    $0x100eb8,%eax
  100358:	66 a3 88 26 10 00    	mov    %ax,0x102688
  10035e:	66 c7 05 8a 26 10 00 	movw   $0x8,0x10268a
  100365:	08 00 
  100367:	c6 05 8c 26 10 00 00 	movb   $0x0,0x10268c
  10036e:	c6 05 8d 26 10 00 8f 	movb   $0x8f,0x10268d
  100375:	c1 e8 10             	shr    $0x10,%eax
  100378:	66 a3 8e 26 10 00    	mov    %ax,0x10268e
  10037e:	b8 bc 0e 10 00       	mov    $0x100ebc,%eax
  100383:	66 a3 90 26 10 00    	mov    %ax,0x102690
  100389:	66 c7 05 92 26 10 00 	movw   $0x8,0x102692
  100390:	08 00 
  100392:	c6 05 94 26 10 00 00 	movb   $0x0,0x102694
  100399:	c6 05 95 26 10 00 8f 	movb   $0x8f,0x102695
  1003a0:	c1 e8 10             	shr    $0x10,%eax
  1003a3:	66 a3 96 26 10 00    	mov    %ax,0x102696
  1003a9:	b8 c0 0e 10 00       	mov    $0x100ec0,%eax
  1003ae:	66 a3 98 26 10 00    	mov    %ax,0x102698
  1003b4:	66 c7 05 9a 26 10 00 	movw   $0x8,0x10269a
  1003bb:	08 00 
  1003bd:	c6 05 9c 26 10 00 00 	movb   $0x0,0x10269c
  1003c4:	c6 05 9d 26 10 00 8f 	movb   $0x8f,0x10269d
  1003cb:	c1 e8 10             	shr    $0x10,%eax
  1003ce:	66 a3 9e 26 10 00    	mov    %ax,0x10269e
  1003d4:	b8 c4 0e 10 00       	mov    $0x100ec4,%eax
  1003d9:	66 a3 a0 26 10 00    	mov    %ax,0x1026a0
  1003df:	66 c7 05 a2 26 10 00 	movw   $0x8,0x1026a2
  1003e6:	08 00 
  1003e8:	c6 05 a4 26 10 00 00 	movb   $0x0,0x1026a4
  1003ef:	c6 05 a5 26 10 00 8f 	movb   $0x8f,0x1026a5
  1003f6:	c1 e8 10             	shr    $0x10,%eax
  1003f9:	66 a3 a6 26 10 00    	mov    %ax,0x1026a6
  1003ff:	b8 c8 0e 10 00       	mov    $0x100ec8,%eax
  100404:	66 a3 a8 26 10 00    	mov    %ax,0x1026a8
  10040a:	66 c7 05 aa 26 10 00 	movw   $0x8,0x1026aa
  100411:	08 00 
  100413:	c6 05 ac 26 10 00 00 	movb   $0x0,0x1026ac
  10041a:	c6 05 ad 26 10 00 8f 	movb   $0x8f,0x1026ad
  100421:	c1 e8 10             	shr    $0x10,%eax
  100424:	66 a3 ae 26 10 00    	mov    %ax,0x1026ae
  10042a:	b8 cc 0e 10 00       	mov    $0x100ecc,%eax
  10042f:	66 a3 b0 26 10 00    	mov    %ax,0x1026b0
  100435:	66 c7 05 b2 26 10 00 	movw   $0x8,0x1026b2
  10043c:	08 00 
  10043e:	c6 05 b4 26 10 00 00 	movb   $0x0,0x1026b4
  100445:	c6 05 b5 26 10 00 8f 	movb   $0x8f,0x1026b5
  10044c:	c1 e8 10             	shr    $0x10,%eax
  10044f:	66 a3 b6 26 10 00    	mov    %ax,0x1026b6
  100455:	b8 d0 0e 10 00       	mov    $0x100ed0,%eax
  10045a:	66 a3 b8 26 10 00    	mov    %ax,0x1026b8
  100460:	66 c7 05 ba 26 10 00 	movw   $0x8,0x1026ba
  100467:	08 00 
  100469:	c6 05 bc 26 10 00 00 	movb   $0x0,0x1026bc
  100470:	c6 05 bd 26 10 00 8f 	movb   $0x8f,0x1026bd
  100477:	c1 e8 10             	shr    $0x10,%eax
  10047a:	66 a3 be 26 10 00    	mov    %ax,0x1026be
  100480:	b8 d4 0e 10 00       	mov    $0x100ed4,%eax
  100485:	66 a3 c0 26 10 00    	mov    %ax,0x1026c0
  10048b:	66 c7 05 c2 26 10 00 	movw   $0x8,0x1026c2
  100492:	08 00 
  100494:	c6 05 c4 26 10 00 00 	movb   $0x0,0x1026c4
  10049b:	c6 05 c5 26 10 00 8f 	movb   $0x8f,0x1026c5
  1004a2:	c1 e8 10             	shr    $0x10,%eax
  1004a5:	66 a3 c6 26 10 00    	mov    %ax,0x1026c6
  1004ab:	b8 d8 0e 10 00       	mov    $0x100ed8,%eax
  1004b0:	66 a3 c8 26 10 00    	mov    %ax,0x1026c8
  1004b6:	66 c7 05 ca 26 10 00 	movw   $0x8,0x1026ca
  1004bd:	08 00 
  1004bf:	c6 05 cc 26 10 00 00 	movb   $0x0,0x1026cc
  1004c6:	c6 05 cd 26 10 00 8f 	movb   $0x8f,0x1026cd
  1004cd:	c1 e8 10             	shr    $0x10,%eax
  1004d0:	66 a3 ce 26 10 00    	mov    %ax,0x1026ce
  1004d6:	b8 dc 0e 10 00       	mov    $0x100edc,%eax
  1004db:	66 a3 d0 26 10 00    	mov    %ax,0x1026d0
  1004e1:	66 c7 05 d2 26 10 00 	movw   $0x8,0x1026d2
  1004e8:	08 00 
  1004ea:	c6 05 d4 26 10 00 00 	movb   $0x0,0x1026d4
  1004f1:	c6 05 d5 26 10 00 8f 	movb   $0x8f,0x1026d5
  1004f8:	c1 e8 10             	shr    $0x10,%eax
  1004fb:	66 a3 d6 26 10 00    	mov    %ax,0x1026d6
  100501:	b8 e0 0e 10 00       	mov    $0x100ee0,%eax
  100506:	66 a3 60 2a 10 00    	mov    %ax,0x102a60
  10050c:	66 c7 05 62 2a 10 00 	movw   $0x8,0x102a62
  100513:	08 00 
  100515:	c6 05 64 2a 10 00 00 	movb   $0x0,0x102a64
  10051c:	c6 05 65 2a 10 00 ef 	movb   $0xef,0x102a65
  100523:	c1 e8 10             	shr    $0x10,%eax
  100526:	66 a3 66 2a 10 00    	mov    %ax,0x102a66
  10052c:	b8 e7 0e 10 00       	mov    $0x100ee7,%eax
  100531:	66 a3 60 27 10 00    	mov    %ax,0x102760
  100537:	66 c7 05 62 27 10 00 	movw   $0x8,0x102762
  10053e:	08 00 
  100540:	c6 05 64 27 10 00 00 	movb   $0x0,0x102764
  100547:	c6 05 65 27 10 00 8e 	movb   $0x8e,0x102765
  10054e:	c1 e8 10             	shr    $0x10,%eax
  100551:	66 a3 66 27 10 00    	mov    %ax,0x102766
  100557:	b8 ee 0e 10 00       	mov    $0x100eee,%eax
  10055c:	66 a3 68 27 10 00    	mov    %ax,0x102768
  100562:	66 c7 05 6a 27 10 00 	movw   $0x8,0x10276a
  100569:	08 00 
  10056b:	c6 05 6c 27 10 00 00 	movb   $0x0,0x10276c
  100572:	c6 05 6d 27 10 00 8e 	movb   $0x8e,0x10276d
  100579:	c1 e8 10             	shr    $0x10,%eax
  10057c:	66 a3 6e 27 10 00    	mov    %ax,0x10276e
  100582:	b8 f5 0e 10 00       	mov    $0x100ef5,%eax
  100587:	66 a3 d0 27 10 00    	mov    %ax,0x1027d0
  10058d:	66 c7 05 d2 27 10 00 	movw   $0x8,0x1027d2
  100594:	08 00 
  100596:	c6 05 d4 27 10 00 00 	movb   $0x0,0x1027d4
  10059d:	c6 05 d5 27 10 00 8e 	movb   $0x8e,0x1027d5
  1005a4:	c1 e8 10             	shr    $0x10,%eax
  1005a7:	66 a3 d6 27 10 00    	mov    %ax,0x1027d6
  1005ad:	66 c7 05 68 25 10 00 	movw   $0x7ff,0x102568
  1005b4:	ff 07 
  1005b6:	b8 60 26 10 00       	mov    $0x102660,%eax
  1005bb:	66 a3 6a 25 10 00    	mov    %ax,0x10256a
  1005c1:	c1 e8 10             	shr    $0x10,%eax
  1005c4:	66 a3 6c 25 10 00    	mov    %ax,0x10256c
  1005ca:	b8 68 25 10 00       	mov    $0x102568,%eax
  1005cf:	0f 01 18             	lidtl  (%eax)
  1005d2:	5b                   	pop    %ebx
  1005d3:	5d                   	pop    %ebp
  1005d4:	c3                   	ret    
  1005d5:	66 90                	xchg   %ax,%ax
  1005d7:	90                   	nop

001005d8 <do_syscall>:
  1005d8:	55                   	push   %ebp
  1005d9:	89 e5                	mov    %esp,%ebp
  1005db:	53                   	push   %ebx
  1005dc:	50                   	push   %eax
  1005dd:	8b 5d 08             	mov    0x8(%ebp),%ebx
  1005e0:	83 7b 1c 04          	cmpl   $0x4,0x1c(%ebx)
  1005e4:	74 1a                	je     100600 <do_syscall+0x28>
  1005e6:	83 ec 08             	sub    $0x8,%esp
  1005e9:	6a 1b                	push   $0x1b
  1005eb:	68 4c 10 10 00       	push   $0x10104c
  1005f0:	e8 57 06 00 00       	call   100c4c <abort>
  1005f5:	83 c4 10             	add    $0x10,%esp
  1005f8:	8b 5d fc             	mov    -0x4(%ebp),%ebx
  1005fb:	c9                   	leave  
  1005fc:	c3                   	ret    
  1005fd:	8d 76 00             	lea    0x0(%esi),%esi
  100600:	83 ec 0c             	sub    $0xc,%esp
  100603:	ff 73 04             	pushl  0x4(%ebx)
  100606:	ff 33                	pushl  (%ebx)
  100608:	ff 73 10             	pushl  0x10(%ebx)
  10060b:	ff 73 18             	pushl  0x18(%ebx)
  10060e:	ff 73 14             	pushl  0x14(%ebx)
  100611:	e8 02 fb ff ff       	call   100118 <write>
  100616:	89 43 1c             	mov    %eax,0x1c(%ebx)
  100619:	83 c4 20             	add    $0x20,%esp
  10061c:	8b 5d fc             	mov    -0x4(%ebp),%ebx
  10061f:	c9                   	leave  
  100620:	c3                   	ret    
  100621:	8d 76 00             	lea    0x0(%esi),%esi

00100624 <irq_handle>:
  100624:	55                   	push   %ebp
  100625:	89 e5                	mov    %esp,%ebp
  100627:	83 ec 08             	sub    $0x8,%esp
  10062a:	8b 55 08             	mov    0x8(%ebp),%edx
  10062d:	8b 42 20             	mov    0x20(%edx),%eax
  100630:	3d 80 00 00 00       	cmp    $0x80,%eax
  100635:	0f 84 81 00 00 00    	je     1006bc <irq_handle+0x98>
  10063b:	7e 3b                	jle    100678 <irq_handle+0x54>
  10063d:	3d e9 03 00 00       	cmp    $0x3e9,%eax
  100642:	74 64                	je     1006a8 <irq_handle+0x84>
  100644:	3d f6 03 00 00       	cmp    $0x3f6,%eax
  100649:	0f 84 8d 00 00 00    	je     1006dc <irq_handle+0xb8>
  10064f:	3d e8 03 00 00       	cmp    $0x3e8,%eax
  100654:	74 6f                	je     1006c5 <irq_handle+0xa1>
  100656:	83 ec 08             	sub    $0x8,%esp
  100659:	50                   	push   %eax
  10065a:	68 95 10 10 00       	push   $0x101095
  10065f:	e8 b8 05 00 00       	call   100c1c <printk>
  100664:	58                   	pop    %eax
  100665:	5a                   	pop    %edx
  100666:	6a 12                	push   $0x12
  100668:	68 4c 10 10 00       	push   $0x10104c
  10066d:	e8 da 05 00 00       	call   100c4c <abort>
  100672:	83 c4 10             	add    $0x10,%esp
  100675:	c9                   	leave  
  100676:	c3                   	ret    
  100677:	90                   	nop
  100678:	83 f8 03             	cmp    $0x3,%eax
  10067b:	74 17                	je     100694 <irq_handle+0x70>
  10067d:	83 f8 0d             	cmp    $0xd,%eax
  100680:	75 d4                	jne    100656 <irq_handle+0x32>
  100682:	83 ec 0c             	sub    $0xc,%esp
  100685:	68 7a 10 10 00       	push   $0x10107a
  10068a:	e8 8d 05 00 00       	call   100c1c <printk>
  10068f:	83 c4 10             	add    $0x10,%esp
  100692:	eb fe                	jmp    100692 <irq_handle+0x6e>
  100694:	83 ec 0c             	sub    $0xc,%esp
  100697:	68 5d 10 10 00       	push   $0x10105d
  10069c:	e8 7b 05 00 00       	call   100c1c <printk>
  1006a1:	83 c4 10             	add    $0x10,%esp
  1006a4:	eb fe                	jmp    1006a4 <irq_handle+0x80>
  1006a6:	66 90                	xchg   %ax,%ax
  1006a8:	83 ec 08             	sub    $0x8,%esp
  1006ab:	6a 10                	push   $0x10
  1006ad:	68 4c 10 10 00       	push   $0x10104c
  1006b2:	e8 95 05 00 00       	call   100c4c <abort>
  1006b7:	83 c4 10             	add    $0x10,%esp
  1006ba:	c9                   	leave  
  1006bb:	c3                   	ret    
  1006bc:	89 55 08             	mov    %edx,0x8(%ebp)
  1006bf:	c9                   	leave  
  1006c0:	e9 13 ff ff ff       	jmp    1005d8 <do_syscall>
  1006c5:	83 ec 08             	sub    $0x8,%esp
  1006c8:	6a 0f                	push   $0xf
  1006ca:	68 4c 10 10 00       	push   $0x10104c
  1006cf:	e8 78 05 00 00       	call   100c4c <abort>
  1006d4:	83 c4 10             	add    $0x10,%esp
  1006d7:	c9                   	leave  
  1006d8:	c3                   	ret    
  1006d9:	8d 76 00             	lea    0x0(%esi),%esi
  1006dc:	83 ec 08             	sub    $0x8,%esp
  1006df:	6a 11                	push   $0x11
  1006e1:	68 4c 10 10 00       	push   $0x10104c
  1006e6:	e8 61 05 00 00       	call   100c4c <abort>
  1006eb:	83 c4 10             	add    $0x10,%esp
  1006ee:	c9                   	leave  
  1006ef:	c3                   	ret    

001006f0 <init_intr>:
  1006f0:	55                   	push   %ebp
  1006f1:	89 e5                	mov    %esp,%ebp
  1006f3:	ba 21 00 00 00       	mov    $0x21,%edx
  1006f8:	b0 ff                	mov    $0xff,%al
  1006fa:	ee                   	out    %al,(%dx)
  1006fb:	ba a1 00 00 00       	mov    $0xa1,%edx
  100700:	ee                   	out    %al,(%dx)
  100701:	ba 20 00 00 00       	mov    $0x20,%edx
  100706:	b0 11                	mov    $0x11,%al
  100708:	ee                   	out    %al,(%dx)
  100709:	ba 21 00 00 00       	mov    $0x21,%edx
  10070e:	b0 20                	mov    $0x20,%al
  100710:	ee                   	out    %al,(%dx)
  100711:	b0 04                	mov    $0x4,%al
  100713:	ee                   	out    %al,(%dx)
  100714:	b0 03                	mov    $0x3,%al
  100716:	ee                   	out    %al,(%dx)
  100717:	ba a0 00 00 00       	mov    $0xa0,%edx
  10071c:	b0 11                	mov    $0x11,%al
  10071e:	ee                   	out    %al,(%dx)
  10071f:	ba a1 00 00 00       	mov    $0xa1,%edx
  100724:	b0 28                	mov    $0x28,%al
  100726:	ee                   	out    %al,(%dx)
  100727:	b0 02                	mov    $0x2,%al
  100729:	ee                   	out    %al,(%dx)
  10072a:	b0 03                	mov    $0x3,%al
  10072c:	ee                   	out    %al,(%dx)
  10072d:	ba 20 00 00 00       	mov    $0x20,%edx
  100732:	b0 68                	mov    $0x68,%al
  100734:	ee                   	out    %al,(%dx)
  100735:	b0 0a                	mov    $0xa,%al
  100737:	ee                   	out    %al,(%dx)
  100738:	ba a0 00 00 00       	mov    $0xa0,%edx
  10073d:	b0 68                	mov    $0x68,%al
  10073f:	ee                   	out    %al,(%dx)
  100740:	b0 0a                	mov    $0xa,%al
  100742:	ee                   	out    %al,(%dx)
  100743:	5d                   	pop    %ebp
  100744:	c3                   	ret    
  100745:	66 90                	xchg   %ax,%ax
  100747:	90                   	nop

00100748 <init_serial>:
  100748:	55                   	push   %ebp
  100749:	89 e5                	mov    %esp,%ebp
  10074b:	ba f9 03 00 00       	mov    $0x3f9,%edx
  100750:	31 c0                	xor    %eax,%eax
  100752:	ee                   	out    %al,(%dx)
  100753:	ba fb 03 00 00       	mov    $0x3fb,%edx
  100758:	b0 80                	mov    $0x80,%al
  10075a:	ee                   	out    %al,(%dx)
  10075b:	ba f8 03 00 00       	mov    $0x3f8,%edx
  100760:	b0 01                	mov    $0x1,%al
  100762:	ee                   	out    %al,(%dx)
  100763:	ba f9 03 00 00       	mov    $0x3f9,%edx
  100768:	31 c0                	xor    %eax,%eax
  10076a:	ee                   	out    %al,(%dx)
  10076b:	ba fb 03 00 00       	mov    $0x3fb,%edx
  100770:	b0 03                	mov    $0x3,%al
  100772:	ee                   	out    %al,(%dx)
  100773:	ba fa 03 00 00       	mov    $0x3fa,%edx
  100778:	b0 c7                	mov    $0xc7,%al
  10077a:	ee                   	out    %al,(%dx)
  10077b:	ba fc 03 00 00       	mov    $0x3fc,%edx
  100780:	b0 0b                	mov    $0xb,%al
  100782:	ee                   	out    %al,(%dx)
  100783:	5d                   	pop    %ebp
  100784:	c3                   	ret    
  100785:	8d 76 00             	lea    0x0(%esi),%esi

00100788 <putchar>:
  100788:	55                   	push   %ebp
  100789:	89 e5                	mov    %esp,%ebp
  10078b:	8b 4d 08             	mov    0x8(%ebp),%ecx
  10078e:	ba fd 03 00 00       	mov    $0x3fd,%edx
  100793:	90                   	nop
  100794:	ec                   	in     (%dx),%al
  100795:	a8 20                	test   $0x20,%al
  100797:	74 fb                	je     100794 <putchar+0xc>
  100799:	ba f8 03 00 00       	mov    $0x3f8,%edx
  10079e:	88 c8                	mov    %cl,%al
  1007a0:	ee                   	out    %al,(%dx)
  1007a1:	5d                   	pop    %ebp
  1007a2:	c3                   	ret    
  1007a3:	90                   	nop

001007a4 <prints>:
  1007a4:	55                   	push   %ebp
  1007a5:	89 e5                	mov    %esp,%ebp
  1007a7:	57                   	push   %edi
  1007a8:	56                   	push   %esi
  1007a9:	53                   	push   %ebx
  1007aa:	83 ec 1c             	sub    $0x1c,%esp
  1007ad:	89 c3                	mov    %eax,%ebx
  1007af:	89 d0                	mov    %edx,%eax
  1007b1:	89 cf                	mov    %ecx,%edi
  1007b3:	85 c9                	test   %ecx,%ecx
  1007b5:	0f 8e 15 01 00 00    	jle    1008d0 <prints+0x12c>
  1007bb:	31 d2                	xor    %edx,%edx
  1007bd:	80 38 00             	cmpb   $0x0,(%eax)
  1007c0:	0f 84 92 00 00 00    	je     100858 <prints+0xb4>
  1007c6:	66 90                	xchg   %ax,%ax
  1007c8:	42                   	inc    %edx
  1007c9:	80 3c 10 00          	cmpb   $0x0,(%eax,%edx,1)
  1007cd:	75 f9                	jne    1007c8 <prints+0x24>
  1007cf:	39 d7                	cmp    %edx,%edi
  1007d1:	0f 8f 81 00 00 00    	jg     100858 <prints+0xb4>
  1007d7:	31 f6                	xor    %esi,%esi
  1007d9:	8b 55 08             	mov    0x8(%ebp),%edx
  1007dc:	83 e2 02             	and    $0x2,%edx
  1007df:	83 fa 01             	cmp    $0x1,%edx
  1007e2:	19 c9                	sbb    %ecx,%ecx
  1007e4:	83 e1 f0             	and    $0xfffffff0,%ecx
  1007e7:	83 c1 30             	add    $0x30,%ecx
  1007ea:	f6 45 08 01          	testb  $0x1,0x8(%ebp)
  1007ee:	0f 85 c8 00 00 00    	jne    1008bc <prints+0x118>
  1007f4:	89 f7                	mov    %esi,%edi
  1007f6:	85 f6                	test   %esi,%esi
  1007f8:	0f 8e f0 00 00 00    	jle    1008ee <prints+0x14a>
  1007fe:	66 90                	xchg   %ax,%ax
  100800:	85 db                	test   %ebx,%ebx
  100802:	0f 84 94 00 00 00    	je     10089c <prints+0xf8>
  100808:	8b 13                	mov    (%ebx),%edx
  10080a:	88 0a                	mov    %cl,(%edx)
  10080c:	ff 03                	incl   (%ebx)
  10080e:	4f                   	dec    %edi
  10080f:	75 ef                	jne    100800 <prints+0x5c>
  100811:	0f be 10             	movsbl (%eax),%edx
  100814:	84 d2                	test   %dl,%dl
  100816:	74 33                	je     10084b <prints+0xa7>
  100818:	29 f0                	sub    %esi,%eax
  10081a:	89 7d e4             	mov    %edi,-0x1c(%ebp)
  10081d:	8d 76 00             	lea    0x0(%esi),%esi
  100820:	85 db                	test   %ebx,%ebx
  100822:	74 5c                	je     100880 <prints+0xdc>
  100824:	8b 3b                	mov    (%ebx),%edi
  100826:	88 17                	mov    %dl,(%edi)
  100828:	ff 03                	incl   (%ebx)
  10082a:	46                   	inc    %esi
  10082b:	0f be 14 30          	movsbl (%eax,%esi,1),%edx
  10082f:	84 d2                	test   %dl,%dl
  100831:	75 ed                	jne    100820 <prints+0x7c>
  100833:	8b 7d e4             	mov    -0x1c(%ebp),%edi
  100836:	85 ff                	test   %edi,%edi
  100838:	7e 11                	jle    10084b <prints+0xa7>
  10083a:	89 f8                	mov    %edi,%eax
  10083c:	85 db                	test   %ebx,%ebx
  10083e:	74 24                	je     100864 <prints+0xc0>
  100840:	8b 13                	mov    (%ebx),%edx
  100842:	88 0a                	mov    %cl,(%edx)
  100844:	ff 03                	incl   (%ebx)
  100846:	48                   	dec    %eax
  100847:	75 f3                	jne    10083c <prints+0x98>
  100849:	01 fe                	add    %edi,%esi
  10084b:	89 f0                	mov    %esi,%eax
  10084d:	8d 65 f4             	lea    -0xc(%ebp),%esp
  100850:	5b                   	pop    %ebx
  100851:	5e                   	pop    %esi
  100852:	5f                   	pop    %edi
  100853:	5d                   	pop    %ebp
  100854:	c3                   	ret    
  100855:	8d 76 00             	lea    0x0(%esi),%esi
  100858:	89 fe                	mov    %edi,%esi
  10085a:	29 d6                	sub    %edx,%esi
  10085c:	e9 78 ff ff ff       	jmp    1007d9 <prints+0x35>
  100861:	8d 76 00             	lea    0x0(%esi),%esi
  100864:	89 45 e0             	mov    %eax,-0x20(%ebp)
  100867:	83 ec 0c             	sub    $0xc,%esp
  10086a:	51                   	push   %ecx
  10086b:	89 4d e4             	mov    %ecx,-0x1c(%ebp)
  10086e:	e8 15 ff ff ff       	call   100788 <putchar>
  100873:	83 c4 10             	add    $0x10,%esp
  100876:	8b 45 e0             	mov    -0x20(%ebp),%eax
  100879:	8b 4d e4             	mov    -0x1c(%ebp),%ecx
  10087c:	eb c8                	jmp    100846 <prints+0xa2>
  10087e:	66 90                	xchg   %ax,%ax
  100880:	89 45 dc             	mov    %eax,-0x24(%ebp)
  100883:	89 4d e0             	mov    %ecx,-0x20(%ebp)
  100886:	83 ec 0c             	sub    $0xc,%esp
  100889:	52                   	push   %edx
  10088a:	e8 f9 fe ff ff       	call   100788 <putchar>
  10088f:	83 c4 10             	add    $0x10,%esp
  100892:	8b 45 dc             	mov    -0x24(%ebp),%eax
  100895:	8b 4d e0             	mov    -0x20(%ebp),%ecx
  100898:	eb 90                	jmp    10082a <prints+0x86>
  10089a:	66 90                	xchg   %ax,%ax
  10089c:	89 45 e0             	mov    %eax,-0x20(%ebp)
  10089f:	83 ec 0c             	sub    $0xc,%esp
  1008a2:	51                   	push   %ecx
  1008a3:	89 4d e4             	mov    %ecx,-0x1c(%ebp)
  1008a6:	e8 dd fe ff ff       	call   100788 <putchar>
  1008ab:	83 c4 10             	add    $0x10,%esp
  1008ae:	8b 45 e0             	mov    -0x20(%ebp),%eax
  1008b1:	8b 4d e4             	mov    -0x1c(%ebp),%ecx
  1008b4:	e9 55 ff ff ff       	jmp    10080e <prints+0x6a>
  1008b9:	8d 76 00             	lea    0x0(%esi),%esi
  1008bc:	0f be 10             	movsbl (%eax),%edx
  1008bf:	89 f7                	mov    %esi,%edi
  1008c1:	31 f6                	xor    %esi,%esi
  1008c3:	84 d2                	test   %dl,%dl
  1008c5:	0f 85 4d ff ff ff    	jne    100818 <prints+0x74>
  1008cb:	e9 66 ff ff ff       	jmp    100836 <prints+0x92>
  1008d0:	f6 45 08 01          	testb  $0x1,0x8(%ebp)
  1008d4:	74 13                	je     1008e9 <prints+0x145>
  1008d6:	0f be 12             	movsbl (%edx),%edx
  1008d9:	84 d2                	test   %dl,%dl
  1008db:	74 18                	je     1008f5 <prints+0x151>
  1008dd:	b9 20 00 00 00       	mov    $0x20,%ecx
  1008e2:	31 f6                	xor    %esi,%esi
  1008e4:	e9 2f ff ff ff       	jmp    100818 <prints+0x74>
  1008e9:	b9 20 00 00 00       	mov    $0x20,%ecx
  1008ee:	31 f6                	xor    %esi,%esi
  1008f0:	e9 1c ff ff ff       	jmp    100811 <prints+0x6d>
  1008f5:	31 f6                	xor    %esi,%esi
  1008f7:	e9 4f ff ff ff       	jmp    10084b <prints+0xa7>

001008fc <printi>:
  1008fc:	55                   	push   %ebp
  1008fd:	89 e5                	mov    %esp,%ebp
  1008ff:	57                   	push   %edi
  100900:	56                   	push   %esi
  100901:	53                   	push   %ebx
  100902:	83 ec 2c             	sub    $0x2c,%esp
  100905:	89 45 d4             	mov    %eax,-0x2c(%ebp)
  100908:	85 d2                	test   %edx,%edx
  10090a:	0f 84 c4 00 00 00    	je     1009d4 <printi+0xd8>
  100910:	89 d3                	mov    %edx,%ebx
  100912:	8b 75 08             	mov    0x8(%ebp),%esi
  100915:	85 f6                	test   %esi,%esi
  100917:	75 6f                	jne    100988 <printi+0x8c>
  100919:	c6 45 e7 00          	movb   $0x0,-0x19(%ebp)
  10091d:	c7 45 d0 00 00 00 00 	movl   $0x0,-0x30(%ebp)
  100924:	85 d2                	test   %edx,%edx
  100926:	0f 84 cc 00 00 00    	je     1009f8 <printi+0xfc>
  10092c:	8d 75 e7             	lea    -0x19(%ebp),%esi
  10092f:	8b 45 14             	mov    0x14(%ebp),%eax
  100932:	8d 78 c6             	lea    -0x3a(%eax),%edi
  100935:	8d 76 00             	lea    0x0(%esi),%esi
  100938:	89 d8                	mov    %ebx,%eax
  10093a:	31 d2                	xor    %edx,%edx
  10093c:	f7 f1                	div    %ecx
  10093e:	83 fa 09             	cmp    $0x9,%edx
  100941:	7e 02                	jle    100945 <printi+0x49>
  100943:	01 fa                	add    %edi,%edx
  100945:	4e                   	dec    %esi
  100946:	83 c2 30             	add    $0x30,%edx
  100949:	88 16                	mov    %dl,(%esi)
  10094b:	89 d8                	mov    %ebx,%eax
  10094d:	31 d2                	xor    %edx,%edx
  10094f:	f7 f1                	div    %ecx
  100951:	89 c3                	mov    %eax,%ebx
  100953:	85 c0                	test   %eax,%eax
  100955:	75 e1                	jne    100938 <printi+0x3c>
  100957:	8b 55 d0             	mov    -0x30(%ebp),%edx
  10095a:	85 d2                	test   %edx,%edx
  10095c:	74 46                	je     1009a4 <printi+0xa8>
  10095e:	8b 45 0c             	mov    0xc(%ebp),%eax
  100961:	85 c0                	test   %eax,%eax
  100963:	74 63                	je     1009c8 <printi+0xcc>
  100965:	f6 45 10 02          	testb  $0x2,0x10(%ebp)
  100969:	74 5d                	je     1009c8 <printi+0xcc>
  10096b:	8b 7d d4             	mov    -0x2c(%ebp),%edi
  10096e:	85 ff                	test   %edi,%edi
  100970:	0f 84 89 00 00 00    	je     1009ff <printi+0x103>
  100976:	8b 07                	mov    (%edi),%eax
  100978:	c6 00 2d             	movb   $0x2d,(%eax)
  10097b:	ff 07                	incl   (%edi)
  10097d:	ff 4d 0c             	decl   0xc(%ebp)
  100980:	bb 01 00 00 00       	mov    $0x1,%ebx
  100985:	eb 1f                	jmp    1009a6 <printi+0xaa>
  100987:	90                   	nop
  100988:	83 f9 0a             	cmp    $0xa,%ecx
  10098b:	75 8c                	jne    100919 <printi+0x1d>
  10098d:	85 d2                	test   %edx,%edx
  10098f:	79 88                	jns    100919 <printi+0x1d>
  100991:	c6 45 e7 00          	movb   $0x0,-0x19(%ebp)
  100995:	f7 db                	neg    %ebx
  100997:	74 78                	je     100a11 <printi+0x115>
  100999:	c7 45 d0 01 00 00 00 	movl   $0x1,-0x30(%ebp)
  1009a0:	eb 8a                	jmp    10092c <printi+0x30>
  1009a2:	66 90                	xchg   %ax,%ax
  1009a4:	31 db                	xor    %ebx,%ebx
  1009a6:	83 ec 0c             	sub    $0xc,%esp
  1009a9:	ff 75 10             	pushl  0x10(%ebp)
  1009ac:	8b 4d 0c             	mov    0xc(%ebp),%ecx
  1009af:	89 f2                	mov    %esi,%edx
  1009b1:	8b 45 d4             	mov    -0x2c(%ebp),%eax
  1009b4:	e8 eb fd ff ff       	call   1007a4 <prints>
  1009b9:	01 d8                	add    %ebx,%eax
  1009bb:	83 c4 10             	add    $0x10,%esp
  1009be:	8d 65 f4             	lea    -0xc(%ebp),%esp
  1009c1:	5b                   	pop    %ebx
  1009c2:	5e                   	pop    %esi
  1009c3:	5f                   	pop    %edi
  1009c4:	5d                   	pop    %ebp
  1009c5:	c3                   	ret    
  1009c6:	66 90                	xchg   %ax,%ax
  1009c8:	c6 46 ff 2d          	movb   $0x2d,-0x1(%esi)
  1009cc:	4e                   	dec    %esi
  1009cd:	31 db                	xor    %ebx,%ebx
  1009cf:	eb d5                	jmp    1009a6 <printi+0xaa>
  1009d1:	8d 76 00             	lea    0x0(%esi),%esi
  1009d4:	c6 45 dc 30          	movb   $0x30,-0x24(%ebp)
  1009d8:	c6 45 dd 00          	movb   $0x0,-0x23(%ebp)
  1009dc:	83 ec 0c             	sub    $0xc,%esp
  1009df:	ff 75 10             	pushl  0x10(%ebp)
  1009e2:	8b 4d 0c             	mov    0xc(%ebp),%ecx
  1009e5:	8d 55 dc             	lea    -0x24(%ebp),%edx
  1009e8:	e8 b7 fd ff ff       	call   1007a4 <prints>
  1009ed:	83 c4 10             	add    $0x10,%esp
  1009f0:	8d 65 f4             	lea    -0xc(%ebp),%esp
  1009f3:	5b                   	pop    %ebx
  1009f4:	5e                   	pop    %esi
  1009f5:	5f                   	pop    %edi
  1009f6:	5d                   	pop    %ebp
  1009f7:	c3                   	ret    
  1009f8:	31 db                	xor    %ebx,%ebx
  1009fa:	8d 75 e7             	lea    -0x19(%ebp),%esi
  1009fd:	eb a7                	jmp    1009a6 <printi+0xaa>
  1009ff:	83 ec 0c             	sub    $0xc,%esp
  100a02:	6a 2d                	push   $0x2d
  100a04:	e8 7f fd ff ff       	call   100788 <putchar>
  100a09:	83 c4 10             	add    $0x10,%esp
  100a0c:	e9 6c ff ff ff       	jmp    10097d <printi+0x81>
  100a11:	8d 75 e7             	lea    -0x19(%ebp),%esi
  100a14:	e9 45 ff ff ff       	jmp    10095e <printi+0x62>
  100a19:	8d 76 00             	lea    0x0(%esi),%esi

00100a1c <print>:
  100a1c:	55                   	push   %ebp
  100a1d:	89 e5                	mov    %esp,%ebp
  100a1f:	57                   	push   %edi
  100a20:	56                   	push   %esi
  100a21:	53                   	push   %ebx
  100a22:	83 ec 2c             	sub    $0x2c,%esp
  100a25:	89 c6                	mov    %eax,%esi
  100a27:	89 d0                	mov    %edx,%eax
  100a29:	89 4d d0             	mov    %ecx,-0x30(%ebp)
  100a2c:	8a 12                	mov    (%edx),%dl
  100a2e:	c7 45 d4 00 00 00 00 	movl   $0x0,-0x2c(%ebp)
  100a35:	84 d2                	test   %dl,%dl
  100a37:	75 2b                	jne    100a64 <print+0x48>
  100a39:	e9 ee 00 00 00       	jmp    100b2c <print+0x110>
  100a3e:	66 90                	xchg   %ax,%ax
  100a40:	0f be 10             	movsbl (%eax),%edx
  100a43:	89 c3                	mov    %eax,%ebx
  100a45:	85 f6                	test   %esi,%esi
  100a47:	0f 84 33 01 00 00    	je     100b80 <print+0x164>
  100a4d:	8b 06                	mov    (%esi),%eax
  100a4f:	88 10                	mov    %dl,(%eax)
  100a51:	ff 06                	incl   (%esi)
  100a53:	ff 45 d4             	incl   -0x2c(%ebp)
  100a56:	8d 43 01             	lea    0x1(%ebx),%eax
  100a59:	8a 53 01             	mov    0x1(%ebx),%dl
  100a5c:	84 d2                	test   %dl,%dl
  100a5e:	0f 84 c8 00 00 00    	je     100b2c <print+0x110>
  100a64:	80 fa 25             	cmp    $0x25,%dl
  100a67:	75 d7                	jne    100a40 <print+0x24>
  100a69:	8d 58 01             	lea    0x1(%eax),%ebx
  100a6c:	0f be 50 01          	movsbl 0x1(%eax),%edx
  100a70:	84 d2                	test   %dl,%dl
  100a72:	0f 84 b4 00 00 00    	je     100b2c <print+0x110>
  100a78:	80 fa 25             	cmp    $0x25,%dl
  100a7b:	74 c8                	je     100a45 <print+0x29>
  100a7d:	80 fa 2d             	cmp    $0x2d,%dl
  100a80:	0f 84 ba 00 00 00    	je     100b40 <print+0x124>
  100a86:	31 c0                	xor    %eax,%eax
  100a88:	eb 09                	jmp    100a93 <print+0x77>
  100a8a:	66 90                	xchg   %ax,%ax
  100a8c:	43                   	inc    %ebx
  100a8d:	83 c8 02             	or     $0x2,%eax
  100a90:	0f be 13             	movsbl (%ebx),%edx
  100a93:	80 fa 30             	cmp    $0x30,%dl
  100a96:	74 f4                	je     100a8c <print+0x70>
  100a98:	8d 4a d0             	lea    -0x30(%edx),%ecx
  100a9b:	80 f9 09             	cmp    $0x9,%cl
  100a9e:	b9 00 00 00 00       	mov    $0x0,%ecx
  100aa3:	77 19                	ja     100abe <print+0xa2>
  100aa5:	89 c7                	mov    %eax,%edi
  100aa7:	90                   	nop
  100aa8:	8d 0c 89             	lea    (%ecx,%ecx,4),%ecx
  100aab:	01 c9                	add    %ecx,%ecx
  100aad:	8d 4c 11 d0          	lea    -0x30(%ecx,%edx,1),%ecx
  100ab1:	43                   	inc    %ebx
  100ab2:	0f be 13             	movsbl (%ebx),%edx
  100ab5:	8d 42 d0             	lea    -0x30(%edx),%eax
  100ab8:	3c 09                	cmp    $0x9,%al
  100aba:	76 ec                	jbe    100aa8 <print+0x8c>
  100abc:	89 f8                	mov    %edi,%eax
  100abe:	80 fa 73             	cmp    $0x73,%dl
  100ac1:	0f 84 8d 00 00 00    	je     100b54 <print+0x138>
  100ac7:	80 fa 64             	cmp    $0x64,%dl
  100aca:	0f 84 c1 00 00 00    	je     100b91 <print+0x175>
  100ad0:	80 fa 78             	cmp    $0x78,%dl
  100ad3:	0f 84 e2 00 00 00    	je     100bbb <print+0x19f>
  100ad9:	80 fa 58             	cmp    $0x58,%dl
  100adc:	0f 84 03 01 00 00    	je     100be5 <print+0x1c9>
  100ae2:	80 fa 75             	cmp    $0x75,%dl
  100ae5:	0f 84 0f 01 00 00    	je     100bfa <print+0x1de>
  100aeb:	80 fa 63             	cmp    $0x63,%dl
  100aee:	0f 85 62 ff ff ff    	jne    100a56 <print+0x3a>
  100af4:	8b 7d d0             	mov    -0x30(%ebp),%edi
  100af7:	89 fa                	mov    %edi,%edx
  100af9:	83 c7 04             	add    $0x4,%edi
  100afc:	89 7d d0             	mov    %edi,-0x30(%ebp)
  100aff:	8b 12                	mov    (%edx),%edx
  100b01:	88 55 e6             	mov    %dl,-0x1a(%ebp)
  100b04:	c6 45 e7 00          	movb   $0x0,-0x19(%ebp)
  100b08:	83 ec 0c             	sub    $0xc,%esp
  100b0b:	50                   	push   %eax
  100b0c:	8d 55 e6             	lea    -0x1a(%ebp),%edx
  100b0f:	89 f0                	mov    %esi,%eax
  100b11:	e8 8e fc ff ff       	call   1007a4 <prints>
  100b16:	01 45 d4             	add    %eax,-0x2c(%ebp)
  100b19:	83 c4 10             	add    $0x10,%esp
  100b1c:	8d 43 01             	lea    0x1(%ebx),%eax
  100b1f:	8a 53 01             	mov    0x1(%ebx),%dl
  100b22:	84 d2                	test   %dl,%dl
  100b24:	0f 85 3a ff ff ff    	jne    100a64 <print+0x48>
  100b2a:	66 90                	xchg   %ax,%ax
  100b2c:	85 f6                	test   %esi,%esi
  100b2e:	74 05                	je     100b35 <print+0x119>
  100b30:	8b 06                	mov    (%esi),%eax
  100b32:	c6 00 00             	movb   $0x0,(%eax)
  100b35:	8b 45 d4             	mov    -0x2c(%ebp),%eax
  100b38:	8d 65 f4             	lea    -0xc(%ebp),%esp
  100b3b:	5b                   	pop    %ebx
  100b3c:	5e                   	pop    %esi
  100b3d:	5f                   	pop    %edi
  100b3e:	5d                   	pop    %ebp
  100b3f:	c3                   	ret    
  100b40:	8d 58 02             	lea    0x2(%eax),%ebx
  100b43:	0f be 50 02          	movsbl 0x2(%eax),%edx
  100b47:	b8 01 00 00 00       	mov    $0x1,%eax
  100b4c:	e9 42 ff ff ff       	jmp    100a93 <print+0x77>
  100b51:	8d 76 00             	lea    0x0(%esi),%esi
  100b54:	8b 55 d0             	mov    -0x30(%ebp),%edx
  100b57:	89 d7                	mov    %edx,%edi
  100b59:	83 c7 04             	add    $0x4,%edi
  100b5c:	8b 12                	mov    (%edx),%edx
  100b5e:	85 d2                	test   %edx,%edx
  100b60:	0f 84 a9 00 00 00    	je     100c0f <print+0x1f3>
  100b66:	83 ec 0c             	sub    $0xc,%esp
  100b69:	50                   	push   %eax
  100b6a:	89 f0                	mov    %esi,%eax
  100b6c:	e8 33 fc ff ff       	call   1007a4 <prints>
  100b71:	01 45 d4             	add    %eax,-0x2c(%ebp)
  100b74:	83 c4 10             	add    $0x10,%esp
  100b77:	89 7d d0             	mov    %edi,-0x30(%ebp)
  100b7a:	e9 d7 fe ff ff       	jmp    100a56 <print+0x3a>
  100b7f:	90                   	nop
  100b80:	83 ec 0c             	sub    $0xc,%esp
  100b83:	52                   	push   %edx
  100b84:	e8 ff fb ff ff       	call   100788 <putchar>
  100b89:	83 c4 10             	add    $0x10,%esp
  100b8c:	e9 c2 fe ff ff       	jmp    100a53 <print+0x37>
  100b91:	8b 55 d0             	mov    -0x30(%ebp),%edx
  100b94:	89 d7                	mov    %edx,%edi
  100b96:	83 c7 04             	add    $0x4,%edi
  100b99:	6a 61                	push   $0x61
  100b9b:	50                   	push   %eax
  100b9c:	51                   	push   %ecx
  100b9d:	6a 01                	push   $0x1
  100b9f:	b9 0a 00 00 00       	mov    $0xa,%ecx
  100ba4:	8b 12                	mov    (%edx),%edx
  100ba6:	89 f0                	mov    %esi,%eax
  100ba8:	e8 4f fd ff ff       	call   1008fc <printi>
  100bad:	01 45 d4             	add    %eax,-0x2c(%ebp)
  100bb0:	83 c4 10             	add    $0x10,%esp
  100bb3:	89 7d d0             	mov    %edi,-0x30(%ebp)
  100bb6:	e9 9b fe ff ff       	jmp    100a56 <print+0x3a>
  100bbb:	8b 7d d0             	mov    -0x30(%ebp),%edi
  100bbe:	89 fa                	mov    %edi,%edx
  100bc0:	83 c7 04             	add    $0x4,%edi
  100bc3:	89 7d d0             	mov    %edi,-0x30(%ebp)
  100bc6:	6a 61                	push   $0x61
  100bc8:	50                   	push   %eax
  100bc9:	51                   	push   %ecx
  100bca:	6a 00                	push   $0x0
  100bcc:	b9 10 00 00 00       	mov    $0x10,%ecx
  100bd1:	8b 12                	mov    (%edx),%edx
  100bd3:	89 f0                	mov    %esi,%eax
  100bd5:	e8 22 fd ff ff       	call   1008fc <printi>
  100bda:	01 45 d4             	add    %eax,-0x2c(%ebp)
  100bdd:	83 c4 10             	add    $0x10,%esp
  100be0:	e9 71 fe ff ff       	jmp    100a56 <print+0x3a>
  100be5:	8b 55 d0             	mov    -0x30(%ebp),%edx
  100be8:	89 d7                	mov    %edx,%edi
  100bea:	83 c7 04             	add    $0x4,%edi
  100bed:	6a 41                	push   $0x41
  100bef:	50                   	push   %eax
  100bf0:	51                   	push   %ecx
  100bf1:	6a 00                	push   $0x0
  100bf3:	b9 10 00 00 00       	mov    $0x10,%ecx
  100bf8:	eb aa                	jmp    100ba4 <print+0x188>
  100bfa:	8b 55 d0             	mov    -0x30(%ebp),%edx
  100bfd:	89 d7                	mov    %edx,%edi
  100bff:	83 c7 04             	add    $0x4,%edi
  100c02:	6a 61                	push   $0x61
  100c04:	50                   	push   %eax
  100c05:	51                   	push   %ecx
  100c06:	6a 00                	push   $0x0
  100c08:	b9 0a 00 00 00       	mov    $0xa,%ecx
  100c0d:	eb 95                	jmp    100ba4 <print+0x188>
  100c0f:	ba 9f 10 10 00       	mov    $0x10109f,%edx
  100c14:	e9 4d ff ff ff       	jmp    100b66 <print+0x14a>
  100c19:	8d 76 00             	lea    0x0(%esi),%esi

00100c1c <printk>:
  100c1c:	55                   	push   %ebp
  100c1d:	89 e5                	mov    %esp,%ebp
  100c1f:	83 ec 08             	sub    $0x8,%esp
  100c22:	8d 4d 0c             	lea    0xc(%ebp),%ecx
  100c25:	8b 55 08             	mov    0x8(%ebp),%edx
  100c28:	31 c0                	xor    %eax,%eax
  100c2a:	e8 ed fd ff ff       	call   100a1c <print>
  100c2f:	c9                   	leave  
  100c30:	c3                   	ret    
  100c31:	8d 76 00             	lea    0x0(%esi),%esi

00100c34 <sprintk>:
  100c34:	55                   	push   %ebp
  100c35:	89 e5                	mov    %esp,%ebp
  100c37:	83 ec 08             	sub    $0x8,%esp
  100c3a:	8d 4d 10             	lea    0x10(%ebp),%ecx
  100c3d:	8b 55 0c             	mov    0xc(%ebp),%edx
  100c40:	8d 45 08             	lea    0x8(%ebp),%eax
  100c43:	e8 d4 fd ff ff       	call   100a1c <print>
  100c48:	c9                   	leave  
  100c49:	c3                   	ret    
  100c4a:	66 90                	xchg   %ax,%ax

00100c4c <abort>:
  100c4c:	55                   	push   %ebp
  100c4d:	89 e5                	mov    %esp,%ebp
  100c4f:	57                   	push   %edi
  100c50:	56                   	push   %esi
  100c51:	53                   	push   %ebx
  100c52:	83 ec 1c             	sub    $0x1c,%esp
  100c55:	8b 55 08             	mov    0x8(%ebp),%edx
  100c58:	fa                   	cli    
  100c59:	8a 02                	mov    (%edx),%al
  100c5b:	b9 52 24 10 00       	mov    $0x102452,%ecx
  100c60:	84 c0                	test   %al,%al
  100c62:	74 0b                	je     100c6f <abort+0x23>
  100c64:	41                   	inc    %ecx
  100c65:	42                   	inc    %edx
  100c66:	88 41 ff             	mov    %al,-0x1(%ecx)
  100c69:	8a 02                	mov    (%edx),%al
  100c6b:	84 c0                	test   %al,%al
  100c6d:	75 f5                	jne    100c64 <abort+0x18>
  100c6f:	c6 01 3a             	movb   $0x3a,(%ecx)
  100c72:	bb 8d 25 10 00       	mov    $0x10258d,%ebx
  100c77:	be 0a 00 00 00       	mov    $0xa,%esi
  100c7c:	bf 67 66 66 66       	mov    $0x66666667,%edi
  100c81:	4b                   	dec    %ebx
  100c82:	8b 45 0c             	mov    0xc(%ebp),%eax
  100c85:	99                   	cltd   
  100c86:	f7 fe                	idiv   %esi
  100c88:	8d 42 30             	lea    0x30(%edx),%eax
  100c8b:	88 45 e7             	mov    %al,-0x19(%ebp)
  100c8e:	88 03                	mov    %al,(%ebx)
  100c90:	89 f8                	mov    %edi,%eax
  100c92:	f7 6d 0c             	imull  0xc(%ebp)
  100c95:	c1 fa 02             	sar    $0x2,%edx
  100c98:	8b 45 0c             	mov    0xc(%ebp),%eax
  100c9b:	c1 f8 1f             	sar    $0x1f,%eax
  100c9e:	29 c2                	sub    %eax,%edx
  100ca0:	89 55 0c             	mov    %edx,0xc(%ebp)
  100ca3:	75 dc                	jne    100c81 <abort+0x35>
  100ca5:	41                   	inc    %ecx
  100ca6:	41                   	inc    %ecx
  100ca7:	43                   	inc    %ebx
  100ca8:	8a 45 e7             	mov    -0x19(%ebp),%al
  100cab:	88 41 ff             	mov    %al,-0x1(%ecx)
  100cae:	8a 03                	mov    (%ebx),%al
  100cb0:	88 45 e7             	mov    %al,-0x19(%ebp)
  100cb3:	84 c0                	test   %al,%al
  100cb5:	75 ef                	jne    100ca6 <abort+0x5a>
  100cb7:	c6 01 0a             	movb   $0xa,(%ecx)
  100cba:	0f be 05 40 24 10 00 	movsbl 0x102440,%eax
  100cc1:	84 c0                	test   %al,%al
  100cc3:	74 1b                	je     100ce0 <abort+0x94>
  100cc5:	bb 40 24 10 00       	mov    $0x102440,%ebx
  100cca:	83 ec 0c             	sub    $0xc,%esp
  100ccd:	50                   	push   %eax
  100cce:	e8 b5 fa ff ff       	call   100788 <putchar>
  100cd3:	43                   	inc    %ebx
  100cd4:	0f be 03             	movsbl (%ebx),%eax
  100cd7:	83 c4 10             	add    $0x10,%esp
  100cda:	84 c0                	test   %al,%al
  100cdc:	75 ec                	jne    100cca <abort+0x7e>
  100cde:	66 90                	xchg   %ax,%ax
  100ce0:	f4                   	hlt    
  100ce1:	eb fd                	jmp    100ce0 <abort+0x94>
  100ce3:	90                   	nop

00100ce4 <init_vga>:
  100ce4:	55                   	push   %ebp
  100ce5:	89 e5                	mov    %esp,%ebp
  100ce7:	5d                   	pop    %ebp
  100ce8:	c3                   	ret    
  100ce9:	8d 76 00             	lea    0x0(%esi),%esi

00100cec <draw_pixel>:
  100cec:	55                   	push   %ebp
  100ced:	89 e5                	mov    %esp,%ebp
  100cef:	8b 45 08             	mov    0x8(%ebp),%eax
  100cf2:	8d 04 80             	lea    (%eax,%eax,4),%eax
  100cf5:	c1 e0 06             	shl    $0x6,%eax
  100cf8:	03 45 0c             	add    0xc(%ebp),%eax
  100cfb:	8b 15 40 25 10 00    	mov    0x102540,%edx
  100d01:	66 c7 04 42 00 00    	movw   $0x0,(%edx,%eax,2)
  100d07:	5d                   	pop    %ebp
  100d08:	c3                   	ret    
  100d09:	66 90                	xchg   %ax,%ax
  100d0b:	90                   	nop

00100d0c <waitdisk>:
  100d0c:	55                   	push   %ebp
  100d0d:	89 e5                	mov    %esp,%ebp
  100d0f:	ba f7 01 00 00       	mov    $0x1f7,%edx
  100d14:	ec                   	in     (%dx),%al
  100d15:	83 e0 c0             	and    $0xffffffc0,%eax
  100d18:	3c 40                	cmp    $0x40,%al
  100d1a:	75 f8                	jne    100d14 <waitdisk+0x8>
  100d1c:	5d                   	pop    %ebp
  100d1d:	c3                   	ret    
  100d1e:	66 90                	xchg   %ax,%ax

00100d20 <readsect>:
  100d20:	55                   	push   %ebp
  100d21:	89 e5                	mov    %esp,%ebp
  100d23:	53                   	push   %ebx
  100d24:	8b 4d 0c             	mov    0xc(%ebp),%ecx
  100d27:	ba f7 01 00 00       	mov    $0x1f7,%edx
  100d2c:	ec                   	in     (%dx),%al
  100d2d:	83 e0 c0             	and    $0xffffffc0,%eax
  100d30:	3c 40                	cmp    $0x40,%al
  100d32:	75 f8                	jne    100d2c <readsect+0xc>
  100d34:	ba f2 01 00 00       	mov    $0x1f2,%edx
  100d39:	b0 01                	mov    $0x1,%al
  100d3b:	ee                   	out    %al,(%dx)
  100d3c:	ba f3 01 00 00       	mov    $0x1f3,%edx
  100d41:	88 c8                	mov    %cl,%al
  100d43:	ee                   	out    %al,(%dx)
  100d44:	89 c8                	mov    %ecx,%eax
  100d46:	c1 f8 08             	sar    $0x8,%eax
  100d49:	ba f4 01 00 00       	mov    $0x1f4,%edx
  100d4e:	ee                   	out    %al,(%dx)
  100d4f:	89 c8                	mov    %ecx,%eax
  100d51:	c1 f8 10             	sar    $0x10,%eax
  100d54:	ba f5 01 00 00       	mov    $0x1f5,%edx
  100d59:	ee                   	out    %al,(%dx)
  100d5a:	89 c8                	mov    %ecx,%eax
  100d5c:	c1 f8 18             	sar    $0x18,%eax
  100d5f:	83 c8 e0             	or     $0xffffffe0,%eax
  100d62:	ba f6 01 00 00       	mov    $0x1f6,%edx
  100d67:	ee                   	out    %al,(%dx)
  100d68:	ba f7 01 00 00       	mov    $0x1f7,%edx
  100d6d:	b0 20                	mov    $0x20,%al
  100d6f:	ee                   	out    %al,(%dx)
  100d70:	ec                   	in     (%dx),%al
  100d71:	83 e0 c0             	and    $0xffffffc0,%eax
  100d74:	3c 40                	cmp    $0x40,%al
  100d76:	75 f8                	jne    100d70 <readsect+0x50>
  100d78:	8b 4d 08             	mov    0x8(%ebp),%ecx
  100d7b:	8d 99 00 02 00 00    	lea    0x200(%ecx),%ebx
  100d81:	ba f0 01 00 00       	mov    $0x1f0,%edx
  100d86:	66 90                	xchg   %ax,%ax
  100d88:	ed                   	in     (%dx),%eax
  100d89:	89 01                	mov    %eax,(%ecx)
  100d8b:	83 c1 04             	add    $0x4,%ecx
  100d8e:	39 d9                	cmp    %ebx,%ecx
  100d90:	75 f6                	jne    100d88 <readsect+0x68>
  100d92:	5b                   	pop    %ebx
  100d93:	5d                   	pop    %ebp
  100d94:	c3                   	ret    
  100d95:	8d 76 00             	lea    0x0(%esi),%esi

00100d98 <readdisk>:
  100d98:	55                   	push   %ebp
  100d99:	89 e5                	mov    %esp,%ebp
  100d9b:	57                   	push   %edi
  100d9c:	56                   	push   %esi
  100d9d:	53                   	push   %ebx
  100d9e:	8b 5d 08             	mov    0x8(%ebp),%ebx
  100da1:	8b 7d 10             	mov    0x10(%ebp),%edi
  100da4:	8b 75 0c             	mov    0xc(%ebp),%esi
  100da7:	01 de                	add    %ebx,%esi
  100da9:	89 f8                	mov    %edi,%eax
  100dab:	25 ff 01 00 80       	and    $0x800001ff,%eax
  100db0:	78 2a                	js     100ddc <readdisk+0x44>
  100db2:	29 c3                	sub    %eax,%ebx
  100db4:	85 ff                	test   %edi,%edi
  100db6:	78 31                	js     100de9 <readdisk+0x51>
  100db8:	c1 ff 09             	sar    $0x9,%edi
  100dbb:	47                   	inc    %edi
  100dbc:	39 de                	cmp    %ebx,%esi
  100dbe:	76 14                	jbe    100dd4 <readdisk+0x3c>
  100dc0:	57                   	push   %edi
  100dc1:	53                   	push   %ebx
  100dc2:	e8 59 ff ff ff       	call   100d20 <readsect>
  100dc7:	81 c3 00 02 00 00    	add    $0x200,%ebx
  100dcd:	47                   	inc    %edi
  100dce:	58                   	pop    %eax
  100dcf:	5a                   	pop    %edx
  100dd0:	39 de                	cmp    %ebx,%esi
  100dd2:	77 ec                	ja     100dc0 <readdisk+0x28>
  100dd4:	8d 65 f4             	lea    -0xc(%ebp),%esp
  100dd7:	5b                   	pop    %ebx
  100dd8:	5e                   	pop    %esi
  100dd9:	5f                   	pop    %edi
  100dda:	5d                   	pop    %ebp
  100ddb:	c3                   	ret    
  100ddc:	48                   	dec    %eax
  100ddd:	0d 00 fe ff ff       	or     $0xfffffe00,%eax
  100de2:	40                   	inc    %eax
  100de3:	29 c3                	sub    %eax,%ebx
  100de5:	85 ff                	test   %edi,%edi
  100de7:	79 cf                	jns    100db8 <readdisk+0x20>
  100de9:	81 c7 ff 01 00 00    	add    $0x1ff,%edi
  100def:	eb c7                	jmp    100db8 <readdisk+0x20>
  100df1:	8d 76 00             	lea    0x0(%esi),%esi

00100df4 <load_umain>:
  100df4:	55                   	push   %ebp
  100df5:	89 e5                	mov    %esp,%ebp
  100df7:	57                   	push   %edi
  100df8:	56                   	push   %esi
  100df9:	53                   	push   %ebx
  100dfa:	c7 05 60 2e 10 00 80 	movl   $0x102e80,0x102e60
  100e01:	2e 10 00 
  100e04:	be c9 00 00 00       	mov    $0xc9,%esi
  100e09:	bb 80 2e 10 00       	mov    $0x102e80,%ebx
  100e0e:	66 90                	xchg   %ax,%ax
  100e10:	56                   	push   %esi
  100e11:	53                   	push   %ebx
  100e12:	e8 09 ff ff ff       	call   100d20 <readsect>
  100e17:	81 c3 00 02 00 00    	add    $0x200,%ebx
  100e1d:	46                   	inc    %esi
  100e1e:	58                   	pop    %eax
  100e1f:	5a                   	pop    %edx
  100e20:	81 fb 80 3e 10 00    	cmp    $0x103e80,%ebx
  100e26:	75 e8                	jne    100e10 <load_umain+0x1c>
  100e28:	a1 60 2e 10 00       	mov    0x102e60,%eax
  100e2d:	8b 58 1c             	mov    0x1c(%eax),%ebx
  100e30:	01 c3                	add    %eax,%ebx
  100e32:	0f b7 78 2c          	movzwl 0x2c(%eax),%edi
  100e36:	c1 e7 05             	shl    $0x5,%edi
  100e39:	01 df                	add    %ebx,%edi
  100e3b:	39 fb                	cmp    %edi,%ebx
  100e3d:	72 0c                	jb     100e4b <load_umain+0x57>
  100e3f:	eb 58                	jmp    100e99 <load_umain+0xa5>
  100e41:	8d 76 00             	lea    0x0(%esi),%esi
  100e44:	83 c3 20             	add    $0x20,%ebx
  100e47:	39 df                	cmp    %ebx,%edi
  100e49:	76 49                	jbe    100e94 <load_umain+0xa0>
  100e4b:	83 3b 01             	cmpl   $0x1,(%ebx)
  100e4e:	75 f4                	jne    100e44 <load_umain+0x50>
  100e50:	8b 43 0c             	mov    0xc(%ebx),%eax
  100e53:	8d b0 00 00 20 00    	lea    0x200000(%eax),%esi
  100e59:	8b 43 04             	mov    0x4(%ebx),%eax
  100e5c:	05 00 90 01 00       	add    $0x19000,%eax
  100e61:	50                   	push   %eax
  100e62:	ff 73 10             	pushl  0x10(%ebx)
  100e65:	56                   	push   %esi
  100e66:	e8 2d ff ff ff       	call   100d98 <readdisk>
  100e6b:	8b 43 10             	mov    0x10(%ebx),%eax
  100e6e:	01 f0                	add    %esi,%eax
  100e70:	8b 53 14             	mov    0x14(%ebx),%edx
  100e73:	01 f2                	add    %esi,%edx
  100e75:	83 c4 0c             	add    $0xc,%esp
  100e78:	39 d0                	cmp    %edx,%eax
  100e7a:	73 c8                	jae    100e44 <load_umain+0x50>
  100e7c:	40                   	inc    %eax
  100e7d:	c6 40 ff 00          	movb   $0x0,-0x1(%eax)
  100e81:	8b 53 14             	mov    0x14(%ebx),%edx
  100e84:	01 f2                	add    %esi,%edx
  100e86:	39 d0                	cmp    %edx,%eax
  100e88:	72 f2                	jb     100e7c <load_umain+0x88>
  100e8a:	83 c3 20             	add    $0x20,%ebx
  100e8d:	39 df                	cmp    %ebx,%edi
  100e8f:	77 ba                	ja     100e4b <load_umain+0x57>
  100e91:	8d 76 00             	lea    0x0(%esi),%esi
  100e94:	a1 60 2e 10 00       	mov    0x102e60,%eax
  100e99:	8b 40 18             	mov    0x18(%eax),%eax
  100e9c:	8d 65 f4             	lea    -0xc(%ebp),%esp
  100e9f:	5b                   	pop    %ebx
  100ea0:	5e                   	pop    %esi
  100ea1:	5f                   	pop    %edi
  100ea2:	5d                   	pop    %ebp
  100ea3:	c3                   	ret    

00100ea4 <vec0>:
  100ea4:	6a 00                	push   $0x0
  100ea6:	eb 58                	jmp    100f00 <asm_do_irq>

00100ea8 <vec1>:
  100ea8:	6a 01                	push   $0x1
  100eaa:	eb 54                	jmp    100f00 <asm_do_irq>

00100eac <vec2>:
  100eac:	6a 02                	push   $0x2
  100eae:	eb 50                	jmp    100f00 <asm_do_irq>

00100eb0 <vec3>:
  100eb0:	6a 03                	push   $0x3
  100eb2:	eb 4c                	jmp    100f00 <asm_do_irq>

00100eb4 <vec4>:
  100eb4:	6a 04                	push   $0x4
  100eb6:	eb 48                	jmp    100f00 <asm_do_irq>

00100eb8 <vec5>:
  100eb8:	6a 05                	push   $0x5
  100eba:	eb 44                	jmp    100f00 <asm_do_irq>

00100ebc <vec6>:
  100ebc:	6a 06                	push   $0x6
  100ebe:	eb 40                	jmp    100f00 <asm_do_irq>

00100ec0 <vec7>:
  100ec0:	6a 07                	push   $0x7
  100ec2:	eb 3c                	jmp    100f00 <asm_do_irq>

00100ec4 <vec8>:
  100ec4:	6a 08                	push   $0x8
  100ec6:	eb 38                	jmp    100f00 <asm_do_irq>

00100ec8 <vec9>:
  100ec8:	6a 09                	push   $0x9
  100eca:	eb 34                	jmp    100f00 <asm_do_irq>

00100ecc <vec10>:
  100ecc:	6a 0a                	push   $0xa
  100ece:	eb 30                	jmp    100f00 <asm_do_irq>

00100ed0 <vec11>:
  100ed0:	6a 0b                	push   $0xb
  100ed2:	eb 2c                	jmp    100f00 <asm_do_irq>

00100ed4 <vec12>:
  100ed4:	6a 0c                	push   $0xc
  100ed6:	eb 28                	jmp    100f00 <asm_do_irq>

00100ed8 <vec13>:
  100ed8:	6a 0d                	push   $0xd
  100eda:	eb 24                	jmp    100f00 <asm_do_irq>

00100edc <vec14>:
  100edc:	6a 0e                	push   $0xe
  100ede:	eb 20                	jmp    100f00 <asm_do_irq>

00100ee0 <vecsys>:
  100ee0:	68 80 00 00 00       	push   $0x80
  100ee5:	eb 19                	jmp    100f00 <asm_do_irq>

00100ee7 <irq0>:
  100ee7:	68 e8 03 00 00       	push   $0x3e8
  100eec:	eb 12                	jmp    100f00 <asm_do_irq>

00100eee <irq1>:
  100eee:	68 e9 03 00 00       	push   $0x3e9
  100ef3:	eb 0b                	jmp    100f00 <asm_do_irq>

00100ef5 <irq14>:
  100ef5:	68 f6 03 00 00       	push   $0x3f6
  100efa:	eb 04                	jmp    100f00 <asm_do_irq>

00100efc <irq_empty>:
  100efc:	6a ff                	push   $0xffffffff
  100efe:	eb 00                	jmp    100f00 <asm_do_irq>

00100f00 <asm_do_irq>:
  100f00:	60                   	pusha  
  100f01:	54                   	push   %esp
  100f02:	66 b8 10 00          	mov    $0x10,%ax
  100f06:	8e c0                	mov    %eax,%es
  100f08:	8e d8                	mov    %eax,%ds
  100f0a:	e8 15 f7 ff ff       	call   100624 <irq_handle>
  100f0f:	66 b8 23 00          	mov    $0x23,%ax
  100f13:	8e c0                	mov    %eax,%es
  100f15:	8e d8                	mov    %eax,%ds
  100f17:	83 c4 04             	add    $0x4,%esp
  100f1a:	61                   	popa   
  100f1b:	83 c4 04             	add    $0x4,%esp
  100f1e:	cf                   	iret   
