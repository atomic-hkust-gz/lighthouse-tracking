# lighthouse_host
## Version 5.0
## Email:    bizhoutao.terry@gmail.com

## base function    
    The host can achicve functions include Serial helper, draw the Coordinates.
## release
    You can directly open the \build\Desktop_Qt_6_9_1_MinGW_64_bit-relase\release\lighthouse_host.exe
    and run the host computer directly, without the need for qt compilation.

## instructions
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
### 8
Display the legend in the current figure
### 9
The various graphics mentioned in 7

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
