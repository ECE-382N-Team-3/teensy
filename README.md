# teensy
Repository to hold code that will be used on the teensy. 

### PlatformIO
This project is powered with [platformio](https://docs.platformio.org/en/latest/what-is-platformio.html), 
which means you can download and build it 
on common IDEs such as CLion and Visual Studio.

Please update the monitor_port in the [platformio.ini](platformio.ini) 
file to monitor your teensy. 

## Contributions/ Code
The main contributions of this project can be found under the **src**
folder. The contributions are as follows.

### main
This file manipulates a gantry by sending Gcode and reads + prints data from an 
FSR Array. 

### gCodeWriter
This C++ classes allows composes and sends messages through Serial1 to 
a machine that can process gCode commands, such as [grbl](https://github.com/gnea/grbl).

### calibrateLoadCell
This file contains two functions related to calibrating an HX711 loadcell.
An example can be found here: [link](https://www.amazon.com/gp/product/B0BLNQVY2X/ref=ox_sc_act_title_1?smid=A1YZW40LYQY3L1&th=1).

