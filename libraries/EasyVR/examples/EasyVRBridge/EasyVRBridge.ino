/*
  EasyVR Bridge

  Soft-connects pins 0 (Rx) to 13 (ERX) and 12 (ETX) to 1 (Tx)
  with roughly 5us sample rate, for connecting with the GUI.

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

#include "EasyVR.h"

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
  Serial.println("Bridge not started!");
}

void loop()
{
}
