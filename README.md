# PWS: Personal Weather Station software

##### COMPLETE HARDWARE-SOFTWARE-ASSEMBLY TUTORIAL ON [INSTRUCTABLES](https://www.instructables.com/id/Particle-Photon-IoT-Personal-Weather-Station)

##### SEE THE LIVE DATA PUBLISHED BY THIS CODE ON:
* [ThingSpeak](https://thingspeak.com/channels/920701)
* [WeatherUnderground](https://www.wunderground.com/dashboard/pws/ILIGURIA189)
* [WeatherCloud](https://app.weathercloud.net/d2801062032#profile)

![The PWS deployed](img/pws1.jpg)
###### IMAGES ARE [CC BY 2.0](https://creativecommons.org/licenses/by/2.0)


## Hardware

Component                                           |Image                              | Store Link            | Documentation
---                                                 | ---                               | ---                   | ---
Particle Photon                                     | ![Photon][1a]                     | [Particle Store][1b]  | [SparkFun Development Guide](https://learn.sparkfun.com/tutorials/photon-development-guide)
\[OPTIONAL\] 2.4GHz u.FL Antenna                    | ![Antenna][2a]                    | [SparkFun][2b]        | [SparkFun Hookup Guide](https://learn.sparkfun.com/tutorials/three-quick-tips-about-using-ufl)
SparkFun OpenLog                                    | ![OpenLog][3a]                    | [SparkFun][3b]        | [SparkFun Hookup Guide](https://learn.sparkfun.com/tutorials/openlog-hookup-guide)
SparkFun Photon Weather Shield                      | ![Weather Shield][4a]             | [SparkFun][4b]        | [SparkFun Hookup Guide](https://learn.sparkfun.com/tutorials/photon-weather-shield-hookup-guide-v11)
SparkFun Weather Meters                             | ![Weather Meters][5a]             | [SparkFun][5b]        | [SparkFun Hookup Guide](https://learn.sparkfun.com/tutorials/weather-meter-hookup-guide)
Dallas DS18B20 Waterproof Temperature Sensor        | ![Soil Temperature Sensor][6a]    | [SparkFun][6b]        | [Example code by SparkFun on GitHub](https://github.com/sparkfun/simple_sketches/tree/master/DS18B20)
SparkFun Soil Moisture Sensor                       | ![Soil Moisture Sensor][7a]       | [SparkFun][7b]        | [SparkFun Hookup Guide](https://learn.sparkfun.com/tutorials/soil-moisture-sensor-hookup-guide)
SparkFun Qwiic VEML6075 UV Light Sensor Breakout    | ![UV Sensor][8a]                  | [SparkFun][8b]        | [SparkFun Hookup Guide](https://learn.sparkfun.com/tutorials/qwiic-uv-sensor-veml6075-hookup-guide)
SparkFun Sunny Buddy                                | ![Sunny Buddy][9a]                | [SparkFun][9b]        | [SparkFun Hookup Guide](https://learn.sparkfun.com/tutorials/qwiic-uv-sensor-veml6075-hookup-guide)
3.5W solar panel                                    | ![Solar Panel][10a]               | [SparkFun][10b]       | --
6Ah Li-Ion battery                                  | ![Li-Ion Battery][11a]            | [SparkFun][11b]       | --
Custom 3D Modelled Stevenson Screen                 | ![Stevenson Screen][12a]          | [Thingiverse][12b]    | --

[1a]:   <img/photon.jpg>
[1b]:   <https://store.particle.io/products/photon>
[2a]:   <img/components/antenna.jpg>
[2b]:   <https://www.sparkfun.com/products/11320>   
[3a]:   <img/components/openlog.jpg>
[3b]:   <https://www.sparkfun.com/products/>
[4a]:   <img/components/shield.jpg>
[4b]:   <https://www.sparkfun.com/products/13674>
[5a]:   <img/components/meter.jpg>
[5b]:   <https://www.sparkfun.com/products/15901>
[6a]:   <img/components/temp.jpg>
[6b]:   <https://www.sparkfun.com/products/11050>
[7a]:   <img/components/moist.jpg>
[7b]:   <https://www.sparkfun.com/products/13322>
[8a]:   <img/components/uv.jpg>
[8b]:   <https://www.sparkfun.com/products/15089>
[9a]:   <img/components/buddy.jpg>
[9b]:   <https://www.sparkfun.com/products/12885>
[10a]:  <img/components/solar.jpg>
[10b]:  <https://www.sparkfun.com/products/13782>
[11a]:  <img/components/battery.jpg>
[11b]:  <https://www.sparkfun.com/products/13856>
[12a]:  <img/components/stevenson.png>
[12b]:  <https://www.thingiverse.com/thing:4435632>


## Setup

### Installation

Just clone or fork this repository from <https://github.com/ciakkig/pws>, and that's it.

#### Dependencies

Library                         | Download
---                             | ---
SparkFun Photon Weather Shield  | [GitHub][11]
OneWire                         | [GitHub][12]
Dallas DS18B20                  | [GitHub][13]
SparkFun VEML6075               | [GitHub][14]
ThingSpeak for Particle         | [GitHub][15]

[11]:   <https://github.com/sparkfun/SparkFun_Photon_Weather_Shield_Particle_Library>
[12]:   <https://github.com/particle-iot/OneWireLibrary>
[13]:   <https://github.com/particle-iot/SparkCoreDallasTemperature>
[14]:   <https://github.com/sparkfun/SparkFun_VEML6075_Arduino_Library>
[15]:   <https://github.com/mathworks/thingspeak-particle>

However, all required libraries are already included in the `lib` directory.

Additionally, this project was made using [Particle Workbench](https://www.particle.io/workbench) in [Visual Studio Code](https://code.visualstudio.com), and while not strictly necessary, using that is warmly recommended.

Otherwise, the code has also been styled (tabbed with 3 spaces) to be consistent with the [Particle Build](https://build.particle.io) cloud IDE, if that's more your thing.

### Registration

#### ThingSpeak

Set up a new channel on ThingSpeak [here](https://thingspeak.com/channels/new), entering your coordinates and elevation, and setting the first 7 fields to the following:
1. Temperature
2. Humidity
3. Pressure
4. UV Index
5. Rain Rate
6. Wind Speed
7. Wind Direction

Then, visit the channel's **API Keys** section and record your credentials to the `tsID` and `tsAPI` variables in the `credentials.h` file.

#### WeatherUnderground

Set up a personal weather station on WeatherUnderground [here](https://www.wunderground.com/member/devices/new), selecting "other" as hardware.     
You'll have to enter the station's location again, then record your credentials from the **Manage Devices** page to the `wuID` and `wuPASS` varaiables in the `credentials.h` file.

#### WeatherCloud

Set up a personal weather station on WeatherCloud [here](https://app.weathercloud.net/device/create), selecting "other" as model.   
Once again, you'll have to enter the station's location, but you'll be required to contact [WeatherCloud Support](mailto:support@weathercloud.net) to register your device to the API and get the necessary credentials -- but fear not, they don't have any particular requirements you need to satisfy.
You can then record your credentials to the `wcID`, `wcKEY` and `wcAPI`  variables in the `credentials.h` file.

For all the credentials mentioned in this section, feel free to use `templates/calibration_template.h` as a template, but don't forget to copy, rename and mention your file in your `.gitignore` if you're planning on redistributing the code to keep your credentials for yourself!

### Calibration

Calibrating sensors is always good practice.
However, the majority of the involved sensors already come factory-calibrated, and calibration methods at the intersection of reliability and affordability are basically non-existent.
Of course, you're free to calibrate as many sensors as you want -- just throw in some more `map()` functions -- but please let me know if you manage to find a good way.

As a requirement, however, you'll only have to run calibration on the sensor that needs it the most: the soil moisture sensor.      
To do that, as shown in [SparkFun's Hookup Guide](https://learn.sparkfun.com/tutorials/soil-moisture-sensor-hookup-guide#calibration), put it about three-quarters of its way inside the soil you want to calibrate it for, in a completely dry condition, and record this first raw reading to the `smCal0` variable.
Then, wet the soil as much as you can, until it's saturated with water, and record this second raw reading to the `smCal100` variable.  
Don't worry if the two values happen to be quite close: I had a difference of about **300** -- **9%** of my maximum value, and a mere tenth of the difference between my minimum value and the actual 0 recorded in air.

Another element that requires calibration is the Sunny Buddy: while not a sensor, its MPPT (Maximum Power Point Transfer) design needs to be calibrated to that point of maximum power transfer.    
To do that, as once again shown in [SparkFun's Hookup Guide](https://learn.sparkfun.com/tutorials/sunny-buddy-solar-charger-v13-hookup-guide), connect it to your solar panel on a sunny day, measure the voltage across the `SET` and `GND` pads, and tweak the nearby potentiometer with a screwdriver until that voltage is about **3V.**


## Usage

### Compiling & Flashing

#### Local

* Local compile the project through the Workbench shortcut
* Flash the compiled project over-the-wire through the Workbench shortcut

#### Cloud

* Cloud compile the project with `particle compile photon`
* Flash the compiled project over-the-air with `particle flash pws pws.bin`

I usually prefer to compile locally for more reliability and control over the code, then flash it over the air for a seamless update from wherever I happen to be.

### Interaction

The [Particle Cloud API](https://docs.particle.io/reference/device-cloud/api) provides a number of useful tools to interface with the Photon through [Particle's CLI](https://docs.particle.io/reference/developer-tools/cli/).     
Mainly, the use of [Particle Variables](https://docs.particle.io/reference/device-os/firmware/photon/#particle-variable), [Particle Functions](https://docs.particle.io/reference/device-os/firmware/photon/#particle-function) and [Particle Events](https://docs.particle.io/reference/device-os/firmware/photon/#particle-publish).

You can `get` a variable's value by issuing:    
```particle get var```      
For example, to get the `soilMoisture` variable:    
```particle get soilMoisture```

You can also `call` a function with:    
```particle call device function argument```    
For example, to call the `setSleepAt0` function with argument `0` on your device `pws`:     
```particle call pws setSleepAt0 0```

Here's the full list of all variables and functions (with their syntax) exposed through the Particle Cloud API:

Variable            | Unit
---                 | ---
`temperature`       | °C
`humidity`          | Pa
`pressure`          | %
`soilTemperature`   | °C
`soilMoisture`      | %
`uv`                | -- (index)
`rain`              | mm/min
`windSpeed`         | km/h
`windDirection`     | °N

Function                    | Arguments                                 | Usage                                                                                                                                                                 | Returns
---                         | ---                                       | ---                                                               | ---
`gotoSleepNow`              | `seconds`: > 0 / none for default         | Sends the Photon into deep sleep immediately for the specified `seconds`. If no value is specified, the code-defined default (currently 300) is used.                     | Nothing, as the Photon will be fast asleep.
`setSleepAt0`               | `flag`: 0 ~ 1                             | Overrides the flag that determines whether the Photon enters deep sleep or stays awake at the end of the loop.                                                            | 2-digit integer: *`ON`*, made up of the *O*ld flag and *N*ew flag.
`setSampleSeconds`          | `seconds`: 1 ~ 300                        | Overrides the value that determines how long the Photon samples the sensors for. If no value is specified, the code-defined default (currently 60) is used as a reset.    | 6-digit integer: *`OLDNEW`*, made up of *OLD* time and *NEW* time. `-1` in case of invalid arguments.
`getWeather`                | `seconds`: 1 ~ 300 / none for default     | Polls all sensors for the current weather, sampling in loop for the specified `seconds`. If no value is specified, the code-defined default (currently 60) is used.       | 8-digit integer: *`TEHUPRES`*, made up of *TE*mperature, *HU*midity and *PRES*sure. `-1` in case of invalid arguments.
`printData`                 | --                                        | Prints the weather data over USB Serial to a PC.                                                                                                                          | `1` in case of success, `-1` in case of failed Serial connection.
`logData`                   | --                                        | Logs the weather data over RX-TX Serial to the OpenLog.                                                                                                                   | `1` in case of success, `-1` in case of failed Serial connection.
`publishData`               | --                                        | Publishes the weather data to the Particle Cloud as private Events.                                                                                                       | `1` in case of success, nothing or error in case of failed Cloud connection.
`publishThingSpeak`         | --                                        | Publishes the weather data to [ThingSpeak](https://thingspeak.com).                                                                                                       | `1` in case of success, nothing or error in case of failed Internet connection.
`publishWeatherUnderground` | --                                        | Publishes the weather data to [WeatherUnderground](https://weatherunderground.com).                                                                                       | `1` in case of success, nothing or error in case of failed Internet connection.
`publishWeatherCloud`       | --                                        | Publishes the weather data to [WeatherCloud](https://weathercloud.net).                                                                                                   | `1` in case of success, nothing or error in case of failed Internet connection.


## Operating Principles

This program focuses on robustness, debugability and power efficiency on remote deployment, while slightly overlooking the already relatively limited precision introduced by the general-purpose sensors on board. Rather, this is expected to be achieved through a bigger set of data, both through a long, frequent and constant gathering and by averaging a number of samples to measure such data.

By default, the weather station will:
* wake up at every **--:-5** minute (i.e. 10:05, 10:15, 10:25, etc.)
* take measurements for each quantity
* do the necessary computations on the measurements
* post the data to the Particle Cloud and to the various weather network websites
* send its sensors to sleep to lower power consumption
* stand by for the next 4-5 minutes to offer a practical window for troubleshooting
* enter deep sleep at every **--:-0** minute (i.e. 10:10, 10:20, 10:30, etc.)
* repeat

However, this default cycle can be easily tailored to any particular need for troubleshooting or power saving using the interfaces mentioned above.


## License

**Copyright (C) 2020 Giorgio Ciacchella**

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License at `COPYING` or <https://www.gnu.org/licenses/#GPL> for more details.