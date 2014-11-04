/** @file
EasyVR library v1.6
Copyright (C) 2014 RoboTech srl

Written for Arduino and compatible boards for use with EasyVR modules or
EasyVR Shield boards produced by VeeaR <www.veear.eu>

Released under the terms of the MIT license, as found in the accompanying
file COPYING.txt or at this address: <http://www.opensource.org/licenses/MIT>
*/

#pragma once

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
    Performs bridge mode between the PC Serial port and the specified port
    in a continuos loop. It can be aborted by sending a question mark ('?').
    @param port is the target serial port
    @note You can use this alternate loop on boards that don't have a separate
    USB/Serial adapter, such as Arduino Leonardo.
  */
  void loop(Stream& port);
};
