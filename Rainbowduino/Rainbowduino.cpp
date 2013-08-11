//*****************************************************************************
//
//   Rainbowduino v3.0 Library                                 Rainbowduino.cpp
//
//*****************************************************************************
/*
 *   2011 Copyright (c) Seeed Technology Inc.
 *   Authors: Albert.Miao, Visweswara R
 *   
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *   
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 *   This library heavily makes use of line, circle and other shape drawing algorithms and code presented
 *   in paper "The Beauty of Bresenham's Algorithm" by Alois Zingl, Vienna, Austria. The original author
 *   has kindly released these example programs with no copyright.
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   11-07-13  Improved source code format. Programify.
 */

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------
#include "Rainbowduino.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants

//-----------------------------------------------------------------------------
//                                                                    Functions
//-----------------------------------------------------------------------------
// Cube functions
//        blankDisplay
//        clearDisplay
//        drawChar
//        drawCircle
//        drawHorizontalLine
//        drawRectangle
//        drawVerticalLine
//        fillCircle
//        fillRectangle
//        setCube
//        setPixelZXY {x2}
//        setZPlane

// Grid functions
//        setPixelXY

// Low level driver code
//        init_timer1
//        ISR
//        latchData
//        send16bitData
//        switchOnDrive

//-----------------------------------------------------------------------------
//                                                                  Global Data
//-----------------------------------------------------------------------------
boolean   Flag_20ms  = false ;
boolean   Flag_300ms = false ;

BYTE      Count_20ms ;
BYTE      lineDrive=0; //used within ISR

// Frame Buffer - placed in RAM
BYTE      frameBuffer [3][8][8] =
{
// Blue
     {
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0}
     },
// Green
     {
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0}
     },
// Red
     {
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0},
          {0,0,0,0,0,0,0,0}
     }
} ;

// Matrix to Cube mapping 
BYTE      ZX [4][4] =
{
     {7,7,0,0},
     {6,6,1,1},
     {5,5,2,2},
     {4,4,3,3}, 
} ;
BYTE      YX [4][4] =
{
     {0,7,0,7},
     {1,6,1,6},
     {2,5,2,5},
     {3,4,3,4}
} ;

WORD      Count_300ms ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Composite Data
Rainbowduino   Rb ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - String Literals

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Externals

//=============================================================================
//                                                                        .init
//-----------------------------------------------------------------------------
// Init the Port lines and invoke timer 1 configuration
void Rainbowduino::init ()
{
    DDR_Lines |= BIT_Lines;
    PORT_Lines &=~ BIT_Lines;

    DDRD |= 0x04;

    DDR_Data |= BIT_Data;
    DDR_Clk |= BIT_Clk;
    PORT_Data &=~ BIT_Data;
    PORT_Clk &=~ BIT_Clk;

    DDRB |= 0x20;

    clearDisplay () ;
// Configure interrupt
    init_timer1 () ;
}

//=============================================================================
//                                                                  init_timer1
//-----------------------------------------------------------------------------
/*
 *   init_timer1() configures Timer 1 ISR for periodic 100uS interrupts.
 */
void Rainbowduino::init_timer1 (void)
{
// clear control register A
    TCCR1A  = 0 ;          
// Set mode as phase and frequency correct pwm, stop the timer       		
    TCCR1B  = _BV (WGM13) ;
// (XTAL * microseconds) / 2000000  1mS cycles
    ICR1    = 10000 ;
    TIMSK1  = _BV (TOIE1) ;
    TCNT1   = 0 ;
    TCCR1B |= _BV (CS10) ;
// Enable global interrupt
    sei () ;
}

//=============================================================================
//                                                               .send16bitData
//-----------------------------------------------------------------------------
/*
 *   send16bitData() sends 16-bit data to MY9221 driver chips.
 */
void Rainbowduino::send16bitData (WORD data)
{
     BYTE      i ;

     for (i = 0 ; i < 16 ; i ++)
     {
          if (data & 0x8000)
               PORT_Data |= BIT_Data ;
          else
               PORT_Data &= ~BIT_Data ;
          PORT_Clk ^= BIT_Clk ;
          data    <<= 1 ;
     }
}

//=============================================================================
//                                                                   .latchData
//-----------------------------------------------------------------------------
/*
 *   latchData() provides a latch routine for MY9221 data exchange.
 */
void Rainbowduino::latchData (void)
{
     BYTE      bLine ;

     PORT_Data  &= ~BIT_Data ;
     delayMicroseconds (10) ;
// Switch off drive
     PORT_Lines &= ~0x80 ;
     for (bLine = 0 ; bLine < 8 ; bLine ++)
          PORT_Data ^= BIT_Data ;
} 

//=============================================================================
//                                                               .switchOnDrive
//-----------------------------------------------------------------------------
void Rainbowduino::switchOnDrive (BYTE line)
{
     BYTE      LineBits ;

     LineBits    = (line << 4) ;
     PORT_Lines &= ~BIT_Lines ;
     PORT_Lines |= LineBits ;
     PORT_Lines |= 0x80 ;
}

//=============================================================================
//                                                                .clearDisplay
//-----------------------------------------------------------------------------
/*
 *   clearDisplay() clears MY9221 lines. Internally used for avoiding flicker.
 *   This is not the same as blank disply.
 */
void Rainbowduino::clearDisplay (void)
{
     BYTE      i ;

// Init
     send16bitData (CmdMode) ;
     PORT_Data &= ~BIT_Data ;
     for (i = 0 ; i < 192 ; i ++)
          PORT_Clk ^= BIT_Clk ;

     send16bitData (CmdMode) ;
     PORT_Data &= ~BIT_Data ;
     for (i = 0 ; i < 192 ; i ++)
          PORT_Clk ^= BIT_Clk ;
     latchData () ;
}

//=============================================================================
//                                                                .blankDisplay
//-----------------------------------------------------------------------------
/*
 *   blankDisplay() blanks all pixels.
 */
void Rainbowduino::blankDisplay (void)
{
     BYTE      x ;
     BYTE      y ;

     for (x = 0 ; x <= 7 ; x ++)
     {
          for (y = 0 ; y <= 7 ; y ++)
          {
               frameBuffer [0][x][y] = 0x00 ;
               frameBuffer [1][x][y] = 0x00 ;
               frameBuffer [2][x][y] = 0x00 ;
          }
     }
}   

//=============================================================================
//                                                                  .setPixelXY
//-----------------------------------------------------------------------------
/*
 *   setPixelXY() sets the pixel (X,Y) of RGB matrix with colour 24-bit RGB 
 *   Colour.
 */
void Rainbowduino::setPixelXY (BYTE x, BYTE y, DWORD colorRGB)
{
     if (x > 7 || y > 7)
     {
     // Do nothing.
     // This check is used to avoid writing to out-of-bound pixels by graphics function. 
     // But this might slow down setting pixels (remove this check if fast disply is desired)
     }
     else
     {
     // Blue Channel
          colorRGB = (colorRGB & 0x00FFFFFF) ;
          frameBuffer [0][x][y] = (colorRGB & 0x0000FF) ;
     // Green Channel
          colorRGB = (colorRGB >> 8);
          frameBuffer [1][x][y] = (colorRGB & 0x0000FF) ;
     // Red Channel
          colorRGB = (colorRGB >> 8);
          frameBuffer [2][x][y] = (colorRGB & 0x0000FF) ;
     }
}

//=============================================================================
//                                                                  .setPixelXY
//-----------------------------------------------------------------------------
/*
 *   setPixelXY() sets the pixel (X,Y) of RGB matrix with colours R,G,B.
 */
void Rainbowduino::setPixelXY (BYTE x, BYTE y, BYTE colorR, BYTE colorG, BYTE colorB)
{
     frameBuffer [0][x][y] = colorB ;
     frameBuffer [1][x][y] = colorG ;
     frameBuffer [2][x][y] = colorR ;
}

//=============================================================================
//                                                                 .setPixelZXY
//-----------------------------------------------------------------------------
/*
 *   setPixelZXY() sets the pixel (Z,X,Y) of RGB Cube with colour 24-bit RGB 
 *   Colour.
 */
void Rainbowduino::setPixelZXY (BYTE z, BYTE x, BYTE y, DWORD colorRGB)
{
     setPixelXY (ZX [z][x], YX [y][x], colorRGB) ;
}

//=============================================================================
//                                                                 .setPixelZXY
//-----------------------------------------------------------------------------
/*
 *   setPixelZXY() sets the pixel (Z,X,Y) of RGB Cube with colours R,G,B.
 */
void Rainbowduino::setPixelZXY (BYTE z, BYTE x, BYTE y, BYTE colorR,  BYTE colorG, BYTE colorB)
{
    setPixelXY(ZX[z][x], YX[y][x], colorR, colorG, colorB);
}

//fill the frame buffer starting from 'start' to ending at 'end' with values in colorRGB array.
//Pixels range is from 0,1,2,.....61,62
/*void Rainbowduino::setPixelXY(BYTE start, BYTE end, DWORD *colorRGB)
{
  BYTE ci =0;
  for(BYTE i=start; i<=end; i++)
    {
      setPixelXY(i/8, i % 8, colorRGB[ci]);
      ci++; 
    }

}*/

//=============================================================================
//                                                                  .drawCircle
//-----------------------------------------------------------------------------
void Rainbowduino::drawCircle (int poX, int poY, int r, DWORD color)
{
     int       err = 2 - 2 * r ;
     int       e2 ;
     int       x = -r ;
     int       y = 0 ;

     do
     {
          setPixelXY (poX - x, poY + y, color) ;
          setPixelXY (poX + x, poY + y, color) ;
          setPixelXY (poX + x, poY - y, color) ;
          setPixelXY (poX - x, poY - y, color) ;
          e2 = err ;
          if (e2 <= y)
          {
               y ++ ;
               err += y * 2 + 1 ;
               if (-x == y && e2 <= x)
                    e2 = 0 ;
          }
          if (e2 > x)
          {
               x ++ ;
               err += x * 2 + 1 ;
          }
     }
     while (x <= 0) ;
}

//=============================================================================
//                                                                  .fillCircle
//-----------------------------------------------------------------------------
void Rainbowduino::fillCircle (int poX, int poY, int r, DWORD color)
{
int x = -r, y = 0, err = 2-2*r, e2;
do {
drawVerticalLine(poX-x,poY-y,2*y,color);
drawVerticalLine(poX+x,poY-y,2*y,color);
e2 = err;
if (e2 <= y) {
err += ++y*2+1;
if (-x == y && e2 <= x) e2 = 0;
}
if (e2 > x) err += ++x*2+1;
}
while (x <= 0);
}

//=============================================================================
//                                                               .drawRectangle
//-----------------------------------------------------------------------------
void Rainbowduino::drawRectangle (WORD poX, WORD poY, WORD length,WORD width, DWORD color)
{
drawHorizontalLine(poX, poY, length, color);
drawHorizontalLine(poX, poY+width-1, length, color);
drawVerticalLine(poX, poY, width,color);
drawVerticalLine(poX+length-1, poY, width,color);
}

//=============================================================================
//                                                            .drawVerticalLine
//-----------------------------------------------------------------------------
void Rainbowduino::drawVerticalLine (WORD poX, WORD poY,WORD length, DWORD color)
{
     drawLine (poX, poY, poX, poY + length - 1, color) ;
}

//=============================================================================
//                                                          .drawHorizontalLine
//-----------------------------------------------------------------------------
void Rainbowduino::drawHorizontalLine (WORD poX, WORD poY,WORD length, DWORD color)
{
     drawLine (poX, poY, poX + length - 1, poY, color) ;
}

//=============================================================================
//                                                               .fillRectangle
//-----------------------------------------------------------------------------
void Rainbowduino::fillRectangle (WORD poX, WORD poY, WORD length, WORD width, DWORD color)
{
for(WORD i=0;i<width;i++)
drawHorizontalLine(poX, poY+i, length, color);
}

//=============================================================================
//                                                                    .drawLine
//-----------------------------------------------------------------------------
void Rainbowduino::drawLine (WORD x0, WORD y0, WORD x1, WORD y1, DWORD color)
{
int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
int err = dx+dy, e2; /* error value e_xy */
for (;;){ /* loop */
setPixelXY(x0,y0,color);
e2 = 2*err;
if (e2 >= dy) { /* e_xy+e_x > 0 */
if (x0 == x1) break;
err += dy;
x0 += sx;
}
if (e2 <= dx) { /* e_xy+e_y < 0 */
if (y0 == y1) break;
err += dx;
y0 += sy;
}
}
}

//=============================================================================
//                                                                    .drawChar
//-----------------------------------------------------------------------------
void Rainbowduino::drawChar (BYTE ascii,WORD poX, WORD poY, WORD colorRGB)
{
if((ascii < 0x20)||(ascii > 0x7e))//Unsupported char.
{
ascii = '?';
}
for(BYTE i=0;i<8;i++)
{
BYTE temp = pgm_read_byte(&simpleFont[ascii-0x20][i]);
for(BYTE f=0;f<8;f++)
{
if((temp>>f)&0x01)
{
setPixelXY(poY+f, poX+i, colorRGB);
}

}
}
}

//-----------------------------------------------------------------------------
//                                                                          ISR
//-----------------------------------------------------------------------------
//Timer1 Interrupt Service Routine
//All frameBuffer data exchange happens here
ISR (TIMER1_OVF_vect)          
{
//An Ideal ISR has to be short and not make any function calls
//But, in this case only data exchange happens.

Rb.clearDisplay(); //clear current display to avoid the next line dim light
if((Count_20ms ++) ==20)
{
Flag_20ms = true;
Count_20ms = 0;
}
if((Count_300ms ++) == 200)
{
Flag_300ms = true;
Count_300ms = 0;
}
BYTE line = (lineDrive & 0x07);

Rb.send16bitData(CmdMode);

Rb.send16bitData(frameBuffer[0][line][7]);
Rb.send16bitData(frameBuffer[0][line][6]);
Rb.send16bitData(frameBuffer[0][line][5]);
Rb.send16bitData(frameBuffer[0][line][4]);
Rb.send16bitData(frameBuffer[0][line][3]);
Rb.send16bitData(frameBuffer[0][line][2]);
Rb.send16bitData(frameBuffer[0][line][1]);
Rb.send16bitData(frameBuffer[0][line][0]);

Rb.send16bitData(frameBuffer[1][line][7]);
Rb.send16bitData(frameBuffer[1][line][6]);
Rb.send16bitData(frameBuffer[1][line][5]);
Rb.send16bitData(frameBuffer[1][line][4]);

Rb.send16bitData(CmdMode);

Rb.send16bitData(frameBuffer[1][line][3]);
Rb.send16bitData(frameBuffer[1][line][2]);
Rb.send16bitData(frameBuffer[1][line][1]);
Rb.send16bitData(frameBuffer[1][line][0]);

Rb.send16bitData(frameBuffer[2][line][7]);
Rb.send16bitData(frameBuffer[2][line][6]);
Rb.send16bitData(frameBuffer[2][line][5]);
Rb.send16bitData(frameBuffer[2][line][4]);
Rb.send16bitData(frameBuffer[2][line][3]);
Rb.send16bitData(frameBuffer[2][line][2]);
Rb.send16bitData(frameBuffer[2][line][1]);
Rb.send16bitData(frameBuffer[2][line][0]);

Rb.latchData();
Rb.switchOnDrive(line);
lineDrive++;

PORTD &=~ 0x04;
}

//=============================================================================
//                                                                     .setCube
//-----------------------------------------------------------------------------
void Rainbowduino::setCube (DWORD colorRGB)
{
     BYTE      bPlane ;

     for (bPlane = 0 ; bPlane < 4 ; bPlane ++)
          setZPlane (bPlane, colorRGB) ;
}

//=============================================================================
//                                                                    .setplane
//-----------------------------------------------------------------------------
void Rainbowduino::setZPlane (BYTE bIndex, DWORD colorRGB)
{
     BYTE      bAxisX ;
     BYTE      bAxisY ;

     for (bAxisX = 0 ; bAxisX < 4 ; bAxisX ++)
     {
          for (bAxisY = 0 ; bAxisY < 4 ; bAxisY ++)
               setPixelZXY (bIndex, bAxisX, bAxisY, colorRGB) ;
     }
}
