
../../test/umain:     file format elf32-i386


Disassembly of section .text:

00200000 <main>:
  200000:	8d 4c 24 04          	lea    0x4(%esp),%ecx
  200004:	83 e4 f0             	and    $0xfffffff0,%esp
  200007:	ff 71 fc             	pushl  -0x4(%ecx)
  20000a:	55                   	push   %ebp
  20000b:	89 e5                	mov    %esp,%ebp
  20000d:	51                   	push   %ecx
  20000e:	83 ec 10             	sub    $0x10,%esp
  200011:	68 24 04 20 00       	push   $0x200424
  200016:	e8 21 02 00 00       	call   20023c <printf>
  20001b:	83 c4 10             	add    $0x10,%esp
  20001e:	b8 00 80 0b 00       	mov    $0xb8000,%eax
  200023:	90                   	nop
  200024:	66 c7 00 68 0d       	movw   $0xd68,(%eax)
  200029:	83 c0 02             	add    $0x2,%eax
  20002c:	3d c8 80 0b 00       	cmp    $0xb80c8,%eax
  200031:	75 f1                	jne    200024 <main+0x24>
  200033:	eb fe                	jmp    200033 <main+0x33>
  200035:	66 90                	xchg   %ax,%ax
  200037:	90                   	nop

00200038 <x2str.part.1>:
  200038:	85 d2                	test   %edx,%edx
  20003a:	74 68                	je     2000a4 <x2str.part.1+0x6c>
  20003c:	55                   	push   %ebp
  20003d:	89 e5                	mov    %esp,%ebp
  20003f:	53                   	push   %ebx
  200040:	89 d3                	mov    %edx,%ebx
  200042:	31 c9                	xor    %ecx,%ecx
  200044:	41                   	inc    %ecx
  200045:	c1 eb 04             	shr    $0x4,%ebx
  200048:	75 fa                	jne    200044 <x2str.part.1+0xc>
  20004a:	01 c8                	add    %ecx,%eax
  20004c:	c6 00 00             	movb   $0x0,(%eax)
  20004f:	90                   	nop
  200050:	89 d3                	mov    %edx,%ebx
  200052:	83 e3 0f             	and    $0xf,%ebx
  200055:	8d 4b f6             	lea    -0xa(%ebx),%ecx
  200058:	83 f9 05             	cmp    $0x5,%ecx
  20005b:	77 3f                	ja     20009c <x2str.part.1+0x64>
  20005d:	ff 24 8d 30 04 20 00 	jmp    *0x200430(,%ecx,4)
  200064:	c6 40 ff 65          	movb   $0x65,-0x1(%eax)
  200068:	48                   	dec    %eax
  200069:	c1 ea 04             	shr    $0x4,%edx
  20006c:	75 e2                	jne    200050 <x2str.part.1+0x18>
  20006e:	5b                   	pop    %ebx
  20006f:	5d                   	pop    %ebp
  200070:	c3                   	ret    
  200071:	8d 76 00             	lea    0x0(%esi),%esi
  200074:	c6 40 ff 64          	movb   $0x64,-0x1(%eax)
  200078:	eb ee                	jmp    200068 <x2str.part.1+0x30>
  20007a:	66 90                	xchg   %ax,%ax
  20007c:	c6 40 ff 63          	movb   $0x63,-0x1(%eax)
  200080:	eb e6                	jmp    200068 <x2str.part.1+0x30>
  200082:	66 90                	xchg   %ax,%ax
  200084:	c6 40 ff 62          	movb   $0x62,-0x1(%eax)
  200088:	eb de                	jmp    200068 <x2str.part.1+0x30>
  20008a:	66 90                	xchg   %ax,%ax
  20008c:	c6 40 ff 61          	movb   $0x61,-0x1(%eax)
  200090:	eb d6                	jmp    200068 <x2str.part.1+0x30>
  200092:	66 90                	xchg   %ax,%ax
  200094:	c6 40 ff 66          	movb   $0x66,-0x1(%eax)
  200098:	eb ce                	jmp    200068 <x2str.part.1+0x30>
  20009a:	66 90                	xchg   %ax,%ax
  20009c:	83 c3 30             	add    $0x30,%ebx
  20009f:	88 58 ff             	mov    %bl,-0x1(%eax)
  2000a2:	eb c4                	jmp    200068 <x2str.part.1+0x30>
  2000a4:	c6 00 00             	movb   $0x0,(%eax)
  2000a7:	c3                   	ret    

002000a8 <strlen>:
  2000a8:	55                   	push   %ebp
  2000a9:	89 e5                	mov    %esp,%ebp
  2000ab:	8b 55 08             	mov    0x8(%ebp),%edx
  2000ae:	31 c0                	xor    %eax,%eax
  2000b0:	80 3a 00             	cmpb   $0x0,(%edx)
  2000b3:	74 0a                	je     2000bf <strlen+0x17>
  2000b5:	8d 76 00             	lea    0x0(%esi),%esi
  2000b8:	40                   	inc    %eax
  2000b9:	80 3c 02 00          	cmpb   $0x0,(%edx,%eax,1)
  2000bd:	75 f9                	jne    2000b8 <strlen+0x10>
  2000bf:	5d                   	pop    %ebp
  2000c0:	c3                   	ret    
  2000c1:	8d 76 00             	lea    0x0(%esi),%esi

002000c4 <strcmp>:
  2000c4:	55                   	push   %ebp
  2000c5:	89 e5                	mov    %esp,%ebp
  2000c7:	56                   	push   %esi
  2000c8:	53                   	push   %ebx
  2000c9:	8b 4d 08             	mov    0x8(%ebp),%ecx
  2000cc:	8b 75 0c             	mov    0xc(%ebp),%esi
  2000cf:	8b 5d 10             	mov    0x10(%ebp),%ebx
  2000d2:	85 f6                	test   %esi,%esi
  2000d4:	7e 17                	jle    2000ed <strcmp+0x29>
  2000d6:	8a 03                	mov    (%ebx),%al
  2000d8:	38 01                	cmp    %al,(%ecx)
  2000da:	75 18                	jne    2000f4 <strcmp+0x30>
  2000dc:	31 c0                	xor    %eax,%eax
  2000de:	eb 08                	jmp    2000e8 <strcmp+0x24>
  2000e0:	8a 14 03             	mov    (%ebx,%eax,1),%dl
  2000e3:	38 14 01             	cmp    %dl,(%ecx,%eax,1)
  2000e6:	75 0c                	jne    2000f4 <strcmp+0x30>
  2000e8:	40                   	inc    %eax
  2000e9:	39 c6                	cmp    %eax,%esi
  2000eb:	75 f3                	jne    2000e0 <strcmp+0x1c>
  2000ed:	31 c0                	xor    %eax,%eax
  2000ef:	5b                   	pop    %ebx
  2000f0:	5e                   	pop    %esi
  2000f1:	5d                   	pop    %ebp
  2000f2:	c3                   	ret    
  2000f3:	90                   	nop
  2000f4:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
  2000f9:	5b                   	pop    %ebx
  2000fa:	5e                   	pop    %esi
  2000fb:	5d                   	pop    %ebp
  2000fc:	c3                   	ret    
  2000fd:	8d 76 00             	lea    0x0(%esi),%esi

00200100 <i2str>:
  200100:	55                   	push   %ebp
  200101:	89 e5                	mov    %esp,%ebp
  200103:	57                   	push   %edi
  200104:	56                   	push   %esi
  200105:	53                   	push   %ebx
  200106:	8b 4d 0c             	mov    0xc(%ebp),%ecx
  200109:	85 c9                	test   %ecx,%ecx
  20010b:	74 5f                	je     20016c <i2str+0x6c>
  20010d:	85 c9                	test   %ecx,%ecx
  20010f:	78 6b                	js     20017c <i2str+0x7c>
  200111:	89 ce                	mov    %ecx,%esi
  200113:	31 db                	xor    %ebx,%ebx
  200115:	bf 67 66 66 66       	mov    $0x66666667,%edi
  20011a:	66 90                	xchg   %ax,%ax
  20011c:	89 f0                	mov    %esi,%eax
  20011e:	f7 ef                	imul   %edi
  200120:	89 d0                	mov    %edx,%eax
  200122:	c1 f8 02             	sar    $0x2,%eax
  200125:	c1 fe 1f             	sar    $0x1f,%esi
  200128:	43                   	inc    %ebx
  200129:	29 f0                	sub    %esi,%eax
  20012b:	89 c6                	mov    %eax,%esi
  20012d:	75 ed                	jne    20011c <i2str+0x1c>
  20012f:	8b 45 08             	mov    0x8(%ebp),%eax
  200132:	c6 04 18 00          	movb   $0x0,(%eax,%ebx,1)
  200136:	03 5d 08             	add    0x8(%ebp),%ebx
  200139:	be 67 66 66 66       	mov    $0x66666667,%esi
  20013e:	66 90                	xchg   %ax,%ax
  200140:	89 c8                	mov    %ecx,%eax
  200142:	f7 ee                	imul   %esi
  200144:	89 d0                	mov    %edx,%eax
  200146:	c1 f8 02             	sar    $0x2,%eax
  200149:	89 ca                	mov    %ecx,%edx
  20014b:	c1 fa 1f             	sar    $0x1f,%edx
  20014e:	29 d0                	sub    %edx,%eax
  200150:	8d 14 80             	lea    (%eax,%eax,4),%edx
  200153:	01 d2                	add    %edx,%edx
  200155:	29 d1                	sub    %edx,%ecx
  200157:	83 c1 30             	add    $0x30,%ecx
  20015a:	88 4b ff             	mov    %cl,-0x1(%ebx)
  20015d:	89 c1                	mov    %eax,%ecx
  20015f:	4b                   	dec    %ebx
  200160:	85 c0                	test   %eax,%eax
  200162:	75 dc                	jne    200140 <i2str+0x40>
  200164:	5b                   	pop    %ebx
  200165:	5e                   	pop    %esi
  200166:	5f                   	pop    %edi
  200167:	5d                   	pop    %ebp
  200168:	c3                   	ret    
  200169:	8d 76 00             	lea    0x0(%esi),%esi
  20016c:	8b 45 08             	mov    0x8(%ebp),%eax
  20016f:	c6 00 30             	movb   $0x30,(%eax)
  200172:	c6 40 01 00          	movb   $0x0,0x1(%eax)
  200176:	5b                   	pop    %ebx
  200177:	5e                   	pop    %esi
  200178:	5f                   	pop    %edi
  200179:	5d                   	pop    %ebp
  20017a:	c3                   	ret    
  20017b:	90                   	nop
  20017c:	81 f9 00 00 00 80    	cmp    $0x80000000,%ecx
  200182:	74 44                	je     2001c8 <i2str+0xc8>
  200184:	f7 d9                	neg    %ecx
  200186:	0f 84 83 00 00 00    	je     20020f <i2str+0x10f>
  20018c:	89 cb                	mov    %ecx,%ebx
  20018e:	31 f6                	xor    %esi,%esi
  200190:	bf 67 66 66 66       	mov    $0x66666667,%edi
  200195:	eb 03                	jmp    20019a <i2str+0x9a>
  200197:	90                   	nop
  200198:	89 d6                	mov    %edx,%esi
  20019a:	89 d8                	mov    %ebx,%eax
  20019c:	f7 ef                	imul   %edi
  20019e:	89 d0                	mov    %edx,%eax
  2001a0:	c1 f8 02             	sar    $0x2,%eax
  2001a3:	c1 fb 1f             	sar    $0x1f,%ebx
  2001a6:	8d 56 01             	lea    0x1(%esi),%edx
  2001a9:	29 d8                	sub    %ebx,%eax
  2001ab:	89 c3                	mov    %eax,%ebx
  2001ad:	75 e9                	jne    200198 <i2str+0x98>
  2001af:	83 c6 02             	add    $0x2,%esi
  2001b2:	89 f3                	mov    %esi,%ebx
  2001b4:	8b 45 08             	mov    0x8(%ebp),%eax
  2001b7:	c6 00 2d             	movb   $0x2d,(%eax)
  2001ba:	c6 04 30 00          	movb   $0x0,(%eax,%esi,1)
  2001be:	85 c9                	test   %ecx,%ecx
  2001c0:	0f 85 70 ff ff ff    	jne    200136 <i2str+0x36>
  2001c6:	eb 9c                	jmp    200164 <i2str+0x64>
  2001c8:	8b 45 08             	mov    0x8(%ebp),%eax
  2001cb:	c6 00 2d             	movb   $0x2d,(%eax)
  2001ce:	c6 40 0a 00          	movb   $0x0,0xa(%eax)
  2001d2:	8d 58 0a             	lea    0xa(%eax),%ebx
  2001d5:	b9 ff ff ff 7f       	mov    $0x7fffffff,%ecx
  2001da:	be 0a 00 00 00       	mov    $0xa,%esi
  2001df:	bf 67 66 66 66       	mov    $0x66666667,%edi
  2001e4:	89 c8                	mov    %ecx,%eax
  2001e6:	99                   	cltd   
  2001e7:	f7 fe                	idiv   %esi
  2001e9:	83 c2 30             	add    $0x30,%edx
  2001ec:	88 13                	mov    %dl,(%ebx)
  2001ee:	89 c8                	mov    %ecx,%eax
  2001f0:	f7 ef                	imul   %edi
  2001f2:	89 d0                	mov    %edx,%eax
  2001f4:	c1 f8 02             	sar    $0x2,%eax
  2001f7:	c1 f9 1f             	sar    $0x1f,%ecx
  2001fa:	29 c8                	sub    %ecx,%eax
  2001fc:	89 c1                	mov    %eax,%ecx
  2001fe:	4b                   	dec    %ebx
  2001ff:	39 5d 08             	cmp    %ebx,0x8(%ebp)
  200202:	75 e0                	jne    2001e4 <i2str+0xe4>
  200204:	8b 45 08             	mov    0x8(%ebp),%eax
  200207:	fe 40 0a             	incb   0xa(%eax)
  20020a:	e9 55 ff ff ff       	jmp    200164 <i2str+0x64>
  20020f:	be 01 00 00 00       	mov    $0x1,%esi
  200214:	bb 01 00 00 00       	mov    $0x1,%ebx
  200219:	eb 99                	jmp    2001b4 <i2str+0xb4>
  20021b:	90                   	nop

0020021c <x2str>:
  20021c:	55                   	push   %ebp
  20021d:	89 e5                	mov    %esp,%ebp
  20021f:	8b 45 08             	mov    0x8(%ebp),%eax
  200222:	8b 55 0c             	mov    0xc(%ebp),%edx
  200225:	85 d2                	test   %edx,%edx
  200227:	74 07                	je     200230 <x2str+0x14>
  200229:	5d                   	pop    %ebp
  20022a:	e9 09 fe ff ff       	jmp    200038 <x2str.part.1>
  20022f:	90                   	nop
  200230:	c6 00 30             	movb   $0x30,(%eax)
  200233:	c6 40 01 00          	movb   $0x0,0x1(%eax)
  200237:	5d                   	pop    %ebp
  200238:	c3                   	ret    
  200239:	8d 76 00             	lea    0x0(%esi),%esi

0020023c <printf>:
  20023c:	55                   	push   %ebp
  20023d:	89 e5                	mov    %esp,%ebp
  20023f:	57                   	push   %edi
  200240:	56                   	push   %esi
  200241:	53                   	push   %ebx
  200242:	83 ec 6c             	sub    $0x6c,%esp
  200245:	8b 55 08             	mov    0x8(%ebp),%edx
  200248:	8a 02                	mov    (%edx),%al
  20024a:	84 c0                	test   %al,%al
  20024c:	74 58                	je     2002a6 <printf+0x6a>
  20024e:	89 d1                	mov    %edx,%ecx
  200250:	c7 45 94 00 00 00 00 	movl   $0x0,-0x6c(%ebp)
  200257:	31 db                	xor    %ebx,%ebx
  200259:	31 ff                	xor    %edi,%edi
  20025b:	8d 75 98             	lea    -0x68(%ebp),%esi
  20025e:	66 90                	xchg   %ax,%ax
  200260:	3c 25                	cmp    $0x25,%al
  200262:	0f 85 c5 00 00 00    	jne    20032d <printf+0xf1>
  200268:	8b 45 94             	mov    -0x6c(%ebp),%eax
  20026b:	8a 44 02 01          	mov    0x1(%edx,%eax,1),%al
  20026f:	3c 73                	cmp    $0x73,%al
  200271:	75 3b                	jne    2002ae <printf+0x72>
  200273:	8b 54 bd 0c          	mov    0xc(%ebp,%edi,4),%edx
  200277:	31 c0                	xor    %eax,%eax
  200279:	80 3a 00             	cmpb   $0x0,(%edx)
  20027c:	74 09                	je     200287 <printf+0x4b>
  20027e:	66 90                	xchg   %ax,%ax
  200280:	40                   	inc    %eax
  200281:	80 3c 02 00          	cmpb   $0x0,(%edx,%eax,1)
  200285:	75 f9                	jne    200280 <printf+0x44>
  200287:	83 ec 08             	sub    $0x8,%esp
  20028a:	50                   	push   %eax
  20028b:	52                   	push   %edx
  20028c:	e8 ef 00 00 00       	call   200380 <print>
  200291:	47                   	inc    %edi
  200292:	43                   	inc    %ebx
  200293:	83 c4 10             	add    $0x10,%esp
  200296:	43                   	inc    %ebx
  200297:	8b 55 08             	mov    0x8(%ebp),%edx
  20029a:	89 5d 94             	mov    %ebx,-0x6c(%ebp)
  20029d:	8d 0c 1a             	lea    (%edx,%ebx,1),%ecx
  2002a0:	8a 01                	mov    (%ecx),%al
  2002a2:	84 c0                	test   %al,%al
  2002a4:	75 ba                	jne    200260 <printf+0x24>
  2002a6:	8d 65 f4             	lea    -0xc(%ebp),%esp
  2002a9:	5b                   	pop    %ebx
  2002aa:	5e                   	pop    %esi
  2002ab:	5f                   	pop    %edi
  2002ac:	5d                   	pop    %ebp
  2002ad:	c3                   	ret    
  2002ae:	3c 64                	cmp    $0x64,%al
  2002b0:	75 30                	jne    2002e2 <printf+0xa6>
  2002b2:	83 ec 08             	sub    $0x8,%esp
  2002b5:	ff 74 bd 0c          	pushl  0xc(%ebp,%edi,4)
  2002b9:	56                   	push   %esi
  2002ba:	e8 41 fe ff ff       	call   200100 <i2str>
  2002bf:	83 c4 10             	add    $0x10,%esp
  2002c2:	31 c0                	xor    %eax,%eax
  2002c4:	80 7d 98 00          	cmpb   $0x0,-0x68(%ebp)
  2002c8:	74 07                	je     2002d1 <printf+0x95>
  2002ca:	40                   	inc    %eax
  2002cb:	80 3c 06 00          	cmpb   $0x0,(%esi,%eax,1)
  2002cf:	75 f9                	jne    2002ca <printf+0x8e>
  2002d1:	83 ec 08             	sub    $0x8,%esp
  2002d4:	50                   	push   %eax
  2002d5:	56                   	push   %esi
  2002d6:	e8 a5 00 00 00       	call   200380 <print>
  2002db:	47                   	inc    %edi
  2002dc:	43                   	inc    %ebx
  2002dd:	83 c4 10             	add    $0x10,%esp
  2002e0:	eb b4                	jmp    200296 <printf+0x5a>
  2002e2:	3c 78                	cmp    $0x78,%al
  2002e4:	75 2a                	jne    200310 <printf+0xd4>
  2002e6:	8b 54 bd 0c          	mov    0xc(%ebp,%edi,4),%edx
  2002ea:	85 d2                	test   %edx,%edx
  2002ec:	75 56                	jne    200344 <printf+0x108>
  2002ee:	c6 45 98 30          	movb   $0x30,-0x68(%ebp)
  2002f2:	c6 45 99 00          	movb   $0x0,-0x67(%ebp)
  2002f6:	31 c0                	xor    %eax,%eax
  2002f8:	40                   	inc    %eax
  2002f9:	80 3c 06 00          	cmpb   $0x0,(%esi,%eax,1)
  2002fd:	75 f9                	jne    2002f8 <printf+0xbc>
  2002ff:	83 ec 08             	sub    $0x8,%esp
  200302:	50                   	push   %eax
  200303:	56                   	push   %esi
  200304:	e8 77 00 00 00       	call   200380 <print>
  200309:	47                   	inc    %edi
  20030a:	43                   	inc    %ebx
  20030b:	83 c4 10             	add    $0x10,%esp
  20030e:	eb 86                	jmp    200296 <printf+0x5a>
  200310:	3c 63                	cmp    $0x63,%al
  200312:	75 1d                	jne    200331 <printf+0xf5>
  200314:	83 ec 08             	sub    $0x8,%esp
  200317:	6a 01                	push   $0x1
  200319:	8d 44 bd 0c          	lea    0xc(%ebp,%edi,4),%eax
  20031d:	50                   	push   %eax
  20031e:	e8 5d 00 00 00       	call   200380 <print>
  200323:	47                   	inc    %edi
  200324:	43                   	inc    %ebx
  200325:	83 c4 10             	add    $0x10,%esp
  200328:	e9 69 ff ff ff       	jmp    200296 <printf+0x5a>
  20032d:	3c 0a                	cmp    $0xa,%al
  20032f:	74 36                	je     200367 <printf+0x12b>
  200331:	83 ec 08             	sub    $0x8,%esp
  200334:	6a 01                	push   $0x1
  200336:	51                   	push   %ecx
  200337:	e8 44 00 00 00       	call   200380 <print>
  20033c:	83 c4 10             	add    $0x10,%esp
  20033f:	e9 52 ff ff ff       	jmp    200296 <printf+0x5a>
  200344:	89 f0                	mov    %esi,%eax
  200346:	e8 ed fc ff ff       	call   200038 <x2str.part.1>
  20034b:	80 7d 98 00          	cmpb   $0x0,-0x68(%ebp)
  20034f:	75 a5                	jne    2002f6 <printf+0xba>
  200351:	31 c0                	xor    %eax,%eax
  200353:	83 ec 08             	sub    $0x8,%esp
  200356:	50                   	push   %eax
  200357:	56                   	push   %esi
  200358:	e8 23 00 00 00       	call   200380 <print>
  20035d:	47                   	inc    %edi
  20035e:	43                   	inc    %ebx
  20035f:	83 c4 10             	add    $0x10,%esp
  200362:	e9 2f ff ff ff       	jmp    200296 <printf+0x5a>
  200367:	83 ec 08             	sub    $0x8,%esp
  20036a:	6a 01                	push   $0x1
  20036c:	68 48 04 20 00       	push   $0x200448
  200371:	e8 0a 00 00 00       	call   200380 <print>
  200376:	83 c4 10             	add    $0x10,%esp
  200379:	e9 18 ff ff ff       	jmp    200296 <printf+0x5a>
  20037e:	66 90                	xchg   %ax,%ax

00200380 <print>:
  200380:	55                   	push   %ebp
  200381:	89 e5                	mov    %esp,%ebp
  200383:	57                   	push   %edi
  200384:	56                   	push   %esi
  200385:	53                   	push   %ebx
  200386:	51                   	push   %ecx
  200387:	8b 4d 08             	mov    0x8(%ebp),%ecx
  20038a:	80 39 5c             	cmpb   $0x5c,(%ecx)
  20038d:	74 65                	je     2003f4 <print+0x74>
  20038f:	a1 cc 15 20 00       	mov    0x2015cc,%eax
  200394:	89 45 f0             	mov    %eax,-0x10(%ebp)
  200397:	89 c6                	mov    %eax,%esi
  200399:	8b 3d d0 15 20 00    	mov    0x2015d0,%edi
  20039f:	ba 01 00 00 00       	mov    $0x1,%edx
  2003a4:	b8 04 00 00 00       	mov    $0x4,%eax
  2003a9:	8b 5d 0c             	mov    0xc(%ebp),%ebx
  2003ac:	cd 80                	int    $0x80
  2003ae:	89 c3                	mov    %eax,%ebx
  2003b0:	8b 4d f0             	mov    -0x10(%ebp),%ecx
  2003b3:	01 c1                	add    %eax,%ecx
  2003b5:	89 0d cc 15 20 00    	mov    %ecx,0x2015cc
  2003bb:	83 f9 4f             	cmp    $0x4f,%ecx
  2003be:	7e 29                	jle    2003e9 <print+0x69>
  2003c0:	b8 67 66 66 66       	mov    $0x66666667,%eax
  2003c5:	f7 e9                	imul   %ecx
  2003c7:	89 d0                	mov    %edx,%eax
  2003c9:	c1 f8 05             	sar    $0x5,%eax
  2003cc:	89 ca                	mov    %ecx,%edx
  2003ce:	c1 fa 1f             	sar    $0x1f,%edx
  2003d1:	29 d0                	sub    %edx,%eax
  2003d3:	01 c7                	add    %eax,%edi
  2003d5:	89 3d d0 15 20 00    	mov    %edi,0x2015d0
  2003db:	8d 04 80             	lea    (%eax,%eax,4),%eax
  2003de:	c1 e0 04             	shl    $0x4,%eax
  2003e1:	29 c1                	sub    %eax,%ecx
  2003e3:	89 0d cc 15 20 00    	mov    %ecx,0x2015cc
  2003e9:	85 db                	test   %ebx,%ebx
  2003eb:	78 05                	js     2003f2 <print+0x72>
  2003ed:	39 5d 0c             	cmp    %ebx,0xc(%ebp)
  2003f0:	74 27                	je     200419 <print+0x99>
  2003f2:	eb fe                	jmp    2003f2 <print+0x72>
  2003f4:	80 79 01 6e          	cmpb   $0x6e,0x1(%ecx)
  2003f8:	75 95                	jne    20038f <print+0xf>
  2003fa:	80 79 02 00          	cmpb   $0x0,0x2(%ecx)
  2003fe:	75 8f                	jne    20038f <print+0xf>
  200400:	a1 d0 15 20 00       	mov    0x2015d0,%eax
  200405:	8d 78 01             	lea    0x1(%eax),%edi
  200408:	89 3d d0 15 20 00    	mov    %edi,0x2015d0
  20040e:	31 f6                	xor    %esi,%esi
  200410:	c7 45 f0 00 00 00 00 	movl   $0x0,-0x10(%ebp)
  200417:	eb 86                	jmp    20039f <print+0x1f>
  200419:	8b 45 0c             	mov    0xc(%ebp),%eax
  20041c:	5a                   	pop    %edx
  20041d:	5b                   	pop    %ebx
  20041e:	5e                   	pop    %esi
  20041f:	5f                   	pop    %edi
  200420:	5d                   	pop    %ebp
  200421:	c3                   	ret    
