/**
  EasyVR Tester

  Dump contents of attached EasyVR module
  and exercise it with playback and recognition.

  Serial monitor can be used to send a few basic commands:
  '?' - display the module setup
  'l' - cycles through available languages
  'c' - cycles through available command groups
  'b' - cycles through built-in word sets
  'g' - cycles through custom grammars
  's123' - play back sound 123 if available (or beep)
  'd0123456789ABCD*#' - dials the specified number ('_' is dial tone)
  'k' - starts detection of tokens
  '4' or '8' - sets token length to 4 or 8 bits
  'n123' - play back token 123 (not checked for validity)
  'm1' - set mic distance to HEADSET
  'm2' - set mic distance to ARMS_LENGTH (default)
  'm3' - set mic distance to FAR_MIC
  'w' - enter sleep mode without audio wakeup (any command interrupts)
  'ww' - enter sleep mode with "whistle" wakeup
  'w2' - enter sleep mode with "double-clap" wakeup
  'w3' - enter sleep mode with "triple-clap" wakeup
  'wl' - enter sleep mode with "loud-sound" wakeup

  With EasyVR Shield, the green LED is ON while the module
  is listening (using pin IO1 of EasyVR).
  Successful recognition is acknowledged with a beep.
  Details are displayed on the serial monitor window.

**
  Example code for the EasyVR library v1.5
  Written in 2014 by RoboTech srl for VeeaR <http:://www.veear.eu>

  To the extent possible under law, the author(s) have dedicated all
  copyright and related and neighboring rights to this software to the
  public domain worldwide. This software is distributed without any warranty.

  You should have received a copy of the CC0 Public Domain Dedication
  along with this software.
  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #include "Platform.h"
  #include "SoftwareSerial.h"
#ifndef CDC_ENABLED
  // Shield Jumper on SW
  SoftwareSerial port(12,13);
#else
  // Shield Jumper on HW (for Leonardo)
  #define port Serial1
#endif
#else
  #error "Arduino version not supported. Please update your IDE."
#endif

#include "EasyVR.h"

EasyVR easyvr(port);

int8_t bits = 4;
int8_t set = 0;
int8_t group = 0;
uint32_t mask = 0;
uint8_t train = 0;
uint8_t grammars = 0;
int8_t lang = 0;
char name[33];
bool useCommands = true;
bool useTokens = false;
bool isSleeping = false;

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
  Serial.begin(9600);
  Serial.println(F("---"));
  Serial.println(F("Bridge not started!"));
#else
  // bridge mode?
  if (bridge.check())
  {
    port.begin(9600);
    bridge.loop(port);
  }
  Serial.println(F("---"));
  Serial.println(F("Bridge connection aborted!"));
#endif
  port.begin(9600);

  while (!easyvr.detect())
  {
    Serial.println(F("EasyVR not detected!"));
    delay(1000);
  }

  easyvr.setPinOutput(EasyVR::IO1, LOW);
  Serial.print(F("EasyVR detected, version "));
  Serial.println(easyvr.getID());
  easyvr.setTimeout(5);
  lang = EasyVR::ENGLISH;
  easyvr.setLanguage(lang);

  int16_t count = 0;

  Serial.print(F("Sound table: "));
  if (easyvr.dumpSoundTable(name, count))
  {
    Serial.println(name);
    Serial.print(F("Sound entries: "));
    Serial.println(count);
  }
  else
    Serial.println(F("n/a"));

  Serial.print(F("Custom Grammars: "));
  grammars = easyvr.getGrammarsCount();
  if (grammars > 4)
  {
    Serial.println(grammars - 4);
    for (set = 4; set < grammars; ++set)
    {
      Serial.print(F("Grammar "));
      Serial.print(set);

      uint8_t flags, num;
      if (easyvr.dumpGrammar(set, flags, num))
      {
        Serial.print(F(" has "));
        Serial.print(num);
        if (flags & EasyVR::GF_TRIGGER)
          Serial.println(F(" trigger"));
        else
          Serial.println(F(" command(s)"));
      }
      else
        Serial.println(F(" error"));

      for (int8_t idx = 0; idx < num; ++idx)
      {
        Serial.print(idx);
        Serial.print(F(" = "));
        if (!easyvr.getNextWordLabel(name))
          break;
        Serial.println(name);
      }
    }
  }
  else
    Serial.println(F("n/a"));

  if (easyvr.getGroupMask(mask))
  {
    uint32_t msk = mask;
    for (group = 0; group <= EasyVR::PASSWORD; ++group, msk >>= 1)
    {
      if (!(msk & 1)) continue;
      if (group == EasyVR::TRIGGER)
        Serial.print(F("Trigger: "));
      else if (group == EasyVR::PASSWORD)
        Serial.print(F("Password: "));
      else
      {
        Serial.print(F("Group "));
        Serial.print(group);
        Serial.print(F(" has "));
      }
      count = easyvr.getCommandCount(group);
      Serial.print(count);
      if (group == 0)
        Serial.println(F(" trigger(s)"));
      else
        Serial.println(F(" command(s)"));
      for (int8_t idx = 0; idx < count; ++idx)
      {
        if (easyvr.dumpCommand(group, idx, name, train))
        {
          Serial.print(idx);
          Serial.print(F(" = "));
          Serial.print(name);
          Serial.print(F(", Trained "));
          Serial.print(train, DEC);
          if (!easyvr.isConflict())
            Serial.println(F(" times, OK"));
          else
          {
            int8_t confl = easyvr.getWord();
            if (confl >= 0)
              Serial.print(F(" times, Similar to Word "));
            else
            {
              confl = easyvr.getCommand();
              Serial.print(F(" times, Similar to Command "));
            }
            Serial.println(confl);
          }
        }
      }
    }
  }
  group = 0;
  useCommands = (mask != 0);
  mask |= 1; // force to use trigger
  isSleeping = false;
  Serial.println(F("---"));
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
  if (rx == '?')
  {
    setup();
    return true;
  }
  if (isSleeping)
  {
    // any character received will exit sleep
    isSleeping = false;
    easyvr.stop();
    Serial.println(F("Forced wake-up!"));
    return true;
  }
  if (rx == 'l')
  {
    easyvr.stop();
    lang++;
    if (easyvr.setLanguage(lang) || easyvr.setLanguage(lang = 0))
    {
      Serial.print(F("Language set to "));
      Serial.println(lang);
    }
    else
      Serial.println(F("Error while setting language!"));
  }
  if (rx == 'b')
  {
    useTokens = false;
    useCommands = false;
    set++;
    if (set > 3)
      set = 0;
  }
  if (rx == 'g' && grammars > 4)
  {
    useTokens = false;
    useCommands = false;
    set++;
    if (set >= grammars)
      set = 4;
  }
  if (rx == 'c')
  {
    useTokens = false;
    useCommands = true;
    do
    {
      group++;
      if (group > EasyVR::PASSWORD)
        group = 0;
    }
    while (!((mask >> group) & 1));
  }
  if (rx == 'k')
  {
    useTokens = true;
  }
  if (rx == '4')
  {
    bits = 4;
  }
  if (rx == '8')
  {
    bits = 8;
  }
  if (rx == 'n')
  {
    int16_t num = 0;
    delay(5);
    while ((rx = Serial.read()) >= 0)
    {
      delay(5);
      if (isdigit(rx))
        num = num * 10 + (rx - '0');
      else
        break;
    }
    Serial.print(F("Play token "));
    Serial.println(num);
    easyvr.stop();
    easyvr.sendToken(bits, num);
  }
  if (rx == 's')
  {
    int16_t num = 0;
    delay(5);
    while ((rx = Serial.read()) >= 0)
    {
      delay(5);
      if (isdigit(rx))
        num = num * 10 + (rx - '0');
      else
        break;
    }
    Serial.print(F("Play sound "));
    Serial.println(num);
    easyvr.stop();
    easyvr.playSound(num, EasyVR::VOL_DOUBLE);
  }
  if (rx == 'd')
  {
    easyvr.stop();
    Serial.println(F("Play tones:"));
    int16_t num = 0;
    delay(5);
    while ((rx = Serial.read()) >= 0)
    {
      delay(5);
      if (isdigit(rx))
        num = rx - '0';
      else if (rx == '*')
        num = 10;
      else if (rx == '#')
        num = 11;
      else if (rx >= 'A' && rx <= 'D')
        num = rx - 'A';
      else if (rx == '_')
        num = -1;
      else
        break;
      Serial.print(num);
      if (easyvr.playPhoneTone(num, 3))
        Serial.println(F(" OK"));
      else
        Serial.println(F(" ERR"));
    }
  }
  if (rx == 'm')
  {
    int16_t num = 0;
    delay(5);
    while ((rx = Serial.read()) >= 0)
    {
      delay(5);
      if (isdigit(rx))
        num = num * 10 + (rx - '0');
      else
        break;
    }
    Serial.print(F("Mic distance "));
    Serial.println(num);
    easyvr.stop();
    easyvr.setMicDistance(num);
  }
  if (rx == 'w')
  {
    int8_t mode = 0;
    delay(5);
    while ((rx = Serial.read()) >= 0)
    {
      delay(5);
      if (rx == 'w')
        mode = EasyVR::WAKE_ON_WHISTLE;
      if (rx == '2')
        mode = EasyVR::WAKE_ON_2CLAPS;
      if (rx == '3')
        mode = EasyVR::WAKE_ON_3CLAPS;
      if (rx == 'l')
        mode = EasyVR::WAKE_ON_LOUDSOUND;
    }
    Serial.print(F("Sleep mode "));
    Serial.println(mode);
    easyvr.stop();
    easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off
    isSleeping = easyvr.sleep(mode);
    return true;
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

  if (!isSleeping)
  {
    easyvr.setPinOutput(EasyVR::IO1, HIGH); // LED on (listening)
    if (useTokens)
    {
      Serial.print(F("Detect a "));
      Serial.print(bits);
      Serial.println(F(" bit token ..."));
      easyvr.detectToken(bits, EasyVR::REJECTION_AVG, 0);
    }
    else if (useCommands)
    {
      Serial.print(F("Say a command in Group "));
      Serial.println(group);
      easyvr.recognizeCommand(group);
    }
    else
    {
      Serial.print(F("Say a word in Wordset "));
      Serial.println(set);
      easyvr.recognizeWord(set);
    }
  }
  do
  {
    if (checkMonitorInput())
      return;
  }
  while (!easyvr.hasFinished());
  isSleeping = false;

  easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off

  if (easyvr.isAwakened())
  {
    Serial.println(F("Audio wake-up!"));
    return;
  }

  int16_t idx;
  if (useTokens)
  {
    idx = easyvr.getToken();
    if (idx >= 0)
    {
      Serial.print(F("Token: "));
      Serial.println(idx);
      easyvr.playSound(0, EasyVR::VOL_FULL);
    }
  }
  // handle voice recognition
  idx = easyvr.getWord();
  if (idx >= 0)
  {
    Serial.print(F("Word: "));
    Serial.print(easyvr.getWord());
    Serial.print(F(" = "));
    if (useCommands)
      Serial.println(ws[group][idx]);
    // --- optional: builtin words can be retrieved from the module
    else if (set < 4)
      Serial.println(ws[set][idx]);
    // ---
    else
    {
      uint8_t flags, num;
      if (easyvr.dumpGrammar(set, flags, num))
        while (idx-- >= 0)
        {
          if (!easyvr.getNextWordLabel(name))
            break;
        }
      if (idx < 0)
        Serial.println(name);
      else
        Serial.println();
    }
    // ok, let's try another set
    if (set < 4)
    {
      set++;
      if (set > 3)
        set = 0;
    }
    else
    {
      set++;
      if (set >= grammars)
        set = 4;
    }
    easyvr.playSound(0, EasyVR::VOL_FULL);
  }
  else
  {
    idx = easyvr.getCommand();
    if (idx >= 0)
    {
      Serial.print(F("Command: "));
      Serial.print(easyvr.getCommand());
      if (easyvr.dumpCommand(group, idx, name, train))
      {
        Serial.print(F(" = "));
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
      }
      while (!((mask >> group) & 1));
      easyvr.playSound(0, EasyVR::VOL_FULL);
    }
    else // errors or timeout
    {
      if (easyvr.isTimeout())
        Serial.println(F("Timed out, try again..."));
      int16_t err = easyvr.getError();
      if (err >= 0)
      {
        Serial.print(F("Error 0x"));
        Serial.println(err, HEX);
      }
    }
  }
}


