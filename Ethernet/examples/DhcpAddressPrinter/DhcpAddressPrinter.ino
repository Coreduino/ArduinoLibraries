//*****************************************************************************
//
//   Ethernet Library Example
//   DhcpAddressPrinter.ino
//
//*****************************************************************************
/*
     DHCP-based IP printer

     This sketch uses the DHCP extensions to the Ethernet library
     to get an IP address via DHCP and print the address obtained.
     using an Arduino Wiznet Ethernet shield. 

     Circuit:
     * Ethernet shield attached to pins 10, 11, 12, 13

     created 12 April 2011
     modified 9 Apr 2012
     by Tom Igoe
 */

//-----------------------------------------------------------------------------
//                                        C O M P I L E R   D I R E C T I V E S
//-----------------------------------------------------------------------------
#include <SPI.h>
#include <Ethernet.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Compiler Control

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - Constants

//-----------------------------------------------------------------------------
//                                                          S T R U C T U R E S
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                                                            F U N C T I O N S
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                                                        G L O B A L   D A T A
//-----------------------------------------------------------------------------
/*
 *   Enter your shield's MAC address for your controller below.
 *   Newer Ethernet shields have a MAC address printed on a sticker 
 *   on the shield. Failing that, make one up.
 */
BYTE gabMAC [] = { 0x90, 0xA2, 0xDA, 0x0D, 0xF3, 0x8D } ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - Composite Data Structures
/*
 *   This calls the constructor function in the Ethernet Client library
 *   with the IP address and port of the server that you want to connect to 
 *   (port 80 is default for HTTP):
 */
EthernetClient client ;


//=============================================================================
//                                                                        setup
//-----------------------------------------------------------------------------
void setup (void)
{
     BYTE      bIndex ;

// Open serial communications and wait for port to open:
     Serial.begin (9600) ;
     delay (3000) ;
// Wait for serial port to connect
     while (! Serial) ;
// Start the Ethernet connection:
     Serial.println ("START ETHERNET CONNECTION") ;
     while (Ethernet.begin (gabMAC) == 0)
     {
          Serial.println ("* FAILED TO CONFIGURE ETHERNET USING DHCP") ;
          delay (1000) ;
     }
// Print your local IP address:
     Serial.print("IP Address: ");
     for (bIndex = 0 ; bIndex < 4 ; bIndex ++)
     {
     // print the value of each byte of the IP address:
          Serial.print (Ethernet.localIP () [bIndex], DEC) ;
          Serial.print (".") ;
     }
     Serial.println () ;
}

//=============================================================================
//                                                                         loop
//-----------------------------------------------------------------------------
void loop (void)
{
}
