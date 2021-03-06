/* net_crypto.c
 *
 * Functions for the core crypto.
 *
 * NOTE: This code has to be perfect. We don't mess around with encryption.
 *
 *  Copyright (C) 2013 Tox project All Rights Reserved.
 *
 *  This file is part of Tox.
 *
 *  Tox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Tox is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crypto_core.h"
#include "util.h"

// Need dht because of ENC_SECRET_KEY and ENC_PUBLIC_KEY
#include "DHT.h"

#if crypto_box_PUBLICKEYBYTES != 32
#error crypto_box_PUBLICKEYBYTES is required to be 32 bytes for public_key_cmp to work,
#endif

/* Extended keypair: curve + ed. Encryption keys are derived from the signature keys.
 * Used for group chats and group DHT announcements.
 * pk and sk must have room for at least EXT_PUBLIC_KEY bytes each.
 */
void create_extended_keypair(uint8_t *pk, uint8_t *sk)
{
    /* create signature key pair */
    crypto_sign_keypair(pk + ENC_PUBLIC_KEY, sk + ENC_SECRET_KEY);

    /* convert public signature key to public encryption key */
    crypto_sign_ed25519_pk_to_curve25519(pk, pk + ENC_PUBLIC_KEY);

    /* convert secret signature key to secret encryption key */
    crypto_sign_ed25519_sk_to_curve25519(sk, sk + ENC_SECRET_KEY);
}

/* compare 2 public keys of length crypto_box_PUBLICKEYBYTES, not vulnerable to timing attacks.
   returns 0 if both mem locations of length are equal,
   return -1 if they are not. */
int public_key_cmp(const uint8_t *pk1, const uint8_t *pk2)
{
    return crypto_verify_32(pk1, pk2);
}

/*  return a random uint32_t.
 */
uint32_t random_int(void)
{
    uint32_t randnum;
    randombytes((uint8_t *)&randnum , sizeof(randnum));
    return randnum;
}

uint64_t random_64b(void)
{
    uint64_t randnum;
    randombytes((uint8_t *)&randnum, sizeof(randnum));
    return randnum;
}

/* Return a value between 0 and upper_bound using a uniform distribution */
uint32_t random_int_range(uint32_t upper_bound)
{
    return randombytes_uniform(upper_bound);
}

/* Check if a Tox public key crypto_box_PUBLICKEYBYTES is valid or not.
 * This should only be used for input validation.
 *
 * return 0 if it isn't.
 * return 1 if it is.
 */
int public_key_valid(const uint8_t *public_key)
{
    if (public_key[31] >= 128) /* Last bit of key is always zero. */
        return 0;

    return 1;
}

/* Precomputes the shared key from their public_key and our secret_key.
 * This way we can avoid an expensive elliptic curve scalar multiply for each
 * encrypt/decrypt operation.
 * enc_key has to be crypto_box_BEFORENMBYTES bytes long.
 */
void encrypt_precompute(const uint8_t *public_key, const uint8_t *secret_key, uint8_t *enc_key)
{
    crypto_box_beforenm(enc_key, public_key, secret_key);
}

int encrypt_data_symmetric(const uint8_t *secret_key, const uint8_t *nonce, const uint8_t *plain, uint32_t length,
                           uint8_t *encrypted)
{
    if (length == 0)
        return -1;

    uint8_t temp_plain[length + crypto_box_ZEROBYTES];
    uint8_t temp_encrypted[length + crypto_box_MACBYTES + crypto_box_BOXZEROBYTES];

    memset(temp_plain, 0, crypto_box_ZEROBYTES);
    memcpy(temp_plain + crypto_box_ZEROBYTES, plain, length); // Pad the message with 32 0 bytes.

    if (crypto_box_afternm(temp_encrypted, temp_plain, length + crypto_box_ZEROBYTES, nonce, secret_key) != 0)
        return -1;

    /* Unpad the encrypted message. */
    memcpy(encrypted, temp_encrypted + crypto_box_BOXZEROBYTES, length + crypto_box_MACBYTES);
    return length + crypto_box_MACBYTES;
}

int decrypt_data_symmetric(const uint8_t *secret_key, const uint8_t *nonce, const uint8_t *encrypted, uint32_t length,
                           uint8_t *plain)
{
    if (length <= crypto_box_BOXZEROBYTES)
        return -1;

    uint8_t temp_plain[length + crypto_box_ZEROBYTES];
    uint8_t temp_encrypted[length + crypto_box_BOXZEROBYTES];

    memset(temp_encrypted, 0, crypto_box_BOXZEROBYTES);
    memcpy(temp_encrypted + crypto_box_BOXZEROBYTES, encrypted, length); // Pad the message with 16 0 bytes.

    if (crypto_box_open_afternm(temp_plain, temp_encrypted, length + crypto_box_BOXZEROBYTES, nonce, secret_key) != 0)
        return -1;

    memcpy(plain, temp_plain + crypto_box_ZEROBYTES, length - crypto_box_MACBYTES);
    return length - crypto_box_MACBYTES;
}

int encrypt_data(const uint8_t *public_key, const uint8_t *secret_key, const uint8_t *nonce,
                 const uint8_t *plain, uint32_t length, uint8_t *encrypted)
{
    uint8_t k[crypto_box_BEFORENMBYTES];
    encrypt_precompute(public_key, secret_key, k);
    return encrypt_data_symmetric(k, nonce, plain, length, encrypted);
}

int decrypt_data(const uint8_t *public_key, const uint8_t *secret_key, const uint8_t *nonce,
                 const uint8_t *encrypted, uint32_t length, uint8_t *plain)
{
    uint8_t k[crypto_box_BEFORENMBYTES];
    encrypt_precompute(public_key, secret_key, k);
    return decrypt_data_symmetric(k, nonce, encrypted, length, plain);
}


/* Increment the given nonce by 1. */
void increment_nonce(uint8_t *nonce)
{
    uint32_t i;

    for (i = crypto_box_NONCEBYTES; i != 0; --i) {
        ++nonce[i - 1];

        if (nonce[i - 1] != 0)
            break;
    }
}
/* increment the given nonce by num */
void increment_nonce_number(uint8_t *nonce, uint32_t num)
{
    uint32_t num1, num2;
    memcpy(&num1, nonce + (crypto_box_NONCEBYTES - sizeof(num1)), sizeof(num1));
    num1 = ntohl(num1);
    num2 = num + num1;

    if (num2 < num1) {
        uint32_t i;

        for (i = crypto_box_NONCEBYTES - sizeof(num1); i != 0; --i) {
            ++nonce[i - 1];

            if (nonce[i - 1] != 0)
                break;
        }
    }

    num2 = htonl(num2);
    memcpy(nonce + (crypto_box_NONCEBYTES - sizeof(num2)), &num2, sizeof(num2));
}

/* Fill the given nonce with random bytes. */
void random_nonce(uint8_t *nonce)
{
    randombytes(nonce, crypto_box_NONCEBYTES);
}

/* Fill a key crypto_box_KEYBYTES big with random bytes */
void new_symmetric_key(uint8_t *key)
{
    randombytes(key, crypto_box_KEYBYTES);
}

/* Gives a nonce guaranteed to be different from previous ones.*/
void new_nonce(uint8_t *nonce)
{
    random_nonce(nonce);
}

/* Create a request to peer.
 * send_public_key and send_secret_key are the pub/secret keys of the sender.
 * recv_public_key is public key of receiver.
 * packet must be an array of MAX_CRYPTO_REQUEST_SIZE big.
 * Data represents the data we send with the request with length being the length of the data.
 * request_id is the id of the request (32 = friend request, 254 = ping request).
 *
 *  return -1 on failure.
 *  return the length of the created packet on success.
 */
int create_request(const uint8_t *send_public_key, const uint8_t *send_secret_key, uint8_t *packet,
                   const uint8_t *recv_public_key, const uint8_t *data, uint32_t length, uint8_t request_id)
{
    if (MAX_CRYPTO_REQUEST_SIZE < length + 1 + crypto_box_PUBLICKEYBYTES * 2 + crypto_box_NONCEBYTES + 1 +
            crypto_box_MACBYTES)
        return -1;

    uint8_t nonce[crypto_box_NONCEBYTES];
    uint8_t temp[MAX_CRYPTO_REQUEST_SIZE];
    memcpy(temp + 1, data, length);
    temp[0] = request_id;
    new_nonce(nonce);
    int len = encrypt_data(recv_public_key, send_secret_key, nonce, temp, length + 1,
                           1 + crypto_box_PUBLICKEYBYTES * 2 + crypto_box_NONCEBYTES + packet);

    if (len == -1)
        return -1;

    packet[0] = NET_PACKET_CRYPTO;
    memcpy(packet + 1, recv_public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(packet + 1 + crypto_box_PUBLICKEYBYTES, send_public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(packet + 1 + crypto_box_PUBLICKEYBYTES * 2, nonce, crypto_box_NONCEBYTES);

    return len + 1 + crypto_box_PUBLICKEYBYTES * 2 + crypto_box_NONCEBYTES;
}

/* Puts the senders public key in the request in public_key, the data from the request
 * in data if a friend or ping request was sent to us and returns the length of the data.
 * packet is the request packet and length is its length.
 *
 *  return -1 if not valid request.
 */
int handle_request(const uint8_t *self_public_key, const uint8_t *self_secret_key, uint8_t *public_key, uint8_t *data,
                   uint8_t *request_id, const uint8_t *packet, uint16_t length)
{
    if (length <= crypto_box_PUBLICKEYBYTES * 2 + crypto_box_NONCEBYTES + 1 + crypto_box_MACBYTES ||
            length > MAX_CRYPTO_REQUEST_SIZE)
        return -1;

    if (memcmp(packet + 1, self_public_key, crypto_box_PUBLICKEYBYTES) != 0)
        return -1;

    memcpy(public_key, packet + 1 + crypto_box_PUBLICKEYBYTES, crypto_box_PUBLICKEYBYTES);
    uint8_t nonce[crypto_box_NONCEBYTES];
    uint8_t temp[MAX_CRYPTO_REQUEST_SIZE];
    memcpy(nonce, packet + 1 + crypto_box_PUBLICKEYBYTES * 2, crypto_box_NONCEBYTES);
    int len1 = decrypt_data(public_key, self_secret_key, nonce,
                            packet + 1 + crypto_box_PUBLICKEYBYTES * 2 + crypto_box_NONCEBYTES,
                            length - (crypto_box_PUBLICKEYBYTES * 2 + crypto_box_NONCEBYTES + 1), temp);

    if (len1 == -1 || len1 == 0)
        return -1;

    request_id[0] = temp[0];
    --len1;
    memcpy(data, temp + 1, len1);
    return len1;
}
