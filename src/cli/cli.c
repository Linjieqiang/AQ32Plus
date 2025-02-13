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

#include "board.h"

///////////////////////////////////////////////////////////////////////////////

uint32_t (*cliPortAvailable)(void);

void     (*cliPortClearBuffer)(void);

uint8_t  (*cliPortRead)(void);

void     (*cliPortPrint)(char *str);

void     (*cliPortPrintF)(const char * fmt, ...);

///////////////////////////////////////

uint8_t cliBusy = false;

static volatile uint8_t cliQuery        = 'x';
static volatile uint8_t validCliCommand = false;

uint8_t gpsDataType = 0;

///////////////////////////////////////////////////////////////////////////////
// Read Character String from CLI
///////////////////////////////////////////////////////////////////////////////

char *readStringCLI(char *data, uint8_t length)
{
    uint8_t index    = 0;
    uint8_t timeout  = 0;

    do
    {
        if (cliPortAvailable() == false)
        {
            delay(10);
            timeout++;
        }
        else
        {
            data[index] = cliPortRead();
            timeout = 0;
            index++;
        }
    }
    while ((index == 0 || data[index-1] != ';') && (timeout < 5) && (index < length));

    data[index] = '\0';

    return data;
}

///////////////////////////////////////////////////////////////////////////////
// Read Float from CLI
///////////////////////////////////////////////////////////////////////////////

float readFloatCLI(void)
{
    uint8_t index    = 0;
    uint8_t timeout  = 0;
    char    data[13] = "";

    do
    {
        if (cliPortAvailable() == false)
        {
            delay(10);
            timeout++;
        }
        else
        {
            data[index] = cliPortRead();
            timeout = 0;
            index++;
        }
    }
    while ((index == 0 || data[index-1] != ';') && (timeout < 5) && (index < sizeof(data)-1));

    data[index] = '\0';

    return stringToFloat(data);
}

///////////////////////////////////////////////////////////////////////////////
// Read PID Values from CLI
///////////////////////////////////////////////////////////////////////////////

void readCliPID(unsigned char PIDid)
{
  struct PIDdata* pid = &eepromConfig.PID[PIDid];

  pid->P               = readFloatCLI();
  pid->I               = readFloatCLI();
  pid->D               = readFloatCLI();
  pid->Limit           = readFloatCLI();
  pid->integratorState = 0.0f;
  pid->filterState     = 0.0f;
  pid->prevResetState  = false;
}

///////////////////////////////////////////////////////////////////////////////
// CLI Communication
///////////////////////////////////////////////////////////////////////////////

void cliCom(void)
{
	uint8_t  index;
	uint8_t  numChannels = 8;
	char mvlkToggleString[5] = { 0, 0, 0, 0, 0 };

	if (eepromConfig.receiverType == PPM)
		numChannels = eepromConfig.ppmChannels;

	if ((cliPortAvailable() && !validCliCommand))
    {
		cliQuery = cliPortRead();

        if (cliQuery == '#')                       // Check to see if we should toggle mavlink msg state
        {
	    	while (cliPortAvailable == false);

        	readStringCLI(mvlkToggleString, 5);

            if ((mvlkToggleString[0] == '#') &&
            	(mvlkToggleString[1] == '#') &&
                (mvlkToggleString[2] == '#') &&
                (mvlkToggleString[3] == '#'))
	    	{
	    	    if (eepromConfig.mavlinkEnabled == false)
	    	    {
	    	 	    eepromConfig.mavlinkEnabled  = true;
	    		    eepromConfig.activeTelemetry = 0x0000;
	    		}
	    		else
	    		{
	    		    eepromConfig.mavlinkEnabled = false;
	    	    }

	    	    if (mvlkToggleString[4] == 'W')
	    	    {
	                cliPortPrint("\nWriting EEPROM Parameters....\n");
	                writeEEPROM();
	    	    }
	    	}
	    }
	}

	validCliCommand = false;

    if ((eepromConfig.mavlinkEnabled == false) && (cliQuery != '#'))
    {
        switch (cliQuery)
        {
            ///////////////////////////////

            case 'a': // Rate PIDs
                cliPortPrintF("\nRoll Rate PID:  %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[ROLL_RATE_PID].P,
                    		                                                    eepromConfig.PID[ROLL_RATE_PID].I,
                    		                                                    eepromConfig.PID[ROLL_RATE_PID].D,
                    		                                                    eepromConfig.PID[ROLL_RATE_PID].Limit);

                cliPortPrintF(  "Pitch Rate PID: %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[PITCH_RATE_PID].P,
                    		                                                    eepromConfig.PID[PITCH_RATE_PID].I,
                    		                                                    eepromConfig.PID[PITCH_RATE_PID].D,
                    		                                                    eepromConfig.PID[PITCH_RATE_PID].Limit);

                cliPortPrintF(  "Yaw Rate PID:   %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[YAW_RATE_PID].P,
                    		                                                    eepromConfig.PID[YAW_RATE_PID].I,
                    		                                                    eepromConfig.PID[YAW_RATE_PID].D,
                    		                                                    eepromConfig.PID[YAW_RATE_PID].Limit);
                cliQuery = 'x';
                validCliCommand = false;
                break;

            ///////////////////////////////

            case 'b': // Attitude PIDs
                cliPortPrintF("\nRoll Attitude PID:  %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[ROLL_ATT_PID].P,
                   		                                                            eepromConfig.PID[ROLL_ATT_PID].I,
                   		                                                            eepromConfig.PID[ROLL_ATT_PID].D,
                   		                                                            eepromConfig.PID[ROLL_ATT_PID].Limit);

                cliPortPrintF(  "Pitch Attitude PID: %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[PITCH_ATT_PID].P,
                   		                                                            eepromConfig.PID[PITCH_ATT_PID].I,
                   		                                                            eepromConfig.PID[PITCH_ATT_PID].D,
                   		                                                            eepromConfig.PID[PITCH_ATT_PID].Limit);

                cliPortPrintF(  "Heading PID:        %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[HEADING_PID].P,
                   		                                                            eepromConfig.PID[HEADING_PID].I,
                   		                                                            eepromConfig.PID[HEADING_PID].D,
                   		                                                            eepromConfig.PID[HEADING_PID].Limit);
                cliQuery = 'x';
                validCliCommand = false;
                break;

            ///////////////////////////////

            case 'c': // Velocity PIDs
                cliPortPrintF("\nnDot PID:  %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[NDOT_PID].P,
                   		                                                   eepromConfig.PID[NDOT_PID].I,
                   		                                                   eepromConfig.PID[NDOT_PID].D,
                   		                                                   eepromConfig.PID[NDOT_PID].Limit);

                cliPortPrintF(  "eDot PID:  %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[EDOT_PID].P,
                   		                                                   eepromConfig.PID[EDOT_PID].I,
                   		                                                   eepromConfig.PID[EDOT_PID].D,
                   		                                                   eepromConfig.PID[EDOT_PID].Limit);

                cliPortPrintF(  "hDot PID:  %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[HDOT_PID].P,
                   		                                                   eepromConfig.PID[HDOT_PID].I,
                   		                                                   eepromConfig.PID[HDOT_PID].D,
                   		                                                   eepromConfig.PID[HDOT_PID].Limit);
                cliQuery = 'x';
                validCliCommand = false;
                break;


            ///////////////////////////////

            case 'd': // Position PIDs
                cliPortPrintF("\nN PID:  %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[N_PID].P,
                   		                                                eepromConfig.PID[N_PID].I,
                   		                                                eepromConfig.PID[N_PID].D,
                   		                                                eepromConfig.PID[N_PID].Limit);

                cliPortPrintF(  "E PID:  %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[E_PID].P,
                   		                                                eepromConfig.PID[E_PID].I,
                   		                                                eepromConfig.PID[E_PID].D,
                   		                                                eepromConfig.PID[E_PID].Limit);

                cliPortPrintF(  "h PID:  %8.4f, %8.4f, %8.4f, %8.4f\n", eepromConfig.PID[H_PID].P,
                   		                                                eepromConfig.PID[H_PID].I,
                   		                                                eepromConfig.PID[H_PID].D,
                   		                                                eepromConfig.PID[H_PID].Limit);
                cliQuery = 'x';
                validCliCommand = false;
              	break;

			///////////////////////////////

			case 'e': // Loop Delta Times
				cliPortPrintF("%7ld, %7ld, %7ld, %7ld, %7ld, %7ld, %7ld\n", deltaTime1000Hz,
																			deltaTime500Hz,
																			deltaTime100Hz,
																			deltaTime50Hz,
																			deltaTime10Hz,
																			deltaTime5Hz,
																			deltaTime1Hz);
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'f': // Loop Execution Times
				cliPortPrintF("%7ld, %7ld, %7ld, %7ld, %7ld, %7ld, %7ld\n", executionTime1000Hz,
																			executionTime500Hz,
																			executionTime100Hz,
																			executionTime50Hz,
																			executionTime10Hz,
																			executionTime5Hz,
																			executionTime1Hz);
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'g': // 100 Hz Accels
				cliPortPrintF("%9.4f, %9.4f, %9.4f\n", sensors.accel100Hz[XAXIS],
													   sensors.accel100Hz[YAXIS],
													   sensors.accel100Hz[ZAXIS]);
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'h': // 100 hz Earth Axis Accels
				cliPortPrintF("%9.4f, %9.4f, %9.4f\n", earthAxisAccels[XAXIS],
													   earthAxisAccels[YAXIS],
													   earthAxisAccels[ZAXIS]);
				validCliCommand = false;
				break;
			///////////////////////////////

			case 'i': // 500 hz Gyros
				cliPortPrintF("%9.4f, %9.4f, %9.4f, %9.4f\n", sensors.gyro500Hz[ROLL ] * R2D,
															  sensors.gyro500Hz[PITCH] * R2D,
															  sensors.gyro500Hz[YAW  ] * R2D,
															  mpu6000Temperature);
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'j': // 10 Hz Mag Data
				cliPortPrintF("%9.4f, %9.4f, %9.4f\n", sensors.mag10Hz[XAXIS],
													   sensors.mag10Hz[YAXIS],
													   sensors.mag10Hz[ZAXIS]);
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'k': // Vertical Axis Variables
				cliPortPrintF("%9.4f, %9.4f, %9.4f, %9.4f, %4ld, %9.4f\n", earthAxisAccels[ZAXIS],
																		   sensors.pressureAlt50Hz,
																		   hDotEstimate,
																		   hEstimate,
																		   ms5611Temperature,
																		   aglRead());
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'l': // Attitudes
				cliPortPrintF("%9.4f, %9.4f, %9.4f\n", sensors.attitude500Hz[ROLL ] * R2D,
													   sensors.attitude500Hz[PITCH] * R2D,
													   sensors.attitude500Hz[YAW  ] * R2D);
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'm': // Axis PIDs
				cliPortPrintF("%9.4f, %9.4f, %9.4f\n", ratePID[ROLL ],
													   ratePID[PITCH],
													   ratePID[YAW  ]);
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'n': // GPS Data
				switch (gpsDataType)
				{
					///////////////////////

					case 0:
						cliPortPrintF("%12ld, %12ld, %12ld, %12ld, %12ld, %12ld, %4d, %4d\n", gps.latitude,
																							  gps.longitude,
																							  gps.hMSL,
																							  gps.velN,
																							  gps.velE,
																							  gps.velD,
																							  gps.fix,
																							  gps.numSats);
						break;

					///////////////////////

					case 1:
						cliPortPrintF("%3d: ", gps.numCh);

						for (index = 0; index < gps.numCh; index++)
							cliPortPrintF("%3d  ", gps.chn[index]);

						cliPortPrint("\n");

						break;

					///////////////////////

					case 2:
						cliPortPrintF("%3d: ", gps.numCh);

						for (index = 0; index < gps.numCh; index++)
							cliPortPrintF("%3d  ", gps.svid[index]);

						cliPortPrint("\n");

						break;

					///////////////////////

					case 3:
						cliPortPrintF("%3d: ", gps.numCh);

						for (index = 0; index < gps.numCh; index++)
							cliPortPrintF("%3d  ", gps.cno[index]);

						cliPortPrint("\n");

						break;

					///////////////////////
				}

				validCliCommand = false;
				break;

			///////////////////////////////

			case 'o':
				cliPortPrintF("%9.4f\n", batteryVoltage);

				validCliCommand = false;
				break;

			///////////////////////////////

			case 'p': // Not Used
				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'q': // Not Used
				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'r':
				if (flightMode == RATE)
					cliPortPrint("Flight Mode:RATE      ");
				else if (flightMode == ATTITUDE)
					cliPortPrint("Flight Mode:ATTITUDE  ");
				else if (flightMode == GPS)
					cliPortPrint("Flight Mode:GPS       ");

				if (headingHoldEngaged == true)
					cliPortPrint("Heading Hold:ENGAGED     ");
				else
					cliPortPrint("Heading Hold:DISENGAGED  ");

				switch (verticalModeState)
				{
					case ALT_DISENGAGED_THROTTLE_ACTIVE:
						cliPortPrint("Alt:Disenaged Throttle Active      ");

						break;

					case ALT_HOLD_FIXED_AT_ENGAGEMENT_ALT:
						cliPortPrint("Alt:Hold Fixed at Engagement Alt   ");

						break;

					case ALT_HOLD_AT_REFERENCE_ALTITUDE:
						cliPortPrint("Alt:Hold at Reference Alt          ");

						break;

					case VERTICAL_VELOCITY_HOLD_AT_REFERENCE_VELOCITY:
						cliPortPrint("Alt:Velocity Hold at Reference Vel ");

						break;

					case ALT_DISENGAGED_THROTTLE_INACTIVE:
						cliPortPrint("Alt:Disengaged Throttle Inactive   ");

						break;
				}

				if (rxCommand[AUX3] > MIDCOMMAND)
					cliPortPrint("Mode:Simple  ");
				else
					cliPortPrint("Mode:Normal  ");

				if (rxCommand[AUX4] > MIDCOMMAND)
					cliPortPrint("Emergency Bail:Active\n");
				else
					cliPortPrint("Emergency Bail:Inactive\n");

				validCliCommand = false;
				break;

			///////////////////////////////

			case 's': // Raw Receiver Commands
				if ((eepromConfig.receiverType == SPEKTRUM) && (maxChannelNum > 0))
                {
		    		for (index = 0; index < maxChannelNum - 1; index++)
                         cliPortPrintF("%4ld, ", spektrumBuf[index]);

                    cliPortPrintF("%4ld\n", spektrumBuf[maxChannelNum - 1]);
			    }
				else if (eepromConfig.receiverType == SBUS)
				{
		    		for (index = 0; index < 7; index++)
                         cliPortPrintF("%4ld, ", sBusRead(index));

                    cliPortPrintF("%4ld\n", sBusRead(7));

				}
				else
				{
					for (index = 0; index < numChannels - 1; index++)
						cliPortPrintF("%4i, ", Inputs[index].pulseWidth);

					cliPortPrintF("%4i\n", Inputs[numChannels - 1].pulseWidth);
				}

				validCliCommand = false;
				break;

			///////////////////////////////

			case 't': // Processed Receiver Commands
				for (index = 0; index < numChannels - 1; index++)
					cliPortPrintF("%8.2f, ", rxCommand[index]);

				cliPortPrintF("%8.2f\n", rxCommand[numChannels - 1]);

				validCliCommand = false;
				break;

			///////////////////////////////

			case 'u': // Command in Detent Discretes
				cliPortPrintF("%s, ", commandInDetent[ROLL ] ? " true" : "false");
				cliPortPrintF("%s, ", commandInDetent[PITCH] ? " true" : "false");
				cliPortPrintF("%s\n", commandInDetent[YAW  ] ? " true" : "false");

				validCliCommand = false;
				break;

			///////////////////////////////

			case 'v': // ESC PWM Outputs
				cliPortPrintF("%4ld, ", TIM8->CCR4);
				cliPortPrintF("%4ld, ", TIM8->CCR3);
				cliPortPrintF("%4ld, ", TIM8->CCR2);
				cliPortPrintF("%4ld, ", TIM8->CCR1);
				cliPortPrintF("%4ld, ", TIM2->CCR1);
				cliPortPrintF("%4ld, ", TIM2->CCR2);
				cliPortPrintF("%4ld, ", TIM3->CCR1);
				cliPortPrintF("%4ld\n", TIM3->CCR2);

				validCliCommand = false;
				break;

			///////////////////////////////

			case 'w': // Servo PWM Outputs
				cliPortPrintF("%4ld, ", TIM5->CCR3);
				cliPortPrintF("%4ld, ", TIM5->CCR2);
				cliPortPrintF("%4ld\n", TIM5->CCR1);

				validCliCommand = false;
				break;

			///////////////////////////////

			case 'x':
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'y': // ESC Calibration
				escCalibration();

				cliQuery = 'x';
				break;

			///////////////////////////////

			case 'z':	// ADC readings
				cliPortPrintF("%8.4f, %8.4f, %8.4f, %8.4f, %8.4f, %8.4f, %8.4f\n", adcValue(1),
																				   adcValue(2),
																				   adcValue(3),
																				   adcValue(4),
																				   adcValue(5),
																				   adcValue(6),
																				   adcValue(7));
				break;

			///////////////////////////////

			///////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////

			///////////////////////////////

			case 'A': // Read Roll Rate PID Values
				readCliPID(ROLL_RATE_PID);
				cliPortPrint( "\nRoll Rate PID Received....\n" );

				cliQuery = 'a';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'B': // Read Pitch Rate PID Values
				readCliPID(PITCH_RATE_PID);
				cliPortPrint( "\nPitch Rate PID Received....\n" );

				cliQuery = 'a';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'C': // Read Yaw Rate PID Values
				readCliPID(YAW_RATE_PID);
				cliPortPrint( "\nYaw Rate PID Received....\n" );

				cliQuery = 'a';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'D': // Read Roll Attitude PID Values
				readCliPID(ROLL_ATT_PID);
				cliPortPrint( "\nRoll Attitude PID Received....\n" );

				cliQuery = 'b';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'E': // Read Pitch Attitude PID Values
				readCliPID(PITCH_ATT_PID);
				cliPortPrint( "\nPitch Attitude PID Received....\n" );

				cliQuery = 'b';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'F': // Read Heading Hold PID Values
				readCliPID(HEADING_PID);
				cliPortPrint( "\nHeading PID Received....\n" );

				cliQuery = 'b';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'G': // Read nDot PID Values
				readCliPID(NDOT_PID);
				cliPortPrint( "\nnDot PID Received....\n" );

				cliQuery = 'c';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'H': // Read eDot PID Values
				readCliPID(EDOT_PID);
				cliPortPrint( "\neDot PID Received....\n" );

				cliQuery = 'c';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'I': // Read hDot PID Values
				readCliPID(HDOT_PID);
				cliPortPrint( "\nhDot PID Received....\n" );

				cliQuery = 'c';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'J': // Read n PID Values
				readCliPID(N_PID);
				cliPortPrint( "\nn PID Received....\n" );

				cliQuery = 'd';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'K': // Read e PID Values
				readCliPID(E_PID);
				cliPortPrint( "\ne PID Received....\n" );

				cliQuery = 'd';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'L': // Read h PID Values
				readCliPID(H_PID);
				cliPortPrint( "\nh PID Received....\n" );

				cliQuery = 'd';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'M': // MAX7456 CLI
				max7456CLI();

				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'N': // Mixer CLI
				mixerCLI();

				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'O': // Receiver CLI
				receiverCLI();

				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'P': // Sensor CLI
				sensorCLI();

				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'Q': // GPS Data Selection
				gpsDataType = (uint8_t)readFloatCLI();

				cliPortPrint("\n");

				cliQuery = 'n';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'R': // Reset to Bootloader
				cliPortPrint("Entering Bootloader....\n\n");
				delay(100);
				systemReset(true);
				break;

			///////////////////////////////

			case 'S': // Reset System
				cliPortPrint("\nSystem Reseting....\n\n");
				delay(100);
				systemReset(false);
				break;

			///////////////////////////////

			case 'T': // Telemetry CLI
				telemetryCLI();

				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'U': // EEPROM CLI
				eepromCLI();

				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'V': // Reset EEPROM Parameters
				cliPortPrint( "\nEEPROM Parameters Reset....\n" );
				checkFirstTime(true);
				cliPortPrint("\nSystem Resetting....\n\n");
				delay(100);
				systemReset(false);
				break;

			///////////////////////////////

			case 'W': // Write EEPROM Parameters
				cliPortPrint("\nWriting EEPROM Parameters....\n");
				writeEEPROM();

				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'X': // Not Used
				cliQuery = 'x';
				validCliCommand = false;
				break;

			///////////////////////////////

			case 'Y': // ADC CLI
				 adcCLI();

				 cliQuery = 'x';
				 validCliCommand = false;
				 break;

			///////////////////////////////

			case 'Z': // Not Used
				computeGeoMagElements();

				cliQuery = 'x';
				break;

			///////////////////////////////

			case '?': // Command Summary
				cliBusy = true;

				cliPortPrint("\n");
				cliPortPrint("'a' Rate PIDs                              'A' Set Roll Rate PID Data   AP;I;D;N\n");
				cliPortPrint("'b' Attitude PIDs                          'B' Set Pitch Rate PID Data  BP;I;D;N\n");
				cliPortPrint("'c' Velocity PIDs                          'C' Set Yaw Rate PID Data    CP;I;D;N\n");
				cliPortPrint("'d' Position PIDs                          'D' Set Roll Att PID Data    DP;I;D;N\n");
				cliPortPrint("'e' Loop Delta Times                       'E' Set Pitch Att PID Data   EP;I;D;N\n");
				cliPortPrint("'f' Loop Execution Times                   'F' Set Hdg Hold PID Data    FP;I;D;N\n");
				cliPortPrint("'g' 500 Hz Accels                          'G' Set nDot PID Data        GP;I;D;N\n");
				cliPortPrint("'h' 100 Hz Earth Axis Accels               'H' Set eDot PID Data        HP;I;D;N\n");
				cliPortPrint("'i' 500 Hz Gyros                           'I' Set hDot PID Data        IP;I;D;N\n");
				cliPortPrint("'j' 10 hz Mag Data                         'J' Set n PID Data           JP;I;D;N\n");
				cliPortPrint("'k' Vertical Axis Variable                 'K' Set e PID Data           KP;I;D;N\n");
				cliPortPrint("'l' Attitudes                              'L' Set h PID Data           LP;I;D;N\n");
				cliPortPrint("\n");

				cliPortPrint("Press space bar for more, or enter a command....\n");

				while (cliPortAvailable() == false);

				cliQuery = cliPortRead();

				if (cliQuery != ' ')
				{
					validCliCommand = true;
					cliBusy = false;
					return;
				}

				cliPortPrint("\n");
				cliPortPrint("'m' Axis PIDs                              'M' MAX7456 CLI\n");
				cliPortPrint("'n' GPS Data                               'N' Mixer CLI\n");
				cliPortPrint("'o' Battery Voltage                        'O' Receiver CLI\n");
				cliPortPrint("'p' Not Used                               'P' Sensor CLI\n");
				cliPortPrint("'q' Not Used                               'Q' GPS Data Selection\n");
				cliPortPrint("'r' Mode States                            'R' Reset and Enter Bootloader\n");
				cliPortPrint("'s' Raw Receiver Commands                  'S' Reset\n");
				cliPortPrint("'t' Processed Receiver Commands            'T' Telemetry CLI\n");
				cliPortPrint("'u' Command In Detent Discretes            'U' EEPROM CLI\n");
				cliPortPrint("'v' Motor PWM Outputs                      'V' Reset EEPROM Parameters\n");
				cliPortPrint("'w' Servo PWM Outputs                      'W' Write EEPROM Parameters\n");
				cliPortPrint("'x' Terminate Serial Communication         'X' Not Used\n");
				cliPortPrint("\n");

				cliPortPrint("Press space bar for more, or enter a command....\n");

				while (cliPortAvailable() == false);

				cliQuery = cliPortRead();

				if (cliQuery != ' ')
				{
					validCliCommand = true;
					cliBusy = false;
					return;
				}

				cliPortPrint("\n");
				cliPortPrint("'y' ESC Calibration/Motor Verification     'Y' ADC CLI\n");
				cliPortPrint("'z' ADC Values                             'Z' WMM Test\n");
				cliPortPrint("                                           '?' Command Summary\n");
				cliPortPrint("\n");

				cliQuery = 'x';
				cliBusy = false;
				break;

				///////////////////////////////
		}
    }
}

///////////////////////////////////////////////////////////////////////////////
