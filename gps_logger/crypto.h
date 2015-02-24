/*
 * crypto.h
 *
 * Created: 26/11/2014 11:57:11
 *  Author: paul.qureshi
 */ 


#ifndef CRYPTO_H_
#define CRYPTO_H_


#define AES_BLOCK_LENGTH	16
#define AES_KEY_LENGTH		16


extern void CRYPT_process_buffer(void *buffer, uint16_t buffer_size, bool decrypt);
extern void CRYPT_generate_intermediate_key(const uint8_t *key);
extern void CRYPT_load_key(const uint8_t *key);



#endif /* CRYPTO_H_ */