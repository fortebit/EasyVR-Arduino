/*
  SoftwareSerial.h - Software serial library
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

#ifndef SoftwareSerial_h
#define SoftwareSerial_h

#include <inttypes.h>

#include "Stream.h"

class SoftwareSerial : public Stream
{
  private:
    uint8_t _receiveBitMask;
    volatile uint8_t *_receivePortRegister;
    uint8_t _transmitBitMask;
    volatile uint8_t *_transmitPortRegister;
    uint8_t rx_pin_read();
    void tx_pin_write(uint8_t pin_state);
    uint8_t _receivePin;
    uint8_t _transmitPin;
    long _baudRate;
    int _bitPeriod;
  public:
    SoftwareSerial(uint8_t, uint8_t);
    void begin(long);
    void end();
  
    // Print interface
    void write(uint8_t);

    // Stream interface
    int read();
    int peek() { return -1; } // not supported
    int available() { return -1; } // not suported
    void flush() { }
};

#endif

