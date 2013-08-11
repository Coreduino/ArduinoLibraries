//*****************************************************************************
//
//   CardInfo
//   Display SD Card Information                                   CardInfo.ino
//
//*****************************************************************************
/*
 *   This example shows how use the utility libraries on which the SD library 
 *   is based in order to get info about your SD card. Use this to test an SD
 *   card when you're not sure whether its working or not.
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   28-03-11  Started development by Limor Fried.
 *   09-04-12  Modified by Tom Igoe.
 *   02-08-13  Upgraded code format. Programify Ltd.
 */

//*****************************************************************************
//                                                               Hardware Notes
//*****************************************************************************
/*
 *   On the Ethernet Shield, CS is pin 4. It's set as an output by default.
 *   Note that even if it's not used as the CS pin, the hardware SS pin (10 on 
 *   most Arduino boards, 53 on the Mega) must be left as an output or the SD 
 *   library functions will not work. 
 *
 *   The circuit: SD card attached to SPI bus as follows:
 *
 *        MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 *        MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 *        CLK  - pin 13 on Arduino Uno/Duemilanove/Diecimila
 *        CS   - depends on your SD card shield or module. 
 */

//-----------------------------------------------------------------------------
//                                        C O M P I L E R   D I R E C T I V E S
//-----------------------------------------------------------------------------
#include <SD.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Compile-Time Options
/*
 *   Change PIN_CHIPSELECT to match your SD shield or module.
 *
 *     10  Adafruit SD shields and modules
 *      4  Arduino Ethernet shield
 *      8  Sparkfun SD shield
 */
#define   PIN_CHIPSELECT      10

/*
 *   Values for PIN_SS:
 *
 *     10  Default - most boards
 *     53  Mega
 */
#define   PIN_SS              10

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Constants

//-----------------------------------------------------------------------------
//                                                          S T R U C T U R E S
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                                                            F U N C T I O N S
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                                                        G L O B A L   D A T A
//-----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - - - Composite Data Structures
Sd2Card   card ;
SdVolume  volume ;
SdFile    root ;


//=============================================================================
//                                                                        setup
//-----------------------------------------------------------------------------
void setup()
{
     uint32_t volumesize ;

     Serial.println ("Card Information Demo") ;
// Wait for developer to open Serial Monitor window
     delay (3000) ;
// Open serial communications and wait for port to open:
     Serial.begin (9600) ;
// Wait for serial port to connect. Needed for Leonardo only
     while (! Serial) ;
// Report action
     Serial.println ("Initializing SD card...") ;
     pinMode (PIN_SS, OUTPUT) ;
// Use the initialization code from utility libraries (just testing if the card is working)
     if (! card.init (SPI_HALF_SPEED, PIN_CHIPSELECT))
     {
          Serial.println ("FAILED TO INITIALIZE. Things to check:") ;
          Serial.println ("* is a card is inserted?") ;
          Serial.println ("* Is your wiring correct?") ;
          Serial.println ("* did you change the PIN_CHIPSELECT pin to match your shield or module?") ;
          return ;
     }
// Continue with report
     Serial.println ("Wiring is correct and a card is present.") ;
// print the type of card
     Serial.print ("\nCard type: ") ;
     switch (card.type ())
     {
          case SD_CARD_TYPE_SD1 :
               Serial.println ("SD1") ;
               break ;
          case SD_CARD_TYPE_SD2 :
               Serial.println ("SD2") ;
               break ;
          case SD_CARD_TYPE_SDHC :
               Serial.println ("SDHC") ;
               break ;
          default :
               Serial.println ("Unknown") ;
     }
// Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
     if (! volume.init (card))
     {
          Serial.println ("* FAILED TO FIND FAT16 OR FAT32 PARTITION.") ;
          Serial.println ("* Make sure the card has been formatted.") ;
          return ;
     }
// Report the type and size of the first FAT-type volume
     Serial.println () ;
     Serial.print   ("Volume type is FAT") ;
     Serial.println (volume.fatType (), DEC) ;
     Serial.println () ;
// Report volume size in various measures
     volumesize  = volume.blocksPerCluster () ;   // Clusters are collections of blocks
     volumesize *= volume.clusterCount () ;       // we'll have a lot of clusters
     volumesize *= 512 ;                          // SD card blocks are always 512 bytes
     Serial.print   ("Volume size (bytes): ") ;
     Serial.println (volumesize) ;
     Serial.print   ("Volume size (Kbytes): ") ;
     volumesize /= 1024 ;
     Serial.println (volumesize) ;
     Serial.print ("Volume size (Mbytes): ") ;
     volumesize /= 1024 ;
     Serial.println (volumesize) ;
// Report files found on the volume
     Serial.println () ;
     Serial.println ("Files found on the card (name, date and size in bytes):") ;
// List all files in the card with date and size
     root.openRoot (volume) ;
     root.ls (LS_R | LS_DATE | LS_SIZE) ;
}

//=============================================================================
//                                                                         loop
//-----------------------------------------------------------------------------
void loop(void)
{
}
