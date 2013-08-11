//*****************************************************************************
//
//   TWI/I2C library for Wiring & Arduino
//   Wire master library for arduino.                                    Wire.h
//
//*****************************************************************************
/*
 *   Two Wire Interface (TWI) and Intra-Integrated Ciruit (I2C).
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
 *
 *   This library is free software; you can redistribute it and/or modify it 
 *   under the terms of the GNU Lesser General Public License as published by 
 *   the Free Software Foundation; either version 2.1 of the License, or (at 
 *   your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
 *   License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License 
 *   along with this library; if not, write to the Free Software Foundation, 
 *   Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   ??-??-12  Modified by Todd Krein (todd@krein.org) to implement repeated 
 *             starts.
 *   06-08-13  Reformat source code. Programify Ltd.
 */

//-----------------------------------------------------------------------------
//                                        C O M P I L E R   D I R E C T I V E S
//-----------------------------------------------------------------------------
#ifndef TWOWIRE_H
#define TWOWIRE_H

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Include Libraries
#include <Arduino.h>
#include <inttypes.h>
#include "Stream.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Compile-Time Options

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants
#define BUFFER_LENGTH 32

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

//-----------------------------------------------------------------------------
//   Derived Class Declaration                                          TwoWire
//-----------------------------------------------------------------------------
class TwoWire : public Stream
{
public:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Constructors
               TwoWire             () ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Public Functions
     void      begin               () ;
     void      begin               (uint8_t) ;
     void      begin               (int) ;
     void      beginTransmission   (uint8_t) ;
     void      beginTransmission   (int) ;
     uint8_t   endTransmission     (void) ;
     uint8_t   endTransmission     (uint8_t) ;
     uint8_t   requestFrom         (uint8_t, uint8_t) ;
     uint8_t   requestFrom         (uint8_t, uint8_t, uint8_t) ;
     uint8_t   requestFrom         (int, int) ;
     uint8_t   requestFrom         (int, int, int) ;
     void      onReceive           (void (*)(int)) ;
     void      onRequest           (void (*)(void)) ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Virtual Functions
     virtual size_t write          (uint8_t) ;
     virtual size_t write          (const uint8_t *, size_t) ;
     virtual int    available      (void) ;
     virtual int    read           (void) ;
     virtual int    peek           (void) ;
     virtual void   flush          (void) ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Inline Wrappers
     inline size_t write (unsigned long n)   { return write ((uint8_t) n) ; }
     inline size_t write (long n)            { return write ((uint8_t) n) ; }
     inline size_t write (unsigned int n)    { return write ((uint8_t) n) ; }
     inline size_t write (int n)             { return write ((uint8_t) n) ; }

     using Print::write ;

private:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Private Functions
     static void (*user_onRequest)(void) ;
     static void (*user_onReceive)(int) ;
     static void onRequestService (void) ;
     static void onReceiveService (uint8_t *, int) ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Private Data
     static uint8_t rxBuffer [] ;
     static uint8_t rxBufferIndex ;
     static uint8_t rxBufferLength ;

     static uint8_t txAddress ;
     static uint8_t txBuffer [] ;
     static uint8_t txBufferIndex ;
     static uint8_t txBufferLength ;
     static uint8_t transmitting ;
} ;

extern TwoWire Wire ;

#endif

