//*****************************************************************************
//
//   DS1307 Real Time Clock (RTC)
//   RTC library for arduino                                              RTC.h
//
//*****************************************************************************
/*
 *   This library allows you to communicate with the DS1307 Real Time Clock 
 *   (RTC) Breakout board by Adafruit.com. Communication is via the 
 *   Inter-Integrated Circuit (I2C) or Two Wire Interface (TWI) features
 *   built into most Arduino boards.
 */

//*****************************************************************************
//                                                              Operation Notes
//*****************************************************************************
/*
 *   The following table shows where Two Wire Interface (TWI) pins are located 
 *   on various Arduino boards:
 *
 *        Board	     I2C / TWI pins
 *        =============  ==================
 *        Uno, Ethernet	A4 (SDA), A5 (SCL)
 *        Mega2560	     20 (SDA), 21 (SCL)
 *        Leonardo	     2  (SDA),  3 (SCL)
 *        Due	          20 (SDA), 21 (SCL)
 *
 *   On Arduino boards with the R3 layout (1.0 pinout), the SDA (System Data)
 *   and SCL (System Clock) lines are on the pin headers close to the AREF pin. 
 *   The Arduino Due has two I2C / TWI interfaces SDA1 and SCL1 are near 
 *   to the AREF pin and the additional one is on pins 20 and 21.
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Original code developed and released to the public domain by JeeLabs 
 *   http://news.jeelabs.org/code/
 *
 *   Copyright (C) 2013, Programify Ltd.
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   27-07-13  Improved source code format.
 */

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Include Libraries
#if (ARDUINO >= 100)
#    include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems
#else
#    include <WProgram.h>
#endif

#ifdef __AVR__
#    include <avr/pgmspace.h>
#endif

#include <Wire.h>
#include <RTC.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants
#define DS1307_ADDRESS             0x68
#define SECONDS_PER_DAY            86400L
#define SECONDS_FROM_1970_TO_2000  946684800

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Macros
#ifdef __AVR__
#    define WIRE Wire
#else
#    define PROGMEM
#    define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#    define WIRE Wire1
#endif

//-----------------------------------------------------------------------------
//                                                                  Global Data
//-----------------------------------------------------------------------------
// Utility code, some of this could be exposed in the DateTime API if needed
BYTE daysInMonth [] = { 31,28,31,30,31,30,31,31,30,31,30,31 } ;

//-----------------------------------------------------------------------------
//                                                                    Functions
//-----------------------------------------------------------------------------
static BYTE      bcd2bin        (BYTE val) ;
static BYTE      bin2bcd        (BYTE val) ;
static BYTE      conv2d         (const char * p) ;
static WORD      date2days      (WORD y, BYTE m, BYTE d) ;
static long      time2long      (WORD days, BYTE h, BYTE m, BYTE s) ;


//=============================================================================
//   Constructor                                                       DateTime
//-----------------------------------------------------------------------------
/*
 *   DateTime implementation - ignores time zones and DST changes and leap
 *   seconds, see http://en.wikipedia.org/wiki/Leap_second.
 */
DateTime::DateTime (DWORD t)
{
     BYTE      daysPerMonth ;
     WORD      leap ;
     WORD      days ;

// Bring to 2000 timestamp from 1970
     t -= SECONDS_FROM_1970_TO_2000 ;

     ss   = t % 60 ;
     t   /= 60 ;
     mm   = t % 60 ;
     t   /= 60 ;
     hh   = t % 24 ;
     days = t / 24 ;
// Adjust days in preceeding years for leap years
     for (yOff = 0 ; ; yOff ++)
     {
          leap = ((yOff % 4) == 0) ;
          if (days < (365 + leap))
               break ;
          days -= 365 + leap ;
     }
// Adjust days in current year for leap years
     for (m = 1 ; ; m ++)
     {
          daysPerMonth = pgm_read_byte (daysInMonth + m - 1) ;
          if (leap && (m == 2))
               daysPerMonth ++ ;
          if (days < daysPerMonth)
               break ;
          days -= daysPerMonth ;
     }
     d = days + 1 ;
}

//=============================================================================
//   Constructor                                                       DateTime
//-----------------------------------------------------------------------------
DateTime::DateTime (WORD year, BYTE month, BYTE day, BYTE hour, BYTE min, BYTE sec)
{
     if (year >= 2000)
          year -= 2000 ;
     yOff = year ;
     m    = month ;
     d    = day ;
     hh   = hour ;
     mm   = min ;
     ss   = sec ;
}

//=============================================================================
//   Constructor                                                       DateTime
//-----------------------------------------------------------------------------
/*
 *   A convenient constructor for using "the compiler's time":
 *   DateTime now (__DATE__, __TIME__);
 *   NOTE: Using PSTR would further reduce the RAM footprint
 */
DateTime::DateTime (const char* date, const char* time)
{
// sample input: date = "Dec 26 2009", time = "12:34:56"
     yOff = conv2d(date + 9);
// Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
     switch (date [0])
     {
          case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break ;
          case 'F': m =  2 ; break ;
          case 'A': m = date[2] == 'r' ? 4 : 8; break ;
          case 'M': m = date[2] == 'r' ? 3 : 5; break ;
          case 'S': m =  9 ; break ;
          case 'O': m = 10 ; break ;
          case 'N': m = 11 ; break ;
          case 'D': m = 12 ; break ;
     }
     d  = conv2d (date + 4) ;
     hh = conv2d (time) ;
     mm = conv2d (time + 3) ;
     ss = conv2d (time + 6) ;
}

//-----------------------------------------------------------------------------
//                                                                   .dayOfWeek
//-----------------------------------------------------------------------------
BYTE DateTime::dayOfWeek () const
{
     WORD      day ;

     day = date2days (yOff, m, d) ;
// Jan 1, 2000 is a Saturday, i.e. returns 6
     return (day + 6) % 7 ;
}

//-----------------------------------------------------------------------------
//                                                                    .unixtime
//-----------------------------------------------------------------------------
DWORD DateTime::unixtime(void) const
{
     DWORD     t ;
     WORD      days ;

     days = date2days (yOff, m, d) ;
     t    = time2long (days, hh, mm, ss) ;
// seconds from 1970 to 2000
     t   += SECONDS_FROM_1970_TO_2000 ;

     return t ;
}

//-----------------------------------------------------------------------------
//                                                                       .begin
//-----------------------------------------------------------------------------
BYTE RTC_DS1307::begin(void)
{
     return 1 ;
}

#if (ARDUINO >= 100)

//-----------------------------------------------------------------------------
//                                                                   .isrunning
//-----------------------------------------------------------------------------
BYTE RTC_DS1307::isrunning (void)
{
     BYTE      ss ;

     WIRE.beginTransmission (DS1307_ADDRESS) ;
     WIRE.write (0) ;
     WIRE.endTransmission () ;

     WIRE.requestFrom (DS1307_ADDRESS, 1) ;
     ss = WIRE.read () ;
     return (! (ss >> 7)) ;
}

//-----------------------------------------------------------------------------
//                                                                      .adjust
//-----------------------------------------------------------------------------
void RTC_DS1307::adjust (const DateTime& dt)
{
     WIRE.beginTransmission (DS1307_ADDRESS) ;
     WIRE.write (0) ;
     WIRE.write (bin2bcd (dt.second ())) ;
     WIRE.write (bin2bcd (dt.minute ())) ;
     WIRE.write (bin2bcd (dt.hour ())) ;
     WIRE.write (bin2bcd (0)) ;
     WIRE.write (bin2bcd (dt.day ())) ;
     WIRE.write (bin2bcd (dt.month ())) ;
     WIRE.write (bin2bcd (dt.year () - 2000)) ;
     WIRE.write (0) ;
     WIRE.endTransmission () ;
}

//-----------------------------------------------------------------------------
//                                                                         .now
//-----------------------------------------------------------------------------
DateTime RTC_DS1307::now ()
{
  WIRE.beginTransmission (DS1307_ADDRESS) ;
  WIRE.write (0) ;
  WIRE.endTransmission () ;

  WIRE.requestFrom (DS1307_ADDRESS, 7) ;
  BYTE ss = bcd2bin (WIRE.read () & 0x7F) ;
  BYTE mm = bcd2bin (WIRE.read ()) ;
  BYTE hh = bcd2bin (WIRE.read ()) ;
  WIRE.read () ;
  BYTE d = bcd2bin (WIRE.read ()) ;
  BYTE m = bcd2bin (WIRE.read ()) ;
  WORD y = bcd2bin (WIRE.read ()) + 2000 ;
  
  return DateTime (y, m, d, hh, mm, ss) ;
}

#else

//-----------------------------------------------------------------------------
//                                                                   .isrunning
//-----------------------------------------------------------------------------
BYTE RTC_DS1307::isrunning (void)
{
     BYTE      ss ;

     WIRE.beginTransmission (DS1307_ADDRESS) ;
     WIRE.send (0) ;
     WIRE.endTransmission () ;

     WIRE.requestFrom (DS1307_ADDRESS, 1) ;
     ss = WIRE.receive () ;
     return (! (ss >> 7)) ;
}

//-----------------------------------------------------------------------------
//                                                                      .adjust
//-----------------------------------------------------------------------------
void RTC_DS1307::adjust (const DateTime & dt)
{
    WIRE.beginTransmission (DS1307_ADDRESS) ;
    WIRE.send (0) ;
    WIRE.send (bin2bcd (dt.second ())) ;
    WIRE.send (bin2bcd (dt.minute ())) ;
    WIRE.send (bin2bcd (dt.hour ())) ;
    WIRE.send (bin2bcd (0)) ;
    WIRE.send (bin2bcd (dt.day ())) ;
    WIRE.send (bin2bcd (dt.month ())) ;
    WIRE.send (bin2bcd (dt.year () - 2000)) ;
    WIRE.send (0) ;
    WIRE.endTransmission () ;
}

//-----------------------------------------------------------------------------
//                                                                         .now
//-----------------------------------------------------------------------------
DateTime RTC_DS1307::now ()
{
     WIRE.beginTransmission(DS1307_ADDRESS) ;
     WIRE.send(0) ;
     WIRE.endTransmission() ;

     WIRE.requestFrom (DS1307_ADDRESS, 7) ;
     BYTE ss = bcd2bin (WIRE.receive () & 0x7F) ;
     BYTE mm = bcd2bin (WIRE.receive ()) ;
     BYTE hh = bcd2bin (WIRE.receive ()) ;
     WIRE.receive () ;
     BYTE d = bcd2bin (WIRE.receive ()) ;
     BYTE m = bcd2bin (WIRE.receive ()) ;
     WORD y = bcd2bin (WIRE.receive ()) + 2000 ;

     return DateTime (y, m, d, hh, mm, ss) ;
}

#endif

// Init protected data
long RTC_Millis::offset = 0 ;

//-----------------------------------------------------------------------------
//                                                                      .adjust
//-----------------------------------------------------------------------------
void RTC_Millis::adjust (const DateTime & dt)
{
     offset = dt.unixtime () - millis () / 1000 ;
}

//-----------------------------------------------------------------------------
//                                                                         .now
//-----------------------------------------------------------------------------
DateTime RTC_Millis::now ()
{
     return (DWORD)(offset + millis () / 1000) ;
}

//-----------------------------------------------------------------------------
//                                                                   .date2days
//-----------------------------------------------------------------------------
/*
 *   date2days() returns the number of days since 2000/01/01, valid for 
 *   2001..2099.
 */
static WORD date2days (WORD y, BYTE m, BYTE d)
{
     BYTE      i ;
     WORD      days ;

     if (y >= 2000)
          y -= 2000 ;
     days = d ;
     for (i = 1 ; i < m ; i ++)
          days += pgm_read_byte (daysInMonth + i - 1) ;
     if ((m > 2) && ((y % 4) == 0))
          days ++ ;
     return (days + 365 * y + (y + 3) / 4 - 1) ;
}

//-----------------------------------------------------------------------------
//                                                                   .time2long
//-----------------------------------------------------------------------------
static long time2long (WORD days, BYTE h, BYTE m, BYTE s)
{
     return ((days * 24L + h) * 60 + m) * 60 + s ;
}

//-----------------------------------------------------------------------------
//                                                                      .conv2d
//-----------------------------------------------------------------------------
static BYTE conv2d (const char* p)
{
     BYTE v = 0 ;

     if (('0' <= *p) && (*p <= '9'))
          v = *p - '0' ;
     return (10 * v + *++p - '0') ;
}

//-----------------------------------------------------------------------------
//                                                                     .bcd2bin
//-----------------------------------------------------------------------------
static BYTE bcd2bin (BYTE val)
{
     return val - 6 * (val >> 4) ;
}

//-----------------------------------------------------------------------------
//                                                                     .bin2bcd
//-----------------------------------------------------------------------------
static BYTE bin2bcd (BYTE val)
{
     return val + 6 * (val / 10) ;
}
