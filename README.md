# flamingo_orientation
To get the orientation of the Flamingo module user in 3D space and send the data via BLE. 


Hardware: Sparkfun nRF52840 Pro Mini -> https://www.sparkfun.com/products/15025 , 
          Adafruit BNO055 -> https://www.adafruit.com/product/2472?gclid=Cj0KCQiAm5ycBhCXARIsAPldzoXCQ0XPyeziLs1tZeQcuHTntOgvcDGiPsR-0HwRyCuQQ5ucUeV5d_kaAooREALw_wcB

How it works: 
 Every second, the BLE board (which is also the main controller) polls the BNO055 sensor for orientation data over I2C, and the BNO055 sensor returns the X Y and Z orientation values. The controller board then takes this data, puts it in a packet and transmits it over BLE to a connected device. [The received data is of the little endian format]
