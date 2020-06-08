/*  calibration.h

* Copy this file, rename it to calibration.h
* and move it to the src directory

* Fill it in with your various calibration readings
for the main code pws.ino to access

* Make sure to mention it in your .gitignore
for redistribution without your subjective data!


* Calibration for the soil moisture sensor is done by placing it
about 3/4 of its way inside the target soil, manipulating its moisture
and recording the values read from the sensor:
    * completely dry soil:              0    %RH
    * completely wet soil:              100  %RH

*/


// Soil moisture sensor calibration readings:
const int smCal0 =      ...;    // soil moisture sensor reading at 0 %RH
const int smCal100 =    ...;    // soil moisture sensor reading at 100 %RH