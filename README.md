# gps_logger
Encrypted GPS logger

Work in progress. This is an encrypted GPS logger. Data is encrypted with an AES 128 bit key, which itself is encrypted with an RSA public key. AES encrypted is accelerated with the XMEGA's AES peripheral, but RSA encryption is done entirely in software.

RSA code courtasy of Emile van der Laan. Thank you for this code, Emile.

Encrypted data is stored in flash memory, and can only be decrypted on a PC with the appropriate RSA private key. Thus the key's own has complete control over the data, and privacy is preserved.

Care is taken to ensure data intergrity, even when the power supply is unexpectedly removed.

Licence: GPL V3.
