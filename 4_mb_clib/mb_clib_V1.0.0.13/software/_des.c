/*
 *  FIPS-46-3 compliant Triple-DES implementation
 *
 *  Copyright (C) 2006-2007  Christophe Devine
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */
/*
 *  DES, on which TDES is based, was originally designed by IBM in
 *  1974 and adopted as a standard by NIST (formerly NBS).
 *
 *  http://csrc.nist.gov/publications/fips/fips46-3/fips46-3.pdf
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>
#include "_des.h"

// define SELF_TEST to verify itself.
#define SELF_TEST

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)  {(n) = ( (unsigned long) (b)[(i)    ] << 24 )| ( (unsigned long) (b)[(i) + 1] << 16 )| ( (unsigned long) (b)[(i) + 2] <<  8 ) | ( (unsigned long) (b)[(i) + 3]);  }

#endif
#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)  {  (b)[(i)    ] = (unsigned char) ( (n) >> 24 );(b)[(i) + 1] = (unsigned char) ( (n) >> 16 );(b)[(i) + 2] = (unsigned char) ( (n) >>  8 );(b)[(i) + 3] = (unsigned char) ( (n)       ); }
#endif

/*
 * Expanded DES S-boxes
 */
static const unsigned long SB1[64] =
{
    0x01010400, 0x00000000, 0x00010000, 0x01010404,
    0x01010004, 0x00010404, 0x00000004, 0x00010000,
    0x00000400, 0x01010400, 0x01010404, 0x00000400,
    0x01000404, 0x01010004, 0x01000000, 0x00000004,
    0x00000404, 0x01000400, 0x01000400, 0x00010400,
    0x00010400, 0x01010000, 0x01010000, 0x01000404,
    0x00010004, 0x01000004, 0x01000004, 0x00010004,
    0x00000000, 0x00000404, 0x00010404, 0x01000000,
    0x00010000, 0x01010404, 0x00000004, 0x01010000,
    0x01010400, 0x01000000, 0x01000000, 0x00000400,
    0x01010004, 0x00010000, 0x00010400, 0x01000004,
    0x00000400, 0x00000004, 0x01000404, 0x00010404,
    0x01010404, 0x00010004, 0x01010000, 0x01000404,
    0x01000004, 0x00000404, 0x00010404, 0x01010400,
    0x00000404, 0x01000400, 0x01000400, 0x00000000,
    0x00010004, 0x00010400, 0x00000000, 0x01010004
};

static const unsigned long SB2[64] =
{
    0x80108020, 0x80008000, 0x00008000, 0x00108020,
    0x00100000, 0x00000020, 0x80100020, 0x80008020,
    0x80000020, 0x80108020, 0x80108000, 0x80000000,
    0x80008000, 0x00100000, 0x00000020, 0x80100020,
    0x00108000, 0x00100020, 0x80008020, 0x00000000,
    0x80000000, 0x00008000, 0x00108020, 0x80100000,
    0x00100020, 0x80000020, 0x00000000, 0x00108000,
    0x00008020, 0x80108000, 0x80100000, 0x00008020,
    0x00000000, 0x00108020, 0x80100020, 0x00100000,
    0x80008020, 0x80100000, 0x80108000, 0x00008000,
    0x80100000, 0x80008000, 0x00000020, 0x80108020,
    0x00108020, 0x00000020, 0x00008000, 0x80000000,
    0x00008020, 0x80108000, 0x00100000, 0x80000020,
    0x00100020, 0x80008020, 0x80000020, 0x00100020,
    0x00108000, 0x00000000, 0x80008000, 0x00008020,
    0x80000000, 0x80100020, 0x80108020, 0x00108000
};

static const unsigned long SB3[64] =
{
    0x00000208, 0x08020200, 0x00000000, 0x08020008,
    0x08000200, 0x00000000, 0x00020208, 0x08000200,
    0x00020008, 0x08000008, 0x08000008, 0x00020000,
    0x08020208, 0x00020008, 0x08020000, 0x00000208,
    0x08000000, 0x00000008, 0x08020200, 0x00000200,
    0x00020200, 0x08020000, 0x08020008, 0x00020208,
    0x08000208, 0x00020200, 0x00020000, 0x08000208,
    0x00000008, 0x08020208, 0x00000200, 0x08000000,
    0x08020200, 0x08000000, 0x00020008, 0x00000208,
    0x00020000, 0x08020200, 0x08000200, 0x00000000,
    0x00000200, 0x00020008, 0x08020208, 0x08000200,
    0x08000008, 0x00000200, 0x00000000, 0x08020008,
    0x08000208, 0x00020000, 0x08000000, 0x08020208,
    0x00000008, 0x00020208, 0x00020200, 0x08000008,
    0x08020000, 0x08000208, 0x00000208, 0x08020000,
    0x00020208, 0x00000008, 0x08020008, 0x00020200
};

static const unsigned long SB4[64] =
{
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802080, 0x00800081, 0x00800001, 0x00002001,
    0x00000000, 0x00802000, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00800080, 0x00800001,
    0x00000001, 0x00002000, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002001, 0x00002080,
    0x00800081, 0x00000001, 0x00002080, 0x00800080,
    0x00002000, 0x00802080, 0x00802081, 0x00000081,
    0x00800080, 0x00800001, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00000000, 0x00802000,
    0x00002080, 0x00800080, 0x00800081, 0x00000001,
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802081, 0x00000081, 0x00000001, 0x00002000,
    0x00800001, 0x00002001, 0x00802080, 0x00800081,
    0x00002001, 0x00002080, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002000, 0x00802080
};

static const unsigned long SB5[64] =
{
    0x00000100, 0x02080100, 0x02080000, 0x42000100,
    0x00080000, 0x00000100, 0x40000000, 0x02080000,
    0x40080100, 0x00080000, 0x02000100, 0x40080100,
    0x42000100, 0x42080000, 0x00080100, 0x40000000,
    0x02000000, 0x40080000, 0x40080000, 0x00000000,
    0x40000100, 0x42080100, 0x42080100, 0x02000100,
    0x42080000, 0x40000100, 0x00000000, 0x42000000,
    0x02080100, 0x02000000, 0x42000000, 0x00080100,
    0x00080000, 0x42000100, 0x00000100, 0x02000000,
    0x40000000, 0x02080000, 0x42000100, 0x40080100,
    0x02000100, 0x40000000, 0x42080000, 0x02080100,
    0x40080100, 0x00000100, 0x02000000, 0x42080000,
    0x42080100, 0x00080100, 0x42000000, 0x42080100,
    0x02080000, 0x00000000, 0x40080000, 0x42000000,
    0x00080100, 0x02000100, 0x40000100, 0x00080000,
    0x00000000, 0x40080000, 0x02080100, 0x40000100
};

static const unsigned long SB6[64] =
{
    0x20000010, 0x20400000, 0x00004000, 0x20404010,
    0x20400000, 0x00000010, 0x20404010, 0x00400000,
    0x20004000, 0x00404010, 0x00400000, 0x20000010,
    0x00400010, 0x20004000, 0x20000000, 0x00004010,
    0x00000000, 0x00400010, 0x20004010, 0x00004000,
    0x00404000, 0x20004010, 0x00000010, 0x20400010,
    0x20400010, 0x00000000, 0x00404010, 0x20404000,
    0x00004010, 0x00404000, 0x20404000, 0x20000000,
    0x20004000, 0x00000010, 0x20400010, 0x00404000,
    0x20404010, 0x00400000, 0x00004010, 0x20000010,
    0x00400000, 0x20004000, 0x20000000, 0x00004010,
    0x20000010, 0x20404010, 0x00404000, 0x20400000,
    0x00404010, 0x20404000, 0x00000000, 0x20400010,
    0x00000010, 0x00004000, 0x20400000, 0x00404010,
    0x00004000, 0x00400010, 0x20004010, 0x00000000,
    0x20404000, 0x20000000, 0x00400010, 0x20004010
};

static const unsigned long SB7[64] =
{
    0x00200000, 0x04200002, 0x04000802, 0x00000000,
    0x00000800, 0x04000802, 0x00200802, 0x04200800,
    0x04200802, 0x00200000, 0x00000000, 0x04000002,
    0x00000002, 0x04000000, 0x04200002, 0x00000802,
    0x04000800, 0x00200802, 0x00200002, 0x04000800,
    0x04000002, 0x04200000, 0x04200800, 0x00200002,
    0x04200000, 0x00000800, 0x00000802, 0x04200802,
    0x00200800, 0x00000002, 0x04000000, 0x00200800,
    0x04000000, 0x00200800, 0x00200000, 0x04000802,
    0x04000802, 0x04200002, 0x04200002, 0x00000002,
    0x00200002, 0x04000000, 0x04000800, 0x00200000,
    0x04200800, 0x00000802, 0x00200802, 0x04200800,
    0x00000802, 0x04000002, 0x04200802, 0x04200000,
    0x00200800, 0x00000000, 0x00000002, 0x04200802,
    0x00000000, 0x00200802, 0x04200000, 0x00000800,
    0x04000002, 0x04000800, 0x00000800, 0x00200002
};

static const unsigned long SB8[64] =
{
    0x10001040, 0x00001000, 0x00040000, 0x10041040,
    0x10000000, 0x10001040, 0x00000040, 0x10000000,
    0x00040040, 0x10040000, 0x10041040, 0x00041000,
    0x10041000, 0x00041040, 0x00001000, 0x00000040,
    0x10040000, 0x10000040, 0x10001000, 0x00001040,
    0x00041000, 0x00040040, 0x10040040, 0x10041000,
    0x00001040, 0x00000000, 0x00000000, 0x10040040,
    0x10000040, 0x10001000, 0x00041040, 0x00040000,
    0x00041040, 0x00040000, 0x10041000, 0x00001000,
    0x00000040, 0x10040040, 0x00001000, 0x00041040,
    0x10001000, 0x00000040, 0x10000040, 0x10040000,
    0x10040040, 0x10000000, 0x00040000, 0x10001040,
    0x00000000, 0x10041040, 0x00040040, 0x10000040,
    0x10040000, 0x10001000, 0x10001040, 0x00000000,
    0x10041040, 0x00041000, 0x00041000, 0x00001040,
    0x00001040, 0x00040040, 0x10000000, 0x10041000
};

/*
 * PC1: left and right halves bit-swap
 */
static const unsigned long LHs[16] =
{
    0x00000000, 0x00000001, 0x00000100, 0x00000101,
    0x00010000, 0x00010001, 0x00010100, 0x00010101,
    0x01000000, 0x01000001, 0x01000100, 0x01000101,
    0x01010000, 0x01010001, 0x01010100, 0x01010101
};

static const unsigned long RHs[16] =
{
    0x00000000, 0x01000000, 0x00010000, 0x01010000,
    0x00000100, 0x01000100, 0x00010100, 0x01010100,
    0x00000001, 0x01000001, 0x00010001, 0x01010001,
    0x00000101, 0x01000101, 0x00010101, 0x01010101,
};

/*
 * Initial Permutation macro
 */
#define DES_IP(X,Y)  {T = ((X >>  4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T <<  4);T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16);T = ((Y >>  2) ^ X) & 0x33333333; X ^= T; Y ^= (T <<  2);T = ((Y >>  8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T <<  8);Y = ((Y << 1) | (Y >> 31)) & 0xFFFFFFFF;T = (X ^ Y) & 0xAAAAAAAA; Y ^= T; X ^= T;X = ((X << 1) | (X >> 31)) & 0xFFFFFFFF;}

/*
 * Final Permutation macro
 */
#define DES_FP(X,Y)  {X = ((X << 31) | (X >> 1)) & 0xFFFFFFFF;T = (X ^ Y) & 0xAAAAAAAA; X ^= T; Y ^= T;Y = ((Y << 31) | (Y >> 1)) & 0xFFFFFFFF;T = ((Y >>  8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T <<  8);T = ((Y >>  2) ^ X) & 0x33333333; X ^= T; Y ^= (T <<  2);T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16);T = ((X >>  4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T <<  4); }

/*
 * DES round macro
 */
#define DES_ROUND(X,Y)  {T = *SK++ ^ X;Y ^= SB8[ (T      ) & 0x3F ] ^ SB6[ (T >>  8) & 0x3F ] ^ SB4[ (T >> 16) & 0x3F ] ^ SB2[ (T >> 24) & 0x3F ];T = *SK++ ^ ((X << 28) | (X >> 4)); Y ^= SB7[ (T      ) & 0x3F ] ^ SB5[ (T >>  8) & 0x3F ] ^ SB3[ (T >> 16) & 0x3F ] ^ SB1[ (T >> 24) & 0x3F ];}

static void des_main_ks( unsigned long SK[32], unsigned char key[8] )
{
    int i;
    unsigned long X, Y, T;

    GET_UINT32_BE( X, key, 0 );
    GET_UINT32_BE( Y, key, 4 );

    /*
     * Permuted Choice 1
     */
    T =  ((Y >>  4) ^ X) & 0x0F0F0F0F;  X ^= T; Y ^= (T <<  4);
    T =  ((Y      ) ^ X) & 0x10101010;  X ^= T; Y ^= (T      );

    X =   (LHs[ (X      ) & 0xF] << 3) | (LHs[ (X >>  8) & 0xF ] << 2)
        | (LHs[ (X >> 16) & 0xF] << 1) | (LHs[ (X >> 24) & 0xF ]     )
        | (LHs[ (X >>  5) & 0xF] << 7) | (LHs[ (X >> 13) & 0xF ] << 6)
        | (LHs[ (X >> 21) & 0xF] << 5) | (LHs[ (X >> 29) & 0xF ] << 4);

    Y =   (RHs[ (Y >>  1) & 0xF] << 3) | (RHs[ (Y >>  9) & 0xF ] << 2)
        | (RHs[ (Y >> 17) & 0xF] << 1) | (RHs[ (Y >> 25) & 0xF ]     )
        | (RHs[ (Y >>  4) & 0xF] << 7) | (RHs[ (Y >> 12) & 0xF ] << 6)
        | (RHs[ (Y >> 20) & 0xF] << 5) | (RHs[ (Y >> 28) & 0xF ] << 4);

    X &= 0x0FFFFFFF;
    Y &= 0x0FFFFFFF;

    /*
     * calculate subkeys
     */
    for( i = 0; i < 16; i++ )
    {
        if( i < 2 || i == 8 || i == 15 )
        {
            X = ((X <<  1) | (X >> 27)) & 0x0FFFFFFF;
            Y = ((Y <<  1) | (Y >> 27)) & 0x0FFFFFFF;
        }
        else
        {
            X = ((X <<  2) | (X >> 26)) & 0x0FFFFFFF;
            Y = ((Y <<  2) | (Y >> 26)) & 0x0FFFFFFF;
        }

        *SK++ =   ((X <<  4) & 0x24000000) | ((X << 28) & 0x10000000)
                | ((X << 14) & 0x08000000) | ((X << 18) & 0x02080000)
                | ((X <<  6) & 0x01000000) | ((X <<  9) & 0x00200000)
                | ((X >>  1) & 0x00100000) | ((X << 10) & 0x00040000)
                | ((X <<  2) & 0x00020000) | ((X >> 10) & 0x00010000)
                | ((Y >> 13) & 0x00002000) | ((Y >>  4) & 0x00001000)
                | ((Y <<  6) & 0x00000800) | ((Y >>  1) & 0x00000400)
                | ((Y >> 14) & 0x00000200) | ((Y      ) & 0x00000100)
                | ((Y >>  5) & 0x00000020) | ((Y >> 10) & 0x00000010)
                | ((Y >>  3) & 0x00000008) | ((Y >> 18) & 0x00000004)
                | ((Y >> 26) & 0x00000002) | ((Y >> 24) & 0x00000001);

        *SK++ =   ((X << 15) & 0x20000000) | ((X << 17) & 0x10000000)
                | ((X << 10) & 0x08000000) | ((X << 22) & 0x04000000)
                | ((X >>  2) & 0x02000000) | ((X <<  1) & 0x01000000)
                | ((X << 16) & 0x00200000) | ((X << 11) & 0x00100000)
                | ((X <<  3) & 0x00080000) | ((X >>  6) & 0x00040000)
                | ((X << 15) & 0x00020000) | ((X >>  4) & 0x00010000)
                | ((Y >>  2) & 0x00002000) | ((Y <<  8) & 0x00001000)
                | ((Y >> 14) & 0x00000808) | ((Y >>  9) & 0x00000400)
                | ((Y      ) & 0x00000200) | ((Y <<  7) & 0x00000100)
                | ((Y >>  7) & 0x00000020) | ((Y >>  3) & 0x00000011)
                | ((Y <<  2) & 0x00000004) | ((Y >> 21) & 0x00000002);
    }
}

/*
 * DES key schedule (56-bit)
 */
void des_set_key( des_context *ctx, unsigned char key[8] )
{
    int i;

    des_main_ks( ctx->esk, key );

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i    ] = ctx->esk[30 - i];
        ctx->dsk[i + 1] = ctx->esk[31 - i];
    }
}

static void des_crypt( unsigned long SK[32],
                       unsigned char input[8],
                       unsigned char output[8] )
{
    unsigned long X, Y, T;

    GET_UINT32_BE( X, input, 0 );
    GET_UINT32_BE( Y, input, 4 );

    DES_IP( X, Y );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_FP( Y, X );

    PUT_UINT32_BE( Y, output, 0 );
    PUT_UINT32_BE( X, output, 4 );
}

/*
 * DES block encryption (ECB mode)
 */
void des_encrypt( des_context *ctx,
                  unsigned char input[8],
                  unsigned char output[8] )
{
    des_crypt( ctx->esk, input, output );
}

/*
 * DES block decryption (ECB mode)
 */
void des_decrypt( des_context *ctx,
                  unsigned char input[8],
                  unsigned char output[8] )
{
    des_crypt( ctx->dsk, input, output );
}

/*
 * DES-CBC buffer encryption
 */
void des_cbc_encrypt( des_context *ctx,
                      unsigned char iv[8],
                      unsigned char *input,
                      unsigned char *output,
                      int len )
{
    int i;

    while( len > 0 )
    {
        for( i = 0; i < 8; i++ )
            output[i] = input[i] ^ iv[i];

        des_crypt( ctx->esk, output, output );
        memcpy( iv, output, 8 );

        input  += 8;
        output += 8;
        len    -= 8;
    }
}

/*
 * DES-CBC buffer decryption
 */
void des_cbc_decrypt( des_context *ctx,
                      unsigned char iv[8],
                      unsigned char *input,
                      unsigned char *output,
                      int len )
{
    int i;
    unsigned char temp[8];

    while( len > 0 )
    {
        memcpy( temp, input, 8 );
        des_crypt( ctx->dsk, input, output );

        for( i = 0; i < 8; i++ )
            output[i] = output[i] ^ iv[i];

        memcpy( iv, temp, 8 );

        input  += 8;
        output += 8;
        len    -= 8;
    }
}

/*
 * Triple-DES key schedule (112-bit)
 */
void des3_set_2keys( des3_context *ctx, unsigned char key[16] )
{
    int i;

    des_main_ks( ctx->esk     , key     );
    des_main_ks( ctx->dsk + 32, key + 8 );

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i     ] = ctx->esk[30 - i];
        ctx->dsk[i +  1] = ctx->esk[31 - i];

        ctx->esk[i + 32] = ctx->dsk[62 - i];
        ctx->esk[i + 33] = ctx->dsk[63 - i];

        ctx->esk[i + 64] = ctx->esk[     i];
        ctx->esk[i + 65] = ctx->esk[ 1 + i];

        ctx->dsk[i + 64] = ctx->dsk[     i];
        ctx->dsk[i + 65] = ctx->dsk[ 1 + i];
    }
}

/*
 * Triple-DES key schedule (168-bit)
 */
void des3_set_3keys( des3_context *ctx, unsigned char key[24] )
{
    int i;

    des_main_ks( ctx->esk     , key      );
    des_main_ks( ctx->dsk + 32, key +  8 );
    des_main_ks( ctx->esk + 64, key + 16 );

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i     ] = ctx->esk[94 - i];
        ctx->dsk[i +  1] = ctx->esk[95 - i];

        ctx->esk[i + 32] = ctx->dsk[62 - i];
        ctx->esk[i + 33] = ctx->dsk[63 - i];

        ctx->dsk[i + 64] = ctx->esk[30 - i];
        ctx->dsk[i + 65] = ctx->esk[31 - i];
    }
}

static void des3_crypt( unsigned long SK[96],
                        unsigned char input[8],
                        unsigned char output[8] )
{
    unsigned long X, Y, T;

    GET_UINT32_BE( X, input, 0 );
    GET_UINT32_BE( Y, input, 4 );

    DES_IP( X, Y );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_FP( Y, X );

    PUT_UINT32_BE( Y, output, 0 );
    PUT_UINT32_BE( X, output, 4 );
}

/*
 * Triple-DES block encryption (ECB mode)
 */
void des3_encrypt( des3_context *ctx,
                   unsigned char input[8],
                   unsigned char output[8] )
{
    des3_crypt( ctx->esk, input, output );
}

/*
 * Triple-DES block decryption (ECB mode)
 */
void des3_decrypt( des3_context *ctx,
                   unsigned char input[8],
                   unsigned char output[8] )
{
    des3_crypt( ctx->dsk, input, output );
}

/*
 * 3DES-CBC buffer encryption
 */
void des3_cbc_encrypt( des3_context *ctx,unsigned char iv[8],unsigned char *input,
                       unsigned char *output,
                       int len )
{
    int i;

    while( len > 0 )
    {
        for( i = 0; i < 8; i++ )
            output[i] = input[i] ^ iv[i];

        des3_crypt( ctx->esk, output, output );
        memcpy( iv, output, 8 );

        input  += 8;
        output += 8;
        len    -= 8;
    }
}

/*
 * 3DES-CBC buffer decryption
 */
void des3_cbc_decrypt( des3_context *ctx,
                       unsigned char iv[8],
                       unsigned char *input,
                       unsigned char *output,
                       int len )
{
    int i;
    unsigned char temp[8];

    while( len > 0 )
    {
        memcpy( temp, input, 8 );
        des3_crypt( ctx->dsk, input, output );

        for( i = 0; i < 8; i++ )
            output[i] = output[i] ^ iv[i];

        memcpy( iv, temp, 8 );

        input  += 8;
        output += 8;
        len    -= 8;
    }
}


void des_in(unsigned char *buf,int buf_len,unsigned char *key)//des加密
{		des_context ctx;
    unsigned char iv[8];	
    memcpy(iv, key, sizeof(iv));
    des_set_key(&ctx, (unsigned char *)key);
    des_cbc_encrypt(&ctx, iv, buf, buf, buf_len);
}

void des_out(unsigned char *buf,int buf_len,unsigned char  *key)//des解密
{		des_context ctx;
    unsigned char iv[8];	
    memcpy(iv, key, sizeof(iv));
    des_set_key(&ctx, (unsigned char *)key);
    des_cbc_decrypt(&ctx, iv,buf,buf, buf_len);
}

int des3_in(unsigned char  *buf,int buf_len,unsigned char *key)//3des加密
{	des3_context	data;
	unsigned char len;
	int i;
	unsigned char out_buf[8];
	len=(buf_len&0x07);
	len=8-len;
	memset(buf+buf_len,len,len);
	buf_len+=len;
	memset(&data,0,sizeof(data));
	des3_set_2keys(&data,key);
	for(i=0;i<buf_len;i+=8)		
	{	des3_encrypt(&data,buf+i,out_buf);
		memcpy(buf+i,out_buf,8);
	}
	return buf_len;	
}

int des3_out(unsigned char *buf,int buf_len,unsigned char *key)//3des解密
{	des3_context	data;
	int i;
	unsigned char out_buf[8];
	memset(&data,0,sizeof(data));
	des3_set_2keys(&data,key);
	for(i=0;i<buf_len;i+=8)		
	{	des3_decrypt(&data,buf+i,out_buf);
		memcpy(buf+i,out_buf,8);
	}
	buf_len=buf_len-out_buf[7];
	return buf_len;
}

















//*================================================================================*//
//*================================================================================*//
//*================================================================================*//
//*================================================================================*//
//*================================================================================*//
unsigned char key_round[32] = { 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1,
		0, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 }; //shift key left //shift key right
unsigned char IPP[64] = { 57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19,
		11, 3, 61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7, 56,
		48, 40, 32, 24, 16, 8, 0, 58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36,
		28, 20, 12, 4, 62, 54, 46, 38, 30, 22, 14, 6 };
unsigned char Choose56[56] = { 56, 48, 40, 32, 24, 16, 8, 0, 57, 49, 41, 33, 25,
		17, 9, 1, 58, 50, 42, 34, 26, 18, 10, 2, 59, 51, 43, 35, 62, 54, 46, 38,
		30, 22, 14, 6, 61, 53, 45, 37, 29, 21, 13, 5, 60, 52, 44, 36, 28, 20,
		12, 4, 27, 19, 11, 3 }; //Choosing the 56bit key from 64bit
unsigned char E[48] = { 13, 16, 10, 23, 0, 4, 2, 27, 14, 5, 20, 9, 22, 18, 11,
		3, 25, 7, 15, 6, 26, 19, 12, 1, 40, 51, 30, 36, 46, 54, 29, 39, 50, 44,
		32, 47, 43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31 }; //Compression and permutation for key
unsigned char Choose48[48] = { 31, 0, 1, 2, 3, 4, 3, 4, 5, 6, 7, 8, 7, 8, 9, 10,
		11, 12, 11, 12, 13, 14, 15, 16, 15, 16, 17, 18, 19, 20, 19, 20, 21, 22,
		23, 24, 23, 24, 25, 26, 27, 28, 27, 28, 29, 30, 31, 0 }; //expands the right half text of 32 bits to 48 bits
unsigned char PP[32] = { 15, 6, 19, 20, 28, 11, 27, 16, 0, 14, 22, 25, 4, 17,
		30, 9, 1, 7, 23, 13, 31, 26, 2, 8, 18, 12, 29, 5, 21, 10, 3, 24 }; //P-box permutation
unsigned char IPN[64] = { 39, 7, 47, 15, 55, 23, 63, 31, 38, 6, 46, 14, 54, 22,
		62, 30, 37, 5, 45, 13, 53, 21, 61, 29, 36, 4, 44, 12, 52, 20, 60, 28,
		35, 3, 43, 11, 51, 19, 59, 27, 34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,
		9, 49, 17, 57, 25, 32, 0, 40, 8, 48, 16, 56, 24 }; //Inverse permutation
unsigned char S[8][64] = { { 14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0,
		7, 0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8, 4, 1, 14, 8,
		13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0, 15, 12, 8, 2, 4, 9, 1, 7, 5,
		11, 3, 14, 10, 0, 6, 13 }, { 15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12,
		0, 5, 10, 3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5, 0, 14,
		7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15, 13, 8, 10, 1, 3, 15, 4,
		2, 11, 6, 7, 12, 0, 5, 14, 9 }, { 10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12,
		7, 11, 4, 2, 8, 13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
		13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7, 1, 10, 13, 0, 6,
		9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12 }, { 7, 13, 14, 3, 0, 6, 9, 10, 1,
		2, 8, 5, 11, 12, 4, 15, 13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10,
		14, 9, 10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4, 3, 15, 0,
		6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14 }, { 2, 12, 4, 1, 7, 10, 11,
		6, 8, 5, 3, 15, 13, 0, 14, 9, 14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10,
		3, 9, 8, 6, 4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14, 11, 8,
		12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3 }, { 12, 1, 10, 15, 9, 2,
		6, 8, 0, 13, 3, 4, 14, 7, 5, 11, 10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13,
		14, 0, 11, 3, 8, 9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
		4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13 }, { 4, 11, 2, 14,
		15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1, 13, 0, 11, 7, 4, 9, 1, 10, 14,
		3, 5, 12, 2, 15, 8, 6, 1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5,
		9, 2, 6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12 }, { 13, 2,
		8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7, 1, 15, 13, 8, 10, 3, 7,
		4, 12, 5, 6, 11, 0, 14, 9, 2, 7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13,
		15, 3, 5, 8, 2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11 } };


unsigned char bitposition[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };


void _checktable(unsigned char line, unsigned char *text,
		unsigned char *lasttext, unsigned char *IDD) {
	unsigned char i, j, k, temp, temp2;

	for (j = 0, k = 0; j < line; j++, k += 8) {
		lasttext[j] = 0;
		for (i = 0; i < 8; i++) {
			temp2 = IDD[k + i];
			temp = text[temp2 / 8]; // get the byte
			temp &= bitposition[temp2 & 0x7]; // get the bit
			if (temp)
				lasttext[j] |= bitposition[i];
		}
	}
}

void SingleDesOperation(unsigned char *plaintext, unsigned char *key,
		unsigned char mode) 
{
	static unsigned char prevtext[8];
	unsigned char prevkey[8], Ltext[4], Rtext[4];
	unsigned char i, j, temp, temp1, Round;

	if (mode & 0x10)
	{
		_checktable(8, plaintext, prevtext, IPP); //Initial permutation
	}
	for (i = 0; i < 4; i++) {
		Ltext[i] = prevtext[i];
		Rtext[i] = prevtext[i + 4];
	}
	_checktable(7, key, prevkey, Choose56);

	for (Round = 0; Round < 16; Round++) 
	{
		//rotate both 28bits block to left(encrypt) or right(decrypt)
		if (mode & 0x20) // encrypt
		{
			for (j = 0; j < key_round[Round]; j++) 
			{
				temp = prevkey[3] & 0x08;
				for (i = 7; i > 0; i--)
				{
					temp1 = prevkey[i - 1] & 0x80;
					prevkey[i - 1] <<= 1;
					if (temp)
						prevkey[i - 1] |= 0x01;
					temp = temp1;
				}
				if (temp)
					prevkey[3] |= 0x10;
				else
					prevkey[3] &= 0xEF;
			}
		} 
		else // decrypt
		{
			for (j = 0; j < key_round[Round + 16]; j++) 
			{
				temp = prevkey[3] & 0x10;
				for (i = 0; i < 7; i++) {
					temp1 = prevkey[i] & 0x01;
					prevkey[i] >>= 1;
					if (temp)
						prevkey[i] |= 0x80;
					temp = temp1;
				}
				if (temp)
					prevkey[3] |= 0x08;
				else
					prevkey[3] &= 0xF7;
			}
		}

		_checktable(6, prevkey, plaintext, E); //Compression permutation
		_checktable(6, Rtext, prevtext, Choose48); //Expansion permutation

		//Expanded right half XOR with the subkey
		prevtext[0] ^= plaintext[0];
		prevtext[1] ^= plaintext[1];
		prevtext[2] ^= plaintext[2];
		prevtext[3] ^= plaintext[3];
		prevtext[4] ^= plaintext[4];
		prevtext[5] ^= plaintext[5];

		for (j = 6, i = 8; j > 0; j -= 3, i -= 4) //S-Box Substitution
		{
			plaintext[i - 1] = prevtext[j - 1];
			plaintext[i - 2] = ((prevtext[j - 1] >> 6) & 0x03)
					| (prevtext[j - 2] << 2);
			plaintext[i - 3] = ((prevtext[j - 2] >> 4) & 0x0f)
					| (prevtext[j - 3] << 4);
			plaintext[i - 4] = prevtext[j - 3] >> 2;
		}

		for (i = 0; i < 8; i++) //Get the S-Box location
		{
			temp = plaintext[i] & 0x21;
			if (temp & 0x01)
				temp = (temp & 0x20) | 0x10;
			plaintext[i] = temp | ((plaintext[i] >> 1) & 0x0F);
		}

		//Get S-Box output
		plaintext[0] = S[0][plaintext[0]];
		plaintext[1] = S[1][plaintext[1]];
		plaintext[2] = S[2][plaintext[2]];
		plaintext[3] = S[3][plaintext[3]];
		plaintext[4] = S[4][plaintext[4]];
		plaintext[5] = S[5][plaintext[5]];
		plaintext[6] = S[6][plaintext[6]];
		plaintext[7] = S[7][plaintext[7]];

		//Combine 4-bit block to form 32-bit block
		plaintext[0] = (plaintext[0] << 4) | plaintext[1];
		plaintext[1] = (plaintext[2] << 4) | plaintext[3];
		plaintext[2] = (plaintext[4] << 4) | plaintext[5];
		plaintext[3] = (plaintext[6] << 4) | plaintext[7];

		_checktable(4, plaintext, prevtext, PP);

		for (i = 0; i < 4; i++) 
		{
			prevtext[i] ^= Ltext[i];
			Ltext[i] = Rtext[i];
			Rtext[i] = prevtext[i];
		}
	}

	for (i = 0; i < 4; i++) {
		prevtext[i] = Rtext[i];
		prevtext[i + 4] = Ltext[i];
	}
	if (mode & 0x40) 
	{
		_checktable(8, prevtext, plaintext, IPN); //Final permutation
	}
}

void Exor8Bytes(unsigned char *Output, unsigned char *Input1,
		unsigned char *Input2) 
{
	unsigned char i;
	for (i = 0; i < 8; i++)
		Output[i] = Input1[i] ^ Input2[i];
}


void CombinedDesOperation(unsigned char *pvInOutputBlock,
		unsigned char *pvCbcBlock, unsigned char *pbKeyString,
		unsigned long eDesMode, unsigned long cnt) {
	unsigned char i, n = 0, bDesMode, bKeyNumber;
	unsigned char abDataBlock[8];

	if (pvCbcBlock != 0 && (eDesMode & 0x10000000L) != 0)
		memset(pvCbcBlock, 0, 8);

	if (!cnt)
		cnt = 1;

	while (cnt--) 
	{
		if (pvCbcBlock != 0 && (eDesMode & 0x20000000L) != 0)
			Exor8Bytes(abDataBlock, &pvInOutputBlock[8 * n], pvCbcBlock);
		else
			memcpy(abDataBlock, &pvInOutputBlock[8 * n], 8);

		for (i = 0; i < 3; i++) {
			bDesMode = (unsigned char) (eDesMode >> 8 * i);

			bKeyNumber = (unsigned char) (bDesMode & 3);

			if (bKeyNumber == 0)
				continue;

			SingleDesOperation(abDataBlock, &pbKeyString[8 * (bKeyNumber - 1)],
					bDesMode);
		}

		if (pvCbcBlock != NULL && (eDesMode & 0x20000000L) == 0) {
			Exor8Bytes(abDataBlock, abDataBlock, pvCbcBlock);
		}

		if (pvCbcBlock != 0) {
			if (eDesMode & 0x20000000L)
				memcpy(pvCbcBlock, abDataBlock, 8);
			else
				memcpy(pvCbcBlock, &pvInOutputBlock[8 * n], 8);
		}

		/*	Copy the total result from the working space to the given output storage.  */
		memcpy(&pvInOutputBlock[8 * n], abDataBlock, 8);
		n++;
	}
}


void CalcDesMac32(unsigned long eDesMode, unsigned char *pvSubKeys,
		unsigned char *pvCbcBlock, unsigned char *pbInputData,
		unsigned long eNBytes, unsigned char *pbMac32Out) 
{
	unsigned char abInOutputBuffer[8], abCbcBlock[8] = { 0 };
	unsigned char bNBytesInInputBuffer = 0;
	unsigned long i;

	if (!pvCbcBlock)
		pvCbcBlock = abCbcBlock;

	for (i = 0; i < eNBytes; i++) {
		abInOutputBuffer[bNBytesInInputBuffer] = pbInputData[i];

		bNBytesInInputBuffer++;

		if (bNBytesInInputBuffer == 8) {
			CombinedDesOperation(abInOutputBuffer, pvCbcBlock, pvSubKeys,
					eDesMode, 1);

			eDesMode &= ~0x10000000L;

			bNBytesInInputBuffer = 0;
		}
	}

	if (bNBytesInInputBuffer != 0) {
		//	do padding with 0
		for (; bNBytesInInputBuffer < 8; bNBytesInInputBuffer++)
			abInOutputBuffer[bNBytesInInputBuffer] = 0;

		//	do the final Mac round
		CombinedDesOperation(abInOutputBuffer, pvCbcBlock, pvSubKeys, eDesMode,
				1);
	}

//	The four left aligned bytes of abOutputBuffer are taken for the Mac
	memcpy(pbMac32Out, abInOutputBuffer, 4);

	return;
}


char CheckDesMac32(unsigned long eDesMode, unsigned char *pvSubKeys,
		unsigned char *pvCbcBlock, unsigned char *pbInputData,
		unsigned long eNBytes) 
{
	unsigned char abMac32[4];
	unsigned long i;

//	we need at least as many bytes as the length of the MAC
	if (eNBytes < 4)
		return 0;
	//printf("test1\n");
//	subtract the length of the MAC (4 bytes) to get the number of bytes included
//	in the MAC computation

//	compute the MAC over the given data
	CalcDesMac32(eDesMode, pvSubKeys, pvCbcBlock, pbInputData, eNBytes - 4,abMac32);
	//printf("test2\n");
//	check it against the MAC from the InputData
//	return the logical result of the comparision
	for (i = 0; i < 4; i++) 
	{
		if (abMac32[i] != pbInputData[(eNBytes - 4) + i])
			return 0;
	}
//printf("test3\n");
	return 1;
}



void CalCrc16(unsigned char* input, unsigned char* Crc16, unsigned long len) 
{
	unsigned int temp = 0x6363;
	unsigned long i;
	unsigned char j;

	for (i = 0; i < len; i++) {
		temp = temp ^ ((unsigned int) input[i]);
		for (j = 0; j < 8; j++) {
			if (temp & 0x0001) {
				temp = (temp >> 1) ^ 0x8408;
			} else {
				temp = (temp >> 1);
			}
		}
	}
	Crc16[0] = (unsigned char) (temp);
	Crc16[1] = (unsigned char) (temp >> 8);
}


char CheckCrc16(unsigned char* input, unsigned long len) {
	unsigned char* CrcChk=NULL;

	CalCrc16(input, CrcChk, len);

	if (CrcChk[0] == input[len] && CrcChk[1] == input[len + 1])
		return 1; // crc16 ok!
	else
		return 0; // crc16 fail!

}






