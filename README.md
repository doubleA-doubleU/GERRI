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
This project allows an off-the-shelf radio controlled car to be converted
into an autonomous vehicle. This is accomplished by replacing the car's 
control system with an Android phone and a PIC32 microcontroller. External
localization is done through HTC vive position sensors. Check out this 
[video][youtube] for a demonstration.


## Required Components 
1. Android phone (I used a Samsung Galaxy S5)
2. Radio Controlled Car (I used a Tamiya TL01)
3. USB OTG cable - micro to mini
4. 3D printed [phone and PCB mounts][parts]
5. Microchip [PIC32MX250F128B][pic] microcontroller in 28-pin SPDIP package
6. VNH5019 [Motor Driver Carrier][hbridge]
7. HTC Vive [Base Station][base]
8. 2x HTC Vive Position Sensor, [TS3633-CM1][vive]
9. Custom [PCB][pcb] with mounting holes for:
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
#### In MPLAB X IDE:
1. Create new 32-bit MPLAB Harmony project, named GERRI, for the PIC32MX250F128B
2. Generate code
3. Copy all code from the provided [app.c][appc], [app.h][apph], 
[main.c][main], [default.mhc][default], [system_config.h][config], 
and [system_interrupt.c][interrupt]	into the auto-generated Harmony 
files of same names
4. Install the project onto the PIC
	
#### In Android Studio:
1. Create new project with an Empty Activity, named GERRI
2. Copy the provided [classes.jar][classes] to <your_local_project_path>/GERRI/app/libs/
3. In the IDE, navigate the left panel to Project/GERRI/app/libs/
4. Right click classes.jar, Add as Library, Add to Module: app
5. In the IDE, navigate the left panel to Android/app/res/
6. Right click res, add driectory, name it "xml"
7. Right click xml, add new XML Resource file, name it "device_filter.xml"
8. Open this new xml file and replace the contents with the code from 
the provided [device_filter.xml][filter] file
9. Copy all code from the provided [activity_main.xml][layout], 
[AndroidManifest.xml][manifest], and [MainActivity.java][java] into 
the auto-generated Android Studio files of same name.

	
## Under the Hood
This is an extension of a much simpler line-following [robot][pathfinder].

#### The Android app:
GERRI begins by using the phone's camera to identify the track ahead. For
me, the track was designated by blue painter's tape, so the key 
lines from `MainActivity.java` are:
```java
for (int startY = 180; startY <= 300; startY = startY + 20) { // which row in the bitmap to analyze to read
    bmp.getPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1);
    // in the row, see if the pixel is blue
    for (int i = 0; i < bmp.getWidth(); i++) {
        if (blue(pixels[i]) > thresh) {
            pixels[i] = rgb(0, 255, 0); // over write the pixel with pure green
            // COM pixel calculation
            M = M + 1;
            sum = sum + i;
        }
    }
    // update the row
    bmp.setPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1);

    // calculate COM for each row
    if (M == 0) {
        COM = 0;
    } else {
        COM = (sum / M) + 1;
        COMavg += COM;
        count++;
    }
}
```
This loop occurs each time there is a new camera frame. We analyze several
rows of pixels, identify which ones are blue, then calculate the average
position for the portion of the track that can be seen. Then, the following
uses a fixed transform from the camera frame to the world frame to determine
where the track is and add that point to a bitmap representing the track:
```java
// average the COMs
if (count == 0) {
    COMavg = 0;
} else {
    COMavg = COMavg / count;
    // calculate a point on the track based on COMavg, xCar, yCar, theta
    float l = (float) (0.025*COMavg); // distance from left of frame to the COM (in cm)
    float d = (float) (Math.sqrt(Math.pow(18.2,2) - Math.pow(l,2))); // distance from the car to the COM (in cm)
    float theta2 = (float) (Math.atan2(8-l,16.35)); // angle from the car to the COM along a line 16.35 cm from car's position (in radians)
    xTrack = (int) (xCar*100 + d*Math.cos(theta + theta2) + 0.5);
    yTrack = (int) (yCar*100 - 200 + d*Math.sin(theta + theta2) +0.5);
    if (lapTime<0.1) {
        xInit = xTrack;
        yInit = yTrack;
    }
    // add that point to the track bitmap
    if (c2 != null) {
		try {
			bmp2.setPixel(xTrack, yTrack, 0xffffffff);
		} catch (Exception e) {}
    }
}
// draw circle at COM
canvas.drawCircle(COMavg, 240, 5, paint1);
// write the COM as text
canvas.drawText("COM = " + COMavg, 10, 30, paint1);
//send COM to PIC
String sendString = String.valueOf(COMavg) + '\n';
try {
    sPort.write(sendString.getBytes(), 1); // 1 is the timeout
} catch (IOException e) {}
```
This process continues until GERRI has completed its first lap and generated
a map of the entire track. It then switches modes from simple line-following 
to a proportional feedback controller that uses the track bitmap as the desired
path. The first step in this phase is to find the nearest point on the track
from the car's current position:
```java
// find nearest white pixel in bmp2 by looking at the rings of pixels around current location
int x = (int) (xCar*100 + 0.5); // convert to cm
int y = (int) (yCar*100 - 200 + 0.5); // convert to cm, shift to fit the bitmap's range
outerLoop:
for (int ring=0; ring<15;ring++) { // looking within 15cm radius
    int[] pixels = new int[2*ring+1];
    for (int row = -ring; row <= ring; row++) {
        try {
            bmp2.getPixels(pixels,0,bmp2.getWidth(),x-ring,y+row,2*ring+1,1);
        } catch (IllegalArgumentException iae) {} // sometimes the data is bad...
        for (int col = -ring; col <= ring; col++) {
            if (pixels[col+ring] == 0xffffffff) {
                // if the pixel is white, define (xDesired,yDesired) as the current row and column of the search
                xDesired = x+col;
                yDesired = y+row;
                if (xDesired_prev == 0) {
                    xDesired_prev = xDesired;
                    yDesired_prev = yDesired;
                }
                // break the both for loops once the track is found
                break outerLoop;
            }
        }
    }
}
```
Next, we calculate the position and angular error as follows:
```java
// calculate position error
posError = (float) Math.sqrt(Math.pow(xDesired - x,2) + Math.pow(yDesired - y,2));

if (posError != 0 && xDesired - xDesired_prev != 0) {
	// calculate desired angle based on current vs previous track points
    thetaDesired = (float) Math.atan2(yDesired - yDesired_prev, xDesired - xDesired_prev);
    if (Math.abs(thetaDesired-thetaDesired_prev) > 0.5) {
        thetaDesired = thetaDesired_prev;
    }
    // calculate angular error
    angError = thetaDesired - theta;
} else {
    if (thetaDesired_prev == 0){
        thetaDesired = theta;
        angError = 0;
    }
    else {
        thetaDesired = thetaDesired_prev;
        angError = thetaDesired - theta;
    }
}
```
Now we have to determine whether the position error should be positive or
negative by determining whether the track is to the left or right of the
car. There is a hard coded assumption here that the track direction is 
always counterclockwise. This was to simplify the if statements below:
```java
// determine whether track is to the left or right (assuming counterclockwise track direction...)
float theta2 = (float) Math.atan2(yDesired - y, xDesired - x); // angle from car to the track
if (thetaDesired >= Math.PI/2 && thetaDesired < Math.PI) {
    if (theta2 >= Math.PI && theta2 < 3*Math.PI/2) { // the track is to the left of the car
        posError = (-1)*posError;
    }
}
else if (thetaDesired >= Math.PI && thetaDesired < 3*Math.PI/2) {
	if (theta2 >= -Math.PI/2 && theta2 < 0) { // the track is to the left of the car
        posError = (-1)*posError;
    }
}
else if (thetaDesired >= -Math.PI/2 && thetaDesired < 0) {
    if (theta2 >=0 && theta2 < Math.PI/2) { // the track is to the left of the car
        posError = (-1)*posError;
    }
}
else if (thetaDesired >= 0 && thetaDesired < Math.PI/2) {
    if (theta2 >= Math.PI/2 && theta2 < Math.PI) { // the track is to the left of the car
        posError = (-1)*posError;
	}
}
```
Finally, we apply the proportional controller based on the [Stanley method][stanley]
from Stanford's entry into the DARPA Grand Challenge.
```java
// use proportional control to calculate a steering angle value between 0 and 640 based on angError (rad) and posError (cm)
// using the Stanley method: http://robots.stanford.edu/papers/thrun.stanley05.pdf
float cmdRad = angError + (float) Math.atan(thresh2*posError/500); // assuming constant velocity, thresh2 is the proportional gain
float cmdDeg = cmdRad*180/((float) Math.PI);
int cmd;
if (cmdDeg <= -30) {
    cmd = 640;
} else if (cmdDeg >= 30) {
	cmd = 1;
} else {
    cmd = (int) (-320*cmdDeg/30 + 320.5);
}
// send steering command to PIC
String sendString = String.valueOf(cmd) + '\n';
try {
    sPort.write(sendString.getBytes(), 1); // 1 is the timeout
} catch (IOException e) {}
```
In both the mapping and controlled cases, the output of the Android app 
is an integer between 0 and 640 that is proportional to desired steering 
angles between positive and negative 30 degrees that is sent to the PIC.


#### The PIC32 firmware:
The PIC32 communicates with the Android app via USB. It takes the integer value
provided by the app and controlls the motor speed/direction and the servo steering 
angle as shown below (from `app.c`):
```c
if (gotRx) { // rxVal should be an int between 0 and 640 (setting motor speed and steering angle from Center of Mass)
    if (rxVal > 640) {          // stop button pressed, no movement, center the steering, reset lap time
        duty = 0;
        LATAbits.LATA1 = 0;
        LATBbits.LATB2 = 1;
        OC3RS = 1000;
        go = 0;
        lapTime = 0.0;
    } else if (rxVal <= 0) {    // reverse direction and previous steering command to find the line again
        duty = 20;
        LATAbits.LATA1 = 1;     // reverse drive motion
        LATBbits.LATB2 = 0;     // reverse drive motion
        rxVal = rxValPrev;
        OC3RS = 680 + rxValPrev;
        go = 1;
    } else {                    // steer towards the COM
        duty = 20;
        LATAbits.LATA1 = 0;     // forward drive motion
        LATBbits.LATB2 = 1;     // forward drive motion
        OC3RS = 1320 - rxVal;   // ranges from 680 to 1320
        go = 1;
    }
	OC1RS = duty*40;            // convert duty cycle based on period register

    rxPos = 0;
    gotRx = 0; // clear the flag
    rxValPrev = rxVal;
    for (j = 0; j < 64; j++) {
        rx[j] = 0; // clear the array
    }
}
```
The PIC32 also provides the position (x,y,theta) as well as the current lap time to the
Android app via USB (also from `app.c`):
```c
// calculate position
x_pos[0] = (LIGHTHOUSEHEIGHT*sin((x_ang[0])*DEG_TO_RAD))/(sin((60+LIGHTHOUSEANGLE-x_ang[0])*DEG_TO_RAD)*sin((120-LIGHTHOUSEANGLE)*DEG_TO_RAD));
y_pos[0] = (2*LIGHTHOUSEHEIGHT*sin(y_ang[0]*DEG_TO_RAD))/sin((150-y_ang[0])*DEG_TO_RAD);
x_pos[1] = (LIGHTHOUSEHEIGHT*sin((x_ang[1])*DEG_TO_RAD))/(sin((60+LIGHTHOUSEANGLE-x_ang[1])*DEG_TO_RAD)*sin((120-LIGHTHOUSEANGLE)*DEG_TO_RAD));
y_pos[1] = (2*LIGHTHOUSEHEIGHT*sin(y_ang[1]*DEG_TO_RAD))/sin((150-y_ang[1])*DEG_TO_RAD);
x = (x_pos[0] + x_pos[1])/2.0;
y = (y_pos[0] + y_pos[1])/2.0;
        
// calculate angle 
if (x_pos[0] == x_pos[1] && y_pos[0] != y_pos[1]) {
	if (y_pos[0] > y_pos[1]) {
        theta = PHI;
    } else {
        theta = PHI + M_PI;
    }
} else if (y_pos[0] == y_pos[1] && x_pos[0] != x_pos[1]) {
    if (x_pos[1] > x_pos[0]){
        theta = PHI + M_PI_2;
    } else {
        theta = PHI + M_PI_2 + M_PI;
    }
} else if (x_pos[0] == x_pos[1] && y_pos[0] == y_pos[1]) {
    theta = 0.0; // should not happen, as sensors are in two different locations, unless sensors are not receiving data
} else {
    theta = atan2(y_pos[1]-y_pos[0],x_pos[1]-x_pos[0]) + PHI + M_PI_2; // always treating left sensor as origin for angle calc
}
            
// calculate lap time
lapTime = lapTime + ((float) (_CP0_GET_COUNT() - startTime)) / 24000000.0;
               
// send data to phone
len = sprintf(dataOut, "%.3f %.3f %.3f %.3f\r\n", x, y, theta, lapTime);
USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
    &appData.writeTransferHandle, dataOut, len,
    USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE); 
```
The determination of position is done by `system_interrupt.c` through two ISRs,
one for each of the vive sensors. One of the ISRs is below (and the other is very
similar):
```c
void __ISR(_INPUT_CAPTURE_1_VECTOR, IPL5SOFT) IC1ISR(void) {
    static unsigned int ptsL=0;     // previous time stamp for left sensor
    static int stateL = -1;         // state of left sensor
    unsigned int tsL;               // time stamp for left sensor
    int unusedL = IC1BUF;           // the value of timer2 right now, doesn't matter
    unusedL = IC1BUF;               // read multiple times to ensure the buffer is cleared
    unusedL = IC1BUF;               // ...
    unusedL = IC1BUF;               // ...
    LATBbits.LATB13=PORTAbits.RA4;   // match the sensor signal on a debugging pin
    if (PORTAbits.RA4) {            // ignore rising edges
        IFS0bits.IC1IF = 0;         // clear interrupt flag
        return;                     // exit the ISR
    }    
    tsL = _CP0_GET_COUNT();         // get the current time
    if (ptsL > 0) {                 // won't calculate anything the first time through
        unsigned int l = tsL - ptsL;// calculate elapsed time
        if  (l > CYCLE) {           // indicates a synchronization cycle
            stateL = 0;             // prepare to read y angle (from right to left)
        } else {
            if (stateL >= 0) {
                switch (stateL) {   
                    case 0: {
                        y_ang[0]=l*0.0006;  // read y angle in degrees (0 to 120)
                        break;
                    }
                    case 2: {
                        x_ang[0]=l*0.0006;  // read x angle in degrees (0 to 120)
                        break;
                    }
                }
                stateL++;                       // cycle through states
                if (stateL >=4 ) stateL = -1;   // reset state
            }
        }
    }
    ptsL=tsL;           // set the previous time stamp for the next time through
    IFS0bits.IC1IF = 0; // clear the interrupt flag
}
```
This works because the base station conducts two infrared sweeps, one horizontal and one
vertical. By timing the pulses seen by the sensors, we can determine the angle at which the
base station "sees" the sensor in both directions. With some basic trigonometry, this can
be converted into meters. By doing this with two sensors, the car's heading can also be
determined, as seen above. 


## Future Work
Although this method allows for some autonomous features, it would be ideal to utilize the
track map that is generated to do some more detailed path planning. This could allow for 
the calculation of a racing line that is optimized to decrease the radius of curvature of 
the path and hence the required steering effort of the car.

I also considered including an accelerometer in order to detect loss of traction and 
adjust motor speed accordingly. Due to the limitations on track size based on my workspace,
I decided to use a fixed (and relatively slow) velocity. If the track size were larger,
it would be possible to modulate velocity based on the current error and speed up or slow
down based on the curvature of the track. In that scenario, an accelerometer would be quite 
useful.


[youtube]: https://www.youtube.com/user/MdoubleAdoubleU
[parts]: https://github.com/weatherman03/GERRI/tree/master/Parts
[pic]: http://www.microchipdirect.com/product/PIC32MX250F128B
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
[pathfinder]: https://github.com/weatherman03/PathFinder
[stanley]: http://robots.stanford.edu/papers/thrun.stanley05.pdf

