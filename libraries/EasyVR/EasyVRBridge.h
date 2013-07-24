/** @file
EasyVR library v1.3
Copyright (C) 2011 RoboTech srl

Written for Arduino and compatible boards for use with EasyVR modules or
EasyVR Shield boards produced by VeeaR <www.veear.eu>

Released under the terms of the MIT license, as found in the accompanying
file COPYING.txt or at this address: <http://www.opensource.org/licenses/MIT>
*/

/**
  An implementation of a software bridge between two series of Rx/Tx pins,
  that enables routing of the hardware serial port (connected to the PC) to
  digital I/O pins used as a software serial port (connected to the %EasyVR).
*/
class EasyVRBridge
{
public:
  /**
    Tests if bridge mode has been requested
    @retval true if bridge mode should be started
    @note The %EasyVR Commander software can request bridge mode using the
    Serial port. This method does not require to reserve EEPROM locations.
  */
  bool check();
  /**
    Tests if bridge mode has been requested (legacy method)
    @retval true if bridge mode should be started
    @note The first two EEPROM locations (bytes 0-1) are used for discovery
    and request of bridge mode from the %EasyVR Commander software. Do not use
    the same locations for other programa data.
    @deprecated
  */
  bool checkEEPROM();
  /**
    Performs bridge mode between port A and B in an endless loop
    @param a_rx is the Rx pin of port A
    @param a_tx is the Tx pin of port A
    @param b_rx is the Rx pin of port B
    @param b_tx is the Tx pin of port B
    @note Bridge mode internally connects Rx:A to Tx:B and Rx:B to Tx:A.
    This is done by reading from a pin and writing to the other in a fast
    loop, that runs until the microcontroller is reset.
  */
  void loop(uint8_t a_rx, uint8_t a_tx, uint8_t b_rx, uint8_t b_tx);
};
