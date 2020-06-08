/*

* pws.ino         [Particle-flavoured C++]
   full version: implements all sensors and meters
   check the "simplified" git branch for the simplified version
   only implementing the Photon Weather Shield


* Description:    Particle Photon Personal Weather Station software
                  dedicated to collecting, processing and uploading the data


* Author:         Giorgio Ciacchella (https://github.com/ciakkig)


* License:        GNU GPLv3 license
   Copyright (C) 2020 Giorgio Ciacchella

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License at COPYING or
   https://www.gnu.org/licenses/#GPL for more details.


* Hardware:
   Particle Photon                                    https://store.particle.io/products/photon
   [OPTIONAL] 2.4GHz u.FL antenna                     https://www.sparkfun.com/products/11320
   SparkFun OpenLog                                   https://www.sparkfun.com/products/13712

   SparkFun Photon Weather Shield                     https://www.sparkfun.com/products/13674
   SparkFun Weather Meters                            https://www.sparkfun.com/products/15901
   Dallas DS18B20 Waterproof Temperature Sensor       https://www.sparkfun.com/products/11050
   SparkFun Soil Moisture Sensor                      https://www.sparkfun.com/products/13322
   SparkFun Qwiic VEML6075 UV Light Sensor            https://www.sparkfun.com/products/15089

   3.5W solar panel                                   https://www.sparkfun.com/products/13782
   SparkFun Sunny Buddy                               https://www.sparkfun.com/products/12885
   Custom 3D Modelled Stevenson Screen                https://www.thingiverse.com/thing:4435632


* Creation:       04/2017
   Latest Update: 06/2020


*/


#include <math.h>

#include <SparkFun_Photon_Weather_Shield_Library.h>
#include <OneWire.h>
#include <spark-dallas-temperature.h>
#include <SparkFun_VEML6075.h>
#include <ThingSpeak.h>

#include <calibration.h>
#include <credentials.h>


Weather shield;
OneWire oneWire(D4);
DallasTemperature thermo(&oneWire);
VEML6075 uv;
ThingSpeakClass ts;
TCPClient client;


//===========================================================
// DECLARATIONS
//===========================================================

int sleepAt0 = 1;                   // default flag to determine wheter the Photon goes to sleep at the end of the loop or stays awake
long sampleSecondsDefault = 60;     // default seconds spent by getWeather() in the sampling loop, when not overridden by getWeather()'s argument

double temperatureC;
double humidityRH;
double pressurePa;
double uvIndex;
double soilTemperatureC;
double soilMoistureRH;

double rainMmMin;
double windSpeedKMH;
double windDirection;

double dewPointC;
double heatIndexC;
double windChillC;

volatile unsigned int lastRainClick;
volatile unsigned int totalRainClicks;

volatile unsigned int lastWindClick;
volatile unsigned int totalWindClicks;

SYSTEM_MODE(SEMI_AUTOMATIC);                    // Particle system mode override: semi-automatic for manual control over connection to the cloud
                                                // but automatic processing once that's done
                                                // <https://docs.particle.io/reference/device-os/firmware/photon/#system-modes>

// Particle cloud functions: must be declared before setup, return int and take one String
uint32_t getWeather(String arg);                // returns unsigned, 32-bit integer for weather data at a glance
int logData(String arg);
int publishData(String arg);
int publishThingSpeak(String arg);
int publishWeatherUnderground(String arg);
int publishWeatherCloud(String arg);


//===========================================================
// ROUTINE
//===========================================================

void setup()
{
   STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));   // make sure the external antenna is engaged
                                                // comment it out or edit to ANT_INTERNAL
                                                //  if you're not using an external u.FL antenna

   Time.zone(1);                                // edit this according to your timezone
   Time.beginDST();                             // comment one of these two out and leave the other insampleMillis
   Time.endDST();                               // depending on what time of the year you flash the code
   
   Particle.function("gotoSleepNow", gotoSleepNow);
   Particle.function("setSleepAt0", setSleepAt0);
   Particle.function("setSampleSeconds", setSampleSeconds);
   Particle.function("getWeather", getWeather);
   Particle.function("logData", logData);
   Particle.function("publishData", publishData);
   Particle.function("publishThingSpeak", publishThingSpeak);
   Particle.function("publishWeatherUnderground", publishWeatherUnderground);
   Particle.function("publishWeatherCloud", publishWeatherCloud);
   Particle.connect();

   initialiseWeatherShield();
   initialiseRainGauge();
   initialiseAnemometer();
   initialiseParticleVariables();

   Serial.begin(9600);                          // USB serial interface to PC
   Serial1.begin(9600);                         // RX-TX serial interface the OpenLog
   ThingSpeak.begin(client);
   delay(1000);                                 // one-second delay to allow sensor, serial and remote interfaces to connect safely
   Serial.printlnf("%s: Setup Completed.", Time.timeStr());
   Serial.print("\r\n\r\n");
}

void loop()
{
   if (sleepAt0 && Time.minute() % 10 == 0)     // once per 10 minutes, at every --:-0 minute
   {
      System.sleep(SLEEP_MODE_DEEP, 300);
   }
   
   if (Time.minute() % 5 == 0)                  // once per 10 minutes, at every --:-5 minute
   {
      getWeather("");
      logData("");

      Particle.connect();

      if (Particle.connected())
      {
         publishData("");
         publishThingSpeak("");
         publishWeatherUnderground("");
         publishWeatherCloud("");
      }

      if (Time.hour() == 0 && Time.day() == 28 && Time.month() == 10)
      {
         Time.beginDST();
      }
      if (Time.hour() == 0 && Time.day() == 31 && Time.month() == 3)
      {
         Time.endDST();
      }
   }
}


//===========================================================
// INITIALISATION
//===========================================================

void initialiseWeatherShield()
{
   shield.begin();
   shield.heaterOff();                          // make sure the Si7021 heater is turned off by default
   shield.setModeBarometer();                   // set MPL3115A2 mode to barometer rather than altimeter
   shield.setOversampleRate(7);                 // sets the time between the different sub-samples to the maximum
   shield.enableEventFlags();                   // enable the registers to get the data

   uv.begin();
   uv.setIntegrationTime(VEML6075::IT_200MS);   //
   uv.setHighDynamic(VEML6075::DYNAMIC_HIGH);   //

   pinMode(D5, OUTPUT);                         // soil moisture sensor power pin
   pinMode(A1, INPUT);                          // soil moisture sensor data pin
}

void initialiseRainGauge()
{
   pinMode(D2, INPUT_PULLUP);
   attachInterrupt(D2, handleRainClick, FALLING);
   lastRainClick = 0;
}

void initialiseAnemometer()
{
   pinMode(D3, INPUT_PULLUP);
   attachInterrupt(D3, handleWindClick, FALLING);
   lastWindClick = 0;
}

void initialiseParticleVariables()
{
   Particle.variable("temperature", temperatureC);
   Particle.variable("humidity", humidityRH);
   Particle.variable("pressure", pressurePa);
   Particle.variable("soilTemperature", soilTemperatureC);
   Particle.variable("soilMoisture", soilMoistureRH);
   Particle.variable("uv", uvIndex);
   Particle.variable("rain", rainMmMin);
   Particle.variable("windSpeed", windSpeedKMH);
   Particle.variable("windDirection", windDirection);
}


//===========================================================
// INTERFACEABLE MODIFIERS
//===========================================================

int gotoSleepNow(String arg)                             // sends the Photon into deep sleep for the specified duration in seconds
{  
   int duration;

   if (arg == "")
   {
      duration = 300;
   }
   else
   {
      duration = arg.toInt();
      if (duration < 0)
      {
         return -1;                                      // returns -1 in case of invalid argument
      }
      else
      {
         System.sleep(SLEEP_MODE_DEEP, duration);
         return 1;                                       // won't return anything in case of success, as the Photon will be sleeping
      }  
   }
}

int setSleepAt0(String arg)                              // overrides the flag that determines whether the Photon goes to sleep or stays awake at the end of the loop
{
   int oldFlag = sleepAt0;
   int newFlag = arg.toInt();

   if (newFlag == 0 || newFlag == 1)
   {
      sleepAt0 = newFlag;
      return oldFlag * 10 + sleepAt0;                    // returns a 2-digit confirmation integer made up of the previous flag followed by the new flag
   }
   else
   {
      return -1;                                         // or the conventional -1 in case of invalid argument
   }
}

int setSampleSeconds(String arg)                         // overrides the default sample loop duration
{
   int oldSeconds = sampleSecondsDefault;
   int newSeconds;
   if (arg == "")
   {
      newSeconds = sampleSecondsDefault;
   }
   else
   {
      newSeconds = arg.toInt();
      if (newSeconds < 1 || newSeconds > 300)
      {
      return -1;                                         // returns -1 for too long a duration
      }
      else
      {
      return oldSeconds * 100 + sampleSecondsDefault;    // or a confirmation number made up of the previous time followed by the new time
      }
   }
}

//===========================================================
// MAIN WEATHER FUNCTION
//===========================================================

uint32_t getWeather(String sampleSecondsArg)    // needs to return unsigned, 32-bit integer for weather data at a glance:
{                                               // the TE-HU-PRES return format is safe from overflowing even with Farenheit temperatures
   uint64_t sampleMillis;                       // calling Particle's System.millis() allows for a 64-bit value instead of the standard 32-bit value 
   int sampleSeconds;
   if (sampleSecondsArg == "")
   {
      sampleMillis = sampleSecondsDefault * 1000;
   }
   else 
   {
      sampleSeconds = sampleSecondsArg.toInt();
      if (sampleSeconds < 1 || sampleSeconds > 300)
      {
         return -1;
      }
      else
      {
         sampleMillis = sampleSeconds * 1000;
      }
   }
   
   double totalTemperatureC = 0;
   double totalHumidityRH = 0;
   double totalPressurePa = 0;
   double totalSoilTemperatureC = 0;
   int totalSoilMoistureRaw = 0;
   double totalUvIndex = 0;

   totalRainClicks = 0;
   totalWindClicks = 0;
   double totalWindSin = 0;
   double totalWindCos = 0;

   // set the shield and sensors up for measurement
   shield.setModeActive();
   digitalWrite(D5, HIGH);
   uv.powerOn();
   delay(100);

   bool shieldHeaterOnFlag = false;                
   double humidityDiscriminant = shield.getRH();   // a decisive humidity reading is taken immediately
                                                   // so that the Si7021 heater can be turned on in case of extreme moisture
   if (humidityDiscriminant >= 90)
   {
      shieldHeaterOnFlag = true;
      shield.heaterOn();
   }
   else
   {
      shield.heaterOff();
   }
   delay(250);
   
   unsigned int sampleCount = 0;
   uint64_t startMillis = System.millis();
   uint64_t endMillis = startMillis + sampleMillis;

   // start the sampling loop
   while (System.millis() < endMillis)
   {
      if (shieldHeaterOnFlag)
      {
         totalTemperatureC += shield.readBaroTemp();  // if that's the case, read the temperature from the alternative MPL3115A2 sensor
                                                      // to prevent the heater' from affecting the measurement
      }
      else
      {
         totalTemperatureC += shield.readTemp();      // otherwise, read the temperature from the usual Si702 sensor
                                                      // since the turned off heater has no effect anyway
      }
      totalHumidityRH += shield.getRH();
      delay(50);

      totalPressurePa += shield.readPressure() / 100;
      delay(50);
      
      // soil temperature
      thermo.requestTemperatures();
      delay(50);
      totalSoilTemperatureC += thermo.getTempCByIndex(0);

      // soil moisture
      totalSoilMoistureRaw += analogRead(A1);
      delay(50);

      // uv
      totalUvIndex += uv.index();
      delay(50);

      // wind vane
      double windRaw = analogRead(A0);
      delay(50);
      double windRadians = lookupRadiansFromRaw(windRaw);
      totalWindSin += sin(windRadians);
      totalWindCos += cos(windRadians);

      sampleCount++;
      delay(100);      
   }

   // send the shield and sensors to standby
   shield.setModeStandby();
   digitalWrite(D5, LOW);
   uv.shutdown();

   // compute the average values 
   temperatureC = totalTemperatureC / sampleCount;
   humidityRH = totalHumidityRH / sampleCount;
   pressurePa = totalPressurePa / sampleCount;
   uvIndex = totalUvIndex / sampleCount;
   soilTemperatureC = totalSoilTemperatureC / sampleCount;
   int soilMoistureRaw = totalSoilMoistureRaw / sampleCount;
   soilMoistureRH = double(map(soilMoistureRaw, smCal0, smCal100, 0, 100));   // convert the soil moisture raw reading to relative humidity percentile
                                                                              // according to the calibration values

   // get the values from the interrupt sensor clicks
   rainMmMin = getRain(totalRainClicks, sampleMillis);
   windSpeedKMH = getWindSpeed(totalWindClicks, sampleMillis);
   windDirection = getWindDirection(totalWindSin / sampleCount, totalWindCos / sampleCount);

   // compute the secondary quantities
   dewPointC = dewPoint(temperatureC, humidityRH);
   heatIndexC = toC(heatIndex(toF(temperatureC), humidityRH));
   windChillC = toC(windChill(toF(temperatureC), toMPH(windSpeedKMH)));

   // return data in the TE-HU-PRES format (TEmperature, HUmidity, PRESsure)
   // to have all the basic weather data at a glance as Particle cloud response
   return uint32_t(int(temperatureC)*pow(10, 6)) + uint32_t(int(humidityRH)*pow(10, 4)) + uint32_t(int(pressurePa));
}


//===========================================================
// RAIN GAUGE
//===========================================================

void handleRainClick()
{
   if (System.millis() - lastRainClick > 10) // ignore switch-bounce glitches less than 10ms after initial edge
   {
      totalRainClicks++;
      lastRainClick = System.millis();       // set up for the next click
   }
}

double getRain(int rainClicks, long sampleTimeMillis)
{  
   double sampleTimeMinutes = sampleTimeMillis / 60000;
   double rainMmMin = 0.2794 * rainClicks / sampleTimeMinutes;

   return rainMmMin;
}


//===========================================================================
// ANEMOMETER
//===========================================================================

void handleWindClick()
{
   if (System.millis() - lastWindClick > 10) // ignore switch-bounce glitches less than 10ms after initial edge
   {
      totalWindClicks++;
      lastWindClick = System.millis();       // set up for the next click
   }
}

double getWindSpeed(int windClicks, long sampleTimeMillis)
{
   double sampleTimeSeconds = sampleTimeMillis / 1000;
   double windSpeedKMH = 2.4 * windClicks / sampleTimeSeconds;
   
   return windSpeedKMH;
}


//===========================================================
// WIND VANE
//===========================================================

double getWindDirection(double windSin, double windCos)
{
   double result = atan(windSin/windCos) * 180.0 / M_PI;

   // atan can only tell where the angle is within 180 degrees. Need to look at cos to tell which half of circle we're in
   if (windCos < 0)
   {
      result += 180.0;
   }
   // atan will return negative angles in the NW quadrant -- push those into positive space.
   if (result < 0)
   {
      result += 360.0;
   }
   windDirection = result;
}

double lookupRadiansFromRaw(unsigned int analogRaw)
{
   if (analogRaw >= 2200 && analogRaw < 2400) return (3.14);   // South
   if (analogRaw >= 2100 && analogRaw < 2200) return (3.53);   // SSW
   if (analogRaw >= 3200 && analogRaw < 3299) return (3.93);   // SW
   if (analogRaw >= 3100 && analogRaw < 3200) return (4.32);   // WSW
   if (analogRaw >= 3890 && analogRaw < 3999) return (4.71);   // West
   if (analogRaw >= 3700 && analogRaw < 3780) return (5.11);   // WNW
   if (analogRaw >= 3780 && analogRaw < 3890) return (5.50);   // NW
   if (analogRaw >= 3400 && analogRaw < 3500) return (5.89);   // NNW
   if (analogRaw >= 3570 && analogRaw < 3700) return (0.00);   // North
   if (analogRaw >= 2600 && analogRaw < 2700) return (0.39);   // NNE
   if (analogRaw >= 2750 && analogRaw < 2850) return (0.79);   // NE
   if (analogRaw >= 1510 && analogRaw < 1580) return (1.18);   // ENE
   if (analogRaw >= 1580 && analogRaw < 1650) return (1.57);   // East
   if (analogRaw >= 1470 && analogRaw < 1510) return (1.96);   // ESE
   if (analogRaw >= 1900 && analogRaw < 2000) return (2.36);   // SE
   if (analogRaw >= 1700 && analogRaw < 1750) return (2.74);   // SSE
}


//===========================================================
// DATA OUTPUT
//===========================================================

// Serial Print to PC
int printData(String arg)
{
   Serial.println(Time.timeStr());
   Serial.printlnf("Temperature:\t%f °C", temperatureC);
   Serial.printlnf("Humidity:\t%f \%", humidityRH);
   Serial.printlnf("Pressure:\t%f Pa", pressurePa);
   Serial.printlnf("UV Index:\t%f", uvIndex);
   Serial.printlnf("Soil Temperature:\t%f °C", soilTemperatureC);
   Serial.printlnf("Soil Moisture:\t%f \%", soilMoistureRH);
   Serial.printlnf("Rain Rate:\t%f mm/min", rainMmMin);
   Serial.printlnf("Wind Speed:\t%f km/h", windSpeedKMH);
   Serial.printlnf("Wind Direction:\t%f °N", windDirection);
   Serial.print("\r\n\r\n");

   return 1;
}

// Serial Log to OpenLog
int logData(String arg)
{
   if (Serial1.available() && Serial1.read() == '<')
   {
      // log the data in CSV format as:
      // unixtime, temp, hum, press, uv, soiltemp, soilmoist, rainrate, windspeed, winddir
      Serial1.print(Time.now());
      Serial1.printf("%s,", Time.timeStr());
      Serial1.printf("%f,", temperatureC);
      Serial1.printf("%f,", humidityRH);
      Serial1.printf("%f,", pressurePa);
      Serial1.printf("%f,", uvIndex);
      Serial1.printf("%f,", soilTemperatureC);
      Serial1.printf("%f,", soilMoistureRH);
      Serial1.printf("%f,", rainMmMin);
      Serial1.printf("%f,", windSpeedKMH);
      Serial1.printlnf("%f", windDirection);

      return 1;
   }
   else
   {
      return -1;
   }
   
}

// Particle Console
int publishData(String arg)
{
   Particle.publish("temperature", String(temperatureC), PRIVATE);
   delay(100);
   Particle.publish("humidity", String(humidityRH), PRIVATE);
   delay(100);
   Particle.publish("pressure", String(pressurePa), PRIVATE);
   delay(100);
   Particle.publish("uvIndex", String(uvIndex), PRIVATE);
   delay(100);
   Particle.publish("soilTemperature", String(soilTemperatureC), PRIVATE);
   delay(100);
   Particle.publish("soilMoisture", String(soilMoistureRH), PRIVATE);
   delay(100);
   Particle.publish("rain", String(rainMmMin), PRIVATE);
   delay(100);
   Particle.publish("windSpeed", String(windSpeedKMH), PRIVATE);
   delay(100);
   Particle.publish("windDirection", String(windDirection), PRIVATE);
   delay(100);

   return 1;
}

// ThingSpeak
int publishThingSpeak(String arg)
{
   ThingSpeak.setField(1, float(temperatureC));
   ThingSpeak.setField(2, float(humidityRH));
   ThingSpeak.setField(3, float(pressurePa));
   ThingSpeak.setField(4, float(uvIndex));
   ThingSpeak.setField(5, float(rainMmMin));
   ThingSpeak.setField(6, float(windSpeedKMH));
   ThingSpeak.setField(7, float(windDirection));
   delay(100);
   ThingSpeak.writeFields(tsID, tsAPI);

   return 1;
}

// Weather Underground: API documentation at <https://support.weather.com/s/weather-underground?language=en_US>
int publishWeatherUnderground(String arg)
{
   Serial.print("Connecting to Weather Underground... ");
   if (client.connect("weatherstation.wunderground.com", 80))
   {
      Serial.println("OK");
      Serial.print("Uploading data... ");
      client.print("GET /weatherstation/updateweatherstation.php");
      client.printf("?ID=%s", wuID);
      client.printf("&PASSWORD=%s", wuPASS);
      client.print("&dateutc=now");
      client.printf("&tempf=%f", toF(temperatureC));
      client.printf("&humidity=%f", humidityRH);
      client.printf("&baromin=%f", pressurePa * 0.000295);
      client.printf("&dewptf=%f", toF(dewPointC));
      client.printf("&uv=%f", uvIndex);
      client.printf("&soiltempf=%f", toF(soilTemperatureC));
      client.printf("&soilmoisture=%f", soilMoistureRH);
      client.printf("&rainin=%f", rainMmMin * 60 * 2.54);
      client.printf("&windspeedmph=%f", windSpeedKMH * 6.213712);
      client.printf("&winddir=%f", windDirection);
      client.print("&softwaretype=Particle-Photon&action=updateraw");
      client.print(" HTTP/1.0 \r\n");
      client.print("Accept: text/html \r\n");
      client.print("Host: weatherstation.wunderground.com \r\n\r\n");
      Serial.println("OK");
      Serial.print("Server Response: ");
      unsigned long lastRead = System.millis();
      while (client.connected() && (System.millis() - lastRead < 750))   // allows for some time to listen for the server's response
      {
         if (client.available())
         {
            char response = client.read();
            Serial.print(response);
         }
      }
      Serial.print("\r\n\r\n");
      return 1;
   }
   else
   {
      Serial.println("Connection failed.");
      Serial.print("\r\n\r\n");
      return -1;
   }
}

// WeatherCloud: API documentation disclosed upon registration, contact <mailto:support@weathercloud.net>
int publishWeatherCloud(String arg)
{
   Serial.print("Connecting to WeatherCloud... ");
   if (client.connect("api.weathercloud.net", 80))
   {
      Serial.println("OK");
      Serial.print("Uploading data... ");
      client.printf("GET /v01/set/wid/%s", wcID);
      client.printf("/key/%s/ver/0.1", wcKEY);
      client.printf("/type/%s", wcAPI);
      client.printf("/temp/%d", int(temperatureC * 10));
      client.printf("/hum/%d", int(humidityRH));
      client.printf("/bar/%d", int(pressurePa * 10));
      client.printf("/dew/%d", int(dewPointC * 10));
      client.printf("/heat/%d", int(heatIndexC * 10));
      client.printf("/chill/%d", int(windChillC * 10));
      client.printf("/uvi/%d", int(uvIndex * 10));
      client.printf("/tempagro01/%d", int(soilTemperatureC * 10));
      client.printf("/soilmoist/%d", int(soilMoistureRH * 10));
      client.printf("/rainrate/%d", int(rainMmMin* 60 * 10));
      client.printf("/wspd%d/", int(windSpeedKMH / 3.6 * 10));
      client.printf("/wdir/%d", int(windDirection));
      client.print("/ HTTP/1.1 \r\n");
      client.print("Accept: text/html \r\n");
      client.print("Host: api.weathercloud.net \r\n\r\n");
      Serial.println("OK");
      Serial.print("Server response: ");
      unsigned long lastRead = System.millis();
      while (client.connected() && (System.millis() - lastRead < 750))   // allows for some time to listen for the server's response
      {
         if (client.available())
         {
            char response = client.read();   // actually scans for each character in the server response
            Serial.print(response);
         }
      }
      Serial.print("\r\n\r\n");
      return 1;
   }
   else
   {
      Serial.println("Connection failed.");
      Serial.print("\r\n\r\n");
      return -1;
   }
}


//===========================================================
// UTILITIES
//===========================================================

double toF(double celsius)
{
   double F = celsius * 9.0 / 5.0 + 32.0;
   return F;
}

double toC(double farenheit)
{
   double C = (farenheit - 32.0) * 5 / 9;
   return C;
}

double toMPH(double kmh)
{
   double MPH = kmh * 0.6213711922;
   return MPH;
}

double dewPoint(double tC, double RH)
{
   double RATIO = 373.15 / (273.15 + tC);                                           // saturation vapour pressure
   double RHS = -7.90298 * (RATIO - 1);
   RHS += 5.02808 * log10(RATIO);
   RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO ))) - 1) ;
   RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
   RHS += log10(1013.246);

   double VP = pow(10, RHS - 3) * RH;                                               // factor -3 is to adjust units

   double T = log(VP / 0.61078);                                                    // dewpoint = f * vapour pressure
   return (241.88 * T) / (17.558 - T);
}

double heatIndex(double tF, double RH)
{
   double HIS = (0.5 * (tF + 61.0 + (tF - 68.0) * 1.2) + (0.094 * RH) + tF) / 2;    // compute the simple formula first and average it with the temperature

   if (HIS >= 80)
   {
      double HI = -42.379 + 2.04901523 * tF + 10.14333127 * RH - 0.22475541 * tF * RH - 0.00683783 * pow(tF, 2) - 0.05481717 * pow(RH, 2) + 0.00122874 * pow(tF, 2) * RH + 0.00085282 * pow(RH, 2) * tF - 0.00000199 * pow(tF, 2) * pow(RH, 2);
      if (RH < 13 && 80 < tF < 112)
      {
         double HIA = HI + ((13 - RH) / 4) * sqrt((17 - abs(tF - 95.0)) / 17);      // apply to the heat index the additional adjustment
         return HIA;
      }
      else if (RH > 85 && 80 < tF < 87)
      {
         double HIA = HI - ( (RH  - 85) / 10) * ((87 - tF) / 5);                    // apply to the heat index te subtractive adjustment
         return HIA;
      }
      else
      {
         return HI;
      }
   }
   else
   {
      return HIS;
   }
}

double windChill(double tF, double wsMPH)
{
   double WC = 35.74 + 0.6215 * tF - 35.75 * pow(wsMPH, 0.16) + 0.3965 * tF * pow(wsMPH, 0.16);
   return WC;
}