# lighthouse_host
## Version 4.0
## Email:    bizhoutao.terry@gmail.com

## base function    
    The host can achicve functions include Serial helper, draw the Coordinates.

## Serial Communication Protocols

    data[0] = 0xFA
    data[1] = device ID (0x01 or 0x02)
    data[2]-data[4] x axis
    data[5]-data[7] y axis
    data[8] = 0xAF


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
