ADC with UART Sample Code
=========================

NOTE: If you use this sample application for your own purposes, follow
      the licensing agreement specified in `Software_Use_Agreement.rtf`
      in the home directory of the installed RSL10 Software
      Development Kit (SDK).

Overview
--------
This sample project demonstrates an application that:

1.  Executes at 8 MHz (divided from the 48 MHz crystal), with a UART baud rate
    of 115200.

2.  Reads VBAT/2 with the ADC. Calculates the average from 100 samples and 
    sends it over the UART port.
 
3.  Sets the battery monitor to 1.6 V and the alarm count value to 100. If the
    the battery monitor alarm is set, sends an error message over the UART
    port.
 
4.  As the error message interrupt is generated after 100 detections, it is
    possible that the average is under the threshold and the error message is
    not sent.
    
**This sample project is structured as follows:**

The source code exists in a `code` folder, all application-related include
header files are in the `include` folder, and the `main()` function `app.c` is 
located in the parent directory.

Code
----
    uart.c     - Support code for handling DMA-based UART data transfers

Include
-------
    app.h      - Overall application header file
    uart.h     - Header file for the DMA-based UART data transfer support code
    
Hardware Requirements
---------------------
This application can be executed on any RSL10 Evaluation and Development 
Board. The application needs to be connected to a terminal application or 
similar that can read and write serial UART data at 115200 baud. No external 
connections are required.

Importing a Project
-------------------
To import the sample code into your IDE workspace, refer to the 
*Getting Started Guide* for your IDE for more information.

Verification
------------
The application can be verified by connecting a terminal application (e.g.
PuTTY or Termite) to RSL10 to send and receive serial data at 115200 baud. The
correct COM port to use can be identified using the computer's Device Manager
to identify the port. Look for **JLink CDC UART Port (COMxx)**. It might be also
necessary to configure the terminal program to interpret each of **CR** and 
**LF** as a newline with carriage return.

Once connected, the values of VBAT are received on the terminal. If VBAT is
lower than 1.6 V, an error message is received. See `RSL10 evaluation_board` 
to find out how to drive VBAT externally.

Notes
-----
Sometimes the firmware in RSL10 cannot be successfully re-flashed, due to the
application going into Sleep Mode or resetting continuously (either by design 
or due to programming error). To circumvent this scenario, a software recovery
mode using DIO12 can be implemented with the following steps:

1.  Connect DIO12 to ground.
2.  Press the RESET button (this restarts the application, which
    pauses at the start of its initialization routine).
3.  Re-flash RSL10. After successful re-flashing, disconnect DIO12 from
    ground, and press the RESET button so that the application can work
    properly.

***
Copyright (c) 2019 Semiconductor Components Industries, LLC
(d/b/a ON Semiconductor).
