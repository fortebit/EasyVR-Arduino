/*
  EasyVR Bridge

  Soft-connects the PC Serial port with the EasyVR module, using Arduino
  like a USB/Serial adapter.

**
  Example code for the EasyVR library v1.6
  Written in 2014 by RoboTech srl for VeeaR <http:://www.veear.eu> 

  To the extent possible under law, the author(s) have dedicated all
  copyright and related and neighboring rights to this software to the 
  public domain worldwide. This software is distributed without any warranty.

  You should have received a copy of the CC0 Public Domain Dedication
  along with this software.
  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#if defined(ARDUINO) && ((ARDUINO >= 106 && ARDUINO < 150) || ARDUINO >= 155)
  #include "Arduino.h"
  #include "Platform.h"
  #include "SoftwareSerial.h"
#else
  #error "Arduino version not supported. Please update your IDE to the latest version."
#endif

#if defined(CDC_ENABLED)
  // Shield Jumper on HW (for Leonardo and Due)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_USBVIRTUAL
#else
  // Shield Jumper on SW (using pins 12/13 or 8/9 as RX/TX)
  #include "SoftwareSerial.h"
  SoftwareSerial port(12,13);
  #define pcSerial SERIAL_PORT_MONITOR
#endif

#include "EasyVR.h"
#include "EasyVRBridge.h"

EasyVRBridge bridge;

void setup()
{
  // setup serial ports
  port.begin(9600);
  pcSerial.begin(9600);

  // bridge mode?
  if (bridge.check())
  {
    // soft-connect the two serial ports (PC and EasyVR)
    bridge.loop(port);
    // resume normally if aborted
    pcSerial.println(F("---"));
    pcSerial.println(F("Bridge connection aborted!"));
  }
  else
  {
    // run normally
    pcSerial.println(F("---"));
    pcSerial.println(F("Bridge not started!"));
  }
}

void loop()
{
}
