//*****************************************************************************
//
//   PN532 library by adafruit/ladyada
//   PN532 library header                                               PN532.h
//
//*****************************************************************************

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   MIT license
 *   authenticateBlock, readMemoryBlock, writeMemoryBlock contributed
 *   by Seeed Technology Inc (www.seeedstudio.com)
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   11-07-13  Improved source code format.
 */

#ifndef __PN532_H__
#define __PN532_H__

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------
#include <Arduino.h>
#include <SPI.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Compile-time Options
/*
 *   PN532_P2P_DEBUG     All the data that initiator and target received
 *                       in the peer to peer communication, will be printed 
 *                       in the serial port tool window.
 *
 *   PN532DEBUG          Display all data transmitted and received - including
 *                       when idle.
 */
///#define   PN532_P2P_DEBUG       1
///#define   PN532DEBUG            1
#define   PN532_DEBUG_VERBOSE   1

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants
// Command codes
#define   PN532_CMD_DIAGNOSE                 0x00
#define   PN532_CMD_FIRMWAREVERSION          0x02
#define   PN532_CMD_GETGENERALSTATUS         0x04
#define   PN532_CMD_INATR                    0x50
#define   PN532_CMD_INAUTOPOLL               0x60
#define   PN532_CMD_INCOMMUNICATETHRU        0x42
#define   PN532_CMD_INDATAEXCHANGE           0x40
#define   PN532_CMD_INDESELECT               0x44
#define   PN532_CMD_INJUMPFORDEP             0x56
#define   PN532_CMD_INJUMPFORPSL             0x46
#define   PN532_CMD_INLISTPASSIVETARGET      0x4A
#define   PN532_CMD_INPSL                    0x4E
#define   PN532_CMD_INRELEASE                0x52
#define   PN532_CMD_INSELECT                 0x54
#define   PN532_CMD_POWERDOWN                0x16
#define   PN532_CMD_READGPIO                 0x0C
#define   PN532_CMD_READREGISTER             0x06
#define   PN532_CMD_RFCONFIGURATION          0x32
#define   PN532_CMD_RFREGULATIONTEST         0x58
#define   PN532_CMD_SAMCONFIGURATION         0x14
#define   PN532_CMD_SETSERIALBAUDRATE        0x10
#define   PN532_CMD_SETPARAMETERS            0x12
#define   PN532_CMD_TGGETDATA                0x86
#define   PN532_CMD_TGGETINITIATORCOMMAND    0x88
#define   PN532_CMD_TGGETTARGETSTATUS        0x8A
#define   PN532_CMD_TGINITASTARGET           0x8C
#define   PN532_CMD_TGRESPONSETOINITIATOR    0x90
#define   PN532_CMD_TGSETDATA                0x8E
#define   PN532_CMD_TGSETGENERALBYTES        0x92
#define   PN532_CMD_TGSETMETADATA            0x94
#define   PN532_CMD_WAKEUP                   0x55
#define   PN532_CMD_WRITEGPIO                0x0E
#define   PN532_CMD_WRITEREGISTER            0x08

// Command acknowledgement codes
#define   PN532_ACK_INDATAEXCHANGE           0x41
#define   PN532_ACK_INLISTPASSIVETARGET      0x4B
#define   PN532_ACK_SAMCONFIGURATION         0x15
#define   PN532_ACK_TGGETDATA                0x87
#define   PN532_ACK_TGSETDATA                0x8F

// SAM (Security Access Module) modes or PN532/SAM configurations
#define   PN532_SAM_MODENORMAL               0x01 // Normal mode: SAM is not used (by default)
#define   PN532_SAM_MODEVIRTUALCARD          0x02 // PN532 & SAM are visible to outside world as a single contactless SAM card
#define   PN532_SAM_MODEWIREDCARD            0x03 // The host controller can access the SAM with standard PCD commands
#define   PN532_SAM_MODEDUALCARD             0x04 // PN532 & SAM are visible to outside world as two separate targets

// Command options
#define   PN532_AUTH_WITH_KEYA               0x60
#define   PN532_AUTH_WITH_KEYB               0x61
#define   PN532_AUTH_KEYA                    1
#define   PN532_AUTH_KEYB                    2

// Command constants
#define   PN532_BAUDRATE_201K                1
#define   PN532_BAUDRATE_424K                2
#define   PN532_HOSTTOPN532                  0xD4
#define   PN532_POSTAMBLE                    0x00
#define   PN532_PREAMBLE                     0x00
#define   PN532_STARTCODE1                   0x00
#define   PN532_STARTCODE2                   0xFF

// Mifare Card data exchange command set (partial) - (Classic & Ultralight)
#define   PN532_MIFARE_AUTHENTICATEA         0x60
#define   PN532_MIFARE_AUTHENTICATEB         0x61
#define   PN532_MIFARE_DECREMENTATION        0xC0
#define   PN532_MIFARE_INCREMENTATION        0xC1
#define   PN532_MIFARE_READPAGE              0x30
#define   PN532_MIFARE_RESTORE               0xC2
#define   PN532_MIFARE_TRANSFER              0xB0
#define   PN532_MIFARE_WRITEPAGE             0xA0
#define   PN532_MIFARE_WRITEDWORD            0xA2

// SPI commands
#define   PN532_SPI_DATAREAD                 0x03
#define   PN532_SPI_DATAWRITE                0x01
#define   PN532_SPI_READY                    0x01
#define   PN532_SPI_STATREAD                 0x02

// Protocol/baud rates
#define   PN532_106KBPS_ISO14443A            0x00      // 106 kbps type A (ISO/IEC14443 Type A)
#define   PN532_106KBPS_ISO14443B            0x03      // 106 kbps type B (ISO/IEC14443-3B)
#define   PN532_106KBPS_JEWEL                0x04      // 106 kbps Innovision Jewel tag
#define   PN532_212KBPS_FELICA               0x01      // 212 kbps (FeliCa polling)
#define   PN532_424KBPS_FELICA               0x02      // 424 kbps (FeliCa polling)

// Tag family codes
#define   PN532_SHIELD                       0x10000000
#define   PN532_TAG_MIFARE_ISO14443A         0x10000001
#define   PN532_TAG_NTAG203                  0x10000002


//-----------------------------------------------------------------------------
//                                                            Class Declaration
//-----------------------------------------------------------------------------
/*
 *   PN532 common parameters:
 *
 *        bCardNum       1 or 2
 *        cid            Card NUID
 *        blockaddress   0 to 63
 *        authtype       Either PN532_AUTH_KEYA or PN532_AUTH_KEYB
 *        baudrate       0=106 kbits/s (not supported), 1=212 k  bits/s, 2=424 kbits/s
 */
class PN532
{
public:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Constructor
               PN532                    (BYTE bChipSelect) ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Public functions
     DWORD     AuthenticateBlock        (BYTE bCardNum, DWORD cid, BYTE blockaddress, BYTE authtype, BYTE * bpKeys) ;
     DWORD     ConfigurePeerAsInitiator (BYTE baudrate) ;
     DWORD     ConfigurePeerAsTarget    (void) ;
     DWORD     GetFirmwareVersion       (void) ;
     boolean   InitiatorTxRx            (IN char * cpDataTx, OUT char * cpDataRx) ;
     boolean   ReadMemoryBlock          (BYTE bCardNum, BYTE blockaddress, BYTE * bpBlock) ;
     QWORD     ReadPassiveTargetID      (void) ;
     boolean   SAMConfig                (void) ;
     boolean   SendCommandCheckAck      (BYTE * abCommand, BYTE bLength, WORD wTimeout = 1000) ;
     DWORD     SetTargetType            (DWORD dwTagType) ;			
     DWORD     TargetTxRx               (IN char * cpDataTx, OUT char * cpDataIn) ;
     boolean   WriteMemoryBlock         (BYTE bCardNum, BYTE blockaddress, BYTE * bpBlock) ;

/*
 *   _bBaudRate     The baud rate and the modulation type to be used 
 *                  during the initialization process and passed to 
 *                  the PN532 NFC Shield.
 *
 *                       0 : 106 kbps Type A (ISO/IEC14443 Type A)
 *                       1 : 212 kbps (FeliCa polling)
 *                       2 : 424 kbps (FeliCa polling)
 *                       3 : 106 kbps Type B (ISO/IEC14443-3B)
 *                       4 : 106 kbps Innovision Jewel tag
 */
     BYTE      _bBaudRate ;
/*
 *   _bMaxTg        The maximum number of concurrent targets to be 
 *                  initialized by the PN532. The PN532 is capable 
 *                  of handling up to two targets at the same time.
 *
 *                  This value should be set to 1 or 2. If any other
 *                  number is supplied, then the value 1 is assumed.
 *                  For Jewel Cards, the maximum value is 1.
 */
     BYTE      _bMaxTg ;

     DWORD     _dwTagType ;        // Value in PN532_TAG_ series, initialized by .Begin()

     WORD      _wBlocks ;          // Number of blocks of memory on the tag
     WORD      _wBlockSize ;       // Number of bytes per block
     WORD      _wMemSize ;         // Amount of memory required to contain all bytes and blocks (limited to 4K)
     WORD      _wPages ;           // Number of pages of memory on the tag (calculated at run-time)
     WORD      _wPageSize ;        // Number of bytes per page (pages can be bigger than blocks, defaults to 16)

private:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Private functions
     BOOL      checkSpiAck         (void) ;
     void      read                (BYTE * buff, int iLength) ;
     BYTE      readSpiStatus       (void) ;
     void      write               (BYTE _data) ;
     void      writeCommand        (BYTE * cmd, BYTE cmdlen) ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Private data
     BYTE      _bPinCS ;           // Arduino's pin # used as the PN532's Chip Select pin

//- - - - - - - - - - - - - - - - - - - - - - - -  Private composite data items
     SPIClass  pn532_SPI ;
} ;

#endif

