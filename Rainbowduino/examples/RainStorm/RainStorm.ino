//*****************************************************************************
//
//   Rainbowdunio Animation
//   Rain and lightning effects                                   Rainstorm.ino
//
//*****************************************************************************
/*
 *   Rainstorm animates a grey cloud base with blue rain drops and the 
 *   occassional lightning flash. This code is based on Programify's 
 *   extended Arduino core library available for free to download at 
 *   http://programify.com/arduino/ (no fee - no warranty, etc.).
 *
 *   You can change the cloud base by altering LAYER_CLOUDBASE in the range
 *   0..3, although 3 is recommended. NUMBEROF_RAINDROPS can be adjusted,
 *   however, a range of 1..4 is recommended.
 *
 *   This cube sketch is best experienced while listening to the sound of
 *   rain and thunder at http://www.rainymood.com.
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   Version: 1.0
 *   Author:  Jonathan Oxer (jon@freetronics.com)
 *   License: GPLv3
 *
 *   Version: 2.0
 *   Author:  Clive Hudson (programify.com)
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   19-07-13  Created version 2. Fixed code format.
 *             Now uses Programify's extended Ardunio library.
 *             Modified for use with Rainbowduino.
 *             Completely reviewed how the code was organised.
 *             Introduced green grass layer, two layer cloud with option to 
 *             vary the cloud base.
 *             Improved lightning effect.
 */

//-----------------------------------------------------------------------------
//                                                          Compiler Directives
//-----------------------------------------------------------------------------
#include  <Rainbowduino.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants
// 24-bit colour codes (0xRRGGBB)
#define   COLOUR_BLACK        0x000000
#define   COLOUR_GROUND       0x000800
#define   COLOUR_GROUNDBRIGHT 0x00FF00
#define   COLOUR_PRESTRIKE    0x2020FF
#define   COLOUR_INTRASTRIKE  0x101080
#define   COLOUR_RAINDROP     0x3838FF
#define   COLOUR_STRIKE       0xFFFFFF

#define   DELAY_LIGHTNING     50        // Random delay between lightning strikes
#define   DELAY_RAINDROP      60        // Milliseconds between raindrop movements

#define   LAYER_CLOUDBASE     3         // Default and suggested cube layer index to the cloud base (0..3)
#define   LAYER_GROUND        0         // Deafult and suggested ground (green grass) layer  (0..3)

#define   MAX_LIGHTNING       3         // Maximum number of flashes per lightning strike

#define   NUMBEROF_RAINDROPS  3         // Number of raindrops to animate (maximum 16)

#define   SIZEOF_CUBE         4         // Number of LEDs to each dimension on the cube

//-----------------------------------------------------------------------------
//                                                                    Functions
//-----------------------------------------------------------------------------
void      DoCloudLayer   (BYTE bLayer, BYTE bFromColor, BYTE bToColor) ;
void      DoClouds       (BYTE bCloudBase) ;
void      DoLightning    (void) ;
void      DoRain         (void) ;
DWORD     GetGreyColour  (BYTE bLevel) ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Global Functions
//        loop
//        setup

//-----------------------------------------------------------------------------
//                                                                  Global Data
//-----------------------------------------------------------------------------
BYTE      gaDropX [16] ;
BYTE      gaDropY [16] ;
BYTE      gaDropZ [16] ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Composite Data Items


//=============================================================================
//                                                                        setup
//-----------------------------------------------------------------------------
void setup ()
{
     BYTE      bRainDrop ;

// Prepare random initial timing for each rain drop
     for (bRainDrop = 0 ; bRainDrop < NUMBEROF_RAINDROPS ; bRainDrop ++)
          gaDropZ [bRainDrop] = random (CUBE_LAYER0, LAYER_CLOUDBASE + 0) ;
// Initialize Rainbowduino driver
     Rb.init () ;
// Inialize cube colours
     Rb.clearDisplay () ;
}

//=============================================================================
//                                                                         loop
//-----------------------------------------------------------------------------
void loop ()
{
// Render cloud base
     DoClouds (LAYER_CLOUDBASE) ;
// Green grass
     Rb.setZPlane (LAYER_GROUND, COLOUR_GROUND) ;
// Animate raindrops
     DoRain () ;
// Animation interval between rain drops
     delay (DELAY_RAINDROP) ;
// Attempt a lightning strike
     DoLightning () ;
}

//-----------------------------------------------------------------------------
//                                                                     DoClouds
//-----------------------------------------------------------------------------
/*
 *   DoClouds() displays a grey cloud layer at the top of the cube.
 *
 *   Call with:
 *
 *        bCloudBase     The Z layer to contain the cloud base. Range 0..3.
 */
void DoClouds (BYTE bCloudBase)
{
     BYTE      bLayer ;

// Enumerate layers above cloud base
     for (bLayer = SIZEOF_CUBE - 1 ; bLayer >= bCloudBase ; bLayer --)
          DoCloudLayer (bLayer, 0x08, 0x14) ;
// Paint cloud layer with random shades of grey
     DoCloudLayer (bCloudBase, 0x08, 0x12) ;
// Check if cloudbase is above ground level
     if (bCloudBase > LAYER_GROUND)
     {
     // Paint second layer under cloud with random shades of darker grey
          DoCloudLayer (bCloudBase - 1, 0x00, 0x02) ;
     }
}

//-----------------------------------------------------------------------------
//                                                                 DoCloudLayer
//-----------------------------------------------------------------------------
/*
 *   DoCloudLayer() creates a grey cloud layer, whose brightness level will
 *   be generated as a random level between 'bFromColor' and 'bToColor'.
 */
void DoCloudLayer (BYTE bLayer, BYTE bFromColor, BYTE bToColor)
{
     BYTE      bAxisX ;
     BYTE      bAxisY ;
     BYTE      bColour ;
     DWORD     dwColorRGB ;

// Establish X axis loop
     for (bAxisX = 0 ; bAxisX < SIZEOF_CUBE ; bAxisX ++)
     {
     // Establish Y axis loop
          for (bAxisY = 0 ; bAxisY < SIZEOF_CUBE ; bAxisY ++)
          {
          // Generate random grey (white)
               bColour    = random (bFromColor, bToColor) ;
               dwColorRGB = GetGreyColour (bColour) ;
          // Set pixel colour
               Rb.setPixelZXY (bLayer, bAxisX, bAxisY, dwColorRGB) ;
          }
     }
}

//-----------------------------------------------------------------------------
//                                                                GetGreyColour
//-----------------------------------------------------------------------------
/*
 *   GetGreyColour() returns the 24-bit RGB colour code for a grey level
 *   between 0 and 255.
 */
DWORD GetGreyColour (BYTE bLevel)
{
     DWORD     dwColorRGB ;

     dwColorRGB   = bLevel ;
     dwColorRGB <<= 8 ;
     dwColorRGB  |= bLevel ;
     dwColorRGB <<= 8 ;
     dwColorRGB  |= bLevel ;
     return (dwColorRGB) ;
}

//-----------------------------------------------------------------------------
//                                                                       DoRain
//-----------------------------------------------------------------------------
/*
 *   DoRain() first decides where in the cloud the new rain drops will appear.
 *   Then it will advance the animation of all current rain drops downwards by
 *   one layer.
 */
void DoRain (void)
{
     BYTE      bRainDrop ;

// Enumerate rain drops
     for (bRainDrop = 0 ; bRainDrop < NUMBEROF_RAINDROPS ; bRainDrop ++)
     {
     // Check if drop is back in the cloud
          if (gaDropZ [bRainDrop] == LAYER_CLOUDBASE + 1)
          {
          // Generate start point for drops within the cloud plane
               gaDropX [bRainDrop] = random (SIZEOF_CUBE) ;
               gaDropY [bRainDrop] = random (SIZEOF_CUBE) ;
          }
     // Cleanup trailing pixel
          Rb.setPixelZXY (gaDropZ [bRainDrop], gaDropX [bRainDrop], gaDropY [bRainDrop], COLOUR_BLACK) ;
     // Check if drop is above ground plane
          if (gaDropZ [bRainDrop] > LAYER_GROUND)
          {
          // Drop the rain drop down one layer/plane
               gaDropZ [bRainDrop] -- ;
          // Paint the rain drop
               Rb.setPixelZXY (gaDropZ [bRainDrop], gaDropX [bRainDrop], gaDropY [bRainDrop], COLOUR_RAINDROP) ;
          }
          else
          {
          // Position the rain drop back in the cloud layer
               gaDropZ [bRainDrop] = LAYER_CLOUDBASE + 1 ;
          }
     }
}

//-----------------------------------------------------------------------------
//                                                                  DoLightning
//-----------------------------------------------------------------------------
/*
 *   DoLightning() displays a lightning strike at random intervals determined
 *   by the value of DELAY_LIGHTNING. The colours used to render the lightning
 *   and the ground are also defined by manifest constants. For example, the
 *   ground need not be green, but blue (over the sea).
 *
 *   The pre-strike flicker starts off at the top of the cube and works its
 *   way down to ground level. By using appropriate colour codes, the ground 
 *   layer may also brighten with the flash event. To best see how the flash
 *   progresses, add 200ms to each call to delay() in this function.
 */
void DoLightning (void)
{
     int       iChance ;
     int       iCount ;
     int       iDepth ;
     int       iFlashes ;
     int       iLayer ;

// Generate number in the range 1..DELAY_LIGHTNING
     iChance = random (DELAY_LIGHTNING) ;
// Check if chance of lightning striking (1 in 'DELAY_LIGHTNING')
     if (iChance == 1)
     {
     // Generate a number in the range 1..MAX_LIGHTNING
          iFlashes = random (1, MAX_LIGHTNING + 1) ;
     // Repeat for each lightning flash
          for (iCount = 0 ; iCount < iFlashes ; iCount ++)
          {
          // Pre-main strike flicker
               for (iDepth = SIZEOF_CUBE - 1 ; iDepth > LAYER_GROUND ; iDepth --)
               {
               // Light up layers in a top-down direction
                    for (iLayer = SIZEOF_CUBE - 1 ; iLayer >= iDepth ; iLayer --)
                         Rb.setZPlane (iLayer, COLOUR_INTRASTRIKE) ;
                    delay (20) ;
                    Rb.setCube (COLOUR_BLACK) ;
                    Rb.setZPlane (LAYER_GROUND, COLOUR_GROUND) ;
                    delay (25) ;
               }
          // Main strike
               Rb.setCube (COLOUR_STRIKE) ;
               Rb.setZPlane (LAYER_GROUND, COLOUR_GROUNDBRIGHT) ;
               delay (55) ;
               Rb.setCube (COLOUR_BLACK) ;
               delay (40) ;
          }
     }
}
