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
  Example code for the EasyVR library v1.6
  Written in 2014 by RoboTech srl for VeeaR <http:://www.veear.eu>

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

#if defined(SERIAL_PORT_USBVIRTUAL)
  // Shield Jumper on HW (for Leonardo and Due)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_USBVIRTUAL
#else
  // Shield Jumper on SW (using pins 12/13 or 8/9 as RX/TX)
  #include "SoftwareSerial.h"
  SoftwareSerial port(12, 13);
  #define pcSerial SERIAL_PORT_MONITOR
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

void setup()
{
  // setup PC serial port
  pcSerial.begin(9600);

  // bridge mode?
  int mode = easyvr.bridgeRequested(pcSerial);
  switch (mode)
  {
  case EasyVR::BRIDGE_NONE:
    // setup EasyVR serial port
    port.begin(9600);
    // run normally
    pcSerial.println(F("---"));
    pcSerial.println(F("Bridge not started!"));
    break;
    
  case EasyVR::BRIDGE_NORMAL:
    // setup EasyVR serial port (low speed)
    port.begin(9600);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("---"));
    pcSerial.println(F("Bridge connection aborted!"));
    break;
    
  case EasyVR::BRIDGE_BOOT:
    // setup EasyVR serial port (high speed)
    port.begin(115200);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("---"));
    pcSerial.println(F("Bridge connection aborted!"));
    break;
  }

  // initialize EasyVR  
  while (!easyvr.detect())
  {
    pcSerial.println(F("EasyVR not detected!"));
    delay(1000);
  }

  easyvr.setPinOutput(EasyVR::IO1, LOW);
  pcSerial.print(F("EasyVR detected, version "));
  pcSerial.println(easyvr.getID());
  easyvr.setTimeout(5);
  lang = EasyVR::ENGLISH;
  easyvr.setLanguage(lang);

  int16_t count = 0;

  pcSerial.print(F("Sound table: "));
  if (easyvr.dumpSoundTable(name, count))
  {
    pcSerial.println(name);
    pcSerial.print(F("Sound entries: "));
    pcSerial.println(count);
  }
  else
    pcSerial.println(F("n/a"));

  pcSerial.print(F("Custom Grammars: "));
  grammars = easyvr.getGrammarsCount();
  if (grammars > 4)
  {
    pcSerial.println(grammars - 4);
    for (set = 4; set < grammars; ++set)
    {
      pcSerial.print(F("Grammar "));
      pcSerial.print(set);

      uint8_t flags, num;
      if (easyvr.dumpGrammar(set, flags, num))
      {
        pcSerial.print(F(" has "));
        pcSerial.print(num);
        if (flags & EasyVR::GF_TRIGGER)
          pcSerial.println(F(" trigger"));
        else
          pcSerial.println(F(" command(s)"));
      }
      else
        pcSerial.println(F(" error"));

      for (int8_t idx = 0; idx < num; ++idx)
      {
        pcSerial.print(idx);
        pcSerial.print(F(" = "));
        if (!easyvr.getNextWordLabel(name))
          break;
        pcSerial.println(name);
      }
    }
  }
  else
    pcSerial.println(F("n/a"));

  if (easyvr.getGroupMask(mask))
  {
    uint32_t msk = mask;
    for (group = 0; group <= EasyVR::PASSWORD; ++group, msk >>= 1)
    {
      if (!(msk & 1)) continue;
      if (group == EasyVR::TRIGGER)
        pcSerial.print(F("Trigger: "));
      else if (group == EasyVR::PASSWORD)
        pcSerial.print(F("Password: "));
      else
      {
        pcSerial.print(F("Group "));
        pcSerial.print(group);
        pcSerial.print(F(" has "));
      }
      count = easyvr.getCommandCount(group);
      pcSerial.print(count);
      if (group == 0)
        pcSerial.println(F(" trigger(s)"));
      else
        pcSerial.println(F(" command(s)"));
      for (int8_t idx = 0; idx < count; ++idx)
      {
        if (easyvr.dumpCommand(group, idx, name, train))
        {
          pcSerial.print(idx);
          pcSerial.print(F(" = "));
          pcSerial.print(name);
          pcSerial.print(F(", Trained "));
          pcSerial.print(train, DEC);
          if (!easyvr.isConflict())
            pcSerial.println(F(" times, OK"));
          else
          {
            int8_t confl = easyvr.getWord();
            if (confl >= 0)
              pcSerial.print(F(" times, Similar to Word "));
            else
            {
              confl = easyvr.getCommand();
              pcSerial.print(F(" times, Similar to Command "));
            }
            pcSerial.println(confl);
          }
        }
      }
    }
  }
  group = 0;
  useCommands = (mask != 0);
  mask |= 1; // force to use trigger
  isSleeping = false;
  pcSerial.println(F("---"));
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
  if (pcSerial.available() <= 0)
    return false;

  // check console commands
  int16_t rx = pcSerial.read();
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
    pcSerial.println(F("Forced wake-up!"));
    return true;
  }
  if (rx == 'l')
  {
    easyvr.stop();
    lang++;
    if (easyvr.setLanguage(lang) || easyvr.setLanguage(lang = 0))
    {
      pcSerial.print(F("Language set to "));
      pcSerial.println(lang);
    }
    else
      pcSerial.println(F("Error while setting language!"));
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
    while ((rx = pcSerial.read()) >= 0)
    {
      delay(5);
      if (isdigit(rx))
        num = num * 10 + (rx - '0');
      else
        break;
    }
    pcSerial.print(F("Play token "));
    pcSerial.println(num);
    easyvr.stop();
    easyvr.sendToken(bits, num);
  }
  if (rx == 's')
  {
    int16_t num = 0;
    delay(5);
    while ((rx = pcSerial.read()) >= 0)
    {
      delay(5);
      if (isdigit(rx))
        num = num * 10 + (rx - '0');
      else
        break;
    }
    pcSerial.print(F("Play sound "));
    pcSerial.println(num);
    easyvr.stop();
    easyvr.playSound(num, EasyVR::VOL_DOUBLE);
  }
  if (rx == 'd')
  {
    easyvr.stop();
    pcSerial.println(F("Play tones:"));
    int16_t num = 0;
    delay(5);
    while ((rx = pcSerial.read()) >= 0)
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
      pcSerial.print(num);
      if (easyvr.playPhoneTone(num, 3))
        pcSerial.println(F(" OK"));
      else
        pcSerial.println(F(" ERR"));
    }
  }
  if (rx == 'm')
  {
    int16_t num = 0;
    delay(5);
    while ((rx = pcSerial.read()) >= 0)
    {
      delay(5);
      if (isdigit(rx))
        num = num * 10 + (rx - '0');
      else
        break;
    }
    pcSerial.print(F("Mic distance "));
    pcSerial.println(num);
    easyvr.stop();
    easyvr.setMicDistance(num);
  }
  if (rx == 'w')
  {
    int8_t mode = 0;
    delay(5);
    while ((rx = pcSerial.read()) >= 0)
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
    pcSerial.print(F("Sleep mode "));
    pcSerial.println(mode);
    easyvr.stop();
    easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off
    isSleeping = easyvr.sleep(mode);
    return true;
  }

  if (rx >= 0)
  {
    easyvr.stop();
    pcSerial.flush();
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
      pcSerial.print(F("Detect a "));
      pcSerial.print(bits);
      pcSerial.println(F(" bit token ..."));
      easyvr.detectToken(bits, EasyVR::REJECTION_AVG, 0);
    }
    else if (useCommands)
    {
      pcSerial.print(F("Say a command in Group "));
      pcSerial.println(group);
      easyvr.recognizeCommand(group);
    }
    else
    {
      pcSerial.print(F("Say a word in Wordset "));
      pcSerial.println(set);
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
    pcSerial.println(F("Audio wake-up!"));
    return;
  }

  int16_t idx;
  if (useTokens)
  {
    idx = easyvr.getToken();
    if (idx >= 0)
    {
      pcSerial.print(F("Token: "));
      pcSerial.println(idx);
      easyvr.playSound(0, EasyVR::VOL_FULL);
    }
  }
  // handle voice recognition
  idx = easyvr.getWord();
  if (idx >= 0)
  {
    pcSerial.print(F("Word: "));
    pcSerial.print(easyvr.getWord());
    pcSerial.print(F(" = "));
    if (useCommands)
      pcSerial.println(ws[group][idx]);
    // --- optional: builtin words can be retrieved from the module
    else if (set < 4)
      pcSerial.println(ws[set][idx]);
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
        pcSerial.println(name);
      else
        pcSerial.println();
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
      pcSerial.print(F("Command: "));
      pcSerial.print(easyvr.getCommand());
      if (easyvr.dumpCommand(group, idx, name, train))
      {
        pcSerial.print(F(" = "));
        pcSerial.println(name);
      }
      else
        pcSerial.println();
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
        pcSerial.println(F("Timed out, try again..."));
      int16_t err = easyvr.getError();
      if (err >= 0)
      {
        pcSerial.print(F("Error 0x"));
        pcSerial.println(err, HEX);
      }
    }
  }
}


