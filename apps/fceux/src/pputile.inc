uint8 *C;
register uint8 cc;
uint32 vadr;

register uint8 zz;

if (X1 >= 2) {
	uint8 *S = PALRAM;
	uint32 pixdata;

	pixdata = ppulut1[(pshift[0] >> (8 - XOffset)) & 0xFF] | ppulut2[(pshift[1] >> (8 - XOffset)) & 0xFF];

	pixdata |= ppulut3[XOffset | (atlatch << 3)];

	P[0] = S[pixdata & 0xF];
	pixdata >>= 4;
	P[1] = S[pixdata & 0xF];
	pixdata >>= 4;
	P[2] = S[pixdata & 0xF];
	pixdata >>= 4;
	P[3] = S[pixdata & 0xF];
	pixdata >>= 4;
	P[4] = S[pixdata & 0xF];
	pixdata >>= 4;
	P[5] = S[pixdata & 0xF];
	pixdata >>= 4;
	P[6] = S[pixdata & 0xF];
	pixdata >>= 4;
	P[7] = S[pixdata & 0xF];
	P += 8;
}

	zz = RefreshAddr & 0x1F;
	C = vnapage[(RefreshAddr >> 10) & 3];
	vadr = (C[RefreshAddr & 0x3ff] << 4) + vofs;				// Fetch name table byte.

  cc = C[0x3c0 + (zz >> 2) + ((RefreshAddr & 0x380) >> 4)];	// Fetch attribute table byte.
  cc = ((cc >> ((zz & 2) + ((RefreshAddr & 0x40) >> 4))) & 3);

atlatch >>= 2;
atlatch |= cc << 2;

pshift[0] <<= 8;
pshift[1] <<= 8;


		C = VRAMADR(vadr);


	if(ScreenON)
		RENDER_LOGP(C);
	pshift[0] |= C[0];
	if(ScreenON)
		RENDER_LOGP(C + 8);
	pshift[1] |= C[8];

if ((RefreshAddr & 0x1f) == 0x1f)
	RefreshAddr ^= 0x41F;
else
	RefreshAddr++;
