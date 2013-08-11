//*****************************************************************************
//
//   Real Time Clock
//   RTC library for arduino                                              RTC.h
//
//*****************************************************************************

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Original code developed and released to the public domain by JeeLabs 
 *   http://news.jeelabs.org/code/
 *
 *   Copyright (C) 2013, Programify Ltd. For improvements to source code
 *   format.
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
#ifndef _RTCLIB_H_
#define _RTCLIB_H_

#include <Arduino.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants

//-----------------------------------------------------------------------------
//   Class Declaration                                                 DateTime
//-----------------------------------------------------------------------------
/*
 *   DateTime is a simple general-purpose date/time class (with no TZ / DST / 
 *   leap second handling!)
 */
class DateTime
{
public:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Constructors
               DateTime       (uint32_t t = 0) ;
               DateTime       (uint16_t year, BYTE month, BYTE day, BYTE hour = 0, BYTE min = 0, BYTE sec = 0) ;
               DateTime       (const char * date, const char * time) ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Public Functions
     WORD      year           () const { return 2000 + yOff ; }
     BYTE      month          () const { return m ; }
     BYTE      day            () const { return d ; }
     BYTE      hour           () const { return hh ; }
     BYTE      minute         () const { return mm ; }
     BYTE      second         () const { return ss ; }
     BYTE      dayOfWeek      () const ;
     DWORD     unixtime       (void) const ; // 32-bit times as seconds since 1/1/1970
     long      secondstime    () const ;     // 32-bit times as seconds since 1/1/2000

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Protected Data
protected:
     BYTE      d ;
     BYTE      hh ;
     BYTE      m ;
     BYTE      mm ;
     BYTE      ss ;
     BYTE      yOff ;
} ;

//-----------------------------------------------------------------------------
//   Class Declaration                                               RTC_DS1307
//-----------------------------------------------------------------------------
/*
 *   RTC based on the DS1307 chip connected via I2C and the Wire library.
 */
class RTC_DS1307
{
public:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Constructors

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Public Functions
     static void         adjust    (const DateTime & dt) ;
     static BYTE         begin     (void) ;
            BYTE         isrunning (void) ;
     static DateTime     now       () ;
} ;

//-----------------------------------------------------------------------------
//   Class Declaration                                               RTC_Millis
//-----------------------------------------------------------------------------
/*
 *   RTC using the internal millis() clock, has to be initialized before use.
 *   This clock won't be correct once the millis() timer rolls over (>49d?)
 */
class RTC_Millis
{
public:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Constructors

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Public Functions
     static void         adjust    (const DateTime & dt) ;
     static void         begin     (const DateTime & dt) { adjust (dt) ; }
     static DateTime     now       () ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Protected Data
protected:
     static long    offset ;
} ;

#endif
