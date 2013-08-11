//*****************************************************************************
//
//   TWI/I2C library for Wiring & Arduino
//   Wire master library for arduino.                                  Wire.cpp
//
//*****************************************************************************
/*
 *   Two Wire Interface (TWI) and Intra-Integrated Ciruit (I2C).
 */

//*****************************************************************************
//                                                                   Tech Notes
//*****************************************************************************
/*
 *   There are both 7- and 8-bit versions of I2C addresses. 7 bits identify 
 *   the device, and the eighth bit determines if it's being written to or 
 *   read from. The Wire library uses 7 bit addresses throughout. If you have 
 *   a datasheet or sample code that uses 8 bit address, you'll want to drop 
 *   the low bit (i.e. shift the value one bit to the right), yielding an 
 *   address between 0 and 127. 
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
 *
 *        This library is free software; you can redistribute it and/or modify 
 *        it under the terms of the GNU Lesser General Public License as 
 *        published by the Free Software Foundation; either version 2.1 of the 
 *        License, or (at your option) any later version.
 *
 *        This library is distributed in the hope that it will be useful, but 
 *        WITHOUT ANY WARRANTY; without even the implied warranty of 
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *        Lesser General Public License for more details.
 *
 *        You should have received a copy of the GNU Lesser General Public 
 *        License along with this library; if not, write to the Free Software 
 *        Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 *        USA.
 *
 *   Copyright (C) 2013 by Programify Ltd.
 *
 *        Copyright in regard to code format and extensions to the 
 *        functionality of this class.
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   ??-??-12  Enabled repeated starts. Todd Krein (todd@krein.org)
 *   28-07-13  Improved source code format. Programify Ltd.
 */

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Include Libraries
extern "C"
{
     #include <stdlib.h>
     #include <string.h>
     #include <inttypes.h>
     #include "twi.h"
}

#include "Wire.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Compile-Time Options

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Macros

//-----------------------------------------------------------------------------
//                                                                  Global Data
//-----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - -  Initialize Class Variables
BYTE TwoWire::txAddress      = 0 ;
BYTE TwoWire::rxBuffer [BUFFER_LENGTH] ;
BYTE TwoWire::rxBufferIndex  = 0 ;
BYTE TwoWire::rxBufferLength = 0 ;
BYTE TwoWire::txBuffer [BUFFER_LENGTH] ;
BYTE TwoWire::txBufferIndex  = 0 ;
BYTE TwoWire::txBufferLength = 0 ;
BYTE TwoWire::transmitting   = 0 ;

void (*TwoWire::user_onRequest)(void) ;
void (*TwoWire::user_onReceive)(int) ;

//-----------------------------------------------------------------------------
//                                                                    Functions
//-----------------------------------------------------------------------------

//=============================================================================
//   Constructor                                                        TwoWire
//-----------------------------------------------------------------------------
TwoWire::TwoWire ()
{
}

//-----------------------------------------------------------------------------
//                                                                       .begin
//-----------------------------------------------------------------------------
void TwoWire::begin (void)
{
// Reset receive (RX) buffer control
     rxBufferIndex  = 0 ;
     rxBufferLength = 0 ;
// Reset transmit (TX) buffer control
     txBufferIndex  = 0 ;
     txBufferLength = 0 ;
/// Initialize two wire interface ???
     twi_init () ;
}

//-----------------------------------------------------------------------------
//                                                                       .begin
//-----------------------------------------------------------------------------
void TwoWire::begin (BYTE address)
{
     twi_setAddress (address) ;
     twi_attachSlaveTxEvent (onRequestService) ;
     twi_attachSlaveRxEvent (onReceiveService) ;
     begin () ;
}

//-----------------------------------------------------------------------------
//                                                                       .begin
//-----------------------------------------------------------------------------
void TwoWire::begin (int address)
{
     begin ((BYTE) address) ;
}

//-----------------------------------------------------------------------------
//                                                                 .requestFrom
//-----------------------------------------------------------------------------
BYTE TwoWire::requestFrom (BYTE address, BYTE quantity, BYTE sendStop)
{
     BYTE      bRead ;

// Clamp to buffer length
     if (quantity > BUFFER_LENGTH)
          quantity = BUFFER_LENGTH ;
// Perform blocking read into buffer
     bRead = twi_readFrom (address, rxBuffer, quantity, sendStop) ;
// Set RX buffer control
     rxBufferIndex  = 0 ;
     rxBufferLength = bRead ;
     return (bRead) ;
}

//-----------------------------------------------------------------------------
//                                                                 .requestFrom
//-----------------------------------------------------------------------------
BYTE TwoWire::requestFrom (BYTE address, BYTE quantity)
{
     return requestFrom (address, quantity, (BYTE) true) ;
}

//-----------------------------------------------------------------------------
//                                                                 .requestFrom
//-----------------------------------------------------------------------------
BYTE TwoWire::requestFrom (int address, int quantity)
{
     return requestFrom ((BYTE) address, (BYTE) quantity, (BYTE) true) ;
}

//-----------------------------------------------------------------------------
//                                                                 .requestFrom
//-----------------------------------------------------------------------------
BYTE TwoWire::requestFrom (int address, int quantity, int sendStop)
{
     return requestFrom ((BYTE) address, (BYTE) quantity, (BYTE) sendStop) ;
}

//-----------------------------------------------------------------------------
//                                                           .beginTransmission
//-----------------------------------------------------------------------------
void TwoWire::beginTransmission (BYTE address)
{
// Enter transmit mode
     transmitting = 1 ;
// Set address of targeted slave
     txAddress = address ;
// Reset TX buffer control
     txBufferIndex  = 0 ;
     txBufferLength = 0 ;
}

//-----------------------------------------------------------------------------
//                                                           .beginTransmission
//-----------------------------------------------------------------------------
void TwoWire::beginTransmission (int address)
{
     beginTransmission ((BYTE) address) ;
}

//-----------------------------------------------------------------------------
//                                                             .endTransmission
//-----------------------------------------------------------------------------
/*
 *   endTransmission() indicates whether or not a STOP should be performed 
 *   on the bus.  Calling endTransmission(false) allows a sketch to perform 
 *   a repeated start.
 *
 *   WARNING: Nothing in the library keeps track of whether the bus tenure has 
 *   been properly ended with a STOP. It is very possible to leave the bus in 
 *   a hung state if no call to endTransmission(true) is made. Some I2C devices 
 *   will behave oddly if they do not see a STOP.
 */
BYTE TwoWire::endTransmission (BYTE sendStop)
{
     char      ret ;

// Transmit buffer (blocking)
     ret = twi_writeTo (txAddress, txBuffer, txBufferLength, 1, sendStop) ;
// Reset TX buffer control
     txBufferIndex  = 0 ;
     txBufferLength = 0 ;
// Indicate that we are done transmitting
     transmitting = 0 ;
     return (ret) ;
}

//-----------------------------------------------------------------------------
//                                                             .endTransmission
//-----------------------------------------------------------------------------
/*
 *   This variant of endTransmission() provides backwards compatibility with 
 *   the original definition, and expected behaviour, of endTransmission()
 */
BYTE TwoWire::endTransmission (void)
{
     return endTransmission (true) ;
}

//-----------------------------------------------------------------------------
//                                                                       .write
//-----------------------------------------------------------------------------
// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write (BYTE data)
{
// Check if in master transmitter mode
     if (transmitting)
     {
     // Check if buffer is full
          if (txBufferLength >= BUFFER_LENGTH)
          {
               setWriteError () ;
               return 0 ;
          }
     // Put byte in TX buffer
          txBuffer [txBufferIndex] = data ;
          txBufferIndex ++ ;
     // update amount in buffer   
          txBufferLength = txBufferIndex ;
     }
     else
     {
     // In slave send mode, reply to master
          twi_transmit (& data, 1) ;
     }
     return 1 ;
}

//-----------------------------------------------------------------------------
//                                                                       .write
//-----------------------------------------------------------------------------
/*
 *   This must be called in slave tx event callback or after 
 *   beginTransmission().
 */
size_t TwoWire::write (const BYTE * data, size_t quantity)
{
     size_t    i ;

     if (transmitting)
     {
     // in master transmitter mode
          for (i = 0 ; i < quantity ; i ++)
               write (data [i]) ;
     }
     else
     {
     // in slave send mode reply to master
          twi_transmit (data, quantity) ;
     }
     return quantity ;
}

//-----------------------------------------------------------------------------
//                                                                   .available
//-----------------------------------------------------------------------------
/*
 *   available() returns the number of bytes remaining available in the RX 
 *   RX buffer.
 *
 *   This must be called in slave rx event callback or after requestFrom().
 */
int TwoWire::available (void)
{
     return rxBufferLength - rxBufferIndex ;
}

//-----------------------------------------------------------------------------
//                                                                        .read
//-----------------------------------------------------------------------------
/*
 *   This must be called in slave rx event callback or after requestFrom().
 */
int TwoWire::read (void)
{
     int       value = -1 ;

// Get each successive byte on each call
     if (rxBufferIndex < rxBufferLength)
     {
          value = rxBuffer [rxBufferIndex] ;
          rxBufferIndex ++ ;
     }
     return value ;
}

//-----------------------------------------------------------------------------
//                                                                        .peek
//-----------------------------------------------------------------------------
/*
 *   This must be called in slave rx event callback or after requestFrom().
 */
int TwoWire::peek (void)
{
     int       value = -1 ;

     if (rxBufferIndex < rxBufferLength)
          value = rxBuffer [rxBufferIndex] ;
     return value ;
}

//-----------------------------------------------------------------------------
//                                                                       .flush
//-----------------------------------------------------------------------------
/// To be implemented.
void TwoWire::flush (void)
{
}

//-----------------------------------------------------------------------------
//                                                            .onReceiveService
//-----------------------------------------------------------------------------
/*
 *   onReceiveService() is called when data is received.
 */
void TwoWire::onReceiveService (BYTE * inBytes, int numBytes)
{
     BYTE      i ;

// Check if user hasn't registered a callback
     if (! user_onReceive)
          return ;
// Check if RX buffer is in use by a master requestFrom()
     if (rxBufferIndex < rxBufferLength)
     {
     // This drops data
          return ;
     }
// Copy TWI RX buffer into local read buffer to enable new reads to happen in parallel
     for (i = 0 ; i < numBytes ; i ++)
          rxBuffer [i] = inBytes [i] ;
// Set RX buffer control
     rxBufferIndex  = 0 ;
     rxBufferLength = numBytes ;
// Alert user program
     user_onReceive (numBytes) ;
}

//-----------------------------------------------------------------------------
//                                                            .onRequestService
//-----------------------------------------------------------------------------
/*
 *   onRequestService() is called when data is requested.
 */
void TwoWire::onRequestService (void)
{
// Check if user hasn't registered a callback
     if (! user_onRequest)
          return ;
// Reset TX buffer control to stop any pending pre-master sendTo() activity
     txBufferIndex  = 0 ;
     txBufferLength = 0 ;
// Alert user program
     user_onRequest () ;
}

//-----------------------------------------------------------------------------
//                                                                   .onReceive
//-----------------------------------------------------------------------------
/*
 *   onReceive() sets a vector to be called on slave write.
 */
void TwoWire::onReceive (void (*function)(int))
{
     user_onReceive = function ;
}

//-----------------------------------------------------------------------------
//                                                                   .onRequest
//-----------------------------------------------------------------------------
/*
 *   onRequest() sets a vector to be called on slave read.
 */
void TwoWire::onRequest (void (*function)(void))
{
     user_onRequest = function ;
}

//-----------------------------------------------------------------------------
//                                                       Preinstantiate Objects
//-----------------------------------------------------------------------------
TwoWire Wire = TwoWire () ;

