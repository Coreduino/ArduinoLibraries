/*
 *   Rainbowduino v3.0 Library examples:  Cube2
 *   Sets pixels on 3D plane (4x4x4 cube)
 */

#include <Rainbowduino.h>

DWORD     gadwColour [4] ;

void setup ()
{
// Initialize Rainbowduino driver
     Rb.init () ;
// Set up coloured layers
     gadwColour [0] = 0x00FF00 ;
     gadwColour [1] = 0x008040 ;
     gadwColour [2] = 0x004080 ;
     gadwColour [3] = 0x0000FF ;
}

void loop ()
{
     DWORD     dwColour ;
     int       iAxisX ;
     int       iAxisY ;
     int       iLayer ;

// Paint layer 24bit RGB
     for (iAxisX = 0 ; iAxisX < 4 ; iAxisX ++)
     {
          for (iAxisY = 0 ; iAxisY < 4 ; iAxisY ++)
          {
               for (iLayer = 0 ; iLayer < 4 ; iLayer ++)
                    Rb.setPixelZXY (iAxisX, iLayer, iAxisY, gadwColour [iLayer]) ;
          }
     }
// Delay between transitions
     delay (300) ;
// Change colours for each layer
     dwColour       = gadwColour [3] ;
     gadwColour [3] = gadwColour [2] ;
     gadwColour [2] = gadwColour [1] ;
     gadwColour [1] = gadwColour [0] ;
     gadwColour [0] = dwColour ;
}

