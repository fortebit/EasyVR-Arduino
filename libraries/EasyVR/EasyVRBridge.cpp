/*
EasyVR library v1.6
Copyright (C) 2014 RoboTech srl

Written for Arduino and compatible boards for use with EasyVR modules or
EasyVR Shield boards produced by VeeaR <www.veear.eu>

Released under the terms of the MIT license, as found in the accompanying
file COPYING.txt or at this address: <http://www.opensource.org/licenses/MIT>
*/

#include "Arduino.h"
#if !defined(SERIAL_PORT_MONITOR)
  #error "Arduino version not supported. Please update your IDE to the latest version."
#endif

#include "EasyVRBridge.h"

#if defined(SERIAL_PORT_USBVIRTUAL)
#define pcSerial SERIAL_PORT_USBVIRTUAL
#else
#define pcSerial SERIAL_PORT_MONITOR
#endif

void EasyVRBridge::loop(Stream& port)
{
  int rx;
  for (;;)
  {
    if (pcSerial.available())
    {
      rx = pcSerial.read();
      if (rx == '?')
        return;
      port.write(rx);
    }
    if (port.available())
      pcSerial.write(port.read());
  }
}

bool EasyVRBridge::check()
{
  // look for a request header
  bool bridge = false;
  int t;
  for (t=0; t<150; ++t)
  {
    delay(10);
    if (pcSerial.available() > 0 && pcSerial.read() == 0xBB)
    {
      pcSerial.write(0xCC);
      delay(1); // flush not reliable on some core libraries
      pcSerial.flush();
      bridge = true;
      break;
    }
  }
  if (bridge)
  {
    // send reply and wait for confirmation
    bridge = false;
    for (t=0; t<50; ++t)
    {
      delay(10);
      if (pcSerial.available() > 0 && pcSerial.read() == 0xDD)
      {
        pcSerial.write(0xEE);
        delay(1); // flush not reliable on some core libraries
        pcSerial.flush();
        bridge = true;
        break;
      }
    }
  }
  return bridge;
}
