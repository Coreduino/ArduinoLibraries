//*****************************************************************************
//
//   Serial Peripheral Interface
//   SPI master library for arduino.                                      SPI.h
//
//*****************************************************************************

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 *
 *   This file is free software; you can redistribute it and/or modify
 *   it under the terms of either the GNU General Public License version 2
 *   or the GNU Lesser General Public License version 2.1, both as
 *   published by the Free Software Foundation.
 *
 *   Copyright (C) 2013 by Programify Ltd. Copyright in regard to code format 
 *   and extensions to the functionality of this class.
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   11-07-13  Improved source code format.
 */

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------
#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>
#include <avr/pgmspace.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants
#define   SPI_CLOCK_DIV4      0x00
#define   SPI_CLOCK_DIV16     0x01
#define   SPI_CLOCK_DIV64     0x02
#define   SPI_CLOCK_DIV128    0x03
#define   SPI_CLOCK_DIV2      0x04
#define   SPI_CLOCK_DIV8      0x05
#define   SPI_CLOCK_DIV32     0x06
//#define   SPI_CLOCK_DIV64     0x07

#define   SPI_MODE0           0x00
#define   SPI_MODE1           0x04
#define   SPI_MODE2           0x08
#define   SPI_MODE3           0x0C

#define   SPI_MODE_MASK       0x0C // CPOL = bit 3, CPHA = bit 2 on SPCR
#define   SPI_CLOCK_MASK      0x03 // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define   SPI_2XCLOCK_MASK    0x01 // SPI2X = bit 0 on SPSR

//-----------------------------------------------------------------------------
//                                                            Class Declaration
//-----------------------------------------------------------------------------
class SPIClass
{
public:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Constructors

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Public Functions
     inline static byte transfer (byte _data) ;

// SPI Configuration methods
     inline static void attachInterrupt () ;
     inline static void detachInterrupt () ; // Default

     static void begin () ; // Default
     static void end   () ;

     static void setBitOrder       (uint8_t) ;
     static void setDataMode       (uint8_t) ;
     static void setClockDivider   (uint8_t) ;
} ;

//=============================================================================
//                                                                    .transfer
//-----------------------------------------------------------------------------
byte SPIClass::transfer (byte _data)
{
     SPDR = _data ;
     while (! (SPSR & _BV (SPIF)))
          ;
     return SPDR ;
}

//=============================================================================
//                                                             .attachInterrupt
//-----------------------------------------------------------------------------
void SPIClass::attachInterrupt ()
{
     SPCR |= _BV (SPIE) ;
}

//=============================================================================
//                                                             .detachInterrupt
//-----------------------------------------------------------------------------
void SPIClass::detachInterrupt ()
{
     SPCR &= ~_BV (SPIE) ;
}

extern SPIClass SPI ;

#endif
