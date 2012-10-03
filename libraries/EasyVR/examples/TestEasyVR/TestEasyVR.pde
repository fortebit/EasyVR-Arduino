/**
  EasyVR Tester
  
  Dump contents of attached EasyVR module
  and exercise it with playback and recognition.
  
  Serial monitor can be used to send a few basic commands:
  'c' - cycles through available command groups
  'b' - cycles through built-in word sets
  's123.' - play back sound 123 if available (or beep)
  
  With EasyVR Shield, the green LED is ON while the module
  is listening (using pin IO1 of EasyVR).
  Successful recognition is acknowledged with a beep.
  Details are displayed on the serial monitor window.

**
  Example code for the EasyVR library v1.0
  Written in 2011 by RoboTech srl for VeeaR <http:://www.veear.eu> 

  To the extent possible under law, the author(s) have dedicated all
  copyright and related and neighboring rights to this software to the 
  public domain worldwide. This software is distributed without any warranty.

  You should have received a copy of the CC0 Public Domain Dedication
  along with this software.
  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #include "SoftwareSerial.h"
  SoftwareSerial port(12,13);
#else // Arduino 0022 - use modified NewSoftSerial
  #include "WProgram.h"
  #include "NewSoftSerial.h"
  NewSoftSerial port(12,13);
#endif

#include "EasyVR.h"

EasyVR easyvr(port);

int8_t set = 0;
int8_t group = 0;
uint32_t mask = 0;  
uint8_t train = 0;
char name[32];
bool useCommands = true;

EasyVRBridge bridge;

void setup()
{
  // bridge mode?
  if (bridge.check())
  {
    cli();
    bridge.loop(0, 1, 12, 13);
  }
  // run normally
  Serial.begin(9600);
  port.begin(9600);

  if (!easyvr.detect())
  {
    Serial.println("EasyVR not detected!");
    for (;;);
  }

  easyvr.setPinOutput(EasyVR::IO1, LOW);
  Serial.println("EasyVR detected!");
  easyvr.setTimeout(5);
  easyvr.setLanguage(EasyVR::ITALIAN);
  
  int16_t count = 0;
  
  Serial.print("Sound table: ");
  if (easyvr.dumpSoundTable(name, count))
  {
    Serial.println(name);
    Serial.print("Sound entries: ");
    Serial.println(count);
  }
  else
    Serial.println("n/a");
  
  if (easyvr.getGroupMask(mask))
  {
    uint32_t msk = mask;  
    for (group = 0; group <= EasyVR::PASSWORD; ++group, msk >>= 1)
    {
      if (!(msk & 1)) continue;
      if (group == EasyVR::TRIGGER)
        Serial.print("Trigger: ");
      else if (group == EasyVR::PASSWORD)
        Serial.print("Password: ");
      else
      {
        Serial.print("Group ");
        Serial.print(group);
        Serial.print(": ");
      }
      count = easyvr.getCommandCount(group);
      Serial.println(count);
      for (int8_t idx = 0; idx < count; ++idx)
      {
        if (easyvr.dumpCommand(group, idx, name, train))
        {
          Serial.print(idx);
          Serial.print(" = ");
          Serial.print(name);
          Serial.print(", Trained ");
          Serial.print(train, DEC);
          if (!easyvr.isConflict())
            Serial.println(" times, OK");
          else
          {
            int8_t confl = easyvr.getWord();
            if (confl >= 0)
              Serial.print(" times, Similar to Word ");
            else
            {
              confl = easyvr.getCommand();
              Serial.print(" times, Similar to Command ");
            }
            Serial.println(confl);
          }
        }
      }
    }
  }
  group = 0;
  mask |= 1; // force to use trigger
  useCommands = (mask != 1);
}

const char* ws0[] =
{
  "ROBOT",
};
const char* ws1[] =
{
  "ACTION",
  "MOVE",
  "TURN",
  "RUN",
  "LOOK",
  "ATTACK",
  "STOP",
  "HELLO",
};
const char* ws2[] =
{
  "LEFT",
  "RIGHT",
  "UP",
  "DOWN",
  "FORWARD",
  "BACKWARD",
};
const char* ws3[] =
{
  "ZERO",
  "ONE",
  "TWO",
  "THREE",
  "FOUR",
  "FIVE",
  "SIX",
  "SEVEN",
  "EIGHT",
  "NINE",
  "TEN",
};
const char** ws[] = { ws0, ws1, ws2, ws3 };

bool checkMonitorInput()
{
  if (Serial.available() <= 0)
    return false;
  
  // check console commands
  int16_t rx = Serial.read();
  if (rx == 'b')
  {
    useCommands = false;
    set++;
    if (set > 3)
      set = 0;
  }
  if (rx == 'c')
  {
    useCommands = true;
    do
    {
      group++;
      if (group > EasyVR::PASSWORD)
        group = 0;
    } while (!((mask >> group) & 1));
  }
  if (rx == 's')
  {
    int16_t num = 0;
    delay(5);
    while ((rx = Serial.read()) >= 0)
    {
      if (isdigit(rx))
        num = num * 10 + (rx - '0');
      else
        break;
      delay(5);
    }
    if (rx == '.')
    {
      easyvr.stop();
      easyvr.playSound(num, EasyVR::VOL_DOUBLE);
    }
  }
  if (rx >= 0)
  {
    easyvr.stop();
    Serial.flush();
    return true;
  }
  return false;
}

void loop()
{
  checkMonitorInput();
  
  easyvr.setPinOutput(EasyVR::IO1, HIGH); // LED on (listening)
  if (useCommands)
  {
    Serial.print("Say a command in Group ");
    Serial.println(group);
    easyvr.recognizeCommand(group);
  }
  else
  {
    Serial.print("Say a word in Wordset ");
    Serial.println(set);
    easyvr.recognizeWord(set);
  }

  do
  {
    if (checkMonitorInput())
      return;
  }
  while (!easyvr.hasFinished());
  
  easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off

  int16_t idx = easyvr.getWord();
  if (idx >= 0)
  {
    Serial.print("Word: ");
    Serial.print(easyvr.getWord());
    Serial.print(" = ");
    if (useCommands)
      Serial.println(ws[group][idx]);
    else
      Serial.println(ws[set][idx]);
    // ok, let's try another set
    set++;
    if (set > 3)
      set = 0;
    easyvr.playSound(0, EasyVR::VOL_FULL);
  }
  else
  {
    idx = easyvr.getCommand();
    if (idx >= 0)
    {
      Serial.print("Command: ");
      Serial.print(easyvr.getCommand());
      if (easyvr.dumpCommand(group, idx, name, train))
      {
        Serial.print(" = ");
        Serial.println(name);
      }
      else
        Serial.println();
      // ok, let's try another group
      do
      {
        group++;
        if (group > EasyVR::PASSWORD)
          group = 0;
      } while (!((mask >> group) & 1));
      easyvr.playSound(0, EasyVR::VOL_FULL);
    }
    else // errors or timeout
    {
      if (easyvr.isTimeout())
        Serial.println("Timed out, try again...");
      int16_t err = easyvr.getError();
      if (err >= 0)
      {
        Serial.print("Error ");
        Serial.println(err, HEX);
      }
    }
  }
}


