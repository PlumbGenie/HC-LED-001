Implementation of the RSA Public-key criptographic system, for 8-bit microcontrollers:

The source can be compiled with Microchip's C18 C compiler, V3.00 or better. Please don't expect very good perfomance as it's not yet optimized for the plataform.

Technical specs:

    * Estandard RSA encryption/decryption/signing algorithm (No key generation, please use OpenSSL, Lockbox, etc.).
    * No need for a dynamic memory manager (malloc)
    * Aprox. 5 Kb of ROM.
    * Aprox. 700 bytes of RAM for 512 bits operation.
    * Speed of a single block of data, 512 bits key : Measured on a PIC18F452@40Mhz: Decryption: 0.5 Seg. Encryption: 240 Seg.  :P
    * Key size limited by free RAM only.

The code is pure C, so it should compile on other 8-bit architectures.

It's based on the BIGDIGITS LIBRARY 2.1.0, but stripped down and using static buffers to avoid malloc().

                  Alfred (ortegaalfredo@gmail.com)

Copyright notice:

"Contains BIGDIGITS multiple-precision arithmetic code originally
written by David Ireland, copyright (c) 2001-6 by D.I. Management
Services Pty Limited <www.di-mgt.com.au>, and is used with
permission."

This software is provided "as is" without express or implied warranty 
of any kind. Our liability will be limited exclusively to the refund 
of the money you paid us for the software, namely nothing. By using the 
software you expressly agree to such a waiver. If you do not agree to 
the terms, do not use the software.