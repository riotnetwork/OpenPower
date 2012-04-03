/*
Arduino Maxim 78M6613 energy monitor SOC driver running FW 6613_OMU_2+2S_URT_V1_00 
using a simple Tx/Rx serial connection.

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
  

  #ifndef PowerMonitor_H
    #define PowerMonitor_H
     
	 #if ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
		#include "WConstants.h"
	#endif
	#include "TextFinder.h"
	#include "PMStreaming.h"
	 
	 
	 
    class PowerMonitor {
    public:
            PowerMonitor();
            ~PowerMonitor();
			// declare all our functions here
			
			void begin(int voltage,int f);
            void softReset(); // Z
			void watchdogReset(); // W
			void applyDefaultSettings(); // U, note, CE must be disabled before sending U, and reenabled afterwards
						
			int calibrate(int channel); // CAL 
			int calibratePower(int channel); // CALW
			int calibrateVoltage(); // CLV
			int calibrateCurrent(int channel); //  CLI
			int calibratePhase(int channel); //  CLP 
			int calibrateTemp(); // CLT
				
			
			
			 // multi channel commands
			
			int Temperature(int channel); // returns temperature of internals  CC.cc (20),(60)
			double lineFrequency(int channel); // returns line freq in Hz.hz (21)(61)
			double power(int channel); // returns active power second +WWW.www (27) (67)
			double vrms(int channel); // returns rms Voltage +VVV.vvv (26) (66)		
			
			char alarms(int channel); // returns alarm bits (22)(62)
			int overCurrentEvent(int channel); // returns amount of overcurrent events sensed (23)(63)
			int underVoltageEvent(); // returns amount of undervoltage events sensed (24)
			int voltageSAGcount(); // returns the amount of voltage SAGs (64)
			int overVoltageEvent(int channel); // returns amount of overvoltage events sensed (25)(65)
										   
			
			double energy(int channel); // returns accumuliated energy per hour ( Wh ) +WWW.www (28)(68)
			double cost(int channel); // returns cost foal units (Wh) used +CCC.ccc (29)(69)				
			double irms(int channel); // returns rms Current +III.iii (2A) (6A)
			double reactivePower(int channel); // returns reactive power (VAR) second +WWW.www (2B)(6B)
			double apparentPower(int channel); // returns apparent power (VA) second +WWW.www (2C) (6C)
			double powerFactor(int channel); // returns the power factor between -0.950 and 1 (2D) (6D)
			double phaseAngle(int channel); // returns the phase angle in degrees (-180 to 180) +DDD.ddd (2E) (6E)
			double vrmsMin(int channel); // returns minimum rms Voltage +VVV.vvv (30) (70)
			double vrmsMax(int channel); // returns maximum rms Voltage +VVV.vvv (31) (71)
			double activePowerMin(int channel); // returns minimum active power +WWW.www (32) (72)
			double activePowerMax(int channel); // returns maximum active power +WWW.www (33) (73)
			double irmsMin(int channel); // returns minimum rms Current +III.iii (34) (74)
			double irmsMax(int channel); // returns maximum rms Current +III.iii (35) (75)
			double reactivePowerMin(int channel); // returns minimum reactive power (VAR) +WWW.www (36) (76)
			double reactivePowerMax(int channel); // returns maximum reactive power (VAR) +WWW.www (37) (77)
			double apparentPowerMin(int channel); // returns minimum apparent power (VA) +WWW.www (38) (78)
			double apparentPowerMax(int channel); // returns maximum apparent power (VA) +WWW.www (39) (79)
			double powerFactorMin(int channel); // returns minimum power factor +DDD.ddd (3A) (7A)
			double powerFactorMax(int channel); // returns maximum power factor  +DDD.ddd (3B) (7B)
			double phaseAngleMin(int channel); // returns minimum phase Angle +DDD.ddd (3C) (7C)
			double phaseAngleMax(int channel); // returns maximum phaseAngle   +DDD.ddd (3D) (7D)
			 
			
			//multiple outlet totals of both channels combined
			double totalPower(); // returns active power second  +WWW.www (90)
			double totalEnergy(); // returns accumuliated energy per hour ( Wh ) +WWW.www (91)
			double totalCost(); // returns cost foal units (Wh) used +CCC.ccc (92)				
			double totalIrms(); // returns rms Current +III.iii (93)
			double totalReactivePower(); // returns reactive power (VAR) second +WWW.www (94)
			double totalApparentPower(); // returns apparent power (VA) second +WWW.www (95)
			int totalOverCurrentEvent(); // returns amount of overcurrent events sensed (96)
			double activePowerMin(); // returns minimum active power +WWW.www (98)
			double activePowerMax(); // returns maximum active power +WWW.www (99)
			double irmsMin(); // returns minimum rms Current +III.iii (9A)
			double irmsMax(); // returns maximum rms Current +III.iii (9B)
			double reactivePowerMin(); // returns minimum reactive power (VAR) +WWW.www (9C)
			double reactivePowerMax(); // returns maximum reactive power (VAR) +WWW.www (9D)
			double apparentPowerMin(); // returns minimum apparent power (VA) +WWW.www (9E)
			double apparentPowerMax(); // returns maximum apparent power (VA) +WWW.www (9F)
			
			// settable parameters
			int vMaxSet(double value);	// set the external RMS voltage corresponding to 250 mVpk input of the ADC (A0)
			int iStartSet(int channel,double value);// set the threshold for when it starts regarding a measurement ( default is 7 mA) (A1)(A3)
			int iMaxSet(int channel,double value);	// set the external RMS current corresponding to 250 mVpk input of the ADC (A2)(A4)
			int costSet(double value);	// set the cost per unit (kWh) in milliunits +CC.ccc (AA) 
			int costUnits (const char* unit); // set the units of cost eg USD, EUR, ZAR, 4 char max, and pad with spaces to fill to 4 (AB)
			void minMax(bool control); // start or stop min max recording, 3 =reset and start, 00 = stop, (F1)
			
			// relay configuration
		//	int relayConfig(boolean polarity,boolean latch); // Bit0 = relay polarity(0,1) = (notrmal,inverted) , Bit1 = Relay type (0,1) = (non-latched, latched)(AC)
			int relay(int channel, int state); // control relay outputs, TC
			int sequenceDelay(double value); // delay between relay actions ( between relay 1 and 2 ) +S.s  defualt is 0.1s (AD)
			int energizeDelay(double value); // energise delay from relay datasheet +S.sss (AE)
			int denergizeDelay(double value); //denergise delay from relay datasheet +S.sss (AF)
			
			// calibration and alarm settables
		//	char configuration();// additional status bytes (BD) (reserved, WPULSE disable, VCal failure, ICal1 failure,WCal1 failure) bits 0-4
			void calPhaseTolerance(double value); // set the phase tolerance for calibration +D.ddd(BF)
			void calLineVoltage(double value);// set the line RMS voltage used during calibration +VVV  defualt is 120V     (C1)
			void calLineCurrent(double value);// set the line RMS current used during calibration +A  defualt is 1.000 A    (C2)
			void calLinePhase(double value); // set the line phase used during calibration +P defualt is 0 degrees		    (C3) 
			void calVoltageTolerance(double value);// set tolerance used during calibration	V.vvv defualt is 0.01 V			(C4)
			void calCurrentTolerance(double value);// set tolerance used during calibration A.aaa defualt is 0.01 A			(C5)
			void calVoltageAveCount(int count); // number of voltage measurements to average during calibration, default = 3 (C6)
			void calCurrentAveCount(int count); // number of voltage measurements to average during calibration, default = 3 (C7)
			void calVoltageIteration(int int_value); // number of attempts to reach the target calibration value before fail, default = 10 (C8)
			void calCurrentIteration(int int_value); // number of attempts to reach the target calibration value before fail, default = 10 (C9)
			void calWattageTolerance(double value); // set the power tolerance for calibration +W.www, defualt = 0.01W(CA)
			void calWattAveCount(int count); // number of power measurements to average during calibration, default = 3 (CB)
			void calWattIteration(int int_value); // number of attempts to reach the target calibration value before fail, default = 10 (CC)
			void calLinePower(double value);// power used for calibration , default = 120, +WWW.www(CF)
			void alarmTempMinThreshold(double value); // minimum threshold for temperature alarm default = 0.0 deg C ( D0)
			void alarmTempMaxThreshold(double value); // maximum threshold for temperature alarm default = 70.0 deg C ( D1)
			void alarmFreqMinThreshold(double value); // minimum threshold for frequency alarm defualt = 59.00 Hz ( D2)
			void alarmFreqMaxThreshold(double value); // maximum threshold for frequency alarm defualt = 61.00 Hz ( D3)
			void alarmVoltageSAGthreshold(double value); // threshold for voltage SAG detection defualt = 80.0( D4)
			void alarmVoltageMinThreshold(double value); // threshold for min voltage alarm, default = 100.000 (D5)
			void alarmVoltageMaxThreshold(double value); // threshold for max voltage alarm, default = 140.000 (D6)
			void alarmCurrentMaxThreshold( double value); // threshold for max current alarm, default = 15.000(D9) // channel A only
			void alarmPFnegThreshold(double value); // threshold for negative power factor alarm, default = -0.700(DC) // channel A only
			void alarmPFposThreshold(double value); // threshold for positive power factor alarm, default = +0.700(DD) // channel A only

			double processResponse(char *cmd); //process response and return the value
	private:
    //int Voltage, Frequency, int_value ,i,channel,state;
	//double value, double_Value;
	//boolean control;
	static char resp_ok[2];
	protected:
	TextFinder _tf;			// textfinder will be used to do string handling 

    };
     
    #endif
