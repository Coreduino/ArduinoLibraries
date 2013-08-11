//*****************************************************************************
//
//   DS1307
//   Display date and time retrieved from DS1307 board               DS1307.ino
//
//*****************************************************************************
/*
 *   Demonstrates date and time functions using a DS1307 RTC connected via 
 *   I2C and Wire lib.
 */

//*****************************************************************************
//                                                               Hardware Notes
//*****************************************************************************
/*
 *   This module is designed to work with Adafruit's DS1307 breakout board.
 *   The breakout board must be connected to the following Arduino pins:
 *
 *   DS1307  Arduino
 *   ======  =======
 *   [GND]   [GND]
 *
 *   [5v]    [5v]
 *
 *   [SDA]   [A4]
 *
 *   [SCL]   [A5]
 *
 *   [SQW]   [A3]
 */

//-----------------------------------------------------------------------------
//                                        C O M P I L E R   D I R E C T I V E S
//-----------------------------------------------------------------------------
#include <RTC.h>
#include <Wire.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Compile-Time Options

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
RTC_DS1307 rtc ;


//=============================================================================
//                                                                        setup
//-----------------------------------------------------------------------------
void setup ()
{
// Init serial comms
     Serial.begin (9600) ;
// Wait for serial port to connect (as required by Leonardo)
     while (! Serial) ;
#ifdef AVR
     Wire.begin () ;
#else
// Shield I2C pins connect to alt I2C bus on Arduino Due
     Wire1.begin () ;
#endif
     rtc.begin () ;
     if (! rtc.isrunning ())
     {
          Serial.println("* RTC is NOT running!");
     // Set the RTC to the date & time this sketch was compiled
          rtc.adjust (DateTime(__DATE__, __TIME__)) ;
          ///rtc.adjust (DateTime ("Jul 27 2013", "20:57:30")) ;
     }
}

//=============================================================================
//                                                                         loop
//-----------------------------------------------------------------------------
void loop ()
{
     char      szDate [12] ;
     char      szTime [12] ;

     DateTime now = rtc.now () ;

// Construct date and time strings
     sprintf (szDate, "%02d-%02d-%04d", now.day (), now.month (), now.year ()) ;
     sprintf (szTime, "%02d:%02d:%02d", now.hour (), now.minute (), now.second ()) ;
// Display date and time
     Serial.print (szDate) ;
     Serial.print (' ') ;
     Serial.println (szTime) ;
     delay (1000) ;
}
