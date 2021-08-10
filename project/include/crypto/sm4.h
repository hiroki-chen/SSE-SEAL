/*
 Copyright (c) 2021 Haobin Chen

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SM4_H_
#define SM4_H_

#include <string>

#define SM4_ENCRYPT 1
#define SM4_DECRYPT 0

#define AES_BLOCK_BYTES 16

std::string
decrypt_SM4_EBC(const std::string& ctext, const std::string& raw_key);

std::string
encrypt_SM4_EBC(const std::string& ptext, const std::string& raw_key);

typedef struct
{
    int mode; /*!<  encrypt/decrypt   */
    unsigned long sk[32]; /*!<  SM4 subkeys       */
} sm4_context;

#ifdef __cplusplus
extern "C" {
#endif

/**   
     * \brief          SM4 key schedule (128-bit, encryption)   
     *   
     * \param ctx      SM4 context to be initialized   
     * \param key      16-byte secret key   
     */
void sm4_setkey_enc(sm4_context* ctx, unsigned char key[16]);

/**   
     * \brief          SM4 key schedule (128-bit, decryption)   
     *   
     * \param ctx      SM4 context to be initialized   
     * \param key      16-byte secret key   
     */
void sm4_setkey_dec(sm4_context* ctx, unsigned char key[16]);

/**   
     * \brief          SM4-ECB block encryption/decryption   
     * \param ctx      SM4 context   
     * \param mode     SM4_ENCRYPT or SM4_DECRYPT   
     * \param length   length of the input data   
     * \param input    input block   
     * \param output   output block   
     */
void sm4_crypt_ecb(sm4_context* ctx,
    int mode,
    int length,
    unsigned char* input,
    unsigned char* output);

/**
     * Not yet implemented.
     */
void sm4_crypt_cbc(sm4_context* ctx,
    int mode,
    int length,
    unsigned char IV[16],
    unsigned char* input,
    unsigned char* output);
#ifdef __cplusplus
}
#endif

template <typename SIZE_T>
static SIZE_T
getBlocks(unsigned int unit, SIZE_T len)
{
    SIZE_T blocks = len / unit;
    if (len > blocks * unit) {
        blocks++;
    }
    return blocks;
}

#endif
