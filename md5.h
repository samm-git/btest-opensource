/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security,
 * Inc. MD5 Message-Digest Algorithm.
 *
 * Written by Solar Designer <solar at openwall.com> in 2001, and placed
 * in the public domain.  See md5.c for more information.
 */

#ifndef __MD5_H__
#define __MD5_H__

#define	MD5_DIGEST_SZ	16
#define	MD5_BLOCK_SZ	64

/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned long MD5_u32plus;

typedef struct {
    MD5_u32plus lo, hi;
    MD5_u32plus a, b, c, d;
    unsigned char buffer[MD5_BLOCK_SZ];
    MD5_u32plus block[MD5_DIGEST_SZ];
} MD5_CTX;

extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, void *data, unsigned long size);
extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);

#endif /* __MD5_H__ */
