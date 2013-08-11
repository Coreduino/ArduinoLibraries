//*****************************************************************************
//
//                                                                 SunnyDay.ino
//
//*****************************************************************************

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Version: 1.0
 *   Author:  Jonathan Oxer (jon@freetronics.com)
 *   License: GPLv3
 */

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------
#include  <Rainbowduino.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants

//-----------------------------------------------------------------------------
//                                                                    Functions
//-----------------------------------------------------------------------------
void      DoMoveCloud    (void) ;

//-----------------------------------------------------------------------------
//                                                                  Global Data
//-----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Composite Data Items


//=============================================================================
//                                                                        setup
//-----------------------------------------------------------------------------
void setup ()
{
// Initialize Rainbowduino driver
     Rb.init () ;
// Blue sky
     Rb.setCube (0x000022) ;
// Green grass
     Rb.setZPlane (0, 0x001100) ;
// Yellow sun
     Rb.setPixelZXY (3, 2, 2, 0xFFFF00) ;
}

//=============================================================================
//                                                                         loop
//-----------------------------------------------------------------------------
void loop (void)
{
// Animate raindrops
     DoMoveCloud () ;
// Animation interval
     delay (100) ;
}

//-----------------------------------------------------------------------------
//                                                                  DoMoveCloud
//-----------------------------------------------------------------------------
void DoMoveCloud (void)
{
     BYTE      bAxisX ;
     BYTE      bAxisY ;

// Generate random start point for cloud

}
