/*
EasyVR library v1.10.1
Copyright (C) 2016 RoboTech srl

Written for Arduino and compatible boards for use with EasyVR modules or
EasyVR Shield boards produced by VeeaR <www.veear.eu>

Released under the terms of the MIT license, as found in the accompanying
file COPYING.txt or at this address: <http://www.opensource.org/licenses/MIT>
*/

#include "Arduino.h"
#include "EasyVR.h"
#include "internal/protocol.h"

/*****************************************************************************/

int EASYVR_RX_TIMEOUT = 100;
int EASYVR_STORAGE_TIMEOUT = 500;
int EASYVR_WAKE_TIMEOUT = 200;
int EASYVR_PLAY_TIMEOUT = 5000;
int EASYVR_TOKEN_TIMEOUT = 1500;

void EasyVR::send(uint8_t c)
{
  delay(1);
  _s->write(c);
}

void EasyVR::sendCmd(uint8_t c)
{
  _s->flush();
  while (_s->available() > 0) _s->read();
  send(c);
}

void EasyVR::sendArg(int8_t c)
{
  send(c + ARG_ZERO);
}

inline void EasyVR::sendGroup(int8_t c)
{
  send(c + ARG_ZERO);
  if (c != _group)
  {
    _group = c;
    // worst case time to cache a full group in memory
    if (_id >= EASYVR3)
      delay(39);
    else
      delay(19);
  }
}

int EasyVR::recv(int16_t timeout) // negative means forever
{
  while (timeout != 0 && _s->available() == 0)
  {
    delay(1);
    if (timeout > 0)
      --timeout;
  }
  return _s->read();
}

bool EasyVR::recvArg(int8_t& c)
{
  send(ARG_ACK);
  int r = recv(DEF_TIMEOUT);
  c = r - ARG_ZERO;
  return r >= ARG_MIN && r <= ARG_MAX;
}

void EasyVR::readStatus(int8_t rx)
{
  _status.v = 0;
  _value = 0;
  
  switch (rx)
  {
  case STS_SUCCESS:
    return;
  
  case STS_SIMILAR:
    _status.b._builtin = true;
    goto GET_WORD_INDEX;

  case STS_RESULT:
    _status.b._command = true;
  
  GET_WORD_INDEX:
    if (recvArg(rx))
    {
      _value = rx;
      return;
    }
    break;
    
  case STS_TOKEN:
    _status.b._token = true;
  
    if (recvArg(rx))
    {
      _value = rx << 5;
      if (recvArg(rx))
      {
        _value |= rx;
        return;
      }
    }
    break;
    
  case STS_AWAKEN:
    _status.b._awakened = true;
    return;
    
  case STS_TIMEOUT:
    _status.b._timeout = true;
    return;
    
  case STS_INVALID:
    _status.b._invalid = true;
    return;
    
  case STS_ERROR:
    _status.b._error = true;
    if (recvArg(rx))
    {
      _value = rx << 4;
      if (recvArg(rx))
      {
        _value |= rx;
        return;
      }
    }
    break;
  }

  // unexpected condition (communication error)
  _status.v = 0;
  _status.b._error = true;
  _value = 0;
}

/*****************************************************************************/

bool EasyVR::detect()
{
  uint8_t i;
  for (i = 0; i < 5; ++i)
  {
    sendCmd(CMD_BREAK);

    if (recv(WAKE_TIMEOUT) == STS_SUCCESS)
      return true;
  }
  return false;
}

bool EasyVR::stop()
{
  sendCmd(CMD_BREAK);

  uint8_t rx = recv(STORAGE_TIMEOUT);
  if (rx == STS_INTERR || rx == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::sleep(int8_t mode)
{
  sendCmd(CMD_SLEEP);
  sendArg(mode);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

int8_t EasyVR::getID()
{
  sendCmd(CMD_ID);
  if (recv(DEF_TIMEOUT) == STS_ID)
  {
    if (recvArg(_id))
      return _id;
  }
  _id = -1;
  return _id;
}

bool EasyVR::setLanguage(int8_t lang)
{        
  sendCmd(CMD_LANGUAGE);
  sendArg(lang);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::setTimeout(int8_t seconds)
{
  sendCmd(CMD_TIMEOUT);
  sendArg(seconds);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::setMicDistance(int8_t dist)
{
  sendCmd(CMD_MIC_DIST);
  sendArg(-1);
  sendArg(dist);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::setKnob(int8_t knob)
{
  sendCmd(CMD_KNOB);
  sendArg(knob);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::setTrailingSilence(int8_t dur)
{
  sendCmd(CMD_TRAILING);
  sendArg(-1);
  sendArg(dur);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::setLevel(int8_t level)
{
  sendCmd(CMD_LEVEL);
  sendArg(level);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::setCommandLatency(int8_t mode)
{
  sendCmd(CMD_FAST_SD);
  sendArg(-1);
  sendArg(mode);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::setDelay(uint16_t millis)
{
  sendCmd(CMD_DELAY);
  if (millis <= 10)
    sendArg(millis);
  else if (millis <= 100)
    sendArg(millis / 10 + 9);
  else if (millis <= 1000)
    sendArg(millis / 100 + 18);
  else
    return false;

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::changeBaudrate(int8_t baud)
{
  sendCmd(CMD_BAUDRATE);
  sendArg(baud);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}


bool EasyVR::addCommand(int8_t group, int8_t index)
{
  sendCmd(CMD_GROUP_SD);
  sendGroup(group);
  sendArg(index);

  int rx = recv(STORAGE_TIMEOUT);
  if (rx == STS_SUCCESS)
    return true;
  _status.v = 0;
  if (rx == STS_OUT_OF_MEM)
    _status.b._memfull = true;
  return false;
}

bool EasyVR::removeCommand(int8_t group, int8_t index)
{
  sendCmd(CMD_UNGROUP_SD);
  sendGroup(group);
  sendArg(index);

  if (recv(STORAGE_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::setCommandLabel(int8_t group, int8_t index, const char* name)
{
  sendCmd(CMD_NAME_SD);
  sendGroup(group);
  sendArg(index);
  
  int8_t len = 31;
  for (const char* p = name; *p != 0 && len > 0; ++p, --len)
  {
    if (isdigit(*p))
      --len;
  }
  len = 31 - len;
  
  sendArg(len);
  for (int8_t i = 0; i < len; ++i)
  {
    char c = name[i];
    if (isdigit(c))
    {
      send('^');
      sendArg(c - '0');
    }
    else if (isalpha(c))
    {
      send(c & ~0x20); // to uppercase
    }
    else
    {
      send('_');
    }
  }

  if (recv(STORAGE_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::eraseCommand(int8_t group, int8_t index)
{
  sendCmd(CMD_ERASE_SD);
  sendGroup(group);
  sendArg(index);

  if (recv(STORAGE_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}


bool EasyVR::getGroupMask(uint32_t& mask)
{
  sendCmd(CMD_MASK_SD);

  if (recv(DEF_TIMEOUT) == STS_MASK)
  {
    int8_t rx;
    mask = 0;
    for (int8_t i = 0; i < 4; ++i)
    {
      if (!recvArg(rx))
        return false;
      ((uint8_t*)&mask)[i] |= rx & 0x0F;
      if (!recvArg(rx))
        return false;
      ((uint8_t*)&mask)[i] |= (rx << 4) & 0xF0;
    }
    return true;
  }
  return false;
}

int8_t EasyVR::getCommandCount(int8_t group)
{
  sendCmd(CMD_COUNT_SD);
  sendArg(group);

  if (recv(DEF_TIMEOUT) == STS_COUNT)
  {
    int8_t rx;
    if (recvArg(rx))
    {
      return rx == -1 ? 32 : rx;
    }
  }
  return -1;
}

bool EasyVR::dumpCommand(int8_t group, int8_t index, char* name, uint8_t& training)
{
  sendCmd(CMD_DUMP_SD);
  sendGroup(group);
  sendArg(index);

  if (recv(DEF_TIMEOUT) != STS_DATA)
    return false;
  
  int8_t rx;
  if (!recvArg(rx))
    return false;
  training = rx & 0x07;
  if (rx == -1 || training == 7)
    training = 0;
  
  _status.v = 0;
  _status.b._conflict = (rx & 0x18) != 0;
  _status.b._command = (rx & 0x08) != 0;
  _status.b._builtin = (rx & 0x10) != 0;
  
  if (!recvArg(rx))
    return false;
  _value = rx;

  if (!recvArg(rx))
    return false;
  int8_t len = rx == -1 ? 32 : rx;
  for ( ; len > 0; --len, ++name)
  {
    if (!recvArg(rx))
      return false;
    if (rx == '^' - ARG_ZERO)
    {
      if (!recvArg(rx))
        return false;
      *name = '0' + rx;
      --len;
    }
    else
    {
      *name = ARG_ZERO + rx;
    }
  }
  *name = 0;
  return true;
}

int8_t EasyVR::getGrammarsCount(void)
{
  sendCmd(CMD_DUMP_SI);
  sendArg(-1);

  if (recv(DEF_TIMEOUT) == STS_COUNT)
  {
    int8_t rx;
    if (recvArg(rx))
    {
      return rx == -1 ? 32 : rx;
    }
  }
  return -1;
}

bool EasyVR::dumpGrammar(int8_t grammar, uint8_t& flags, uint8_t& count)
{
  sendCmd(CMD_DUMP_SI);
  sendArg(grammar);

  if (recv(DEF_TIMEOUT) != STS_GRAMMAR)
    return false;
  
  int8_t rx;
  if (!recvArg(rx))
    return false;
  flags = rx == -1 ? 32 : rx;
  
  if (!recvArg(rx))
    return false;
  count = rx;
  return true;
}

bool EasyVR::getNextWordLabel(char* name)
{
  int8_t count;
  if (!recvArg(count))
    return false;
  if (count == -1)
    count = 32;
  
  for ( ; count > 0; --count, ++name)
  {
    int8_t rx;
    if (!recvArg(rx))
      return false;
    
    if (rx == '^' - ARG_ZERO)
    {
      if (!recvArg(rx))
        return false;
      
      *name = '0' + rx;
      --count;
    }
    else
    {
      *name = ARG_ZERO + rx;
    }
  }
  *name = 0;
  return true;
}

void EasyVR::trainCommand(int8_t group, int8_t index)
{
  sendCmd(CMD_TRAIN_SD);
  sendGroup(group);
  sendArg(index);
}

void EasyVR::recognizeCommand(int8_t group)
{
  sendCmd(CMD_RECOG_SD);
  sendArg(group);
}

void EasyVR::recognizeWord(int8_t wordset)
{
  sendCmd(CMD_RECOG_SI);
  sendArg(wordset);
}

bool EasyVR::hasFinished()
{
  int8_t rx = recv(NO_TIMEOUT);
  if (rx < 0)
    return false;
  
  readStatus(rx);
  return true;
}

bool EasyVR::setPinOutput(int8_t pin, int8_t value)
{
  sendCmd(CMD_QUERY_IO);
  sendArg(pin);
  sendArg(value);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

int8_t EasyVR::getPinInput(int8_t pin, int8_t config)
{
  sendCmd(CMD_QUERY_IO);
  sendArg(pin);
  sendArg(config);

  if (recv(DEF_TIMEOUT) == STS_PIN)
  {
    int8_t rx;
    if (recvArg(rx))
      return rx;
  }
  return -1;
}

bool EasyVR::playPhoneTone(int8_t tone, uint8_t duration)
{
  sendCmd(CMD_PLAY_DTMF);
  sendArg(-1); // distinguish DTMF from SX
  sendArg(tone);
  sendArg(duration - 1);

  if (recv((tone < 0 ? duration * 1000 : duration * 40) + DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::playSound(int16_t index, int8_t volume)
{
  sendCmd(CMD_PLAY_SX);
  sendArg((index >> 5) & 0x1F);
  sendArg(index & 0x1F);
  sendArg(volume);

  if (recv(PLAY_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

void EasyVR::playSoundAsync(int16_t index, int8_t volume)
{
  sendCmd(CMD_PLAY_SX);
  sendArg((index >> 5) & 0x1F);
  sendArg(index & 0x1F);
  sendArg(volume);
}

void EasyVR::detectToken(int8_t bits, int8_t rejection, uint16_t timeout)
{
  sendCmd(CMD_RECV_SN);
  sendArg(bits);
  sendArg(rejection);
  if (timeout > 0)
    timeout = (timeout * 2 + 53)/ 55; // approx / 27.46 - err < 0.15%
  sendArg((timeout >> 5) & 0x1F);
  sendArg(timeout & 0x1F);
}

bool EasyVR::sendToken(int8_t bits, uint8_t token)
{
  sendCmd(CMD_SEND_SN);
  sendArg(bits);
  sendArg((token >> 5) & 0x1F);
  sendArg(token & 0x1F);
  sendArg(0);
  sendArg(0);

  if (recv(TOKEN_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

void EasyVR::sendTokenAsync(int8_t bits, uint8_t token)
{
  sendCmd(CMD_SEND_SN);
  sendArg(bits);
  sendArg((token >> 5) & 0x1F);
  sendArg(token & 0x1F);
  sendArg(0);
  sendArg(0);
}

bool EasyVR::embedToken(int8_t bits, uint8_t token, uint16_t delay)
{
  sendCmd(CMD_SEND_SN);
  sendArg(bits);
  sendArg((token >> 5) & 0x1F);
  sendArg(token & 0x1F);
  delay = (delay * 2 + 27) / 55; // approx / 27.46 - err < 0.15%
  if (delay == 0) // must be > 0 to embed in some audio
    delay = 1;
  sendArg((delay >> 5) & 0x1F);
  sendArg(delay & 0x1F);

  if (recv(DEF_TIMEOUT) == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::dumpSoundTable(char* name, int16_t& count)
{
  sendCmd(CMD_DUMP_SX);

  if (recv(DEF_TIMEOUT) != STS_TABLE_SX)
    return false;
  
  int8_t rx;
  if (!recvArg(rx))
    return false;
  count = rx << 5;
  if (!recvArg(rx))
    return false;
  count |= rx;
  
  if (!recvArg(rx))
    return false;
  int len = rx;
  for (int8_t i = 0, k = 0; i < len; ++i, ++k)
  {
    if (!recvArg(rx))
      return false;
    if (rx == '^' - ARG_ZERO)
    {
      if (!recvArg(rx))
        return false;
      ++i;
      name[k] = '0' + rx;
    }
    else
    {
      name[k] = ARG_ZERO + rx;
    }
  }
  name[len] = 0;
  return true;
}

bool EasyVR::resetAll(bool wait)
{
  sendCmd(CMD_RESETALL);
  sendArg('R' - ARG_ZERO);

  if (!wait)
    return true;

  int timeout = 40; // seconds
  if (getID() >= EASYVR3)
    timeout = 5;
  while (timeout != 0 && _s->available() == 0)
  {
    delay(1000);
    --timeout;
  }
  if (_s->read() == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::resetCommands(bool wait)
{
  sendCmd(CMD_RESETALL);
  sendArg('D' - ARG_ZERO);

  if (!wait)
    return true;

  int timeout = 5; // seconds
  while (timeout != 0 && _s->available() == 0)
  {
    delay(1000);
    --timeout;
  }
  if (_s->read() == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::resetMessages(bool wait)
{
  sendCmd(CMD_RESETALL);
  sendArg('M' - ARG_ZERO);

  if (!wait)
    return true;

  int timeout = 15; // seconds
  while (timeout != 0 && _s->available() == 0)
  {
    delay(1000);
    --timeout;
  }
  if (_s->read() == STS_SUCCESS)
    return true;
  return false;
}

bool EasyVR::checkMessages()
{
  sendCmd(CMD_VERIFY_RP);
  sendArg(-1);
  sendArg(0);

  int rx = recv(STORAGE_TIMEOUT);
  readStatus(rx);
  return (_status.v == 0);
}

bool EasyVR::fixMessages(bool wait)
{
  sendCmd(CMD_VERIFY_RP);
  sendArg(-1);
  sendArg(1);

  if (!wait)
    return true;

  int timeout = 25; // seconds
  while (timeout != 0 && _s->available() == 0)
  {
    delay(1000);
    --timeout;
  }
  if (_s->read() == STS_SUCCESS)
    return true;
  return false;
}

void EasyVR::recordMessageAsync(int8_t index, int8_t bits, int8_t timeout)
{
  sendCmd(CMD_RECORD_RP);
  sendArg(-1);
  sendArg(index);
  sendArg(bits);
  sendArg(timeout);
}

void EasyVR::playMessageAsync(int8_t index, int8_t speed, int8_t atten)
{
  sendCmd(CMD_PLAY_RP);
  sendArg(-1);
  sendArg(index);
  sendArg((speed << 2) | (atten & 3));
}

void EasyVR::eraseMessageAsync(int8_t index)
{
  sendCmd(CMD_ERASE_RP);
  sendArg(-1);
  sendArg(index);
}

bool EasyVR::dumpMessage(int8_t index, int8_t& type, int32_t& length)
{
  sendCmd(CMD_DUMP_RP);
  sendArg(-1);
  sendArg(index);

  int sts = recv(STORAGE_TIMEOUT);
  if (sts != STS_MESSAGE)
  {
    readStatus(sts);
    return false;
  }

  // if communication should fail
  _status.v = 0;
  _status.b._error = true;

  if (!recvArg(type))
    return false;

  int8_t rx;
  length = 0;
  if (type == 0)
    return true; // skip reading if empty

  for (int8_t i = 0; i < 6; ++i)
  {
    if (!recvArg(rx))
      return false;
    ((uint8_t*)&length)[i] |= rx & 0x0F;
    if (!recvArg(rx))
      return false;
    ((uint8_t*)&length)[i] |= (rx << 4) & 0xF0;
  }
  _status.v = 0;
  return true;
}

bool EasyVR::realtimeLipsync(int16_t threshold, uint8_t timeout)
{
  sendCmd(CMD_LIPSYNC);
  sendArg(-1);
  sendArg((threshold >> 5) & 0x1F);
  sendArg(threshold & 0x1F);
  sendArg((timeout >> 4) & 0x0F);
  sendArg(timeout & 0x0F);

  int sts = recv(DEF_TIMEOUT);
  if (sts != STS_LIPSYNC)
  {
    readStatus(sts);
    return false;
  }
  return true;
}

bool EasyVR::fetchMouthPosition(int8_t& value)
{
  send(ARG_ACK);
  int rx = recv(DEF_TIMEOUT);
  if (rx >= ARG_MIN && rx <= ARG_MAX)
  {
    value = rx - ARG_ZERO;
    return true;
  }
  // check if finished
  if (rx >= 0)
    readStatus(rx);
  return false;
}

// Service functions

bool EasyVR::exportCommand(int8_t group, int8_t index, uint8_t* data)
{
  sendCmd(CMD_SERVICE);
  sendArg(SVC_EXPORT_SD - ARG_ZERO);
  sendGroup(group);
  sendArg(index);
  
  if (recv(STORAGE_TIMEOUT) != STS_SERVICE)
    return false;
  
  int8_t rx;
  if (!recvArg(rx) || rx != SVC_DUMP_SD - ARG_ZERO)
    return false;
  
  for (int i = 0; i < 258; ++i)
  {
    if (!recvArg(rx))
      return false;
    data[i] = (rx << 4) & 0xF0;
    if (!recvArg(rx))
      return false;
    data[i] |= (rx & 0x0F);
  }
  return true;
}

bool EasyVR::importCommand(int8_t group, int8_t index, const uint8_t* data)
{
  sendCmd(CMD_SERVICE);
  sendArg(SVC_IMPORT_SD - ARG_ZERO);
  sendGroup(group);
  sendArg(index);
  
  int8_t tx;
  for (int i = 0; i < 258; ++i)
  {
    tx = (data[i] >> 4) & 0x0F;
    sendArg(tx);
    tx = data[i] & 0x0F;
    sendArg(tx);
  }
  if (recv(STORAGE_TIMEOUT) != STS_SUCCESS)
    return false;
  return true;
}

void EasyVR::verifyCommand(int8_t group, int8_t index)
{
  sendCmd(CMD_SERVICE);
  sendArg(SVC_VERIFY_SD - ARG_ZERO);
  sendGroup(group);
  sendArg(index);
}

// Bridge Mode implementation

void EasyVR::bridgeLoop(Stream& pcSerial)
{
  unsigned long time = millis();
  int rx, cmd = -1;
  for (;;)
  {
    if (cmd >= 0 && millis() >= time)
      return;
    if (pcSerial.available())
    {
      rx = pcSerial.read();
      if (rx == EasyVR::BRIDGE_ESCAPE_CHAR && millis() >= time)
      {
        cmd = rx;
        time = millis() + 100;
        continue;
      }
      _s->write(rx);
      cmd = -1;
      time = millis() + 100;
    }
    if (_s->available())
      pcSerial.write(_s->read());
  }
}

int EasyVR::bridgeRequested(Stream& pcSerial)
{
  pcSerial.write(0x99);
  // look for a request header
  int bridge = BRIDGE_NONE;
  bool request = false;
  int t, rx;
  for (t=0; t<150; ++t)
  {
    delay(10);
    rx = pcSerial.read();
    if (rx < 0)
      continue;
    if (!request)
    {
      if (rx == 0xBB)
      {
        pcSerial.write(0xCC);
        delay(1); // flush not reliable on some core libraries
        pcSerial.flush();
        request = true;
        continue;
      }
      request = false;
    }
    else
    {
      if (rx == 0xDD)
      {
        pcSerial.write(0xEE);
        delay(1); // flush not reliable on some core libraries
        pcSerial.flush();
        bridge = BRIDGE_NORMAL;
        break;
      }
      if (rx == 0xAA)
      {
        pcSerial.write(0xFF);
        delay(1); // flush not reliable on some core libraries
        pcSerial.flush();
        bridge = BRIDGE_BOOT;
        break;
      }
      request = false;
    }
  }
  return bridge;
}

/** @mainpage

  The EasyVR library implements the serial communication protocol to
  manage the %EasyVR module and the %EasyVR Shield from Arduino boards and 
  controllers and it enables easy access to all the %EasyVR features.

  <table><tr><td>
  ![EasyVR module](@ref EasyVR_3.jpg)
  </td><td>
  ![EasyVR Shield](@ref EasyVR_Shield_3.jpg)
  </td></tr></table>

  ### Installation

  To install the EasyVR library on your Arduino IDE use the menu
  Sketch > Import Library ... > Add Library and open the released zip archive.
  
  ### Examples

  You can easily open the example sketches included with the EasyVR library
  from inside the Arduino IDE, using the menu File > Examples > %EasyVR and
  choosing one of the available sketches.
*/
