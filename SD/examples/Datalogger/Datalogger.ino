//*****************************************************************************
//
//   Datalogger
//   Data Logger using SD Card Media                             Datalogger.ino
//
//*****************************************************************************
/*
 *   This example shows how log data from three analog sensors to an SD card 
 *   using the SD library.
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   This example code is in the public domain.
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   24-11-10  Created.
 *   09-04-12  Modified by Tom Igoe.
 *   02-08-13  Upgraded source code. Programify Ltd.
 *   03-08-13  Added support for a Start/Start button and 'running' LED.
 */

//*****************************************************************************
//                                                               Hardware Notes
//*****************************************************************************
/*
 *   1)   This code assumes analog sensors on Input Lines 0, 1, and 2. If
 *        nothing is connected to these lines, the A2D convertor will report
 *        spurious data relating to its 'floating' inputs.
 *
 *   2)   SD card attached to SPI bus as follows:
 *
 *        MOSI - pin 11
 *        MISO - pin 12
 *        CLK  - pin 13
 *        CS   - pin 4
 *
 *   3)   Attach a button to ground and a pull-up resistor (10K) to 5V.
 *        Attach the junction between the button and resistor to pin 5.
 *
 *   4)   Attach an LED to ground and current limiting resistor to pin 6.
 *        Be sure to connect the LED the right way around.
 *
 *   The shield select pin (CS) is set to pin 10 on most Arduino boards (53 
 *   on the Mega) which must be configured as OUTPUT for the standard SD 
 *   library functions to work.
 */

//-----------------------------------------------------------------------------
//                                        C O M P I L E R   D I R E C T I V E S
//-----------------------------------------------------------------------------
#include <SD.h>
#include <RTC.h>
#include <Wire.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Compile-Time Options
// Prevent compiler from pouting
#pragma GCC diagnostic ignored "-Wwrite-strings"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants
/*
 *   Change PIN_CHIPSELECT to match your SD shield or module.
 *
 *     10  Adafruit SD shields and modules
 *      4  Arduino Ethernet shield
 *      8  Sparkfun SD shield
 */
#define   PIN_CHIPSELECT      10

/*
 *   This defines the pin number used to detect events from a Stop/Start 
 *   button (momentary, push-to-make) used to toggle between start and stop 
 *   states. This allows the SD card to be safely removed without risk to the 
 *   recorded data. This pin will be configured as INPUT. Change this value to 
 *   zero if no Stop/Start button is present.
 */
#define   PIN_BTN_STOPSTART   5

/*
 *   This defines the pin number used to drive and LED which indicates that 
 *   the unit is recording data. This pin will be configured as OUTPUT. Change 
 *   this value to zero if no LED is present.
 *
 *   State     Description
 *   ========  ==============================================
 *   OFF       Unit waiting for button to Start.
 *   ON        Unit logging data, waiting for button to Stop.
 *   FLASHING  Software limit error or all other faults.
 */
#define   PIN_LED_RUNSTATE    6

//-----------------------------------------------------------------------------
//                                                          S T R U C T U R E S
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                                                            F U N C T I O N S
//-----------------------------------------------------------------------------
void      ActivateLogging     (BOOL bfEnable) ;
void      DoFlash             (int iPin, int iDelay) ;
void      DoStartStop         (void) ;
BOOL      OpenLogFile         (IN char * cpFilespec) ;
void      UnitFailed          (IN char * cpMessage) ;

//-----------------------------------------------------------------------------
//                                                        G L O B A L   D A T A
//-----------------------------------------------------------------------------
BOOL      gbfLogging ;

//- - - - - - - - - - - - - - - - - - - - - - - - - - Composite Data Structures
File           g_logfile ;

RTC_DS1307     g_rtc ;


//=============================================================================
//                                                                        setup
//-----------------------------------------------------------------------------
void setup ()
{
     char      szDate  [12] ;
     char      szStamp [64] ;
     char      szTime  [12] ;

// Global data inits
     gbfLogging = INVALID ;
// Configure Arduino's Slave Select pin
     pinMode (SS_PIN, OUTPUT) ;
// Configure user interface (LED and button)
     if (PIN_LED_RUNSTATE)
          pinMode (PIN_LED_RUNSTATE, OUTPUT) ;
     if (PIN_BTN_STOPSTART)
          pinMode (PIN_BTN_STOPSTART, INPUT) ;
// Open serial communications and wait for port to open:
     Serial.begin (9600) ;
// Wait for serial port to connect
     while (! Serial)
          DoFlash (PIN_LED_RUNSTATE, 100) ;
// Join I2C bus
     Wire.begin () ;
// Init access to RTC (clock)
     g_rtc.begin () ;
     if (! g_rtc.isrunning ())
          UnitFailed ("* RTC IS NOT RUNNING.");
// Report event
     Serial.println ("RTC INITIALIZED") ;
// Get time stamp
     GetDateStamp (szDate, szTime) ;
     sprintf (szStamp, "Date: %s\nTime: %s", szDate, szTime) ;
     Serial.println (szStamp) ;
// Check if SD card is present and can be initialized
     if (! SD.begin (PIN_CHIPSELECT))
          UnitFailed ("* FAILED TO INITIALIZE SD CARD READER.") ;
// Report event
     Serial.println ("SD CARD INITIALIZED") ;
// Set initial logging state to 'off' (button must be pressed to start)
     ActivateLogging (false) ;
}

//=============================================================================
//                                                                         loop
//-----------------------------------------------------------------------------
void loop ()
{
     char      szData  [32] ;
     char      szDate  [12] ;
     char      szStamp [32] ;
     char      szTime  [12] ;
     int       aiData   [3] ;
     int       iPin ;
     int       iButton ;

// Check if start/stop control is present
     if (PIN_BTN_STOPSTART)
     {
     // Check if Start/Stop button has been pressed
          iButton = digitalRead (PIN_BTN_STOPSTART) ;
          if (iButton == LOW)
               DoStartStop () ;
     // Check if logging has been suspended
          if (! gbfLogging)
               return ;
     }
// Read three sensors and append to the string:
     for (iPin = 0 ; iPin < 3 ; iPin ++)
          aiData [iPin] = analogRead (iPin) ;
// Convert readings to reportablt string
     sprintf (szData, "%d,%d,%d", aiData [0], aiData [1], aiData [2]) ;
// Get time stamp
     GetDateStamp (szDate, szTime) ;
     sprintf (szStamp, "%s,%s,", szDate, szTime) ;
// Append data to log
     LogString (szStamp) ;
     LogString (szData) ;
     LogString ("\n") ;
}

//-----------------------------------------------------------------------------
//                                                                 GetDateStamp
//-----------------------------------------------------------------------------
void GetDateStamp (OUT char * cpDate, OUT char * cpTime)
{
     DateTime  timestamp ;

// Get time stamp
     timestamp = g_rtc.now () ;
// Construct date and time strings
     sprintf (cpDate, "%02d-%02d-%04d", timestamp.day (), timestamp.month (), timestamp.year ()) ;
     sprintf (cpTime, "%02d:%02d:%02d", timestamp.hour (), timestamp.minute (), timestamp.second ()) ;
}

//-----------------------------------------------------------------------------
//                                                                    LogString
//-----------------------------------------------------------------------------
void LogString (IN char * cpData)
{
// Append data to log
     g_logfile.print (cpData) ;
// Echo data to the serial port
     Serial.print (cpData) ;
}

//-----------------------------------------------------------------------------
//                                                                   UnitFailed
//-----------------------------------------------------------------------------
/*
 *   UnitFailed() is a non-returning function that will continue to flash the
 *   indicator LED.
 */
void UnitFailed (IN char * cpMessage)
{
// Report error
     Serial.println (cpMessage) ;
// Flash until unit is powered down or reset
     while (true)
          DoFlash (PIN_LED_RUNSTATE, 200) ;
}

//-----------------------------------------------------------------------------
//                                                                      DoFlash
//-----------------------------------------------------------------------------
void DoFlash (int iPin, int iDelay)
{
// Turn the LED on (HIGH is the voltage level)
     digitalWrite (iPin, HIGH) ;
     delay (iDelay) ;
// Turn the LED off by making the voltage LOW
     digitalWrite (iPin, LOW) ;
     delay (iDelay) ;
}

//-----------------------------------------------------------------------------
//                                                                  DoStartStop
//-----------------------------------------------------------------------------
void DoStartStop (void)
{
     int       iButton ;

// Wait for Start/Stop button to be released
     while (true)
     {
          iButton = digitalRead (PIN_BTN_STOPSTART) ;
          if (iButton == HIGH)
               break ;
     }
// Toggle run flag
     gbfLogging = ! gbfLogging ;
// Start/stop logging
     ActivateLogging (gbfLogging) ;
}

//-----------------------------------------------------------------------------
//                                                              ActivateLogging
//-----------------------------------------------------------------------------
void ActivateLogging (BOOL bfEnable)
{
     if (bfEnable)
     {
     // Open log file
          if (! OpenLogFile ("datalog.txt"))
               UnitFailed ("* FAILED TO CREATE/OPEN LOG FILE") ;
     // Report new state
          Serial.println ("DATA LOGGING STARTED") ;
          digitalWrite (PIN_LED_RUNSTATE, HIGH) ;
          goto exit_function ;
     }
// Switch off 'running' LED
     digitalWrite (PIN_LED_RUNSTATE, LOW) ;
// Ignore if first time through
     if (gbfLogging == INVALID)
     {
          Serial.println ("WAITING FOR STOP/START BUTTON") ;
          goto exit_function ;
     }
// Report new state
     Serial.println ("DATA LOGGING STOPPED") ;
// Close log file
     g_logfile.close () ;

exit_function:
// Save logging state
     gbfLogging = bfEnable ;
}

//-----------------------------------------------------------------------------
//                                                                  OpenLogFile
//-----------------------------------------------------------------------------
BOOL OpenLogFile (IN char * cpFilespec)
{
     BOOL      bfAppend ;
     DWORD     dwSize ;

// Check if file already exists
     bfAppend = SD.exists (cpFilespec) ;
// Open log file for output
     g_logfile = SD.open (cpFilespec, FILE_WRITE) ;
// Check if failed to create file
     if (! g_logfile)
          return false ;
// Move file pointer to EOF
     if (bfAppend)
     {
          dwSize = g_logfile.size () ;
          g_logfile.seek (dwSize) ;
          return true ;
     }
// Write header to new file
     g_logfile.println ("DATE,TIME,SENSOR-1,SENSOR-2,SENSOR-3") ;
     return true ;
}

//*****************************************************************************
//                                                         Further Developments
//*****************************************************************************
/*
 *   Slave setup() & loop() code
 *

#include <Wire.h>

void setup ()
{
// Join I2C bus with address #4
     Wire.begin (4) ;
// Register event
     Wire.onReceive (receiveEvent) ;
// Start serial for output
     Serial.begin (9600) ;
}

void loop ()
{
     delay (100) ;
}

void receiveEvent(int howMany)
{
     char      cChar ;
     int       iChar ;

// loop through all but the last
     while (1 < Wire.available ())
     {
     // receive byte as a character
          cChar = Wire.receive () ;
     // print the character
          Serial.print (cChar) ;
     }
// receive byte as an integer
     iChar = Wire.receive () ;
// print the integer
     Serial.println (iChar) ;
}

 *
 *   Master setup() & loop() code
 *

#include <Wire.h>

int  giCount ;

void setup ()
{
// Init globals
     giCount = 1 ;
// Join i2c bus (address optional for master)
     Wire.begin () ;
}

void loop ()
{
// Transmit to device #4
     Wire.beginTransmission (4) ;
// Report counter
     Wire.send ("giCount is ") ;
     Wire.send (giCount) ;
// Stop transmitting
     Wire.endTransmission () ;
// Increment counter to prove things are changing
     giCount ++ ;
     delay (500) ;
}

*/
