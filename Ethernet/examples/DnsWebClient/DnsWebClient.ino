//*****************************************************************************
//
//   Ethernet Library Example
//   DnsWebClient.ino
//
//*****************************************************************************
/*
     DNS and DHCP-based Web client

     This sketch connects to a website (http://www.google.com)
     using an Arduino Wiznet Ethernet shield. 

     Circuit:
     * Ethernet shield attached to pins 10, 11, 12, 13

     created 18 Dec 2009
     by David A. Mellis
     modified 9 Apr 2012
     by Tom Igoe, based on work by Adrian McEwen
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

char gszServerName [] = "programify.com" ;

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
// Open serial communications and wait for port to open
     Serial.begin (9600) ;
     delay (3000) ;
// Wait for serial port to connect
     while (! Serial) ;
// Start the Ethernet connection
     while (Ethernet.begin (gabMAC) == 0)
     {
          Serial.println ("* FAILED TO CONFIGURE ETHERNET USING DHCP") ;
          delay (1000) ;
     }
// Give the Ethernet shield a second to initialize
     delay (1000) ;
     Serial.println ("CONNECTING...");
// If you get a connection, report back via serial
     if (client.connect (gszServerName, 80))
     {
          Serial.print   ("CONNECTED TO ") ;
          Serial.println (gszServerName) ;
     // Make HTTP request for homepage
          client.println ("GET /index.html HTTP/1.0") ;
          client.println () ;
     } 
     else
     {
     // If you didn't get a connection to the server
          Serial.println ("* CONNECTION FAILED") ;
     }
}

//=============================================================================
//                                                                         loop
//-----------------------------------------------------------------------------
void loop (void)
{
     char      cChar ;

// If there are incoming bytes available from the server, read them and print them
     if (client.available ())
     {
          cChar = client.read () ;
          Serial.print (cChar) ;
     }
// If the server's disconnected, stop the client
     if (! client.connected ())
     {
          Serial.println () ;
          Serial.println ("SERVER DISCONNECTED") ;
          client.stop () ;
     // Halt
          while (true) ;
     }
}

