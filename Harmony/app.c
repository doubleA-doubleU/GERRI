/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include <stdio.h>
#include <xc.h>
#include <math.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
 */

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

/*******************************************************
 * USB CDC Device Events - Application Event Handler
 *******************************************************/

USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler
(
        USB_DEVICE_CDC_INDEX index,
        USB_DEVICE_CDC_EVENT event,
        void * pData,
        uintptr_t userData
        ) {
    APP_DATA * appDataObject;
    appDataObject = (APP_DATA *) userData;
    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;

    switch (event) {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  */

            USB_DEVICE_ControlSend(appDataObject->deviceHandle,
                    &appDataObject->getLineCodingData, sizeof (USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host */

            USB_DEVICE_ControlReceive(appDataObject->deviceHandle,
                    &appDataObject->setLineCodingData, sizeof (USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. */

            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *) pData;
            appDataObject->controlLineStateData.dtr = controlLineStateData->dtr;
            appDataObject->controlLineStateData.carrier = controlLineStateData->carrier;

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:

            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration */

            appDataObject->breakData = ((USB_DEVICE_CDC_EVENT_DATA_SEND_BREAK *) pData)->breakDuration;

            /* Complete the control transfer by sending a ZLP  */
            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

            /* This means that the host has sent some data*/
            appDataObject->isReadComplete = true;
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

            /* The data stage of the last control transfer is
             * complete. For now we accept all the data */

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:

            /* This means the GET LINE CODING function data is valid. We dont
             * do much with this data in this demo. */
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

            /* This means that the data write got completed. We can schedule
             * the next read. */

            appDataObject->isWriteComplete = true;
            break;

        default:
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

/***********************************************
 * Application USB Device Layer Event Handler.
 ***********************************************/
void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void * eventData, uintptr_t context) {
    USB_DEVICE_EVENT_DATA_CONFIGURED *configuredEventData;

    switch (event) {
        case USB_DEVICE_EVENT_SOF:

            /* This event is used for switch debounce. This flag is reset
             * by the switch process routine. */
            appData.sofEventHasOccurred = true;
            break;

        case USB_DEVICE_EVENT_RESET:

            /* Update LED to show reset state */

            appData.isConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuration. We only support configuration 1 */
            configuredEventData = (USB_DEVICE_EVENT_DATA_CONFIGURED*) eventData;
            if (configuredEventData->configurationValue == 1) {
                /* Update LED to show configured state */

                /* Register the CDC Device application event handler here.
                 * Note how the appData object pointer is passed as the
                 * user data */

                USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0, APP_USBDeviceCDCEventHandler, (uintptr_t) & appData);

                /* Mark that the device is now configured */
                appData.isConfigured = true;

            }
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS was detected. We can attach the device */
            USB_DEVICE_Attach(appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not available any more. Detach the device. */
            USB_DEVICE_Detach(appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

            /* Switch LED to show suspended state */
            break;

        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*****************************************************
 * This function is called in every step of the
 * application state machine.
 *****************************************************/

bool APP_StateReset(void) {
    /* This function returns true if the device
     * was reset  */

    bool retVal;

    if (appData.isConfigured == false) {
        appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
        appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appData.isReadComplete = true;
        appData.isWriteComplete = true;
        retVal = true;
    } else {
        retVal = false;
    }

    return (retVal);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    
    /* Device Layer Handle  */
    appData.deviceHandle = USB_DEVICE_HANDLE_INVALID;

    /* Device configured status */
    appData.isConfigured = false;

    /* Initial get line coding state */
    appData.getLineCodingData.dwDTERate = 9600;
    appData.getLineCodingData.bParityType = 0;
    appData.getLineCodingData.bParityType = 0;
    appData.getLineCodingData.bDataBits = 8;

    /* Read Transfer Handle */
    appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Write Transfer Handle */
    appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Initialize the read complete flag */
    appData.isReadComplete = true;

    /*Initialize the write complete flag*/
    appData.isWriteComplete = true;

    /* Reset other flags */
    appData.sofEventHasOccurred = false;
    //appData.isSwitchPressed = false;

    /* Set up the read buffer */
    appData.readBuffer = &readBuffer[0];
    
    // initialize TMR2 (servo 50 Hz), TMR3 (motor 20 kHz), OC1 (motor), OC3 (servo)
    T2CONbits.TCKPS = 0b110;    // Timer2 prescaler N=64 (1:64)
	PR2 = 24999;                // period = (PR2+1) * N * 12.5 ns = 20 ms, 50 Hz
	TMR2 = 0;                   // initial TMR2 count is 0
    T2CONbits.ON = 1;           // turn on Timer2
    
    T3CONbits.TCKPS = 0;        // Timer3 prescaler N=1 (1:1)
	PR3 = 3999;                 // period = (PR3+1) * N * 12.5 ns = 50 us, 20 kHz
	TMR3 = 0;                   // initial TMR3 count is 0
    T3CONbits.ON = 1;           // turn on Timer3
    
    RPA0Rbits.RPA0R = 0b0101;   // A0 is OC1 (motor)
    TRISAbits.TRISA1 = 0;       // A1 is digital output for motor
    TRISBbits.TRISB2 = 0;       // B2 is digital output for motor
    LATAbits.LATA1 = 0;         // forward drive motion
    LATBbits.LATB2 = 1;         // forward drive motion
	OC1CONbits.OCM = 0b110;     // PWM mode without fault pin; other OC1CON bits are defaults
	OC1CONbits.OCTSEL = 1;      // OC1 uses Timer3
	OC1RS = 0;                  // duty cycle = OC1RS/(PR3+1) = 0%
	OC1R = 0;                   // initialize before turning OC1 on; afterward it is read-only
    OC1CONbits.ON = 1;			// turn on OC1
    
    RPB14Rbits.RPB14R = 0b0101; // B14 is OC3 (servo)
    OC3CONbits.OCM = 0b110;     // PWM mode without fault pin; other OC3CON bits are defaults
	OC3CONbits.OCTSEL = 0;      // OC3 uses Timer2
	OC3RS = 1000;               // duty cycle = OC3RS/(PR2+1)
	OC3R = 1000;                // initialize before turning OC3 on; afterward it is read-only
    OC3CONbits.ON = 1;			// turn on OC3
    
    INTCONbits.MVEC = 1;        // enable multiple interrupts vectors
    
    // initialize the left sensor variables on A4, IC1
    TRISAbits.TRISA4 = 1;       // connect the left TS3633 ENV pin to A4
    IC1Rbits.IC1R = 0b0010;     // A4 is IC1 (input capture 1)
    IC1CONbits.ICM = 1;         // detect rising and falling edges
    IC1CONbits.ICI = 0;         // interrupt on an edge
    IC1CONbits.ICTMR = 1;       // store the value of timer2, but we're actually just using the interrupt and ignoring timer3
    IC1CONbits.FEDGE = 0;       // first event is falling edge, doesn't really matter
    IC1CONbits.ON = 1;
    IPC1bits.IC1IP = 5;         // step 4: interrupt priority 5
    IPC1bits.IC1IS = 1;         // step 4: interrupt priority 1
    IFS0bits.IC1IF = 0;         // step 5: clear the int flag
    IEC0bits.IC1IE = 1;         // step 6: enable INT0 by setting IEC0<3>
    
    // initialize the right sensor variables on B15, IC4
    TRISBbits.TRISB7 = 1;       // connect the right TS3633 ENV pin to B7
    IC4Rbits.IC4R = 0b0100;     // B7 is IC4 (input capture 4)
    IC4CONbits.ICM = 1;         // detect rising and falling edges
    IC4CONbits.ICI = 0;         // interrupt on an edge
    IC4CONbits.ICTMR = 1;       // store the value of timer3, but we're actually just using the interrupt and ignoring timer3
    IC4CONbits.FEDGE = 0;       // first event is falling edge, doesn't really matter
    IC4CONbits.ON = 1;
    IPC4bits.IC4IP = 5;         // step 4: interrupt priority 5
    IPC4bits.IC4IS = 1;         // step 4: interrupt priority 1
    IFS0bits.IC4IF = 0;         // step 5: clear the int flag
    IEC0bits.IC4IE = 1;         // step 6: enable INT0 by setting IEC0<3>
    
    // debugging pins
    TRISBbits.TRISB13 = 0;      // output to mirror IC1
    TRISBbits.TRISB15 = 0;      // output to mirror IC4
    
    // initialize IMU here!!!
    
    startTime = _CP0_GET_COUNT();
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void) {
    /* Update the application state machine based
     * on the current state */

    switch (appData.state) {
        case APP_STATE_INIT:      
            /* Open the device layer */
            appData.deviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);

            if (appData.deviceHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(appData.deviceHandle, APP_USBDeviceEventHandler, 0);

                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
            } else {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }         

            break;

        case APP_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device was configured */
            if (appData.isConfigured) {
                /* If the device is configured then lets start reading */
                appData.state = APP_STATE_SCHEDULE_READ;
            }
            break;

        case APP_STATE_SCHEDULE_READ:  
            if (APP_StateReset()) {
                break;
            }

            /* If a read is complete, then schedule a read
             * else wait for the current read to complete */

            appData.state = APP_STATE_WAIT_FOR_READ_COMPLETE;
            if (appData.isReadComplete == true) {
                appData.isReadComplete = false;
                appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

                USB_DEVICE_CDC_Read(USB_DEVICE_CDC_INDEX_0,
                        &appData.readTransferHandle, appData.readBuffer,
                        APP_READ_BUFFER_SIZE);

                if (appData.readTransferHandle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID) {
                    appData.state = APP_STATE_ERROR;
                    break;
                }
                int ii = 0;
                // loop thru the characters in the buffer
                while (appData.readBuffer[ii] != 0) {
                    // if you got a newline
                    if (appData.readBuffer[ii] == '\n' || appData.readBuffer[ii] == '\r') {
                        rx[rxPos] = 0; // end the array
                        sscanf(rx, "%d", &rxVal); // get the int out of the array
                        gotRx = 1; // set the flag
                        break; // get out of the while loop
                    } else if (appData.readBuffer[ii] == 0) {
                        break; // there was no newline, get out of the while loop
                    } else {
                        // save the character into the array
                        rx[rxPos] = appData.readBuffer[ii];
                        rxPos++;
                        ii++;
                    }
                }
            }

            break;

        case APP_STATE_WAIT_FOR_READ_COMPLETE:
        case APP_STATE_CHECK_TIMER:

            if (APP_StateReset()) {
                break;
            }

            /* Check if a character was received or a switch was pressed.
             * The isReadComplete flag gets updated in the CDC event handler. */

            if (gotRx || _CP0_GET_COUNT() - startTime > (48000000 / 2 / 25)) { // 25 Hz
                appData.state = APP_STATE_SCHEDULE_WRITE;
            }
            break;


        case APP_STATE_SCHEDULE_WRITE:

            if (APP_StateReset()) {
                break;
            }

            /* Setup the write */

            appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
            appData.isWriteComplete = false;
            appData.state = APP_STATE_WAIT_FOR_WRITE_COMPLETE;

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

            if (go == 1) {
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
                //len = sprintf(dataOut, "L: (%.3f, %.3f) R: (%.3f %.3f)\r\n", x_ang[0], y_ang[0], x_ang[1], y_ang[1]);
                len = sprintf(dataOut, "%.3f %.3f %.3f %.3f\r\n", x, y, theta, lapTime);
                USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                        &appData.writeTransferHandle, dataOut, len,
                        USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);                
            } else {
                // send nothing
                len = 1;
                dataOut[0] = 0;
                USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                        &appData.writeTransferHandle, dataOut, len,
                        USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
            }
            startTime = _CP0_GET_COUNT();

            break;

        case APP_STATE_WAIT_FOR_WRITE_COMPLETE:

            if (APP_StateReset()) {
                break;
            }

            /* Check if a character was sent. The isWriteComplete
             * flag gets updated in the CDC event handler */

            if (appData.isWriteComplete == true) {
                appData.state = APP_STATE_SCHEDULE_READ;
            }

            break;

        case APP_STATE_ERROR:
            break;
        default:
            break;
    }
}

/*******************************************************************************
 End of File
 */