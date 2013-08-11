//*****************************************************************************
//
//   Near Field Communications (NFC)
//   PN532 NFC/RFID Controller Shield for Arduino
//   PN523 Driver Class                                               PN532.cpp
//
//*****************************************************************************

//*****************************************************************************
//                                                            Applied Standards
//*****************************************************************************
/*
 *   This software adheres to the following standards and specifications:
 *
 *        Type 2 Tag Operation Specification (T2TOP):
 *             Technical Specification, T2TOP 1.1, NFC Forum[TM]
 *             NFCForum-TS-Type-2-Tag_1.1  (2011-05-31)
 *
 *             You may obtain a copy of this Specification and implementation 
 *             rights at the following page of Licensor's website:
 *             http://www.nfc-forum.org/specs/spec_license
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Copyright (C) 2013 by Programify Ltd. Copyright in regard to code format 
 *   and extensions to the functionality of this class.
 *
 *   Minor initial contributions:
 *
 *        MIT license
 *        adafruit/ladyada
 *        authenticateBlock, readMemoryBlock, writeMemoryBlock contributed 
 *        by Seeed Technology Inc (www.seeedstudio.com)
 */

//*****************************************************************************
//                                                                      Remarks
//*****************************************************************************
/*
 *   The Adafruit NFC shield uses the PN532 chip-set (the most popular NFC chip 
 *   on the market) and is what is embedded in most mobile phones and devices
 *   that are NFC enabled. This chipset allows you to read and write to tags 
 *   and cards, communicate with phones, and emulate a passive NFC tag.
 *
 *   While the Adafruit controller has many capabilities, this Arduino library 
 *   currently only supports reading and writing tags, and does not support 
 *   phone-to-shield communication, tag emulation (which requires an external 
 *   'secure element' only available from NXP) or other more advanced features 
 *   at this time.
 *
 *   NFC (Near Field Communications) is a way for two devices very close to 
 *   each other to communicate without authentication. NFC is an all inclusive
 *   superset of RFID technology.
 *
 *   NFC Documentation:
 *
 *        http://apps4android.org/nfc-specifications/
 *
 *   NFC Data Exchange Format (NDEF):
 *
 *        http://developer.android.com/reference/android/nfc/NdefMessage.html
 *        http://www.developer.nokia.com/Community/Wiki/Understanding_NFC_Data_Exchange_Format_%28NDEF%29_messages
 */

//*****************************************************************************
//                                                    Tag Memory Configurations
//*****************************************************************************
/*
 *   NFC
 *   Forum
 *   Type   Compatible Products
 *   =====  ===========================================================
 *     1    Broadcom Topaz
 *     2    NXP Mifare Ultralight, NXP Mifare Ultralight C, NXP NTAG203
 *     3    Sony FeliCa
 *     4    NXP DESFire, NXP SmartFX with JCOP
 *
 *   NFC    Number    Block               
 *   Forum    of       Size  Memory             
 *   Type   Blocks  (Bytes)  Sizes 
 *   =====  ======  =======  ==========
 *     1s       15        8  96    
 *     1d        ?        8  96    
 *     2s       15        4  48/144
 *     2d        ?        4  48/144
 *     3         ?       16  1K/4K/9K
 *     4         ?        ?  4K/32K
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   05-11-12  Modified by Frankie.Chu at Seeed.
 *             Added some program comments of some key functions.
 *             Modified the basic interface functions spiwrite() and spiread()
 *             to be write() and read(),and they will call the functions of
 *             class SPIClass but not to simulate timing of SPI.
 *	          Rename all function names to head-down camel-case.
 *             Modified the constructor of PN532 class.
 *   11-07-13  Modified by programify.com.
 *             Improved source code format.
 *             Added additional commentary and information.
 */

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------
#include <Arduino.h>
#include <SPI.h>
#include "PN532.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants
#define   PN532_PACK_BUFF_SIZE     64

//-----------------------------------------------------------------------------
//                                                                    Functions
//-----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Public Functions
//        PN532
//
//        AuthenticateBlock
//        SetTargetType
//        ConfigurePeerAsInitiator
//        ConfigurePeerAsTarget
//        GetFirmwareVersion
//        InitiatorTxRx
//        ReadMemoryBlock
//        ReadPassiveTargetID
//        SAMConfig
//        SendCommandCheckAck
//        TargetTxRx
//        WriteMemoryBlock

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Private Functions
//        checkSpiAck
//        read
//        readSpiStatus
//        write
//        writeCommand

//-----------------------------------------------------------------------------
//                                                                 Library Data
//-----------------------------------------------------------------------------
byte pn532response_firmwarevers [] = { 0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03 } ;
byte pn532_packetbuffer [PN532_PACK_BUFF_SIZE] ;


//=============================================================================
//   Constructor                                                          PN532
//-----------------------------------------------------------------------------
/*
 *   Construct a PN532 object and construct PN532's own SPI interface object
 *   at the same time.
 *
 *   Developer's note: digitalWrite() should be performed before pinMode().
 *   This ensures there is no timing gap of a few microseconds where the CS
 *   line could go LOW instead of floating?
 */
PN532::PN532 (BYTE bChipSelect):pn532_SPI ()
{
// Deselect the PN532 shield
     _bPinCS = bChipSelect ;
// Set pin high before setting direction
     digitalWrite (_bPinCS, HIGH) ;
// Set pin's data direction
     pinMode (_bPinCS, OUTPUT) ;
}

//-----------------------------------------------------------------------------
//                                                               .SetTargetType
//-----------------------------------------------------------------------------
/*
 *   SetTargetType() initializes comms and data required to access the NFC 
 *   Shield for a specific type of chipset or family of tags.
 *
 *   Parameters:
 *
 *        dwTagType      Indicates the type of tag being used within the NFC
 *                       application you are developing. The types supported
 *                       by this library are assigned a manifest constant (in
 *                       PN532.h) in the PN532_TAG_ series:
 *
 *                            PN532_TAG_MIFARE_ISO14443A
 *                            PN532_TAG_NTAG203
 */
DWORD PN532::SetTargetType (DWORD dwTagType) 
{
     DWORD   dwVersion ;

// Init defaults
     _dwTagType = dwTagType ;
     _bBaudRate = PN532_106KBPS_ISO14443A ;
     _bMaxTg    = 2 ;
     _wPageSize = 16 ;
     ///Serial.println ("PN532::SetTargetType()") ;
// Determine settings for given tag/chipset
     switch (dwTagType)
     {
          case PN532_TAG_MIFARE_ISO14443A :
               _wBlocks    = 64 ;
               _wBlockSize = 16 ;
               break ;

          case PN532_TAG_NTAG203 :
               _wBlocks    = 42 ;
               _wBlockSize =  4 ;
               break ;

          default :
#ifdef PN532DEBUG
               Serial.println ("* TAG TYPE CODE NOT RECOGNISED") ;
#endif
               return false ;
     }
// Determine safe total memory required
     _wMemSize = _wBlocks * _wBlockSize ;
     _wPages   = _wMemSize / _wPageSize ;
     if (_wMemSize % _wPageSize)
          _wPages ++ ;
// Init SPI interface on PN532 shield
     pn532_SPI.begin () ;
// Set SPI interface mode as per PN532's datasheet
     pn532_SPI.setDataMode (SPI_MODE0) ;
     pn532_SPI.setBitOrder (LSBFIRST) ;
// Set SPI frequency to 1/16th of system clock frequency
     pn532_SPI.setClockDivider (SPI_CLOCK_DIV16) ;

#ifdef PN532DEBUG
     Serial.println ("Enable PN532 shield") ;
#endif

/// Enable PN532 shield
     ///digitalWrite (_bPinCS, LOW) ;
     ///delay (1000) ;
// Request firmware version but ignore the response
     dwVersion = GetFirmwareVersion () ;
     return (dwVersion) ;
}

//-----------------------------------------------------------------------------
//                                                          .GetFirmwareVersion
//-----------------------------------------------------------------------------
DWORD PN532::GetFirmwareVersion (void) 
{
     DWORD   dwVersion ;

// Init
     dwVersion = 0 ;
     ///Serial.println ("PN532::GetFirmwareVersion()") ;
// Prepare FIRMWAREVERSION command packet
     pn532_packetbuffer [0] = PN532_CMD_FIRMWAREVERSION ;
// Send 1-byte command
     if (! SendCommandCheckAck (pn532_packetbuffer, 1))
          goto exit_function ;
// Read data packet
     read (pn532_packetbuffer, 12) ;
// Check ???
     if (strncmp ((char *) pn532_packetbuffer, (char *) pn532response_firmwarevers, 6) != 0)
          goto exit_function ;
// Prepare firmware version from data in buffer
     dwVersion   = pn532_packetbuffer [6] ;
     dwVersion <<= 8 ;
     dwVersion  |= pn532_packetbuffer [7] ;
     dwVersion <<= 8 ;
     dwVersion  |= pn532_packetbuffer [8] ;
     dwVersion <<= 8 ;
     dwVersion  |= pn532_packetbuffer [9] ;

exit_function:
     return (dwVersion) ;
}

//-----------------------------------------------------------------------------
//                                                         .SendCommandCheckAck
//-----------------------------------------------------------------------------
/*
 *   SendCommandCheckAck() sends a command to the PN532 with SPI and checks the
 *   ACK.
 *
 *   Parameters:
 *
 *        abCommand      The pointer that saves the command code to be sent.
 *
 *        bLength        The number of bytes of the command.
 *
 *       [wTimeout]      Optional timeout in milliseconds. This will default
 *                       to 1000ms (1 second) if this parameter is missing.
 *
 *   Returns:
 *
 *        TRUE if command sent successfully
 */
boolean PN532::SendCommandCheckAck (BYTE * abCommand, BYTE bLength, WORD wTimeout) 
{
     BOOL      bfOkay ;
     WORD      wTimer ;

// Init
     bfOkay = FALSE ;
     wTimer = 0 ;

#ifdef PN532DEBUG
     Serial.println ("PN532::SendCommandCheckAck()") ;
#endif

// Write the command
     writeCommand (abCommand, bLength) ;
// Wait for chip to say it's ready
     while (true)
     {
          if (readSpiStatus () == PN532_SPI_READY)
               break ;
          if (wTimeout != 0)
          {
               wTimer += 10 ;
               if (wTimer > wTimeout)
                    goto exit_function ;
          }
          delay (10) ;
     }
// Read acknowledgement
     if (! checkSpiAck ())
          goto exit_function ;

     wTimer = 0 ;
// Wait for chip to say its ready!
     while (true)
     {
          if (readSpiStatus () == PN532_SPI_READY)
               break ;
          if (wTimeout != 0)
          {
               wTimer += 10 ;
               if (wTimer > wTimeout)
                    goto exit_function ;
          }
          delay (10) ;
     }
// ack'd command
     bfOkay = TRUE ;

exit_function:

#ifdef PN532DEBUG
     if (! bfOkay)
          Serial.println ("PN532::SendCommandCheckAck() FAILED") ;
#endif

     return (bfOkay) ;
}

//-----------------------------------------------------------------------------
//                                                                   .SAMConfig
//-----------------------------------------------------------------------------
/*
 *   SAMConfig() sends a code 0x14 (PN532_CMD_SAMCONFIGURATION) to the 
 *   PN532 shield. This PN532 command selects the data flow path by 
 *   configuring the internal serial data switch.
 *
 *   A SAM (Security Access Module) companion chip can be used for added 
 *   security. It is connected to the PN532 by using a S2C interface :
 *   (SigIn (pin #36), SigOut (pin #35) and CLAD (pin #34)). The CLAD line is 
 *   optional. The arrangement of PN532 to SAM is defined by command byte [1]
 *   and the use of PN532_SAM_ manifest constants.
 *
 */
boolean PN532::SAMConfig (void) 
{
// Prepare SAMCONFIGURATION command packet
     pn532_packetbuffer [0] = PN532_CMD_SAMCONFIGURATION ;
     pn532_packetbuffer [1] = PN532_SAM_MODENORMAL ;
     pn532_packetbuffer [2] = 20 ;      // Timeout (1 sec : 20*50ms) (mandatory if in PN532_SAM_MODEVIRTUALCARD mode)
     pn532_packetbuffer [3] = 0x01 ;    // Enable PN532 to use the IRQ pin (this is the default)
// Send 4-byte command and check for acknowledgement
     if (! SendCommandCheckAck (pn532_packetbuffer, 4))
          return false ;
// Read data packet
     read (pn532_packetbuffer, 8) ;
     if (pn532_packetbuffer [5] == PN532_ACK_SAMCONFIGURATION)
          return true ;
// Request failed
     return false ;
}

//-----------------------------------------------------------------------------
//                                                    .ConfigurePeerAsInitiator
//-----------------------------------------------------------------------------
/*
 *   ConfigurePeerAsInitiator() configures the NFC shield as initiator in the 
 *   peer to peer commnunication and only the initiator set the baud rate.
 *
 *   Parameter:
 *
 *        bBaudrate  0 = 106kbps
 *                   1 = 201kbps
 *                   2 = 424kbps
 *
 *   Returns TRUE if the shield finds the target and is configured as 
 *   initiator.
 */
DWORD PN532::ConfigurePeerAsInitiator (BYTE bBaudrate) 
{
#ifdef PN532DEBUG
     BYTE      bIndex ;
#endif

// Prepare INJUMPFORDEP command packet
     pn532_packetbuffer [0] = PN532_CMD_INJUMPFORDEP ;
     pn532_packetbuffer [1] = 0x01 ;     // Active Mode
     pn532_packetbuffer [2] = bBaudrate ;// Use 1 or 2; Mode 0 (106kps) is not supported yet
     pn532_packetbuffer [3] = 0x01 ;     // Indicates Optional Payload is present
// Polling request payload
     pn532_packetbuffer [4] = 0x00 ; 
     pn532_packetbuffer [5] = 0xFF ; 
     pn532_packetbuffer [6] = 0xFF ; 
     pn532_packetbuffer [7] = 0x00 ; 
     pn532_packetbuffer [8] = 0x00 ; 
// Send 9-byte command and check for acknowledgement
     if (! SendCommandCheckAck (pn532_packetbuffer, 9))
          return false ;
// Read data packet
     read (pn532_packetbuffer, 19 + 6) ;

#ifdef PN532DEBUG
     Serial.println () ;
// Check the response
     Serial.println ("PEER_INITIATOR") ;
     for (bIndex = 0 ; bIndex < 19 + 6 ; bIndex ++)
     {
          Serial.print (pn532_packetbuffer [bIndex], HEX) ;
          Serial.print (" ") ;
     }
#endif

// Check if no error
     return (pn532_packetbuffer [7] == 0x00) ;
}

///-----------------------------------------------------------------------------
///                                                               .InitiatorTxRx
///-----------------------------------------------------------------------------
/*
 *   InitiatorTxRx() transmits to the target and receives from the target.
 *
 *   Parameters:
 *
 *        cpDataTx       Pointer to data buffer to trasmit.
 *        cpDataRx       Pointer to buffer to save the data received.
 *
 *   Returns:
 *
 *        TRUE if successfull.
 */
boolean PN532::InitiatorTxRx (IN char * cpDataTx, OUT char * cpDataRx)
{
     BYTE      bTarget ;
     BYTE      bIndex ;

// Init
     bTarget = 1 ;
// Prepare INDATAEXCHANGE command packet
     pn532_packetbuffer [0] = PN532_CMD_INDATAEXCHANGE ;
     pn532_packetbuffer [1] = bTarget ;
// Pack the data to be sent
     for (bIndex = (2 + 0) ; bIndex < (2 + 16) ; bIndex ++)
          pn532_packetbuffer [bIndex] = cpDataTx [bIndex - 2] ;
// Send 18-byte command and check for acknowledgement
     if (! SendCommandCheckAck (pn532_packetbuffer, 18))
          return false ;
// Read 24-byte response packet
     read (pn532_packetbuffer, 18 + 6) ;

#ifdef PN532_P2P_DEBUG
// Report the response
     Serial.println () ;
     Serial.println ("INITIATOR receive:") ;
     for (bIndex = 0 ; bIndex < 18 + 6 ; bIndex ++)
     {
          Serial.print (pn532_packetbuffer [bIndex], HEX) ;
          Serial.print (" ") ;
     }
#endif

// Store data received from target
     for (bIndex = 8 ; bIndex < (8 + 16) ; bIndex ++)
          cpDataRx [bIndex - 8] = pn532_packetbuffer [bIndex] ;
// Check if no error
     return (pn532_packetbuffer [7] == 0x00) ;
}

///-----------------------------------------------------------------------------
///                                                       .ConfigurePeerAsTarget
///-----------------------------------------------------------------------------
/// DO NOT USE UNTIL MADE GENERIC
DWORD PN532::ConfigurePeerAsTarget (void)
{
     BYTE      i ;
     byte      pbuffer [38] =
               {
                    PN532_CMD_TGINITASTARGET, 
                    0x00,
                    0x08, 0x00,         // SENS_RES
                    0x12, 0x34, 0x56,   // NFCID1
                    0x40,               // SEL_RES
                    0x01, 0xFE, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, // POL_RES
                    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
                    0xFF, 0xFF,
                    0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // NFCID3t: Change this to desired value
                    0x00, 0x00          // Length of general and historical bytes
               } ;

// Prepare TGINITASTARGET command packet
     for (i = 0 ; i < 38 ; i ++)
          pn532_packetbuffer [i] = pbuffer [i] ;
// Send 38-byte command to PN532
     if (! SendCommandCheckAck (pn532_packetbuffer, 38))
          return false ;
// Read data packet
     read (pn532_packetbuffer, 18 + 6) ;

#ifdef PN532DEBUG
     Serial.println () ;
// check some basic stuff
     Serial.println("PEER_TARGET");
     for (i = 0 ; i < 18 + 6 ; i ++)
     {
          Serial.print   (pn532_packetbuffer [i], HEX) ;
          Serial.println (" ") ;
     }
#endif

// Check if received all of response
     return (pn532_packetbuffer [23] == 0x00) ;
}

//-----------------------------------------------------------------------------
//                                                                  .TargetTxRx
//-----------------------------------------------------------------------------
/*
 *   TargetTxRx() receives data first and then transmits data to the initiator.
 */
DWORD PN532::TargetTxRx (IN char * cpDataTx, OUT char * cpDataRx)
{
     BYTE      bAck ;
     BYTE      bIndex ;
     BYTE      bStatus ;

// Prepare TGGETDATA command packet (PN532 is a target for DEP - Data Exchange Protocol)
     pn532_packetbuffer [0] = PN532_CMD_TGGETDATA ;
// Send 1-byte command to PN532
     if (! SendCommandCheckAck (pn532_packetbuffer, 1))
          return false ;
// Read data packet
     read (pn532_packetbuffer, 18 + 6) ;
     bAck    = pn532_packetbuffer [6] ;
     bStatus = pn532_packetbuffer [7] ;

#ifdef PN532_P2P_DEBUG
     Serial.println () ;
     Serial.println ("TARGET RX:") ;
     for (bIndex = 0 ; bIndex < 18 + 6 ; bIndex ++)
     {
          Serial.print (pn532_packetbuffer [bIndex], HEX) ;
          Serial.print (" ") ;
     }
#endif

// Check for errors
     if (bAck != PN532_ACK_TGGETDATA)
          return false ;
     if (bStatus != 0x00)
          return false ;
// Extract incoming data
     for (bIndex = 8 ; bIndex < 8 + 16 ; bIndex ++)
          cpDataRx [bIndex - 8] = pn532_packetbuffer [bIndex] ;
// Prepare TGSETDATA command packet
     pn532_packetbuffer [0] = PN532_CMD_TGSETDATA ;
// Pack the data to send to target
     for (bIndex = 1 + 0 ; bIndex < 1 + 16 ; bIndex ++)
          pn532_packetbuffer [bIndex] = cpDataTx [bIndex - 1] ;
// Send 17-byte command to PN532
     if (! SendCommandCheckAck (pn532_packetbuffer, 17))
          return false ;
// Read data packet
     read (pn532_packetbuffer, 2 + 6) ;
     bAck    = pn532_packetbuffer [6] ;
     bStatus = pn532_packetbuffer [7] ;

#ifdef PN532_P2P_DEBUG
     Serial.println () ;
// check the response
     Serial.println ("TARGET get response after transmiting: ") ;
     for (bIndex = 0 ; bIndex < 2 + 6 ; bIndex ++)
     {
          Serial.print (pn532_packetbuffer [bIndex], HEX) ;
          Serial.print (" ") ;
     }
#endif

// Check for errors
     if (bAck != PN532_ACK_TGSETDATA)
          return false ;
     if (bStatus != 0x00)
          return false ;
// Success
     return true ;
}

//-----------------------------------------------------------------------------
//                                                           .AuthenticateBlock
//-----------------------------------------------------------------------------
/*
 *   AuthenticateBlock() authenticates a target?
 *
 *   Call with:
 *
 *        bTarget        The logical number of the relevant target.
 *
 *                       The PN532 can handle up to two targets, therefore 
 *                       this value should be in the range 1..2.
 *
 *                       This function does not support the use of the 
 *                       'More Information' (MI) bit (bit 6). See the PN532
 *                       User Manual for details.
 *
 *        dwTgId         Target's ID.
 *
 *        bBlockAddr     The block number being addressed.
 *
 *                       The valid range of this address is dependent on the
 *                       type of target.
 *
 *                            NTAG203             0..41
 *                            MIFARE 1K card      0..63
 *
 *        bAuthType      Authenication key?
 *
 *                            PN532_AUTH_KEYA (1) Default.
 *                            PN532_AUTH_KEYB (2)
 *
 *        bpKeys         Pointer to an array of 6 bytes containing a key?
 */
/// UNTESTED
DWORD PN532::AuthenticateBlock (BYTE bTarget, DWORD dwTgId, BYTE bBlockAddr, BYTE bAuthType, BYTE * bpKeys)
{
     BYTE      bAck ;
     BYTE      bKey ;
     BYTE      bStatus ;

#ifdef PN532DEBUG
     BYTE      bIndex ;
#endif

// Init
     bKey = PN532_AUTH_WITH_KEYA ;
     if (bAuthType == PN532_AUTH_KEYB)
          bKey = PN532_AUTH_WITH_KEYB ;
// Prepare INDATAEXCHANGE command packet
     pn532_packetbuffer  [0] = PN532_CMD_INDATAEXCHANGE ;
     pn532_packetbuffer  [1] = bTarget ;
     pn532_packetbuffer  [2] = bKey ;
     pn532_packetbuffer  [3] = bBlockAddr ;
     pn532_packetbuffer  [4] = bpKeys [0] ;
     pn532_packetbuffer  [5] = bpKeys [1] ;
     pn532_packetbuffer  [6] = bpKeys [2] ;
     pn532_packetbuffer  [7] = bpKeys [3] ;
     pn532_packetbuffer  [8] = bpKeys [4] ;
     pn532_packetbuffer  [9] = bpKeys [5] ;
     pn532_packetbuffer [10] = ((dwTgId >> 24) & 0xFF) ;
     pn532_packetbuffer [11] = ((dwTgId >> 16) & 0xFF) ;
     pn532_packetbuffer [12] = ((dwTgId >>  8) & 0xFF) ;
     pn532_packetbuffer [13] = ((dwTgId >>  0) & 0xFF) ;
// Send 14-byte command and check for acknowledgement
     if (! SendCommandCheckAck (pn532_packetbuffer, 14))
          return false ;
// Read 8-byte data packet
     read (pn532_packetbuffer, 2 + 6) ;
// Extract fields from packet
     bAck    = pn532_packetbuffer [6] ;
     bStatus = pn532_packetbuffer [7] ;

#ifdef PN532DEBUG
// Display contents of returned packet
     for (bIndex = 0 ; bIndex < 14 ; bIndex ++)
     {
          Serial.print (pn532_packetbuffer [bIndex], HEX) ;
          Serial.print (" ") ;
     }
     Serial.println () ;
     Serial.println ("AUTH") ;
     for (bIndex = 0 ; bIndex < 2 + 6 ; bIndex ++)
     {
          Serial.print   (pn532_packetbuffer [bIndex], HEX) ;
          Serial.println (" ") ;
     }
#endif

// Check if call failed
     if (bAck != PN532_ACK_INDATAEXCHANGE)
          return false ;
     if (bStatus != 0x00)
          return false ;
// Success
     return true ;
}

//-----------------------------------------------------------------------------
//                                                             .ReadMemoryBlock
//-----------------------------------------------------------------------------
/*
 *   ReadMemoryBlock() reads a block of data from the tag.
 *
 *   Parameters:
 *
 *        bTarget        The logical number of the target being addressed.
 *
 *                       The PN532 is capable of handling up to two logical
 *                       targets at the same time, so this value should not 
 *                       exceed 2. For Jewel Cards, the maximum value is 1.
 *
 *                       Although the PN532 expects a value in the range 1..2, 
 *                       this function has been developed using logical 
 *                       target 1.
 *
 *        bBlockAddr     The block number being addressed.
 *
 *                       The valid range of this address is dependent on the
 *                       type of target.
 *
 *                            NTAG203             0..41
 *                            MIFARE 1K card      0..63
 *
 *        bpBlock        Pointer to buffer to store data from tag.
 *
 *   Returns:
 *
 *        TRUE if successful.
 */
boolean PN532::ReadMemoryBlock (BYTE bTarget, BYTE bBlockAddr, BYTE * bpBlock)
{
     BYTE      bAck ;
     BYTE      bIndex ;
     BYTE      bStatus ;
     int       iLength ;

// Check for over limit
     if (bBlockAddr > _wBlocks)
          return false ;
// Init
     iLength = 18 + 6 ;
// Prepare INDATAEXCHANGE command packet to read 16 bytes
     pn532_packetbuffer [0] = PN532_CMD_INDATAEXCHANGE ;
     pn532_packetbuffer [1] = bTarget ;
     pn532_packetbuffer [2] = PN532_MIFARE_READPAGE ;
     pn532_packetbuffer [3] = bBlockAddr ;
// Send 4-byte command and check for acknowledgement
     if (! SendCommandCheckAck (pn532_packetbuffer, 4))
          return false ;
// Read data packet (fetches 16 bytes!)
     read (pn532_packetbuffer, iLength) ;
// Extract fields from packet
     bAck    = pn532_packetbuffer [6] ;
     bStatus = pn532_packetbuffer [7] ;
// Adjust data length based on blocksize
     ///iLength -= 16 ;
     ///iLength += _wBlockSize ;
// Transfer contents of packet to destination store
     for (bIndex = 8 ; bIndex < iLength ; bIndex ++)
          bpBlock [bIndex - 8] = pn532_packetbuffer [bIndex] ;

#ifdef PN532DEBUG
     char      szHex [8] ;

// Report some contents of returned packet
     Serial.println ("READ ") ;
     for (bIndex = 0 ; bIndex < iLength ; bIndex ++)
     {
          sprintf (szHex, "%02X ", pn532_packetbuffer [bIndex]) ;
          Serial.print (szHex) ;
     }
     Serial.println ("") ;
#endif

// Check if call failed
     if (bAck != PN532_ACK_INDATAEXCHANGE)
          return false ;
     if (bStatus != 0x00)
          return false ;
// Success
     return true ;
}

//-----------------------------------------------------------------------------
//                                                            .WriteMemoryBlock
//-----------------------------------------------------------------------------
/*
 *   WriteMemoryBlock() writes a block of data to the tag. DO NOT write to 
 *   Sector Trailer Block unless you know what you are doing.
 *
 *   Parameter:
 *
 *        bTarget        can be 1 or 2.
 *
 *        bBlockAddr     The block number being addressed.
 *
 *                       The valid range of this address is dependent on the
 *                       type of target.
 *
 *                            NTAG203             0..41
 *                            MIFARE 1K card      0..63
 *
 *	     block          Pointer to block of data that will be written to
 *                       the tag.
 *
 *   Returns true if successfully written to the tag.
 *
 */
/// UNTESTED
boolean PN532::WriteMemoryBlock (BYTE bTarget, BYTE bBlockAddr, BYTE * bpBlock) 
{
     BYTE      bAck ;
     BYTE      bIndex ;
     BYTE      bStatus ;

// Prepare INDATAEXCHANGE command packet
     pn532_packetbuffer [0] = PN532_CMD_INDATAEXCHANGE ;
     pn532_packetbuffer [1] = bTarget ;
     pn532_packetbuffer [2] = PN532_MIFARE_WRITEPAGE ;
     pn532_packetbuffer [3] = bBlockAddr ;

     for (bIndex = 0 ; bIndex < _wBlockSize ; bIndex ++)
          pn532_packetbuffer [bIndex + 4] = bpBlock [bIndex] ;

// Send command and check for acknowledgement
     if (! SendCommandCheckAck (pn532_packetbuffer, _wBlockSize + 4))
          return false ;
// Read data packet
     read (pn532_packetbuffer, 2 + 6) ;
// Extract fields from packet
     bAck    = pn532_packetbuffer [6] ;
     bStatus = pn532_packetbuffer [7] ;
// Check if call failed
     if (bAck != PN532_ACK_INDATAEXCHANGE)
          return false ;
     if (bStatus != 0x00)
          return false ;

#ifdef PN532DEBUG
// Report response
     Serial.println("WRITE");
     for (bIndex = 0 ; bIndex < 2 + 6 ; bIndex ++)
     {
          Serial.print   (pn532_packetbuffer [bIndex], HEX) ;
          Serial.println (" ") ;
     }
#endif

// Success
     return true ;
}

//-----------------------------------------------------------------------------
//                                                         .ReadPassiveTargetID
//-----------------------------------------------------------------------------
/*
 *   ReadPassiveTargetID() attempts to activate the specified number of 
 *   targets that operate at the specified baud rate and modulation type.
 *
 *   Returns the unique identifier of the first target encountered expressed 
 *   as a QWORD (Quad-Word, an unsigned 64-bit value).
 */
QWORD PN532::ReadPassiveTargetID (void)
{
     BYTE      bAck ;
     BYTE      bTargets ;
     char *    cpInfo ;
     char      szHex [24] ;
     char      szSel  [8] ;
     char      szSens [8] ;
     int       iIndex ;
     QWORD     qwNFCid ;

// Init
     qwNFCid = 0 ;
// Ensure maximum number of targets is within acceptable range
     if (_bMaxTg != 2)
          _bMaxTg = 1 ;
     if (_bBaudRate == 4)
          _bMaxTg = 1 ;
// Prepare INLISTPASSIVETARGET command packet
     pn532_packetbuffer [0] = PN532_CMD_INLISTPASSIVETARGET ;
     pn532_packetbuffer [1] = _bMaxTg ;
     pn532_packetbuffer [2] = _bBaudRate ;
// Send 3-byte command and check for acknowledgement
     if (! SendCommandCheckAck (pn532_packetbuffer, 3))
     {
          ///Serial.print (".") ;
          goto exit_function ;
     }
// Receive 20-byte response packet
     read (pn532_packetbuffer, 20) ;
// Extract fields from packet
     bAck     = pn532_packetbuffer [6] ;
     bTargets = pn532_packetbuffer [7] ;
// Check if call failed
     if (bAck != PN532_ACK_INLISTPASSIVETARGET)
     {
          Serial.print ("!") ;
          goto exit_function ;
     }
// Check how many targets where found
     switch (bTargets)
     {
          case 0 :
               Serial.println ("No targets found.") ;
               goto exit_function ;

          case 1 :
          case 2 :
          // Report number of targets found
               Serial.print   ("Targets found : ") ;
               Serial.print   (bTargets, DEC) ;
               Serial.println () ;
               break ;

          default :
          // Report error
               Serial.println ("INLISTPASSIVETARGET command returned invalid data.") ;
               goto exit_function ;
     }
// Construct NFC ID
     for (iIndex = 0 ; iIndex < pn532_packetbuffer [12] ; iIndex ++)
     {
          qwNFCid <<= 8 ;
          qwNFCid  |= pn532_packetbuffer [13 + iIndex] ;
     }

#ifdef PN532_DEBUG_VERBOSE
// Report SEL which specifies the Cascade Level (CLn) (SELECT command?)
     sprintf (szSens, "%02X%02X", pn532_packetbuffer [9], pn532_packetbuffer [10]) ;
     sprintf (szSel,  "%02X",     pn532_packetbuffer [11]) ;
     Serial.print   ("SENS Response : 0x") ;
     Serial.println (szSens) ;
     Serial.print   ("SEL Response  : 0x") ;
     Serial.println (szSel) ;
// Display manufacturer data
     Serial.println () ;
     Serial.println ("Target Info:") ;
     for (iIndex = 0 ; iIndex < 20 ; iIndex ++)
     {
          sprintf (szHex, "%2d) 0x%02X (%3d) ", iIndex, pn532_packetbuffer [iIndex], pn532_packetbuffer [iIndex]) ;
          Serial.print (szHex) ;
          switch (iIndex)
          {
               case  7 : cpInfo = (char *) "Tags Found" ; break ;
               case  9 :
               case 10 : cpInfo = (char *) "SENS" ; break ;
               case 11 : cpInfo = (char *) "SEL" ; break ;
               case 12 : cpInfo = (char *) "NFCIDLength" ; break ;
               case 13 : cpInfo = (char *) "ID (MSB)" ; break ;
               case 14 :
               case 15 :
               case 16 :
               case 17 :
               case 18 : cpInfo = (char *) "ID" ; break ;
               case 19 : cpInfo = (char *) "ID (LSB)" ; break ;
               default : cpInfo = (char *) "" ; break ;
          }
          Serial.println (cpInfo) ;
     }
#endif

exit_function:
// Return tag's ID or zero if failed
     return (qwNFCid) ;
}

//-----------------------------------------------------------------------------
//                                                               .readSpiStatus
//-----------------------------------------------------------------------------
/*
 *   readSpiStatus()
 */
BYTE PN532::readSpiStatus (void) 
{
     BYTE      bStatus ;

#ifdef PN532DEBUG
     char      szInfo [32] ;
#endif

     digitalWrite (_bPinCS, LOW) ;
     delay (2) ;
     write (PN532_SPI_STATREAD) ;
     bStatus = pn532_SPI.transfer (0) ;
     digitalWrite (_bPinCS, HIGH) ;

#ifdef PN532DEBUG
     sprintf (szInfo, "  readSpiStatus() : %d (%02X)", bStatus, (int) bStatus) ;
     Serial.println (szInfo) ;
#endif

     return (bStatus) ;
}

//-----------------------------------------------------------------------------
//                                                                 .checkSpiAck
//-----------------------------------------------------------------------------
/*
 *   checkSpiAck() reads 6 bytes from the PN532 shield through the SPI 
 *   interface and checks to see if it matches .
 */
BOOL PN532::checkSpiAck (void)
{
     BYTE      abAck [] = { 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00 } ;
     BYTE      abBuffer [6] ;

// Receive 6-byte response
     read (abBuffer, 6) ;
// Check if received correct ACK
     if (strncmp ((char *) abBuffer, (char *) abAck, 6) == 0)
          return TRUE ;
// Failed
     return FALSE ;
}

//-----------------------------------------------------------------------------
//                                                                        .read
//-----------------------------------------------------------------------------
/*
 *   read() reads a specified number of bytes data from the PN532 shield.
 *
 *   Call with:
 *
 *        abBuffer  Pointer to an array of bytes to receive the data.
 *
 *        iLength   The number of bytes to read from the PN532 shield.
 */
void PN532::read (BYTE * abBuffer, int iLength)
{
     int       iIndex ;

// Select the PN532 shield
     digitalWrite (_bPinCS, LOW) ;
// Issue SPI DATAREAD command byte
     delay (2) ;
     write (PN532_SPI_DATAREAD) ;
// Continue to receive for predetermined number of bytes
     for (iIndex = 0 ; iIndex < iLength ; iIndex ++) 
     {
          delay (1) ;
          abBuffer [iIndex] = pn532_SPI.transfer (0) ;
     }

#ifdef PN532DEBUG
// Report data loaded
     Serial.print ("Reading: ") ;
     for (iIndex = 0 ; iIndex < iLength ; iIndex ++) 
     {
          Serial.print (" 0x") ;
          Serial.print (abBuffer [iIndex], HEX) ;
     }
     Serial.println () ;
#endif

// Deselect the PN532 shield
     digitalWrite (_bPinCS, HIGH) ;
}

//-----------------------------------------------------------------------------
//                                                                .writeCommand
//-----------------------------------------------------------------------------
void PN532::writeCommand (BYTE * abCommand, BYTE bLength) 
{
     BYTE      bIndex ;
     BYTE      checksum ;
     BYTE      checksum_1 ;
     BYTE      cmdlen_1 ;

// Init
     bLength ++ ;

#ifdef PN532DEBUG
     Serial.println ("PN532::writeCommand()") ;
#endif

// Select the PN532 shield
     digitalWrite (_bPinCS, LOW) ;
     delay (2) ;
// Enter SPI DATAWRITE mode
     write (PN532_SPI_DATAWRITE) ;
// 
     checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2 ;
     write (PN532_PREAMBLE) ;
     write (PN532_PREAMBLE) ;
     write (PN532_STARTCODE2) ;

     write (bLength) ;
     cmdlen_1 = ~bLength + 1 ;
     write (cmdlen_1) ;

     write (PN532_HOSTTOPN532) ;
     checksum += PN532_HOSTTOPN532 ;

#ifdef PN532DEBUG
     Serial.print ("   ") ;
     Serial.print (" 0x") ;
     Serial.print (PN532_PREAMBLE, HEX) ;
     Serial.print (" 0x") ;
     Serial.print (PN532_PREAMBLE, HEX) ;
     Serial.print (" 0x") ;
     Serial.print (PN532_STARTCODE2, HEX) ;
     Serial.print (" 0x") ;
     Serial.print (bLength, HEX) ;
     Serial.print (" 0x") ;
     Serial.print (cmdlen_1, HEX) ;
     Serial.print (" 0x") ;
     Serial.print (PN532_HOSTTOPN532, HEX) ;

     Serial.println ("\n  Command String") ;
     Serial.print ("   ") ;
#endif

     for (bIndex = 0 ; bIndex < bLength - 1 ; bIndex ++) 
     {
          write (abCommand [bIndex]) ;
          checksum += abCommand [bIndex] ;
#ifdef PN532DEBUG
          Serial.print (" 0x") ; 
          Serial.print (abCommand [bIndex], HEX) ;
#endif
     }
     checksum_1 = ~checksum ;
     write (checksum_1) ;
     write (PN532_POSTAMBLE) ;
// Deselect the PN532 shield
     digitalWrite (_bPinCS, HIGH) ;

#ifdef PN532DEBUG
     Serial.println ("\n  Command Postamble") ;
     Serial.print ("   ") ;
     Serial.print (" 0x") ;
     Serial.print (checksum_1, HEX) ;
     Serial.print (" 0x") ;
     Serial.print (PN532_POSTAMBLE, HEX) ;
     Serial.println () ;
#endif
} 

//-----------------------------------------------------------------------------
//                                                                       .write
//-----------------------------------------------------------------------------
/*
 *   write() transmits a byte to the PN532 shield through the SPI interface.
 */
inline void PN532::write (BYTE bData)
{
     pn532_SPI.transfer (bData) ;
}
