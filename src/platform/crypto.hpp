/**
 * @file    crypto.hpp
 * @brief   AES-128-CTR encryption and decryption for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-09-25
 */

#pragma once

#include <Arduino.h>
#include <mbedtls/aes.h>
#include <string.h>

// ============================================================
// Constants
// ============================================================

constexpr size_t AES_KEY_SIZE = 16; // AES-128 key size in bytes
constexpr size_t AES_IV_SIZE  = 16; // AES-128 IV size in bytes

// ============================================================
// Private helpers
// ============================================================

/**
 * @brief Build an initialization vector (IV) using a timestamp and random data.
 * @param iv        Output IV buffer (16 bytes).
 * @param timestamp Timestamp to include in the IV.
 */
static void buildIV(uint8_t iv[AES_IV_SIZE], uint32_t timestamp)
{
    memcpy(iv, &timestamp, sizeof(uint32_t));
    for (int i = sizeof(uint32_t); i < AES_IV_SIZE; i++)
        iv[i] = (uint8_t)esp_random();
}

/**
 * @brief Perform AES-128-CTR encryption or decryption.
 * @param input      Pointer to the input buffer.
 * @param inputSize  Size of the input buffer in bytes.
 * @param key        Pointer to the 16-byte AES key.
 * @param iv         Pointer to the 16-byte initialization vector.
 * @param output     Pointer to the output buffer (must be inputSize bytes).
 * @return true if the operation was successful, false otherwise.
 */
static bool aesCtr(const uint8_t* input, size_t inputSize, const uint8_t key[AES_KEY_SIZE], uint8_t iv[AES_IV_SIZE], uint8_t* output)
{
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);

    if (mbedtls_aes_setkey_enc(&ctx, key, AES_KEY_SIZE * 8) != 0)
    {
        mbedtls_aes_free(&ctx);
        return false;
    }

    size_t  ncOffset = 0;
    uint8_t streamBlock[16] = {0};
    uint8_t ivCopy[AES_IV_SIZE];
    memcpy(ivCopy, iv, AES_IV_SIZE);

    int ret = mbedtls_aes_crypt_ctr(&ctx, inputSize, &ncOffset, ivCopy, streamBlock, input, output);

    mbedtls_aes_free(&ctx);
    return ret == 0;
}

// ============================================================
// Public API
// ============================================================

/**
 * @brief Encrypt a buffer using AES-128-CTR.
 * @param input      Pointer to the input buffer.
 * @param inputSize  Size of the input buffer in bytes.
 * @param key        Pointer to the 16-byte AES key.
 * @param output     Pointer to the output buffer (must be inputSize + AES_IV_SIZE bytes).
 * @param timestamp  Timestamp used as part of the IV.
 * @return Total size of the output (inputSize + AES_IV_SIZE), or 0 on error.
 */
inline size_t encrypt(const uint8_t* input, size_t inputSize, const uint8_t key[AES_KEY_SIZE], uint8_t* output, uint32_t timestamp)
{
    if (input == nullptr || output == nullptr || inputSize == 0) return 0;

    uint8_t iv[AES_IV_SIZE];
    buildIV(iv, timestamp);
    memcpy(output, iv, AES_IV_SIZE);

    if (!aesCtr(input, inputSize, key, iv, output + AES_IV_SIZE))
        return 0;

    return AES_IV_SIZE + inputSize;
}

/**
 * @brief Decrypt a buffer using AES-128-CTR.
 * @param input      Pointer to the input buffer (IV + encrypted data).
 * @param inputSize  Size of the input buffer in bytes.
 * @param key        Pointer to the 16-byte AES key.
 * @param output     Pointer to the output buffer (must be inputSize - AES_IV_SIZE bytes).
 * @return Size of the decrypted data, or 0 on error.
 */
inline size_t decrypt(const uint8_t* input, size_t inputSize, const uint8_t key[AES_KEY_SIZE], uint8_t* output)
{
    if (input == nullptr || output == nullptr || inputSize <= AES_IV_SIZE) return 0;

    uint8_t iv[AES_IV_SIZE];
    memcpy(iv, input, AES_IV_SIZE);

    size_t dataSize = inputSize - AES_IV_SIZE;

    if (!aesCtr(input + AES_IV_SIZE, dataSize, key, iv, output))
        return 0;

    return dataSize;
}

/**
 * @brief Parse a hex string into a 16-byte AES key.
 * @param hexStr  Hex string (exactly 32 characters).
 * @param key     Output byte array (16 bytes).
 * @return true if parsing was successful, false otherwise.
 */
inline bool parseHexKey(const String& hexStr, uint8_t key[AES_KEY_SIZE])
{
    if (hexStr.length() != AES_KEY_SIZE * 2) return false;

    for (int i = 0; i < AES_KEY_SIZE; i++)
    {
        String byteStr = hexStr.substring(i * 2, i * 2 + 2);
        key[i] = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
    }

    return true;
}