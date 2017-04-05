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

#include <Servo.h>
Servo myservo;  // create servo object to control a servo

#define SERVO_PIN 11
#define LIPSYNC_TIMEOUT 0 // in seconds, 0 means no timeout

unsigned long t;

void setup()
{
  myservo.attach(SERVO_PIN);  // attaches the servo

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
    pcSerial.println(F("Update firmware to use Lip-Sync!"));
    for(;;);
  }

  pcSerial.println(F("---"));

  if (!easyvr.realtimeLipsync(EasyVR::RTLS_THRESHOLD_DEF, LIPSYNC_TIMEOUT))
  {
    pcSerial.println(F("Failed to start Lip-Sync!"));
    for(;;);
  }

  t = millis() + 2000; // wait for lipsync to be ready
}

void loop()
{
  // wait for next lipsync value
  // lipsync refreshes every 27ms, but we wait 25ms
  // the next fetch will synchronize
  while (millis() < t)
  {
    // do something else
    delay(1);
    // test interruption
    int rx = pcSerial.read();
    if (rx == '.')
    {
      if (easyvr.stop() || easyvr.stop())
      {
        pcSerial.println("lipsync stopped");
        easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
        for(;;);
      }
    }
    if (rx == EasyVR::BRIDGE_ESCAPE_CHAR)
    {
      setup();
      return;
    }
  }

  // fetch new lipsync value
  int8_t pos = -1;
  if (easyvr.fetchMouthPosition(pos))
  {
    t = millis() + 25;
    
    // map mouth position (0-31) to the pwm range (0-255)
    uint8_t pwm = map(pos, 0, 31, 0, 255);
    analogWrite(A3, pwm);

    // map mouth to servo angle (45-135), adjust based on mechanical setup
    uint16_t angle = map(pos, 0, 31, 45, 135);
    myservo.write(angle);

    // send to PC (can use Serial Plotter to view graphically)
    int amp = map(pos, 0, 31, 0, 100);
    pcSerial.print(amp, DEC);
    pcSerial.print(" , ");
    pcSerial.println(-amp, DEC);
  }
  else
  {
    t = millis() + 25;

    if (easyvr.isTimeout())
    {
      pcSerial.println("lipsync completed");
      easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
      for(;;);
    }
    else
    if (easyvr.getError() >= 0)
    {
      pcSerial.println("lipsync error");
      easyvr.stop();
      easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
      for(;;);
    }
    else
      pcSerial.println("no lipsync");
  }
}
