//*****************************************************************************
//
//   Near Field Communications (NFC)
//   Serial Peripheral Interface
//   SPI Driver Class                                                   SPI.cpp
//
//*****************************************************************************
/*
 *   This library allows you to communicate with SPI devices, with the Arduino 
 *   as the master device. 
 *
 *   A Brief Introduction to the Serial Peripheral Interface (SPI):
 *
 *   Serial Peripheral Interface (SPI) is a synchronous serial data protocol 
 *   used by microcontrollers for communicating with one or more peripheral 
 *   devices quickly over short distances. It can also be used for 
 *   communication between two microcontrollers.
 *
 *   With an SPI connection there is always one master device (usually a 
 *   microcontroller) which controls the peripheral devices. Typically there 
 *   are three lines common to all the devices:
 *
 *        MISO      (Master In Slave Out) the Slave line for sending 
 *                  data to the master,
 *
 *        MOSI      (Master Out Slave In) the Master line for sending 
 *                  data to the peripherals,
 *
 *        SCK       (Serial Clock) the clock pulses which synchronize 
 *                  data transmission generated by the master
 *
 *   and one line specific for every device:
 *
 *        SS        (Slave Select) the pin on each device that the master 
 *                  can use to enable and disable specific devices. This
 *                  may also be referred to as the Chip Select (CS) line.
 *
 *                  When a device's Slave Select pin is low, it communicates 
 *                  with the master. When it's high, it ignores the master.
 *                  This permits multiple SPI devices to share the same 
 *                  MISO, MOSI, and SCK lines.
 *
 *   On Arduino boards, the MISO, MOSI and SCK pins are presented as the ICSP
 *   connector, a 2x3 array of 6 pins. The ICSP connector is in a consistent 
 *   physical location and enables designing a shield that works on every
 *   board.
 *
 *   Unfortunately, the SS pin depends on the shield, and this is often
 *   Digital Pin 10. This design feature prevents multiple shields from being 
 *   stacked onto the same Arduino board without some physical alteration made 
 *   to the shield. However, system designers should be reminded that the 
 *   Arduino series of boards are based on a low-cost, low-power RISC-based 
 *   microcontroller with limited system resources (RAM, I/O pins, etc.)
 */

//*****************************************************************************
//                                                              Developer Notes
//*****************************************************************************
/*
 *   SPI data can be shifted in Most Significant Bit (MSB) or Least Significant 
 *   Bit (LSB) first. This is controlled by the SPI.setBitOrder().
 *
 *   The data clock line can be idle when high or low. Samples can be on the 
 *   rising or falling edge of clock pulses. These modes are controlled by 
 *   the SPI.setDataMode().
 *
 *   SPI can run at different speeds. This is controlled by the 
 *   SPI.setClockDivider().
 *
 *   The SPI standard is loose and each device might implements it a little 
 *   differently. This means special attention must be paid to the device's 
 *   datasheet when developing code. 
 *
 *   Refer to http://arduino.cc/en/Reference/SPI
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 *   SPI Master library for arduino.
 *
 *   This file is free software; you can redistribute it and/or modify
 *   it under the terms of either the GNU General Public License version 2
 *   or the GNU Lesser General Public License version 2.1, both as
 *   published by the Free Software Foundation.
 *
 *   Copyright (C) 2013 by Programify Ltd. Copyright in regard to code format, 
 *   extensions to functionality and commentary of this class.
 */

//*****************************************************************************
//                                                                      Remarks
//*****************************************************************************
/*
 *   MISO pin automatically overrides to INPUT.
 *
 *   By doing this AFTER enabling SPI, we avoid accidentally clocking in a 
 *   single bit since the lines go directly from "input" to SPI control.  
 *
 *   http://code.google.com/p/arduino/issues/detail?id=888
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------
#include "pins_arduino.h"
#include "SPI.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants

//-----------------------------------------------------------------------------
//                                                                    Functions
//-----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Public Functions

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Private Functions

//-----------------------------------------------------------------------------
//                                                                  Global Data
//-----------------------------------------------------------------------------

SPIClass SPI ;

//=============================================================================
//                                                                       .begin
//-----------------------------------------------------------------------------
void SPIClass::begin ()
{
// Set SS to high so a connected chip will be "deselected" by default
     digitalWrite (SS, HIGH) ;
// When the SS pin is set as OUTPUT, it can be used as a general purpose output port (it doesn't influence SPI operations)
     pinMode (SS, OUTPUT) ;
// Warning: if the SS pin ever becomes a LOW INPUT then SPI automatically switches to Slave, so the data direction of the SS pin MUST be kept as OUTPUT
     SPCR |= _BV (MSTR) ;
     SPCR |= _BV (SPE) ;
// Set direction register for SCK and MOSI pin.
     pinMode (SCK,  OUTPUT) ;
     pinMode (MOSI, OUTPUT) ;
}

//=============================================================================
//                                                                         .end
//-----------------------------------------------------------------------------
void SPIClass::end ()
{
     SPCR &= ~_BV (SPE) ;
}

//=============================================================================
//                                                                 .setBitOrder
//-----------------------------------------------------------------------------
void SPIClass::setBitOrder (uint8_t bitOrder)
{
     if (bitOrder == LSBFIRST)
          SPCR |= _BV(DORD) ;
     else
          SPCR &= ~(_BV(DORD)) ;
}

//=============================================================================
//                                                                 .setDataMode
//-----------------------------------------------------------------------------
void SPIClass::setDataMode (uint8_t mode)
{
     SPCR = (SPCR & ~SPI_MODE_MASK) | mode ;
}

//=============================================================================
//                                                             .setClockDivider
//-----------------------------------------------------------------------------
void SPIClass::setClockDivider (uint8_t rate)
{
     SPCR = (SPCR & ~SPI_CLOCK_MASK)   | (rate & SPI_CLOCK_MASK) ;
     SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK) ;
}