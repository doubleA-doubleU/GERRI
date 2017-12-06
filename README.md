## GERRI (Guidance Enabled Robotic Raceline Interpreter)
#### _Aaron Weatherly_
#### Northwestern University

<p>
<br>
<img src="./Images/car2.jpg" width="640"
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
	28 Pin DIP socket
	Mini USB connector
	Slideswitch
	MCP1702 voltage regulator (TO92)
	3mm red LED with 330 Ohm resistor
	Reset button with 10 kOhm resistor
	3mm green LED with 330 Ohm resistor
	User button with 10 kOhm resistor
	8 MHz crystal resonator
	4x 0.1 uF capacitors
	2x 10 uF polarized capacitors
	5-pin breakout for PICkit3 programmer
	2x 8-pin breakouts for remaining I/O pins and power/ground connections

## Software
Harmony files modified (from new blank project, after generating code for first time):
	copy all code from provided app.c, app.h, main.c, default.mhc, system_config.h, 
	system_interrupt.c into Harmony files of same name
Android files modified (from new Empty Activity project):
	copy classes.jar to .../<your_app_name>/app/libs/ directory
	in IDE navigate left panel to Project/<your_app_name>/app/libs/
	right click classes.jar, Add as Library, Add to Module: app
	in IDE navigate left panel to Android/app/res/, right click 
	res, add driectory, name it xml
	right click xml, add new XML Resource file, name it device_filter, open file, 
	replace with code from provided file
	copy all code from provided activity_main.xml, AndroidManifest.xml, MainActivity.java 
	into Android Studio files of same name

	
[parts]: https://github.com/weatherman03/GERRI/tree/master/Parts
[pic]: http://www.microchipdirect.com/product/PIC32MX250F128B
[accel]: https://www.pololu.com/product/2736
[hbridge]: https://www.pololu.com/product/1451
[base]: https://www.vive.com/us/accessory/base-station/
[vive]: https://www.triadsemi.com/product/ts3633-cm1/
[pcb]: https://github.com/weatherman03/GERRI/tree/master/Eagle

