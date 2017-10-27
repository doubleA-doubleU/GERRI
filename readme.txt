GERRI (Guidance Enabled Robotic Raceline Interpreter)

Android phone - Samsung Galaxy S5 (CAD model: https://grabcad.com/library/samsung-galaxy-s5-3)
Tamiya TL01 RC car (CAD model: https://grabcad.com/library/tamiya-car/files)
RS 540SH Motor
S3003 Servo
7.2V NiMH battery pack
USB OTG cable - micro to mini
3D printed phone and PCB mounts
Microchip PIC32MX250F128B microcontroller
LSM6DS33 Accelerometer https://www.pololu.com/product/2736 
VNH5019 Motor Driver Carrier (https://www.pololu.com/product/1451)
HTC Vive Base Station (https://www.vive.com/us/accessory/base-station/)
2x HTC Vive Position Sensor (TS3633-CM1) https://www.triadsemi.com/product/ts3633-cm1/
Custom PCB with mounting holes for:
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
	