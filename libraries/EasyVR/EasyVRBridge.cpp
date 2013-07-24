/*
EasyVR library v1.3
Copyright (C) 2011 RoboTech srl

Written for Arduino and compatible boards for use with EasyVR modules or
EasyVR Shield boards produced by VeeaR <www.veear.eu>

Released under the terms of the MIT license, as found in the accompanying
file COPYING.txt or at this address: <http://www.opensource.org/licenses/MIT>
*/

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "pins_arduino.h"
#include <avr/eeprom.h>

#define BRIDGE_ID0 ((uint8_t *) 0)
#define BRIDGE_ID1 ((uint8_t *) 1)

#include "EasyVRBridge.h"

#define tx_pin_write(pin_state) \
    if (!(pin_state)) \
      *tx_reg &= ~tx_mask; \
    else \
      *tx_reg |= tx_mask;

#define rx_pin_read() (*rx_reg & rx_mask)

#define vtx_pin_write(pin_state) \
    if (!(pin_state)) \
      *vtx_reg &= ~vtx_mask; \
    else \
      *vtx_reg |= vtx_mask;

#define vrx_pin_read() (*vrx_reg & vrx_mask)

void EasyVRBridge::loop(uint8_t a_rx, uint8_t a_tx, uint8_t b_rx, uint8_t b_tx)
{
  uint8_t rx_mask;
  volatile uint8_t *rx_reg;
  uint8_t tx_mask;
  volatile uint8_t *tx_reg;

  uint8_t vrx_mask;
  volatile uint8_t *vrx_reg;
  uint8_t vtx_mask;
  volatile uint8_t *vtx_reg;

  pinMode(a_rx, INPUT);
  digitalWrite(a_rx, HIGH);
  pinMode(a_tx, OUTPUT);
  digitalWrite(a_tx, HIGH);

  pinMode(b_rx, INPUT);
  digitalWrite(b_rx, HIGH);
  pinMode(b_tx, OUTPUT);
  digitalWrite(b_tx, HIGH);

  rx_mask = digitalPinToBitMask(a_rx);
  rx_reg = portInputRegister(digitalPinToPort(a_rx));
  tx_mask = digitalPinToBitMask(a_tx);
  tx_reg = portOutputRegister(digitalPinToPort(a_tx));

  vrx_mask = digitalPinToBitMask(b_rx);
  vrx_reg = portInputRegister(digitalPinToPort(b_rx));
  vtx_mask = digitalPinToBitMask(b_tx);
  vtx_reg = portOutputRegister(digitalPinToPort(b_tx));

  for (;;)
  {
    vtx_pin_write(rx_pin_read());
    tx_pin_write(vrx_pin_read());
  }
}

bool EasyVRBridge::checkEEPROM()
{
  uint8_t b0 = eeprom_read_byte(BRIDGE_ID0);
  uint8_t b1 = eeprom_read_byte(BRIDGE_ID1);
  if (b0 != 'B')
  {
    while (!eeprom_is_ready());
    eeprom_write_byte(BRIDGE_ID0, 'B');
  }
  else if (b1 == 'b')
  {
    while (!eeprom_is_ready());
    eeprom_write_byte(BRIDGE_ID1, 'B');
    return true;
  }
  if (b1 != 'B')
  {
    while (!eeprom_is_ready());
    eeprom_write_byte(BRIDGE_ID1, 'B');
  }    
  return false;
}

bool EasyVRBridge::check()
{
#if defined(UBRRH) || defined(UBRR0H)
#define pcSerial Serial
#elif defined(UBRR1H)
#define pcSerial Serial1
#else
#error "Target platform does not have a serial port!"
#endif
  pcSerial.begin(9600);
  // look for a request header
  bool bridge = false;
  int t;
  for (t=0; t<20; ++t)
  {
    delay(10);
    if (pcSerial.available() > 0 && pcSerial.read() == 0xBB)
    {
      pcSerial.write(0xCC);
      pcSerial.flush();
      bridge = true;
      break;
    }
  }
  if (bridge)
  {
    // send reply and wait for confirmation
    bridge = false;
    for (t=0; t<20; ++t)
    {
      delay(10);
      if (pcSerial.available() > 0 && pcSerial.read() == 0xDD)
      {
        pcSerial.write(0xEE);
        pcSerial.flush();
        bridge = true;
        break;
      }
    }
  }
  pcSerial.end();
  return bridge;
}