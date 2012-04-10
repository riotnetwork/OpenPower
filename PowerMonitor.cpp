 /*
Arduino Maxim 78M6613 energy monitor SOC driver running FW 6613_OMU_2+2S_URT_V1_00 
using a simple Tx/Rx serial connection over Arduino hardware UART( 78M6613 speaks at 38400 which is a bit fast fo me to get softwareserial to work reliably)

send a request/command
get a reply.

Credits:
	TextFinder 						http://www.arduino.cc/playground/Code/TextFinder
	modified Streaming library  	Mikal Hart http://arduiniana.org/libraries/streaming/
	Maxim Dallas semiconductor firmware description documentation people (6613_OMU_2+2S_URT_V1_00 )
	and to Massimo and the Arduino team.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Copyright GPL 2.0 Daniel de Kock 2012
*/   
	#if ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
		#include "WConstants.h"
	#endif
		#include "PowerMonitor.h" //include the declaration for this class
		//#include <avr/pgmspace.h>
		#include "TextFinder.h"
		#include "PMStreaming.h"

	char PowerMonitor::resp_ok[2] = {'O','K'};
	
    //<<constructor>> 
    PowerMonitor::PowerMonitor():_tf(Serial,1) // setup that powermonitor uses the Serial serial software serial port , and textfinder string procesing also uses it
	{
	
    }; // not sure about this semicolon, try to verify with a c/c++ dev
     
    //<<destructor>>
    PowerMonitor::~PowerMonitor(){/*nothing to destruct*/};// also not sure about using a semicolon here
     
	void PowerMonitor::begin(int voltage,int f)								// initialisation of powermionitor, **** include the option to have this initialise for 110V systems or 220/230 etc 
	{
	Serial.begin(38400); // 78M6613 only speaks at 38400 baud, this will hopefully change in later revisions
	//set the defualt values for the shield, Vmax + alarms, frequency + alarms, IAmax, IBmax, IAcreep, IBcreep,
	// VMAX = 356,892 according to the Vdivider on the board
	// f = 50.00 Hz +- 1 Hz)
	// Imax A = 65.4, IAcreep =default, IAmax
	// Imax B = 0  IBcreep =default, IBmax	
	// note that all the factory defaults are for 120 Vrms, 60 Hz power systems, we will change these to correspond to 230V 50 Hz systems now
				delay(2000);	
				Serial.println("I");
				delay(100); // wait 1.5 second for pMon to send its welcome string
				Serial.flush();
				delay(20);
				vMaxSet(374.80);	// set the external RMS voltage corresponding to 250 mVpk input of the ADC (A0) // 332.222 on V0.2, 356.892 on V 1, V0.2 477.8021536

				iMaxSet(1,65.470);	// set the external RMS current corresponding to 250 mVpk input of the ADC (A2)(A4)
				alarmCurrentMaxThreshold( 65.000); // threshold for max current alarm, factory  default = 15.000(D9) // channel A only
				
				if(voltage >=100 && voltage <=120)
		{
				alarmVoltageSAGthreshold(80.0); // threshold for voltage SAG detection  factory defualt = 80.0( D4)
				alarmVoltageMinThreshold(100.000); // threshold for min voltage alarm, factory  default = 100.000 (D5)
				alarmVoltageMaxThreshold(140.000); // threshold for max voltage alarm, factory  default = 140.000 (D6)		
		}
				if(voltage >=200 && voltage <=240)
		{
				alarmVoltageSAGthreshold(160.0); // threshold for voltage SAG detection  factory defualt = 80.0( D4)
				alarmVoltageMinThreshold(200.000); // threshold for min voltage alarm, factory  default = 100.000 (D5)
				alarmVoltageMaxThreshold(260.000); // threshold for max voltage alarm, factory  default = 140.000 (D6)
				alarmCurrentMaxThreshold( 65.000); // threshold for max current alarm, factory  default = 15.000(D9) // channel A only
		}
				
				if(f >=49 &&f <=51)
		{
				alarmFreqMinThreshold(49.00); // minimum threshold for frequency alarm factory  defualt = 59.00 Hz ( D2)
				alarmFreqMaxThreshold(51.00); // maximum threshold for frequency alarm factory defualt = 61.00 Hz ( D3)
		}
				if(f >=59 &&f <=61)
		{
				alarmFreqMinThreshold(59.00); // minimum threshold for frequency alarm factory  defualt = 59.00 Hz ( D2)
				alarmFreqMaxThreshold(61.00); // maximum threshold for frequency alarm factory defualt = 61.00 Hz ( D3)
		}
		
				applyDefaultSettings(); // U, note, CE must be disabled before sending U, and reenabsed afterwards			
				softReset();			// reset device
				delay(1000);
				Serial.flush();
	}
	
	void PowerMonitor::alarmFreqMinThreshold(double value) // minimum threshold for frequency alarm defualt = 59.00 Hz ( D2)
	{
	Serial << ")D2=+" << value <<crl;// set parameter
	processResponse(")D2");
	}	
			
	void PowerMonitor::alarmFreqMaxThreshold(double value) // maximum threshold for frequency alarm defualt = 61.00 Hz ( D3)
	{
	Serial << ")D3=+" << value <<crl;// set parameter
	processResponse(")D3");
	}	
			
	void PowerMonitor::alarmVoltageSAGthreshold(double value) // threshold for voltage SAG detection defualt = 80.0( D4)
	{
	Serial << ")D4=+" << value <<crl;// set parameter
	processResponse(")D4");
	}	
			
	void PowerMonitor::alarmVoltageMinThreshold(double value) // threshold for min voltage alarm, default = 100.000 (D5)
	{
	Serial << ")D5=+" << value <<crl;// set parameter
	processResponse(")D5");
	}
	
	void PowerMonitor::alarmVoltageMaxThreshold(double value) // threshold for max voltage alarm, default = 140.000 (D6)
	{
	Serial << ")D6=+" << value <<crl;// set parameter
	processResponse(")D6");
	}	
	
	void PowerMonitor::alarmCurrentMaxThreshold(double value)// threshold for max current alarm, default = 15.000(D9) // channel A only
	{
	Serial << ")D9=+" << value <<crl;// set parameter
	processResponse(")D9");
	}
	int PowerMonitor::vMaxSet(double value)	// set the external RMS voltage corresponding to 250 mVpk input of the ADC (A0)			
	{
	Serial << ")A0=+" << value <<crl;// set voltage when ADC reads maximum
	return int(processResponse(")A0"));
	}
	
	int PowerMonitor::iMaxSet(int channel,double value)	// set the external RMS current corresponding to 250 mVpk input of the ADC (A2)(A4)
	{
	switch (channel)
		{
		case 1: // first channel
			Serial << ")A2=+" << value <<crl;// set current when ADC reads maximum
			return int(processResponse(")A2"));
			// process response	and determine return value
		break;
		case 2: // second channel
			Serial << ")A4=+" << value <<crl;// set current when ADC reads maximum
			return int(processResponse(")A4"));
		break;
		}	
	}
	
	void PowerMonitor::applyDefaultSettings() // U, note:  CE must be disabled before sending U, and reenabled afterwards			
	{
	//_cell << "AT+CGREG?" <<  crl << endl;
	Serial << "CE0" <<crl;// stop CE
	processResponse("CE0");
	Serial << ")U" <<crl;// apply defaults
	processResponse(")U");
	Serial << "CE1" <<crl;// start CE
	processResponse("CE1");
	Serial.flush();
	}

	void PowerMonitor::softReset() // does a software reset (Z), all accumilators cleared
	{
	Serial << "Z" << crl;// soft reset
	processResponse("Z");
	}
	
	//********************************************************************************************************//
	//											Multi channel coommands
	//********************************************************************************************************//
	double PowerMonitor::Temperature(int channel) // returns temperature of internals  (20),(60)
	{
	switch(channel)
		{
		case 1:
			Serial << ")20?" << crl;// get channel 1 temperature
			return processResponse(")20")+22;
		break;
		
		case 2:
			Serial << ")60?" << crl;// get channel 2 temperature
			return processResponse(")60")+22;
		break;
		}
	}
	double PowerMonitor::lineFrequency(int channel) // returns line freq in Hz.hz (21)(61)
	{
	double returnvalue ;
	switch(channel)
		{
		case 1:
			Serial << ")21?" << crl;// get channel 1 temperature
			 returnvalue = processResponse(")21");
			return returnvalue;
		break;
		
		case 2:
			Serial << ")61?" << crl;// get channel 2 temperature
			 returnvalue = processResponse(")61");
			return returnvalue;
		break;
		}
	}
	
	
	double PowerMonitor::vrms(int channel) // returns rms Voltage +VVV.vvv (26) (66)		
	{
	double returnvalue;
	switch(channel)
		{
		case 1:
			Serial << ")26?" << crl;// get channel 1
			returnvalue = processResponse(")26");
			return returnvalue;
		break;
		
		case 2:
			Serial << ")66?" << crl;// get channel 2
			returnvalue = processResponse(")66");
			return returnvalue;
		break;
		}
	}	
	
	double PowerMonitor::power(int channel) // returns active power second +WWW.www (27) (67)
	{
	switch(channel)
		{
		case 1:
			Serial << ")27?" << crl;// get channel 1
			return processResponse(")27");
		break;
		
		case 2:
			Serial << ")67?" << crl;// get channel 2
			return processResponse(")67");
		break;
		}
	}
	
	
	double PowerMonitor::powerFactor(int channel) // returns the power factor between -0.950 and 1 (2D) (6D)
	{
	switch(channel)
		{
		case 1:
			Serial << ")2D?" << crl;// get channel 1
			return processResponse(")2D");
		break;
		
		case 2:
			Serial << ")6D?" << crl;// get channel 2
			return processResponse(")6D");
		break;
		}
	}
	//char PowerMonitor::alarms(int channel); // returns alarm bits (22)(62) *##*
	
	double PowerMonitor::irms(int channel) // returns rms Current +III.iii (2A) (6A)
	{
	switch(channel)
		{
		case 1:
			Serial << ")2A?" << crl;// get channel 1
			return processResponse(")2A");
		break;
		
		case 2:
			Serial << ")6A?" << crl;// get channel 2
			return processResponse(")6A");
		break;
		}
	}
	
	

	// void PowerMonitor::watchdogReset() // does a watchdog reset, all accumilators retain their values
	// {
	// Serial << "W" << crl;// watchdog reset
	// delay(1000); // watchdog reset takes at least 1 second
	// processResponse("W");
	// }
	
	// int PowerMonitor::calibrate(int channel) // CAL 
	// {
	// /*	
	// send CAL<cr>
	
	// response
	// TCAL OK
	// VCAL OK
	// ICAL# OK
	// >
	
	// so we want to count 3 OK messages in the final response
	// */	
	// switch(channel)
		// {
		// case 1: // channel 1
			// Serial << "CAL1" << crl;// start channel 1 calibration sequence
		// break;
		
		// case 2:	 // channel 2
			// Serial << "CAL2" << crl;// start channel 2 calibration sequence
		// break;
		// }
		// if (_tf.find(resp_ok)) // TCAL OK
		// {
			// if (_tf.find(resp_ok)) // VCAL OK
			// {
				// if (_tf.find(resp_ok)) // ICAL OK
				// {
				// return 1;
				// }
				// else{return -3;}//ICAL fail
			// }
			// else{return -2;}//VCAL fail
		// }
		// else{return -1;}//TCAL fail
		
	// }
	
	// int PowerMonitor::calibratePower(int channel) // CALW
	// {
	// /*	
	// send CALW<cr>
	
	// response
	// TCAL OK
	// VCAL OK
	// WCAL# OK
	// >
	
	// so we want to count 3 OK messages in the final response
	// */	
	// switch(channel)
		// {
		// case 1: // channel 1
			// Serial << "CALW1" << crl;// start channel 1 calibration sequence
		// break;
		
		// case 2:	 // channel 2
			// Serial << "CALW2" << crl;// start channel 2 calibration sequence
		// break;
		// }
		// if (_tf.find(resp_ok)) // TCAL OK
		// {
			// if (_tf.find(resp_ok)) // VCAL OK
			// {
				// if (_tf.find(resp_ok)) // WCAL OK
				// {
				// Serial.flush(); // get rid of the "\r\n>" still in the buffer
				// return 1;
				// }
				// else{return -3;}//WCAL fail
			// }
			// else{return -2;}//VCAL fail
		// }
		// else{return -1;}//TCAL fail
		
	// }
	
	// int PowerMonitor::calibrateVoltage() // CLV
	// {
	// /*	
	// send CALV<cr>
	
	// response
	// VCAL OK
	// >
	
	// */	
			// Serial << "CLV" << crl;// start Voltage calibration

		// if (_tf.find(resp_ok)) // TCAL OK
		// {
			// return 1;
		// }
		// else{return -1;}//TCAL fail		
	// }
	
	// int PowerMonitor::calibrateCurrent(int channel) //  CLI
	// {
	// /*	
	// send CALI<cr>
	
	// response
	// VCAL OK
	// >
	
	// */	
			// switch(channel)
		// {
		// case 1: // channel 1
			// Serial << "CLI1" << crl;// start channel 1 calibration sequence
		// break;
		
		// case 2:	 // channel 2
			// Serial << "CLI2" << crl;// start channel 2 calibration sequence
		// break;
		// }		
			// if (_tf.find(resp_ok)) // ICAL OK
			// {
			// return 1;
			// }
			// else{return -1;}//ICAL fail			
	// }
	
	// int PowerMonitor::calibratePhase(int channel) //  CLP 
	// {
	// /*	
	
 // one has to set the target phase that you want to calibrate to
	// send CLP<cr>
	
	// response
	// PCAL OK
	// >
	
	// */	
			// switch(channel)
		// {
		// case 1: // channel 1
			// Serial << "CLP1" << crl;// start channel 1 calibration sequence
		// break;
		
		// case 2:	 // channel 2
			// Serial << "CLP2" << crl;// start channel 2 calibration sequence
		// break;
		// }		
			// if (_tf.find(resp_ok)) // PCAL OK
			// {
			// return 1;
			// }
			// else{return -1;}//PCAL fail			
	// }		
	
	// int PowerMonitor::calibrateTemp() // CLT
	// {
	// /*	
	// send CLT<cr>
	
	// response
	// TCAL OK
	// >
	
	// */	
			// Serial << "CLT" << crl;// start Temperature calibration

		// if (_tf.find(resp_ok)) // TCAL OK
		// {
			// return 1;
		// }
		// else{return -1;}//TCAL fail		
	// }
	
	
	// int PowerMonitor::overCurrentEvent(int channel) // returns amount of overcurrent events sensed (23)(63)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")23?" << crl;// get channel 1 overcurrent events
			// return int(processResponse(")23"));
		// break;
		
		// case 2:
			// Serial << ")63?" << crl;// get channel 2 overcurrent events
			// return int(processResponse(")63"));
		// break;
		// }
	// }
	
	// int PowerMonitor::underVoltageEvent() // returns amount of undervoltage events sensed (24)
	// {
			// Serial << ")24?" << crl;
			// return int(processResponse(")24"));
	// }
	// int PowerMonitor::voltageSAGcount() // returns the amount of voltage SAGs (64)
	// {
			// Serial << ")64?" << crl;// get channel 1 overcurrent events
			// return int(processResponse(")64"));	
	// }
	// int PowerMonitor::overVoltageEvent(int channel) // returns amount of overvoltage events sensed (25)(65)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")25?" << crl;// get channel 1
			// return int(processResponse(")25"));
		// break;
		
		// case 2:
			// Serial << ")65?" << crl;// get channel 2
			// return int(processResponse(")65"));
		// break;
		// }
	// }
	
	
	// double PowerMonitor::energy(int channel) // returns accumuliated energy per hour ( Wh ) +WWW.www (28)(68)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")28?" << crl;// get channel 1
			// return processResponse(")28");
		// break;
		
		// case 2:
			// Serial << ")68?" << crl;// get channel 2
			// return processResponse(")68");
		// break;
		// }
	// }
			
	// double PowerMonitor::cost(int channel) // returns cost foal units (Wh) used +CCC.ccc (29)(69)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")29?" << crl;// get channel 1
			// return processResponse(")29");
		// break;
		
		// case 2:
			// Serial << ")69?" << crl;// get channel 2
			// return processResponse(")69");
		// break;
		// }
	// }	
	
	
	
	// double PowerMonitor::reactivePower(int channel) // returns reactive power (VAR) second +WWW.www (2B)(6B)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")2B?" << crl;// get channel 1
			// return processResponse(")2B");
		// break;
		
		// case 2:
			// Serial << ")6B?" << crl;// get channel 2
			// return processResponse(")6B");
		// break;
		// }
	// }
	
	// double PowerMonitor::apparentPower(int channel) // returns apparent power (VA) second +WWW.www (2C) (6C)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")2C?" << crl;// get channel 1
			// return processResponse(")2C");
		// break;
		
		// case 2:
			// Serial << ")6C?" << crl;// get channel 2
			// return processResponse(")6C");
		// break;
		// }
	// }
	
	
	
	// double PowerMonitor::phaseAngle(int channel) // returns the phase angle in degrees (-180 to 180) +DDD.ddd (2E) (6E)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")2E?" << crl;// get channel 1
			// return processResponse(")2E");
		// break;
		
		// case 2:
			// Serial << ")6E?" << crl;// get channel 2
			// return processResponse(")6E");
		// break;
		// }
	// }
	
	// double PowerMonitor::vrmsMin(int channel) // returns minimum rms Voltage +VVV.vvv (30) (70)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")30?" << crl;// get channel 1
			// return processResponse(")30");
		// break;
		
		// case 2:
			// Serial << ")70?" << crl;// get channel 2
			// return processResponse(")70");
		// break;
		// }
	// }
	
	// double PowerMonitor::vrmsMax(int channel) // returns maximum rms Voltage +VVV.vvv (31) (71)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")31?" << crl;// get channel 1
			// return processResponse(")31");
		// break;
		
		// case 2:
			// Serial << ")71?" << crl;// get channel 2
			// return processResponse(")71");
		// break;
		// }
	// }
	
	// double PowerMonitor::activePowerMin(int channel) // returns minimum active power +WWW.www (32) (72)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")32?" << crl;// get channel 1
			// return processResponse(")32");
		// break;
		
		// case 2:
			// Serial << ")72?" << crl;// get channel 2
			// return processResponse(")72");
		// break;
		// }
	// }
	
	// double PowerMonitor::activePowerMax(int channel) // returns maximum active power +WWW.www (33) (73)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")33?" << crl;// get channel 1
			// return processResponse(")33");
		// break;
		
		// case 2:
			// Serial << ")73?" << crl;// get channel 2
			// return processResponse(")73");
		// break;
		// }
	// }
	
	// double PowerMonitor::irmsMin(int channel) // returns minimum rms Current +III.iii (34) (74)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")34?" << crl;// get channel 1
			// return processResponse(")34");
		// break;
		
		// case 2:
			// Serial << ")74?" << crl;// get channel 2
			// return processResponse(")74");
		// break;
		// }
	// }
	
	// double PowerMonitor::irmsMax(int channel) // returns maximum rms Current +III.iii (35) (75)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")35?" << crl;// get channel 1
			// return processResponse(")35");
		// break;
		
		// case 2:
			// Serial << ")75?" << crl;// get channel 2
			// return processResponse(")75");
		// break;
		// }
	// }
	
	// double PowerMonitor::reactivePowerMin(int channel) // returns minimum reactive power (VAR) +WWW.www (36) (76)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")36?" << crl;// get channel 1
			// return processResponse(")36");
		// break;
		
		// case 2:
			// Serial << ")76?" << crl;// get channel 2
			// return processResponse(")76");
		// break;
		// }
	// }
	
	// double PowerMonitor::reactivePowerMax(int channel) // returns maximum reactive power (VAR) +WWW.www (37) (77)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")37?" << crl;// get channel 1
			// return processResponse(")37");
		// break;
		
		// case 2:
			// Serial << ")77?" << crl;// get channel 2
			// return processResponse(")77");
		// break;
		// }
	// }
	
	// double PowerMonitor::apparentPowerMin(int channel) // returns minimum apparent power (VA) +WWW.www (38) (78)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")38?" << crl;// get channel 1
			// return processResponse(")38");
		// break;
		
		// case 2:
			// Serial << ")78?" << crl;// get channel 2
			// return processResponse(")78");
		// break;
		// }
	// }
	
	// double PowerMonitor::apparentPowerMax(int channel) // returns maximum apparent power (VA) +WWW.www (39) (79)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")39?" << crl;// get channel 1
			// return processResponse(")39");
		// break;
		
		// case 2:
			// Serial << ")79?" << crl;// get channel 2
			// return processResponse(")79");
		// break;
		// }
	// }
	
	// double PowerMonitor::powerFactorMin(int channel) // returns minimum power factor +DDD.ddd (3A) (7A)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")3A?" << crl;// get channel 1
			// return processResponse(")3A");
		// break;
		
		// case 2:
			// Serial << ")7A?" << crl;// get channel 2
			// return processResponse(")7A");
		// break;
		// }
	// }
	
	// double PowerMonitor::powerFactorMax(int channel) // returns maximum power factor  +DDD.ddd (3B) (7B)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")3B?" << crl;// get channel 1
			// return processResponse(")3B");
		// break;
		
		// case 2:
			// Serial << ")7B?" << crl;// get channel 2
			// return processResponse(")7B");
		// break;
		// }
	// }
	
	// double PowerMonitor::phaseAngleMin(int channel) // returns minimum phase Angle +DDD.ddd (3C) (7C)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")3C?" << crl;// get channel 1
			// return processResponse(")3C");
		// break;
		
		// case 2:
			// Serial << ")7C?" << crl;// get channel 2
			// return processResponse(")7C");
		// break;
		// }
	// }
	
	// double PowerMonitor::phaseAngleMax(int channel) // returns maximum phaseAngle   +DDD.ddd (3D) (7D)
	// {
	// switch(channel)
		// {
		// case 1:
			// Serial << ")3D?" << crl;// get channel 1
			// return processResponse(")3D");
		// break;
		
		// case 2:
			// Serial << ")7D?" << crl;// get channel 2
			// return processResponse(")7D");
		// break;
		// }
	// }
//	********************************************************************************************************//
							//multiple outlet totals of both channels combined
//	********************************************************************************************************//
	// double PowerMonitor::totalPower() // returns active power second  +WWW.www (90)
	// {
	// Serial << ")90?" << crl;// get value
			// return processResponse(")90");
	// }
	
	// double PowerMonitor::totalEnergy() // returns accumuliated energy per hour ( Wh ) +WWW.www (91)
	// {
	// Serial << ")91?" << crl;// get value
			// return processResponse(")91");
	// }
	
	// double PowerMonitor::totalCost() // returns cost foal units (Wh) used +CCC.ccc (92)	
	// {
	// Serial << ")92?" << crl;// get value
			// return processResponse(")92");
	// }			
	// double PowerMonitor::totalIrms() // returns rms Current +III.iii (93)
	// {
	// Serial << ")93?" << crl;// get value
			// return processResponse(")93");
	// }
	// double PowerMonitor::totalReactivePower() // returns reactive power (VAR) second +WWW.www (94)
	// {
	// Serial << ")94?" << crl;// get value
			// return processResponse(")94");
	// }
	// double PowerMonitor::totalApparentPower() // returns apparent power (VA) second +WWW.www (95)
	// {
	// Serial << ")95?" << crl;// get value
			// return processResponse(")95");
	// }
	// int PowerMonitor::totalOverCurrentEvent() // returns amount of overcurrent events sensed (96)
	// {
	// Serial << ")96?" << crl;// get value
			// return int(processResponse(")96"));
	// }
	// double PowerMonitor::activePowerMin() // returns minimum active power +WWW.www (98)
	// {
	// Serial << ")98?" << crl;// get value
			// return processResponse(")98");
	// }
	// double PowerMonitor::activePowerMax() // returns maximum active power +WWW.www (99)
	// {
	// Serial << ")99?" << crl;// get value
			// return processResponse(")99");
	// }
	// double PowerMonitor::irmsMin() // returns minimum rms Current +III.iii (9A)
	// {
	// Serial << ")9A?" << crl;// get value
			// return processResponse(")9A");
	// }
	// double PowerMonitor::irmsMax() // returns maximum rms Current +III.iii (9B)
	// {
	// Serial << ")9B?" << crl;// get value
			// return processResponse(")9B");
	// }
	// double PowerMonitor::reactivePowerMin() // returns minimum reactive power (VAR) +WWW.www (9C)
	// {
	// Serial << ")9C?" << crl;// get value
			// return processResponse(")9C");
	// }
	// double PowerMonitor::reactivePowerMax() // returns maximum reactive power (VAR) +WWW.www (9D)
	// {
	// Serial << ")9D?" << crl;// get value
			// return processResponse(")9D");
	// }
	// double PowerMonitor::apparentPowerMin() // returns minimum apparent power (VA) +WWW.www (9E)
	// {
	// Serial << ")9E?" << crl;// get value
			// return processResponse(")9E");
	// }
	// double PowerMonitor::apparentPowerMax() // returns maximum apparent power (VA) +WWW.www (9F)
	// {
	// Serial << ")9F?" << crl;// get value
			// return processResponse(")9F");
	// }
	
	 
	
//	********************************************************************************************************//
							// settable parameters
//	********************************************************************************************************//
	
	// int PowerMonitor::iStartSet(int channel,double value)// set the threshold for when it starts regarding a measurement ( default is 7 mA) (A1)(A3)
	// {
	// switch (channel)
		// {
		// case 1: // first channel
			// Serial << ")A1=+" << value <<crl;// set current when regarded as measurable ( usually after the knee point of a CT )
			// return int(processResponse(")A1"));
		//	process response	and determine return value
		// break;
		// case 2: // second channel
			// Serial << ")A3=+" << value <<crl;// set current when regarded as measurable ( usually after the knee point of a CT )
			// return int(processResponse(")A3"));
		// break;
		// }	
	// }
	// int PowerMonitor::costSet(double value)	// set the cost per unit (kWh) in milliunits +CC.ccc (AA) 
	// {
	// Serial << ")AA=+" << value <<crl;// set cost per kWh
	// return int(processResponse(")AA"));
	// }
	
	// int PowerMonitor::costUnits (const char* unit) // set the units of cost eg USD, EUR, ZAR, 4 char max, and pad with spaces to fill to 4 (AB)
	// {
	// char costunit[] = {0x20,0x20,0x20,0x20}; // start with spaces only, and fill up: 0x20 is ASCII space
	// for(char i=0;i<4;i++)					//0,1,2,3
		// {
		// costunit[i] = unit[i];
		// }
	// Serial << ")AB=\"" << costunit <<"\""<<crl;// set the monetary units
	// return int(processResponse(")AB"));
	// }
	
	// void PowerMonitor::minMax(bool control) // start or stop min max recording, 3 =reset and start, 00 = stop, (F1), thus, True= reset and start, false= stop
	 // {
	 // switch(control)
		 // {
		 // case true://reset and start
			// Serial << ")F1=3" << crl;
			// processResponse(")F1");
		 // break;
		 
		 // case false://stop
			// Serial << ")F1=0" << crl;
			// processResponse(")F1");
		 // break;		 
		 // }
	 // }
//	********************************************************************************************************//
							// relay configuration
//	********************************************************************************************************//
	// /*
	// int relayConfig(boolean polarity,boolean latch) // Bit0 = relay polarity(0,1) = (notrmal,inverted) , Bit1 = Relay type (0,1) = (non-latched, latched)(AC)
	// {
	// switch(polarity)
		// {
		// case true: // inverted polarity	
		// break;
		
		// case false:// normal polarity
		// break;
		// }
	// }
	// */
	// int PowerMonitor::relay(int channel, int state) // control relay outputs, TC
	// {
//	toggle relays
		// switch(channel)
		// {
		// case 0 : 								// both channels off
				// Serial << "TC0" << crl;	// turn both relays off
				// if(processResponse("TC0"))
				// {
				// return 1;
				// }
		// break;
		
		// case 1 :								// relay 1
				// Serial << "TC1" << crl;	// turn relay 1 on
				// if(processResponse("TC1"))
				// {
				// return 1;
				// }
		// break;
		
		// case 2 :								// relay 2
				// Serial << "TC2" << crl;	// turn relay 2 on
				// if(processResponse("TC2"))
				// {
				// return 1;
				// }
		// break;
		
		// case 3 :								// all on
				// Serial << "TC3" << crl;	// turn both relays on
				// if(processResponse("TC3"))
				// {
				// return 1;
				// }
		// break;	
		// }
		// return 0; // we did not get the proper response from the module		
	// }
	// int PowerMonitor::sequenceDelay(double value) // delay between relay actions ( between relay 1 and 2 ) +S.s  defualt is 0.1s (AD)
	// {
	// Serial << ")AD=+" << value <<crl;// set delay between firing both relays
	// return int(processResponse(")AD"));
	// }
	
	// int PowerMonitor::energizeDelay(double value) // energise delay from relay datasheet +S.sss (AE), default = 0.000s
	// {
	// Serial << ")AE=+" << value <<crl;// set energise delay
	// return int(processResponse(")AE"));
	// }
	// int PowerMonitor::denergizeDelay(double value) //denergise delay from relay datasheet +S.sss (AF) default = 0.000s
	// {
	// Serial << ")AF=+" << value <<crl;// set de-energise delay
	// return int(processResponse(")AF"));
	// }
//	********************************************************************************************************//
							// calibration and alarm settables
//	********************************************************************************************************//
	// void PowerMonitor::calPhaseTolerance(double value) // set the phase tolerance for calibration +D.ddd(BF)
	// {
	// Serial << ")BF=+" << value <<crl;//
	// processResponse(")BF");
	// }
	// void PowerMonitor::calLineVoltage(double value)// set the line RMS voltage used during calibration +VVV  defualt is 120V     (C1)
	// {
	// Serial << ")C1=+" << value <<crl;//
	// processResponse(")C1");
	// }
	// void PowerMonitor::calLineCurrent(double value)// set the line RMS current used during calibration +A  defualt is 1.000 A    (C2)
	// {
	// Serial << ")C2=+" << value <<crl;//
	// processResponse(")C2");
	// }
	// void PowerMonitor::calLinePhase(double value) // set the line phase used during calibration +P defualt is 0 degrees		    (C3) 
	// {
	// Serial << ")C3=+" << value <<crl;//
	// processResponse(")C3");
	// }
	// void PowerMonitor::calVoltageTolerance(double value)// set tolerance used during calibration	V.vvv defualt is 0.01 V			(C4)
	// {
	// Serial << ")C4=+" << value <<crl;//
	// processResponse(")C4");
	// }
	// void PowerMonitor::calCurrentTolerance(double value)// set tolerance used during calibration A.aaa defualt is 0.01 A			(C5)
	// {
	// Serial << ")C5=+" << value <<crl;//
	// processResponse(")C5");
	// }
	// void PowerMonitor::calVoltageAveCount(int count) // number of voltage measurements to average during calibration, default = 3 (C6)
	// {
	// Serial << ")C6=+" << count <<crl;//
	// processResponse(")C6");
	// }
	// void PowerMonitor::calCurrentAveCount(int count) // number of voltage measurements to average during calibration, default = 3 (C7)
	// {
	// Serial << ")C7=+" << count <<crl;//
	// processResponse(")C7");
	// }
	// void PowerMonitor::calVoltageIteration(int int_value) // number of attempts to reach the target calibration value before fail, default = 10 (C8)
	// {
	// Serial << ")C8=+" << int_value <<crl;//
	// processResponse(")C8");
	// }
	// void PowerMonitor::calCurrentIteration(int int_value) // number of attempts to reach the target calibration value before fail, default = 10 (C9)
	// {
	// Serial << ")C9=+" << int_value <<crl;//
	// processResponse(")C9");
	// }
	// void PowerMonitor::calWattageTolerance(double value) // set the power tolerance for calibration +W.www, defualt = 0.01W(CA)
	// {
	// Serial << ")CA=+" << value <<crl;//
	// processResponse(")CA");
	// }
	// void PowerMonitor::calWattAveCount(int count) // number of power measurements to average during calibration, default = 3 (CB)
	// {
	// Serial << ")CB=+" << count <<crl;//
	// processResponse(")CB");
	// }
	// void PowerMonitor::calWattIteration(int int_value) // number of attempts to reach the target calibration value before fail, default = 10 (CC)
	// {
	// Serial << ")CC=+" << int_value <<crl;//
	// processResponse(")CC");
	// }
	// void PowerMonitor::calLinePower(double value)// power used for calibration , default = 120, +WWW.www(CF)
	// {
	// Serial << ")CF=+" << value <<crl;//
	// processResponse(")CF");
	// }
	// void PowerMonitor::alarmTempMinThreshold(double value) // minimum threshold for temperature alarm default = 0.0 deg C ( D0)
	// {
	// Serial << ")D0=+" << value <<crl;//
	// processResponse(")D0");
	// }
	// void PowerMonitor::alarmTempMaxThreshold(double value) // maximum threshold for temperature alarm default = 70.0 deg C ( D1)
	// {
	// Serial << ")D1=+" << value <<crl;//
	// processResponse(")D1");
	// }
	
	
	
	// void PowerMonitor::alarmPFnegThreshold(double value) // threshold for negative power factor alarm, default = -0.700(DC) // channel A only
	// {
	// Serial << ")DC=-" << value <<crl;// set parameter
	// processResponse(")DC");
	// }
	// void PowerMonitor::alarmPFposThreshold(double value) // threshold for positive power factor alarm, default = +0.700(DD) // channel A only
	// {
	// Serial << ")DD=+" << value <<crl;// set parameter
	// processResponse(")DD");
	// }
	
	
	//********************************************************************************************************//
	//											RESPONSE HANDLER
	//********************************************************************************************************//
	double PowerMonitor::processResponse(char *cmd) //process response and return the value			
	{
	delay(50); // give the monitor time to respond
	double value = 0.00;
	if(_tf.find(cmd)) 			// find the command we sent ( it gets echoed back to me)
		{
		switch(Serial.read()) // check response type
				{
					case 0x3D:	// =, This is a response to a SET command  CMD,=,sign,value,\r\n\r\n,>					
						// get all the data until we get th ">" character then return 1
						if(_tf.find(">"))
						{
						return 1;
						}
						else
						{
						return 0; // did not find the ">"
						}
					break;
					
					case 0x3E:	// >, This is a response to a reset or something that does not return a value	
						Serial.flush(); // clear the buffer , this is used because the unit replies with a very long message that we are not interested in
						return 1;
					break;
					
					case 0x3F:	// ?,  This is a response to a QUERY command : CMD,?,\r\n,sign,value,\r\n,>
						if(_tf.find("\r\n"))		// find the delimiter
						{
							switch(Serial.read()) // check the sign (pos or neg value)
							{
								case 0x2B:	// 0x2B, plus sign, +
								value = _tf.getFloat(); // get the value from the string
								break;
								
								case 0x2D:	// 0x2D, minus sign, -
								value = _tf.getFloat()*-1; // get the value from the string and negate it
								break;
							}
							if(_tf.find(">"))		// find the delimiter
								{
									Serial.flush();
									return value;
								}

						}			
						else // did not find the 1st delimiter
						{
						return 0;
						}
					break;
				}
					
		}
		else // did not find the command string
		{
		return 0;
		}
	}