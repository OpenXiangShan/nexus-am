uint8 *C;
uint32 vadr;

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

	vadr = (C0[RefreshAddr & 0x3ff] << 4) + vofs;				// Fetch name table byte.

  if (RefreshAddr % 4 == 0) {
    uint8 zz = RefreshAddr >> 2;
    cc = (C0[0x3c0 | (zz & 0x7) | ((zz >> 2) & 0x38)] << 2) >> ((zz >> 2) & 0x4);	// Fetch attribute table byte.
  }
  cc2 = (cc >> (RefreshAddr & 0x2)) & 0xc;

atlatch >>= 2;
atlatch |= cc2;

pshift[0] <<= 8;
pshift[1] <<= 8;


		C = VRAMADR(vadr);


	if(ScreenON)
		RENDER_LOGP(C);
	pshift[0] |= C[0];
	if(ScreenON)
		RENDER_LOGP(C + 8);
	pshift[1] |= C[8];

if ((RefreshAddr & 0x1f) == 0x1f) {
	RefreshAddr ^= 0x41F;
	C0 = vnapage[(RefreshAddr >> 10) & 3];
}
else
	RefreshAddr++;
