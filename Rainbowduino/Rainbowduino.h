/*
 Rainbowduino v3.0 Library. 
 
 2011 Copyright (c) Seeed Technology Inc.
 
 Authors: Albert.Miao, Visweswara R

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

/*
This library heavily makes use of line, circle and other shape drawing algorithms and code presented
in paper "The Beauty of Bresenham's Algorithm" by Alois Zingl, Vienna, Austria. The original author
has kindly released these example programs with no copyright.
*/

#ifndef RAINBOWDUINO_h
#define RAINBOWDUINO_h

#include <Arduino.h>
#include <avr/pgmspace.h>


#define   CUBE_PLANE_Z   0
#define   CUBE_PLANE_X   1
#define   CUBE_PLANE_Y   2

#define   CUBE_LAYER0    0
#define   CUBE_LAYER1    1
#define   CUBE_LAYER2    2
#define   CUBE_LAYER3    3

// MY9221 driver interfaces 

#define   DDR_Data  DDRB
#define   DDR_Clk   DDRB

#define   PORT_Data PORTB
#define   PORT_Clk  PORTB

#define   BIT_Data  0x01
#define   BIT_Clk   0x02

// 3-to-8 Decoder Lines

#define   DDR_Lines  DDRD
#define   PORT_Lines PORTD
#define   BIT_Lines  0xF0
#define   switchOffDrive	{PORT_Lines &=~ 0x80;}

#define   CmdMode 0b0000000000000000

class Rainbowduino
{
public:
     void      blankDisplay        (void) ;
     void      clearDisplay        (void) ; // Not made private method as used in ISR. Use this like a private method.
     void      drawChar            (BYTE ascii, WORD poX, WORD poY, WORD colorRGB) ;
     void      drawCircle          (int poX, int poY, int r, DWORD colorRGB) ;
     void      drawHorizontalLine  (WORD poX, WORD poY, WORD length, DWORD colorRGB) ;
     void      drawLine            (WORD x0, WORD y0, WORD x1, WORD y1, DWORD colorRGB) ;
     void      drawRectangle       (WORD poX, WORD poY, WORD length, WORD width, DWORD colorRGB) ;
     void      drawVerticalLine    (WORD poX, WORD poY, WORD length, DWORD colorRGB) ;
     void      fillCircle          (int poX, int poY, int r, DWORD colorRGB) ;
     void      fillRectangle       (WORD poX, WORD poY, WORD length, WORD width, DWORD colorRGB) ;
     void      init                () ;
     void      init_timer1         (void) ;
     void      latchData           (void) ;
     void      send16bitData       (WORD data) ;
     void      setCube             (DWORD colorRGB) ;
     void      setPixelXY          (BYTE x, BYTE y, DWORD colorRGB) ;
     void      setPixelXY          (BYTE x, BYTE y, BYTE colorR, BYTE colorG, BYTE colorB) ;
     void      setPixelZXY         (BYTE z, BYTE x, BYTE y, DWORD ) ;
     void      setPixelZXY         (BYTE z, BYTE x, BYTE y, BYTE  colorR,  BYTE colorG, BYTE colorB) ;
     ///void      setPlane            (BYTE bPlane, BYTE bIndex, DWORD colorRGB) ;
     void      setZPlane           (BYTE bIndex, DWORD colorRGB) ;
     void      switchOnDrive       (BYTE line) ;

private:

} ;

extern Rainbowduino Rb ;

extern boolean Flag_20ms ;
extern boolean Flag_300ms ;
extern BYTE simpleFont[][8] ;

#endif
