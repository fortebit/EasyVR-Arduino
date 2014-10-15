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
#else
  #error "Arduino version not supported. Please update your IDE to the latest version."
#endif

#if defined(CDC_ENABLED)
  // Shield Jumper on HW (for Leonardo and Due)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_USBVIRTUAL
#else
  // Shield Jumper on SW (using pins 12/13 as RX/TX)
  #include "SoftwareSerial.h"
  SoftwareSerial port(12,13);
  #define pcSerial SERIAL_PORT_MONITOR
#endif

#include "EasyVR.h"
#include "EasyVRBridge.h"

EasyVRBridge bridge;

void setup()
{
#ifndef CDC_ENABLED
  // bridge mode?
  if (bridge.check())
  {
    cli();
    bridge.loop(0, 1, 12, 13);
  }
  // run normally
  pcSerial.begin(9600);
  pcSerial.println(F("---"));
  pcSerial.println(F("Bridge not started!"));
#else
  // bridge mode?
  if (bridge.check())
  {
    port.begin(9600);
    bridge.loop(port);
  }
  pcSerial.println(F("---"));
  pcSerial.println(F("Bridge connection aborted!"));
#endif
  port.begin(9600);
}

void loop()
{
}
