#include <PMStreaming.h>
#include <PowerMonitor.h>
#include <TextFinder.h>
/*
This sketch is compatible with openPower v0.2 board.
it uses the "native" UART on arduino ( usually reserved for USB ) to speak to the 
Maxim 78M6613 energy monitor IC ( at 38400 baudrate, so software serial does not work reliably)

remove the openPower shield when you upload a sketch to arduino, otherwise it wont upload.
use jumpers to link the software serial port from this arduino to rx and tx of another arduino that has been programmed with "void setup(){}void loop(){}"
or alternatively use a sparkfun usb-ttl connector

ensure that all power is turned off on whtever you want to measure and thread one of the live wires throght the current sensor.
insert the 3 pin plug into a socket to be able to monitor the voltage.

High voltage bites... Caution

see the library files for the available functions, Powermonitor.h lists all available functions and what they return
*/

#include <SoftwareSerial.h>
SoftwareSerial swSerial(2,3); // create a software serial port for th epc to listen on ( through another arduino or xbee)
PowerMonitor pmon;            // instance of a powermonitor


#define INTERVAL 2000                    // update interval in milliseconds
#define smillis() ((long)millis())       // This is used to calculate timeouts
long updatetime;                         // used to store th etime till next upodate

double vRms,iRms,realPower,temp; // Vrms, RealPower,aqqumilated energy, Temperature

void setup()  
 {
   swSerial.begin(38400);
   pmon.begin(220,50); // 220V and 50Hz are nominal parameters for south africa
   updatetime= smillis();
 }

void loop() // run over and over
{
  if(after(updatetime))            // when its time to update
  {
   swSerial.print("rms Voltage: \t");
   swSerial.println(pmon.vrms(1));       // rms line Voltage
   swSerial.print("rms Current: \t");
   swSerial.println(pmon.irms(1));       // rms line current
   swSerial.print("Power usage: \t");
   swSerial.println(pmon.power(1));      // power usage ( in Watts)
   swSerial.print("Temperature: \t");
   swSerial.println(pmon.Temperature(1));  // in IC temperature sensor
   swSerial.println();
   
    updatetime= smillis()+INTERVAL;  // "reset" the timer to update again in the next interval
  }
}

boolean after(long timeout)                    // Returns true if a timeout has happened
{
  return smillis()-timeout>0;
}

