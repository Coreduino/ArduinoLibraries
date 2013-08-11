//*****************************************************************************
//
//   Near Field Communications (NFC)
//   Adafruit PN532 NFC/RFID Controller Shield for Arduino
//   PN523 Driver Class                                               PN532.cpp
//
//*****************************************************************************

/*
 *   Initial memory organization
 *
 *   -------  --------------------------
 *   Block            Byte Number
 *   Address    0      1      2      3
 *   ==  ===  =====  =====  =====  =====
 *    0  00h  UID0   UID1   UID2   BCC0
 *    1  01h  UID3   UID4   UID5   UID6
 *    2  02h  BCC1   intrn  00h    00h
 *    3  03h  E1h    10h    12h    00h
 *    4  04h  01h    03h    A0h    10h
 *    5  05h  44h    03h    00h    FEh
 *    6  06h  00h    00h    00h    00h
 *    7  07h  00h    00h    00h    00h
 *    8  08h  00h    00h    00h    00h
 *    9  09h  00h    00h    00h    00h
 *   10  0Ah  00h    00h    00h    00h
 *   11  0Bh  00h    00h    00h    00h
 *       ...
 *   40  28h  00h    00h    rfu    rfu
 *   41  29h  00h    00h    rfu    rfu
 *
 *   Where:
 *
 *        UID       Unique Identifier
 *        BCC       Block Check Character
 *        intrn     Reserved for interal use only
 *        rfu       ?
 */