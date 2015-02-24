/*
 * crypto.c
 *
 * Created: 26/11/2014 11:57:23
 *  Author: paul.qureshi
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

#include "global.h"
#include "hw.h"
#include "eeprom.h"
#include "crypto.h"


volatile uint8_t	*aes_encryption_key = (uint8_t *)EEP_MAPPED_ADDR(0, 0);
uint8_t				aes_intermediate_key[AES_KEY_LENGTH];


/**************************************************************************************************
** Encrypt or decrypt a block of data
*/
void CRYPT_process_buffer(void *buffer, uint16_t buffer_size, bool decrypt)
{
	uint8_t	i;
	uint8_t	*p = (uint8_t *)buffer;
	
	AES.CTRL = AES_RESET_bm;
	NOP();
	AES.CTRL = 0;
	
	// load key
	if (!decrypt)
	{
		EEP_EnableMapping();
		for (i = 0; i < AES_KEY_LENGTH; i++)
			AES.KEY = aes_encryption_key[i];
		EEP_DisableMapping();
	}
	else
	{
		for (i = 0; i < AES_KEY_LENGTH; i++)
			AES.KEY = aes_intermediate_key[i];
	}
	
	// clear state memory
	for (i = 0; i < AES_BLOCK_LENGTH; i++)
		AES.STATE = 0x00;
	
	AES.CTRL = AES_XOR_bm;
	if (decrypt)
		AES.CTRL |= AES_DECRYPT_bm;
	
	do
	{
		uint8_t	bytes = AES_BLOCK_LENGTH;
		if (buffer_size < AES_BLOCK_LENGTH)
			bytes = buffer_size;
		
		// load block
		for (i = 0; i < bytes; i++)
			AES.STATE = p[i];
		
		// encrypt it
		AES.CTRL |= AES_START_bm;
		while (AES.STATUS == 0);
		
		// overwrite with encrypted data
		for (i = 0; i < bytes; i++)
			*p++ = AES.STATE;
		
		buffer_size -= bytes;
	} while(buffer_size);
}

/**************************************************************************************************
** Generate the intermediate key
*/
void CRYPT_generate_intermediate_key(const uint8_t *key)
{
	uint8_t	i;
	
	AES.CTRL = AES_RESET_bm;
	NOP();
	AES.CTRL = 0;
	
	for (i = 0; i < AES_KEY_LENGTH; i++)
		AES.KEY = key[i];
	
	for (i = 0; i < AES_BLOCK_LENGTH; i++)
		AES.STATE = 0x00;
	
	AES.CTRL |= AES_START_bm;
	while (AES.STATUS == 0);

	for (i = 0; i < AES_KEY_LENGTH; i++)
		aes_intermediate_key[i] = AES.KEY;
}

/**************************************************************************************************
** Store encryption key in EEPROM
*/
void CRYPT_load_key(const uint8_t *key)
{
	EEP_LoadPageBuffer(key, AES_KEY_LENGTH);
	EEP_AtomicWritePage(0);
}