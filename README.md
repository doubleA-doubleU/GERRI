## GERRI (Guidance Enabled Robotic Raceline Interpreter)
#### _Aaron Weatherly_
#### _Northwestern University_

<p>
<br>
<img src="./Images/car2.JPG" width="640"
      style="margin-left:auto; margin-right:auto; display:block;"/>
<br>
</p>

## Overview
This project allows an off-the-shelf radio controlled car to be converted into an autonomous
vehicle. This is accomplished by replacing the car's control system with an Android phone and 
a PIC32 microcontroller. 


## Required Components 
1. Android phone (I used a Samsung Galaxy S5)
2. Radio Control Car (I used a Tamiya TL01)
3. USB OTG cable - micro to mini
4. 3D printed [phone and PCB mounts][parts]
5. Microchip [PIC32MX250F128B][pic] microcontroller in 28-pin SPDIP package
6. LSM6DS33 [Accelerometer][accel] 
7. VNH5019 [Motor Driver Carrier][hbridge]
8. HTC Vive [Base Station][base]
9. 2x HTC Vive Position Sensor, [TS3633-CM1][vive]
10. Custom [PCB][pcb] with mounting holes for:
	* 28 Pin DIP socket
	* Mini USB connector
	* Slideswitch
	* MCP1702 voltage regulator (TO92)
	* 3mm red LED with 330 Ohm resistor
	* Reset button with 10 kOhm resistor
	* 3mm green LED with 330 Ohm resistor
	* User button with 10 kOhm resistor
	* 8 MHz crystal resonator
	* 4x 0.1 uF capacitors
	* 2x 10 uF polarized capacitors
	* 5-pin breakout for PICkit3 programmer
	* 2x 8-pin breakouts for remaining I/O pins and power/ground connections

	
## Software
In MPLAB X IDE:
	* Create new 32-bit MPLAB Harmony project, named GERRI, for the PIC32MX250F128B
	* Generate code
	* Copy all code from the provided [app.c][appc], [app.h][apph], [main.c][main], 
	[default.mhc][default], [system_config.h][config], and [system_interrupt.c][interrupt]
	into the auto-generated Harmony files of same names
	* Install the project onto the PIC
	
In Android Studio:
	* Create new project with an Empty Activity, named GERRI
	* Copy the provided [classes.jar][classes] to <your_local_project_path>/GERRI/app/libs/
	* In the IDE, navigate the left panel to Project/GERRI/app/libs/
	* Right click classes.jar, Add as Library, Add to Module: app
	* In the IDE, navigate the left panel to Android/app/res/
	* Right click res, add driectory, name it "xml"
	* Right click xml, add new XML Resource file, name it "device_filter.xml"
	* Open this new xml file and replace the contents with the code from the provided 
	[device_filter.xml][filter] file
	* Copy all code from the provided [activity_main.xml][layout], [AndroidManifest.xml][manifest], 
	and [MainActivity.java][java] into auto-generated Android Studio files of same name.

	
## Under the Hood
Detailed description of what each script is doing and why...

	
[parts]: https://github.com/weatherman03/GERRI/tree/master/Parts
[pic]: http://www.microchipdirect.com/product/PIC32MX250F128B
[accel]: https://www.pololu.com/product/2736
[hbridge]: https://www.pololu.com/product/1451
[base]: https://www.vive.com/us/accessory/base-station/
[vive]: https://www.triadsemi.com/product/ts3633-cm1/
[pcb]: https://github.com/weatherman03/GERRI/tree/master/Eagle
[appc]: https://github.com/weatherman03/GERRI/blob/master/Harmony/app.c
[apph]: https://github.com/weatherman03/GERRI/blob/master/Harmony/app.h
[main]: https://github.com/weatherman03/GERRI/blob/master/Harmony/main.c
[default]: https://github.com/weatherman03/GERRI/blob/master/Harmony/default.mhc
[config]: https://github.com/weatherman03/GERRI/blob/master/Harmony/system_config.h
[interrupt]: https://github.com/weatherman03/GERRI/blob/master/Harmony/system_interrupt.c
[classes]: https://github.com/weatherman03/GERRI/blob/master/Android/classes.jar
[filter]: https://github.com/weatherman03/GERRI/blob/master/Android/device_filter.xml
[layout]: https://github.com/weatherman03/GERRI/blob/master/Android/activity_main.xml
[manifest]: https://github.com/weatherman03/GERRI/blob/master/Android/AndroidManifest.xml
[java]: https://github.com/weatherman03/GERRI/blob/master/Android/MainActivity.java

