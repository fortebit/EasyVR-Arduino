/**
  Example code for the EasyVR library v1.10.1
  Written in 2017 by RoboTech srl for VeeaR <http:://www.veear.eu>

  To the extent possible under law, the author(s) have dedicated all
  copyright and related and neighboring rights to this software to the
  public domain worldwide. This software is distributed without any warranty.

  You should have received a copy of the CC0 Public Domain Dedication
  along with this software.
  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include "Arduino.h"
#if !defined(SERIAL_PORT_MONITOR)
  #error "Arduino version not supported. Please update your IDE to the latest version."
#endif

#if defined(__SAMD21G18A__)
  // Shield Jumper on HW (for Zero, use Programming Port)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_MONITOR
#elif defined(SERIAL_PORT_USBVIRTUAL)
  // Shield Jumper on HW (for Leonardo and Due)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_USBVIRTUAL
#else
  // Shield Jumper on SW (using pins 12/13 or 8/9 as RX/TX)
  #include "SoftwareSerial.h"
  SoftwareSerial port(12, 13);
  #define pcSerial SERIAL_PORT_MONITOR
#endif

// PLEASE NOTE:
// Using SoftwareSerial with Servo library can produce glitches in servo position update!
// --------------------------------------------------------------------------------------

#include "EasyVR.h"
EasyVR easyvr(port);

void setup()
{
  // setup PC serial port
  pcSerial.begin(9600);
bridge:
  // bridge mode?
  int mode = easyvr.bridgeRequested(pcSerial);
  switch (mode)
  {
  case EasyVR::BRIDGE_NONE:
    // setup EasyVR serial port
    port.begin(9600);
    // run normally
    pcSerial.println(F("Bridge not requested, run normally"));
    pcSerial.println(F("---"));
    break;
    
  case EasyVR::BRIDGE_NORMAL:
    // setup EasyVR serial port (low speed)
    port.begin(9600);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("Bridge connection aborted"));
    pcSerial.println(F("---"));
    break;
    
  case EasyVR::BRIDGE_BOOT:
    // setup EasyVR serial port (high speed)
    port.begin(115200);
    pcSerial.end();
    pcSerial.begin(115200);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("Bridge connection aborted"));
    pcSerial.println(F("---"));
    break;
  }

  // initialize EasyVR  
  while (!easyvr.detect())
  {
    pcSerial.println(F("EasyVR not detected!"));
    for (int i = 0; i < 10; ++i)
    {
      if (pcSerial.read() == EasyVR::BRIDGE_ESCAPE_CHAR)
        goto bridge;
      delay(100);
    }
  }

  pcSerial.print(F("EasyVR detected, version "));
  pcSerial.println(easyvr.getID());
  
  if (easyvr.getID() < EasyVR::EASYVR3_4)
  {
    pcSerial.println(F("Update firmware to use Import-Export!"));
    for(;;);
  }

  easyvr.setDelay(0); // speed-up replies when exporting commands

  pcSerial.println(F("Import/Export example, using the same format as the EasyVR Commander"));
  pcSerial.println(F("---"));
}

void loop()
{
  pcSerial.println(F("Type 'i' to import or 'e' to export all custom commands:"));
  while (!pcSerial.available());
  int rx = pcSerial.read();
  if (rx == 'e')
  {  
    exportAll();
  }
  else if (rx == 'i')
  {  
    importAll();
  }
  else if (rx == EasyVR::BRIDGE_ESCAPE_CHAR)
  {
    setup();
    return;
  }
}

uint8_t data[258];

void exportAll()
{
  pcSerial.println(F("Exporting commands..."));
  for (int8_t group = 0; group <= EasyVR::PASSWORD; ++group)
  {
    pcSerial.print(F("Group "));
    pcSerial.print(group);
    pcSerial.print(F(" : "));
    int8_t count = easyvr.getCommandCount(group);
    pcSerial.println(count);

    for (int8_t idx = 0; idx < count; ++idx)
    {
      if (easyvr.exportCommand(group, idx, data))
      {
        pcSerial.print(F("Command "));
        pcSerial.print(idx);
        pcSerial.print(F(" : "));

        for (int i = 0; i < 258; ++i)
        {
          pcSerial.print((data[i] >> 4) & 0x0F, HEX);
          pcSerial.print((data[i] & 0x0F), HEX);
        }
        pcSerial.println();
      }
      else
      {
        pcSerial.println(F("Error during command export, aborting."));
        break;
      }
    }
  }
  pcSerial.println(F("Done!"));
}

#define IMPORT_TIMEOUT 10000

void importAll()
{
  pcSerial.println(F("Importing commands... (10s timeout)"));
  easyvr.resetCommands();
  
  String s;
  int8_t count = 0;
  int8_t group = 0;
  pcSerial.setTimeout(IMPORT_TIMEOUT);
  for (;;)
  {
    long t = millis();
    while (!pcSerial.available() && millis() - t < IMPORT_TIMEOUT);
    if (!pcSerial.available())
    {
      pcSerial.println(F("Timed out."));
      break;
    }

    s = pcSerial.readStringUntil(' ');
    s.trim();
    if (s.endsWith(F("Group")))
    {
      group = pcSerial.parseInt();
      s = pcSerial.readStringUntil(':');
      count = pcSerial.parseInt();

      pcSerial.print(F("Creating "));
      pcSerial.print(count);
      pcSerial.print(F(" commands in group "));
      pcSerial.println(group);

      for (int8_t idx = 0; idx < count; ++idx)
        easyvr.addCommand(group, idx);
    }
    else if (s.endsWith(F("Command")))
    {
      int8_t idx = pcSerial.parseInt();
      s = pcSerial.readStringUntil(':');
      
      pcSerial.print(F("Importing command "));
      pcSerial.print(idx);
      pcSerial.print(F(" with data: "));

      int rx = 0, i = 0;
      for (i = 0; i < 258 * 2; )
      {
        long t = millis();
        do {
          rx = pcSerial.peek();
        } while (rx < 0 && millis() - t < IMPORT_TIMEOUT);
        if (rx < 0)
          break;
        if (rx == '\r' && rx == '\n')
          break;
        if (rx == ' ' || rx == '\t')
        {
          pcSerial.read(); // discard/ignore
          continue;
        }
        rx = pcSerial.read();
        if (rx >= '0' && rx <= '9')
          rx -= '0';
        else if (rx >= 'A' && rx <= 'F')
          rx = rx - 'A' + 10;
        else
          break;
        // fill buffer
        if (i & 1)
          data[i >> 1] |= (rx & 0x0F);
        else
          data[i >> 1] = (rx << 4) & 0xF0;
        ++i;
        //pcSerial.print(rx, HEX);
      } 
      if (i != 258*2)
      {
        pcSerial.println(F("Error parsing command data, aborting."));
        break;
      }
      
      for (int i = 0; i < 258; ++i)
      {
        pcSerial.print((data[i] >> 4) & 0x0F, HEX);
        pcSerial.print((data[i] & 0x0F), HEX);
      }
      pcSerial.println();
      
      if (easyvr.importCommand(group, idx, data))
      {
        easyvr.verifyCommand(group, idx);
        while (!easyvr.hasFinished());
        if (!easyvr.isInvalid())
        {
          pcSerial.print(F(" done,"));
          if (easyvr.getCommand() >= 0)
          {
            pcSerial.print(F(" conflict with command "));
            pcSerial.println(easyvr.getCommand());
          }
          else if (easyvr.getWord() >= 0)
          {
            pcSerial.print(F(" conflict with word "));
            pcSerial.println(easyvr.getWord());
          }
          else if (easyvr.getError() >= 0)
          {
            pcSerial.print(F(" error "));
            pcSerial.println(easyvr.getError());
          }
          else
            pcSerial.println(F(" ok."));
        }
        else
          pcSerial.println(F(" failed."));
      }
      else
      {
        pcSerial.println();
        pcSerial.println(F("Error during command import, aborting."));
        break;
      }
    }
  }
  pcSerial.println(F("Done!"));
}

