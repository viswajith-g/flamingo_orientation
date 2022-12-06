# flamingo_orientation
To get the orientation of the Flamingo module user in 3D space and send the data via BLE. 


Hardware: 
  1. [Sparkfun nRF52840 Pro Mini](https://www.sparkfun.com/products/15025)
  2. [Adafruit BNO055](https://www.adafruit.com/product/2472)

Setup: 
  1. Follow the sparkfun nrf52840 hookup [guide](https://learn.sparkfun.com/tutorials/sparkfun-pro-nrf52840-mini-hookup-guide?_ga=2.261429448.497871716.1668802234-2067808309.1668802234)
  2. In the /nrf52/ver_num/variants folder, change the pin assignment for the I2C pins, because the original pins need a QWIC connector. If you have the connector, you don't need to do this step. I chose Pin 4 for SDA and Pin 5 for SCL. (this directory is present in your arduino home folder, when you are copying the variants folder for the above step).
  3. Add the Adafruit 9DOF and Adafruit BNO055 libraries via the arduino library manager. The first one should also install Unified Sensors library with it.

How it works: 
  1. Every second, the BLE board (which is also the main controller) polls the BNO055 sensor for orientation data over I2C, and the BNO055 sensor returns the X Y and Z orientation values. 
   2. The controller board then takes this data, resolves it into one of the 8 cardinal directions and transmits it over BLE to a connected device. 
