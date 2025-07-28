# lighthouse_host
## Version 5.0
## Email:    bizhoutao.terry@gmail.com

## base function    
    The host can achicve functions include Serial helper, draw the Coordinates.
## release
    you can loadown the zip, and click the lighthouse_host.exe
## Introduction

Serial port communication

Automatic enumeration of serial ports
Supports custom baud rate, data bits, check bits, stop bits, and flow control
Real-time data transmission and reception display (HEX + ASCII)
Support manual sending of any data
Supports one-click connection/disconnection

Multi-device trajectory visualization

Supports up to 50 devices online simultaneously
Each device has an independent color, name and trajectory line
Real-time drawing of trajectory points (highlighting the current point with a cursor)
Support dynamic addition/deletion of devices (with a minimum of 2 retained)
Support clearing all or specified device trajectories

Coordinate system and view control

Support one-click switching between the original coordinate system and the affine transformation coordinate system
Supports mouse wheel zooming and panning of views
Support the fine control of the center point by the scroll bar
Cross reference lines (X=0, Y=0) assist in positioning

Three-point calibration system

Use Device 1 for three-point calibration (origin,+10,+10, -10,+10)
Automatically calculate the affine transformation matrix
After calibration, all device coordinates are mapped to the physical coordinate system in real time
Calibration status prompt + misuse protection

Manually mark the system

Support manual input of coordinates and marking of any device
Support device number 0x00 as the "universal point" (gray square)
Support clearing marks by device number or clearing all with one click
The marker points are separated from the trajectory points and do not affect each other

Equipment status monitoring

Real-time display of each device
Online/offline status
Data reception rate (Hz
Offline duration (seconds)
The status table is automatically updated, and the color distinction is between online and offline

User experience optimization

The device label is automatically displayed at the end of the equipment trajectory
Automatic color allocation of the equipment (the first 20 standard colors + HSV cycle)
Chart anti-aliasing and smooth zooming
The status bar provides real-time prompts for the current operation/received data
All input boxes have format checks (integer range restrictions)

Extensibility

Support custom device ids (0x01 to 0xFF)
Support manual construction and sending of complete 9-byte protocol frames
Support "common points" for debugging or marking purposes
Support long-term caching of trajectory data (in memory)

Protocol support

Supports standard 9-byte data frame format:
[0xFA][ID][X3][X2][X1][Y3][Y2][Y1][0xAF]
Supports 24-bit complement code parsing (±8M range)

Debugging and Visualization

Real-time display of original HEX and ASCII data
Supports manual triggering of redrawing, screen clearing, and reconnection
Support mouse interaction (zooming, panning) in the chart area


![image](https://github.com/atomic-hkust-gz/lighthouse-tracking/blob/nrf52833_new/lighthouse_host/png/p1.png)
### 1 
Connect to the serial port and then click "Refresh". The upper computer will automatically detect it. 
This upper computer only supports two baud rates: 115200 and 9600.
### 2
If the data format is correct, the coordinates will be printed for display.
### 3
Display the received data
### 4
Display the current number of devices, the frequency of the data, and whether it is offline (offline time)

![image](https://github.com/atomic-hkust-gz/lighthouse-tracking/blob/nrf52833_new/lighthouse_host/png/p2.png)
### 5
Devices can be added or removed, with a maximum of 50 devices added
### 6
Three-point calibration, with the three points being (0,0),(10,10), and (10,-10).
Please place lighthouseMag at these three positions and calibrate them in sequence. 
Please note that calibration must be done using Device 1!!

When conducting the test, the author chose cm as the unit and placed three points within 10cm.
After the test, the error was approximately 1 to 3mm.
### 7
Manually mark, select the device, fill in the coordinates, start marking, and then the corresponding pattern will appear in the coordinate system (0x00 is a special device and will draw a gray square, while other devices will draw triangles of the corresponding color), and at the same time, the serial port will send data to the single-chip microcomputer. The format for sending is described below.

At the same time, you can also choose to clear the mark of a certain device or clear all marks.

The coordinates before and after conversion can be switched to each other
### 8
Display the legend in the current figure
### 9
The various graphics mentioned in 7
### 10
Pay attention to whether the coordinate axis is before or after the conversion

### 11
clear locus

## Serial Communication Protocols

The MCU is sent to the host computer

    data[0] = 0xFA
    data[1] = device ID (0x01 or 0x02...)
    data[2]-data[4] x axis
    data[5]-data[7] y axis
    data[8] = 0xAF

 The host computer is sent to the MCU

    data[0] = 0xFB
    data[1] = device ID (0x01 or 0x02...)
    data[2]-data[4] x axis
    data[5]-data[7] y axis
    data[8] = 0xAF



    

### update v5.0
Tons of features are added, the interface is modified

### update v4.0
1. Add the lighthouse coordinate system conversion function
    The conversion can be completed by providing three coordinates with a matrix.
2. Fixed several bugs

### update v3.1 
 update a lightmark in the map

### update v3.0 
 Update the four quadrants, thicken the XY axes, scale and move the center points
### update v2 

1. Use a circular buffer (or QByteArray) to cache data without immediate processing
2. Use QTimer to batch process once every 50ms
3. Limit the update frequency of the chart; do not refresh it every time a point is received 

### update v1 

1. The data stream if more than 2000Hz(Byte), the host will destory.
2. Need a clear button in the to clear the x,y trajectory.
