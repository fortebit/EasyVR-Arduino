/*
  SoftwareSerial.cpp - Software serial library
  Copyright (c) 2006 David A. Mellis.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
  modified 8-Sep-2011 by Paolo Messina (RoboTech srl)
  to implement Stream interface and fix communication issues
  (rx/tx code borrowed from NewSoftSerial)
*/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "WConstants.h"
#include "pins_arduino.h"
#include "SoftwareSerial.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

SoftwareSerial::SoftwareSerial(uint8_t receivePin, uint8_t transmitPin)
{
  _receivePin = receivePin;
  _transmitPin = transmitPin;
  _baudRate = 0;
}

void SoftwareSerial::tx_pin_write(uint8_t pin_state)
{
  if (pin_state == LOW)
    *_transmitPortRegister &= ~_transmitBitMask;
  else
    *_transmitPortRegister |= _transmitBitMask;
}

uint8_t SoftwareSerial::rx_pin_read()
{
  return *_receivePortRegister & _receiveBitMask;
}

/******************************************************************************
 * User API
 ******************************************************************************/

void SoftwareSerial::begin(long speed)
{
  pinMode(_receivePin, INPUT);
  digitalWrite(_receivePin, HIGH);
  _receiveBitMask = digitalPinToBitMask(_receivePin);
  _receivePortRegister = portInputRegister(digitalPinToPort(_receivePin));

  pinMode(_transmitPin, OUTPUT);
  digitalWrite(_transmitPin, HIGH);
  _transmitBitMask = digitalPinToBitMask(_transmitPin);
  _transmitPortRegister = portOutputRegister(digitalPinToPort(_transmitPin));
  
  _baudRate = speed;
  _bitPeriod = 1000000 / _baudRate;
}

void SoftwareSerial::end()
{
  pinMode(_receivePin, INPUT);
  digitalWrite(_receivePin, HIGH);

  pinMode(_transmitPin, INPUT);
  digitalWrite(_transmitPin, HIGH);
}

int SoftwareSerial::read()
{
  int val = 0;
  int bitDelay = _bitPeriod - 1; // inter-bit overhead is about 1us
  
  // one byte of serial data (LSB first)
  // ...--\    /--\/--\/--\/--\/--\/--\/--\/--\/--...
  //	 \--/\--/\--/\--/\--/\--/\--/\--/\--/
  //	start  0   1   2   3   4   5   6   7 stop

  while (rx_pin_read());

  // confirm that this is a real start bit, not line noise
  if (rx_pin_read() == LOW) {
    // frame start indicated by a falling edge and low start bit
    // jump to the middle of the low start bit
    delayMicroseconds(_bitPeriod / 2 - 1); // inter-bit overhead is about 1us
	
    for (uint8_t i=0x1; i; i <<= 1)
    {
        delayMicroseconds(bitDelay);
        uint8_t noti = ~i;
        if (rx_pin_read())
            val |= i;
        else // else clause added to ensure function timing is ~balanced
            val &= noti;
    }
	
    delayMicroseconds(_bitPeriod);
    
    return val;
  }
  
  return -1;
}

void SoftwareSerial::write(uint8_t b)
{
  if (_baudRate == 0)
    return;
    
  int bitDelay = _bitPeriod - 1; // inter-bit overhead is about 1us
  byte mask;

  tx_pin_write(LOW);
  delayMicroseconds(bitDelay);

  for (mask = 0x01; mask; mask <<= 1) {
      if (b & mask) // choose bit
        tx_pin_write(HIGH); // send 1
      else
        tx_pin_write(LOW); // send 0
    delayMicroseconds(bitDelay);
  }

  tx_pin_write(HIGH);
  delayMicroseconds(bitDelay);
}
