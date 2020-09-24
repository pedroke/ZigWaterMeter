# ZigWaterMeter

Simple Zigbee low power - battery powered sensor which monitors water consumption on water meters equiped with reflective area.
Hardware is based on IR reflective sensor connected to cc2530 module.
Software is based on Texas Instruments Z-Stack 1.2 application.
Provided are sources for IAR Embedded Workbench 8051 and compiled firmware in .hex format. TI CCDebugger should be used to upload the firmware to CC2530.

Still in beta testing phase!

TODO:
- Change identifier of the Zigbee device. For now "ZigUp" is used so that no changes in zigbee2mqtt are necessary. s0_counts attribute is used as a counter and switch indicates if water is flowing or not.
- Integrate the new device to zigbee2mqtt - requires changing the current identifiers.
- Provide possibility to calibrate the counter from Zigbee and set sensitivity of sensor.
- Find suitable Zigbee cluster
- Introduce joining mechanism. Now the device is joining the network automatically when battery is connected. Currently the firmware has to be re-flashed to clear nvram.
- Find suitable enclosures for the main unit and the sensor.

It can be used with watermeters with the counter wheel with reflective area.
<img src="https://github.com/pedroke/ZigWaterMeter/blob/master/images/watermeter.jpg" width="300px">

The following cc2530 module is recommended:
<img src="https://github.com/pedroke/ZigWaterMeter/blob/master/images/module.jpg" width="300px">

Wiring diagram of the sensor unit:
![Reflection sensor schematic](/images/schematic.jpg?raw=true "Reflection sensor schematic")

PCB for the sensor is provided in gerber:
![Reflection sensor pcb](/images/pcb.jpg?raw=true "Reflection sensor pcb")

The sensor unit is connected to the cc2530 module. VCC is taken from the I/O port and is switched on only in short intervals when the sensor is reading to save battery. The output of the sensor is evaluated on analog input and the read value is compared with (for now) hardcoded threshold.
![Connection](/images/connection.jpg?raw=true "Connection")

The device joins Zigbee network automatically when batteries are connected and network related stuff is stored in NVRAM. To join different network, re-flashing of fw is necessary (for now).
Any help with the TODO list is highly appreciated.
