/**
 * Copyright (C) 2006-2020 Henning Norén
 * Copyright (C) 1996-2005 Glyph & Cog, LLC.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
 * USA.
 */

#include "md5.h"

#define ROTATE_LEFT(x, r) ((x << r) | (x >> (32 - r)))

#define RnA(a, b, s) a = ROTATE_LEFT(a, s); a += b

/** MD5_ROUND1 optimized according to Colin Plumb's implementation */
#define MD5_ROUND1(a, b, c, d, Xk, s, Ti) \
  a += (d ^ (b & (c ^ d))) + Xk + Ti;	  \
  RnA(a,b,s)

/** MD5_ROUND2 optimized like above */
#define MD5_ROUND2(a, b, c, d, Xk, s, Ti) \
  a += (c ^ (d & (b ^ c))) + Xk + Ti;	  \
  RnA(a,b,s)

#define MD5_ROUND3(a, b, c, d, Xk, s, Ti) \
  a += (b ^ c ^ d) + Xk + Ti;		  \
  RnA(a,b,s)

#define MD5_ROUND4(a, b, c, d, Xk, s, Ti) \
  a += (c ^ (b | ~d)) + Xk + Ti;	  \
  RnA(a,b,s)

#define AA 0x67452301
#define BB 0xefcdab89
#define CC 0x98badcfe
#define DD 0x10325476

static void (*md5_50_variant)();
static void md5_50f(uint8_t *msg, const unsigned int msgLen);
static void md5_50s(uint8_t *msg, const unsigned int msgLen);

void 
md5(const uint8_t *msg, const unsigned int msgLen, uint8_t *digest) {
  uint32_t x[16];
  register uint32_t a, b, c, d;
  uint32_t aa, bb, cc, dd;
  int n64;
  int i, j;
  unsigned int k;
  
  /** compute number of 64-byte blocks
      (length + pad byte (0x80) + 8 bytes for length) */
  n64 = (msgLen + 72) / 64;
  
  /** initialize a, b, c, d */
  a = AA;
  b = BB;
  c = CC;
  d = DD;

  /** loop through blocks */
  k = 0;
  for (i = 0; i < n64; ++i) {

    /** grab a 64-byte block */
    for (j = 0; j < 16 && (signed)k < (signed)msgLen - 3; ++j, k += 4)
      x[j] = ((((((unsigned)msg[k+3] << 8) + (unsigned)msg[k+2]) << 8) + (unsigned)msg[k+1]) << 8) + msg[k];
    if (i == n64 - 1) {
      if (k == msgLen - 3)
	x[j] = 0x80000000 + ((((unsigned)msg[k+2] << 8) + (unsigned)msg[k+1]) << 8) + msg[k];
      else if (k == msgLen - 2)
	x[j] = 0x800000 + ((unsigned)msg[k+1] << 8) + msg[k];
      else if (k == msgLen - 1)
	x[j] = 0x8000 + msg[k];
      else
	x[j] = 0x80;
      ++j;
      while (j < 16)
	x[j++] = 0;
      x[14] = msgLen << 3;
    }
    
    /** save a, b, c, d */
    aa = a;
    bb = b;
    cc = c;
    dd = d;
    
    /** round 1 */
    MD5_ROUND1(a, b, c, d, x[0],   7, 0xd76aa478);
    MD5_ROUND1(d, a, b, c, x[1],  12, 0xe8c7b756);
    MD5_ROUND1(c, d, a, b, x[2],  17, 0x242070db);
    MD5_ROUND1(b, c, d, a, x[3],  22, 0xc1bdceee);
    MD5_ROUND1(a, b, c, d, x[4],   7, 0xf57c0faf);
    MD5_ROUND1(d, a, b, c, x[5],  12, 0x4787c62a);
    MD5_ROUND1(c, d, a, b, x[6],  17, 0xa8304613);
    MD5_ROUND1(b, c, d, a, x[7],  22, 0xfd469501);
    MD5_ROUND1(a, b, c, d, x[8],   7, 0x698098d8);
    MD5_ROUND1(d, a, b, c, x[9],  12, 0x8b44f7af);
    MD5_ROUND1(c, d, a, b, x[10], 17, 0xffff5bb1);
    MD5_ROUND1(b, c, d, a, x[11], 22, 0x895cd7be);
    MD5_ROUND1(a, b, c, d, x[12],  7, 0x6b901122);
    MD5_ROUND1(d, a, b, c, x[13], 12, 0xfd987193);
    MD5_ROUND1(c, d, a, b, x[14], 17, 0xa679438e);
    MD5_ROUND1(b, c, d, a, x[15], 22, 0x49b40821);

    /** round 2 */
    MD5_ROUND2(a, b, c, d, x[1],   5, 0xf61e2562);
    MD5_ROUND2(d, a, b, c, x[6],   9, 0xc040b340);
    MD5_ROUND2(c, d, a, b, x[11], 14, 0x265e5a51);
    MD5_ROUND2(b, c, d, a, x[0],  20, 0xe9b6c7aa);
    MD5_ROUND2(a, b, c, d, x[5],   5, 0xd62f105d);
    MD5_ROUND2(d, a, b, c, x[10],  9, 0x02441453);
    MD5_ROUND2(c, d, a, b, x[15], 14, 0xd8a1e681);
    MD5_ROUND2(b, c, d, a, x[4],  20, 0xe7d3fbc8);
    MD5_ROUND2(a, b, c, d, x[9],   5, 0x21e1cde6);
    MD5_ROUND2(d, a, b, c, x[14],  9, 0xc33707d6);
    MD5_ROUND2(c, d, a, b, x[3],  14, 0xf4d50d87);
    MD5_ROUND2(b, c, d, a, x[8],  20, 0x455a14ed);
    MD5_ROUND2(a, b, c, d, x[13],  5, 0xa9e3e905);
    MD5_ROUND2(d, a, b, c, x[2],   9, 0xfcefa3f8);
    MD5_ROUND2(c, d, a, b, x[7],  14, 0x676f02d9);
    MD5_ROUND2(b, c, d, a, x[12], 20, 0x8d2a4c8a);

    /** round 3 */
    MD5_ROUND3(a, b, c, d, x[5],   4, 0xfffa3942);
    MD5_ROUND3(d, a, b, c, x[8],  11, 0x8771f681);
    MD5_ROUND3(c, d, a, b, x[11], 16, 0x6d9d6122);
    MD5_ROUND3(b, c, d, a, x[14], 23, 0xfde5380c);
    MD5_ROUND3(a, b, c, d, x[1],   4, 0xa4beea44);
    MD5_ROUND3(d, a, b, c, x[4],  11, 0x4bdecfa9);
    MD5_ROUND3(c, d, a, b, x[7],  16, 0xf6bb4b60);
    MD5_ROUND3(b, c, d, a, x[10], 23, 0xbebfbc70);
    MD5_ROUND3(a, b, c, d, x[13],  4, 0x289b7ec6);
    MD5_ROUND3(d, a, b, c, x[0],  11, 0xeaa127fa);
    MD5_ROUND3(c, d, a, b, x[3],  16, 0xd4ef3085);
    MD5_ROUND3(b, c, d, a, x[6],  23, 0x04881d05);
    MD5_ROUND3(a, b, c, d, x[9],   4, 0xd9d4d039);
    MD5_ROUND3(d, a, b, c, x[12], 11, 0xe6db99e5);
    MD5_ROUND3(c, d, a, b, x[15], 16, 0x1fa27cf8);
    MD5_ROUND3(b, c, d, a, x[2],  23, 0xc4ac5665);

    /** round 4 */
    MD5_ROUND4(a, b, c, d, x[0],   6, 0xf4292244);
    MD5_ROUND4(d, a, b, c, x[7],  10, 0x432aff97);
    MD5_ROUND4(c, d, a, b, x[14], 15, 0xab9423a7);
    MD5_ROUND4(b, c, d, a, x[5],  21, 0xfc93a039);
    MD5_ROUND4(a, b, c, d, x[12],  6, 0x655b59c3);
    MD5_ROUND4(d, a, b, c, x[3],  10, 0x8f0ccc92);
    MD5_ROUND4(c, d, a, b, x[10], 15, 0xffeff47d);
    MD5_ROUND4(b, c, d, a, x[1],  21, 0x85845dd1);
    MD5_ROUND4(a, b, c, d, x[8],   6, 0x6fa87e4f);
    MD5_ROUND4(d, a, b, c, x[15], 10, 0xfe2ce6e0);
    MD5_ROUND4(c, d, a, b, x[6],  15, 0xa3014314);
    MD5_ROUND4(b, c, d, a, x[13], 21, 0x4e0811a1);
    MD5_ROUND4(a, b, c, d, x[4],   6, 0xf7537e82);
    MD5_ROUND4(d, a, b, c, x[11], 10, 0xbd3af235);
    MD5_ROUND4(c, d, a, b, x[2],  15, 0x2ad7d2bb);
    MD5_ROUND4(b, c, d, a, x[9],  21, 0xeb86d391);

    /** increment a, b, c, d */
    a += aa;
    b += bb;
    c += cc;
    d += dd;
  }

  /** break digest into bytes */
  digest[ 0] = (uint8_t)(a & 0xff);
  digest[ 1] = (uint8_t)((a >>= 8) & 0xff);
  digest[ 2] = (uint8_t)((a >>= 8) & 0xff);
  digest[ 3] = (uint8_t)((a >>= 8) & 0xff);
  digest[ 4] = (uint8_t)(b & 0xff);
  digest[ 5] = (uint8_t)((b >>= 8) & 0xff);
  digest[ 6] = (uint8_t)((b >>= 8) & 0xff);
  digest[ 7] = (uint8_t)((b >>= 8) & 0xff);
  digest[ 8] = (uint8_t)(c & 0xff);
  digest[ 9] = (uint8_t)((c >>= 8) & 0xff);
  digest[10] = (uint8_t)((c >>= 8) & 0xff);
  digest[11] = (uint8_t)((c >>= 8) & 0xff);
  digest[12] = (uint8_t)(d & 0xff);
  digest[13] = (uint8_t)((d >>= 8) & 0xff);
  digest[14] = (uint8_t)((d >>= 8) & 0xff);
  digest[15] = (uint8_t)((d >>= 8) & 0xff);
}

static void
md5_50s(uint8_t *msg, const unsigned int msgLen) {
  int i;
  for(i=0; i<50; i++) { md5(msg, msgLen, msg); }
}

/** fast version of "for(i=0; i<50; i++) { md5(msg, 16, msg); }" */
static void 
md5_50f(uint8_t *msg, const unsigned int msgLen __attribute__((unused))) {
  register uint32_t a, b, c, d;
  int i;

  a = ((((((unsigned)msg[ 3] << 8) + (unsigned)msg[ 2]) << 8) + (unsigned)msg[ 1]) << 8) + msg[ 0];
  b = ((((((unsigned)msg[ 7] << 8) + (unsigned)msg[ 6]) << 8) + (unsigned)msg[ 5]) << 8) + msg[ 4];
  c = ((((((unsigned)msg[11] << 8) + (unsigned)msg[10]) << 8) + (unsigned)msg[ 9]) << 8) + msg[ 8];
  d = ((((((unsigned)msg[15] << 8) + (unsigned)msg[14]) << 8) + (unsigned)msg[13]) << 8) + msg[12];

  for(i = 0; i < 50; i++) {
    const uint32_t aa=a, bb=b, cc=c, dd=d;

    /** round 1 */
    /**MD5_ROUND1(a,BB,CC,DD, aa, 7, 0xd76aa478);
       MD5_ROUND1(d, a,BB,CC, bb,12, 0xe8c7b756);
       MD5_ROUND1(c, d, a,BB, cc,17, 0x242070db);
       MD5_ROUND1(b, c, d, a, dd,22, 0xc1bdceee);*/
    a += 0xd76aa477;
    RnA(a,BB,7);
    d = 0xf8fa0bcc + b + (CC ^ (a & 0x77777777));
    RnA(d,a,12);
    c += 0xbcdb4dd9 + (BB ^ (d & (a ^ BB)));
    RnA(c,d,17);
    b = 0xb18b7a77 + dd + ( a ^ (c & (d ^ a)));
    RnA(b,c,22);
    MD5_ROUND1(a, b, c, d, 0x80, 7, 0xf57c0faf);
    MD5_ROUND1(d, a, b, c, 0,   12, 0x4787c62a);
    MD5_ROUND1(c, d, a, b, 0,   17, 0xa8304613);
    MD5_ROUND1(b, c, d, a, 0,   22, 0xfd469501);
    MD5_ROUND1(a, b, c, d, 0,    7, 0x698098d8);
    MD5_ROUND1(d, a, b, c, 0,   12, 0x8b44f7af);
    MD5_ROUND1(c, d, a, b, 0,   17, 0xffff5bb1);
    MD5_ROUND1(b, c, d, a, 0,   22, 0x895cd7be);
    MD5_ROUND1(a, b, c, d, 0,    7, 0x6b901122);
    MD5_ROUND1(d, a, b, c, 0,   12, 0xfd987193);
    MD5_ROUND1(c, d, a, b, 0x80,17, 0xa679438e);
    MD5_ROUND1(b, c, d, a, 0,   22, 0x49b40821);
      
    /** round 2 */
    MD5_ROUND2(a, b, c, d, bb,   5, 0xf61e2562);
    MD5_ROUND2(d, a, b, c, 0,    9, 0xc040b340);
    MD5_ROUND2(c, d, a, b, 0,   14, 0x265e5a51);
    MD5_ROUND2(b, c, d, a, aa,  20, 0xe9b6c7aa);
    MD5_ROUND2(a, b, c, d, 0,    5, 0xd62f105d);
    MD5_ROUND2(d, a, b, c, 0,    9, 0x02441453);
    MD5_ROUND2(c, d, a, b, 0,   14, 0xd8a1e681);
    MD5_ROUND2(b, c, d, a, 0x80,20, 0xe7d3fbc8);
    MD5_ROUND2(a, b, c, d, 0,    5, 0x21e1cde6);
    MD5_ROUND2(d, a, b, c, 0x80, 9, 0xc33707d6);
    MD5_ROUND2(c, d, a, b, dd,  14, 0xf4d50d87);
    MD5_ROUND2(b, c, d, a, 0,   20, 0x455a14ed);
    MD5_ROUND2(a, b, c, d, 0,    5, 0xa9e3e905);
    MD5_ROUND2(d, a, b, c, cc,   9, 0xfcefa3f8);
    MD5_ROUND2(c, d, a, b, 0,   14, 0x676f02d9);
    MD5_ROUND2(b, c, d, a, 0,   20, 0x8d2a4c8a);
    
    /** round 3 */
    MD5_ROUND3(a, b, c, d, 0,    4, 0xfffa3942);
    MD5_ROUND3(d, a, b, c, 0,   11, 0x8771f681);
    MD5_ROUND3(c, d, a, b, 0,   16, 0x6d9d6122);
    MD5_ROUND3(b, c, d, a, 0x80,23, 0xfde5380c);
    MD5_ROUND3(a, b, c, d, bb,   4, 0xa4beea44);
    MD5_ROUND3(d, a, b, c, 0x80,11, 0x4bdecfa9);
    MD5_ROUND3(c, d, a, b, 0,   16, 0xf6bb4b60);
    MD5_ROUND3(b, c, d, a, 0,   23, 0xbebfbc70);
    MD5_ROUND3(a, b, c, d, 0,    4, 0x289b7ec6);
    MD5_ROUND3(d, a, b, c, aa,  11, 0xeaa127fa);
    MD5_ROUND3(c, d, a, b, dd,  16, 0xd4ef3085);
    MD5_ROUND3(b, c, d, a, 0,   23, 0x04881d05);
    MD5_ROUND3(a, b, c, d, 0,    4, 0xd9d4d039);
    MD5_ROUND3(d, a, b, c, 0,   11, 0xe6db99e5);
    MD5_ROUND3(c, d, a, b, 0,   16, 0x1fa27cf8);
    MD5_ROUND3(b, c, d, a, cc,  23, 0xc4ac5665);
      
    /** round 4 */
    MD5_ROUND4(a, b, c, d, aa,   6, 0xf4292244);
    MD5_ROUND4(d, a, b, c, 0,   10, 0x432aff97);
    MD5_ROUND4(c, d, a, b, 0x80,15, 0xab9423a7);
    MD5_ROUND4(b, c, d, a, 0,   21, 0xfc93a039);
    MD5_ROUND4(a, b, c, d, 0,    6, 0x655b59c3);
    MD5_ROUND4(d, a, b, c, dd,  10, 0x8f0ccc92);
    MD5_ROUND4(c, d, a, b, 0,   15, 0xffeff47d);
    MD5_ROUND4(b, c, d, a, bb,  21, 0x85845dd1);
    MD5_ROUND4(a, b, c, d, 0,    6, 0x6fa87e4f);
    MD5_ROUND4(d, a, b, c, 0,   10, 0xfe2ce6e0);
    MD5_ROUND4(c, d, a, b, 0,   15, 0xa3014314);
    MD5_ROUND4(b, c, d, a, 0,   21, 0x4e0811a1);
    MD5_ROUND4(a, b, c, d, 0x80, 6, 0xf7537e82);
    MD5_ROUND4(d, a, b, c, 0,   10, 0xbd3af235);
    MD5_ROUND4(c, d, a, b, cc,  15, 0x2ad7d2bb);
    MD5_ROUND4(b, c, d, a, 0,   21, 0xeb86d391);

    a += AA;
    b += BB;
    c += CC;
    d += DD;
  }

  /** break digest into bytes */
  msg[ 0] = (uint8_t)(a & 0xff);
  msg[ 1] = (uint8_t)((a >>= 8) & 0xff);
  msg[ 2] = (uint8_t)((a >>= 8) & 0xff);
  msg[ 3] = (uint8_t)((a >>= 8) & 0xff);
  msg[ 4] = (uint8_t)(b & 0xff);
  msg[ 5] = (uint8_t)((b >>= 8) & 0xff);
  msg[ 6] = (uint8_t)((b >>= 8) & 0xff);
  msg[ 7] = (uint8_t)((b >>= 8) & 0xff);
  msg[ 8] = (uint8_t)(c & 0xff);
  msg[ 9] = (uint8_t)((c >>= 8) & 0xff);
  msg[10] = (uint8_t)((c >>= 8) & 0xff);
  msg[11] = (uint8_t)((c >>= 8) & 0xff);
  msg[12] = (uint8_t)(d & 0xff);
  msg[13] = (uint8_t)((d >>= 8) & 0xff);
  msg[14] = (uint8_t)((d >>= 8) & 0xff);
  msg[15] = (uint8_t)((d >>= 8) & 0xff);
}

void
md5_50_init(const unsigned int msgLen) {
  if(msgLen == 16)
    md5_50_variant = &md5_50f;
  else
    md5_50_variant = &md5_50s;
}

void
md5_50(uint8_t *msg, const unsigned int msgLen) {
  md5_50_variant(msg, msgLen);
}
