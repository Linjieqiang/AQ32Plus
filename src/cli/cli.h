/*
  October 2012

  aq32Plus Rev -

  Copyright (c) 2012 John Ihlein.  All rights reserved.

  Open Source STM32 Based Multicopter Controller Software

  Includes code and/or ideas from:

  1)AeroQuad
  2)BaseFlight
  3)CH Robotics
  4)MultiWii
  5)S.O.H. Madgwick
  6)UAVX

  Designed to run on the AQ32 Flight Control Board

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

extern uint32_t (*cliPortAvailable)(void);

extern void     (*cliPortClearBuffer)(void);

extern uint8_t  (*cliPortRead)(void);

extern void     (*cliPortPrint)(char *str);

extern void     (*cliPortPrintF)(const char * fmt, ...);

///////////////////////////////////////

extern uint8_t cliBusy;

///////////////////////////////////////////////////////////////////////////////
// Read Float from CLI
///////////////////////////////////////////////////////////////////////////////

float readFloatCLI(void);

///////////////////////////////////////////////////////////////////////////////
// Read Character String from CLI
///////////////////////////////////////////////////////////////////////////////

char *readStringCLI(char *data, uint8_t length);

///////////////////////////////////////////////////////////////////////////////
// CLI Communication
///////////////////////////////////////////////////////////////////////////////

void cliCom(void);

///////////////////////////////////////////////////////////////////////////////
// ADC CLI
///////////////////////////////////////////////////////////////////////////////

void adcCLI(void);

///////////////////////////////////////////////////////////////////////////////
// EEPROM CLI
///////////////////////////////////////////////////////////////////////////////

void eepromCLI(void);

///////////////////////////////////////////////////////////////////////////////
// GPS CLI
///////////////////////////////////////////////////////////////////////////////

void gpsCLI(void);

///////////////////////////////////////////////////////////////////////////////
// MAX7456 CLI
///////////////////////////////////////////////////////////////////////////////

void max7456CLI(void);

///////////////////////////////////////////////////////////////////////////////
// Mixer CLI
///////////////////////////////////////////////////////////////////////////////

void mixerCLI(void);

///////////////////////////////////////////////////////////////////////////////
// Receiver CLI
///////////////////////////////////////////////////////////////////////////////

void receiverCLI(void);

///////////////////////////////////////////////////////////////////////////////
// Sensor CLI
///////////////////////////////////////////////////////////////////////////////

void sensorCLI(void);

///////////////////////////////////////////////////////////////////////////////
// Telemetry CLI
///////////////////////////////////////////////////////////////////////////////

void telemetryCLI(void);

///////////////////////////////////////////////////////////////////////////////


