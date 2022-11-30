/*************************************************************************************************************************
* Flamingo User Orientation
* Desc: Get the orientation of the user in 3D space using the BNO055 module and transmit it via BLE
* Date: 11/30/2022
* Author: Viswajith Govinda Rajan
* Ver 2.0
**************************************************************************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

// // BLE Service
// BLEDfu  bledfu;  // OTA DFU service
   BLEDis  bledis;  // device information
// BLEUart bleuart; // uart over ble
// BLEBas  blebas;  // battery

// max concurrent connections supported by this example
#define MAX_PRPH_CONNECTION   1
// uint8_t connection_count = 0;

/* Flamingo Orientation Service
   BNO055 Service: 00000000-A154-EFDE-0000-0000A1540000
   BNO055 Orient : 00000001-A154-0000-0000-0000A1540001
 */

const uint8_t BNO055_UUID_SERVICE[] =
{
  0x00, 0x00, 0x54, 0xA1, 0x5F, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x54, 0xA1, 0x00, 0x00, 0x00, 0x00
};

const uint8_t BNO055_UUID_CHR_ORIENTATION[] =
{
  0x01, 0x00, 0x54, 0xA1, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x54, 0xA1, 0x01, 0x00, 0x00, 0x00
};


BLEService        bnoSensor(BNO055_UUID_SERVICE);                       // BNO sensor service
BLECharacteristic bnoReading(BNO055_UUID_CHR_ORIENTATION);        // Characteristic to send sensor data

//Struct to hold the sensor readings to be sent via BLE
struct message {
  uint16_t x = 0;   // x value
  uint16_t y = 0;   // y value  
  uint16_t z = 0;   // z value  
}packet;
char direction[3];

Adafruit_BNO055 bno = Adafruit_BNO055(55);    //initialize I2C communication with BNO055 sensor

bool conn_flag = false;       // flag to check if a connection is in effect. 

void setup(void) 
{
  Serial.begin(115200);
  // Serial.println("Orientation Sensor Test"); Serial.println("");

  #if CFG_DEBUG
    // Blocking wait for connection when debug mode is enabled via IDE
    while ( !Serial ) yield();
  #endif
  Serial.println("BNO005 Sensor Readings");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behavior, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin(MAX_PRPH_CONNECTION, 0);
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  bno_init();   //init BNO sensor

  bledis.setManufacturer("Sparkfun");
  bledis.setModel("Flamingo D2");
  bledis.begin();

  // Note: You must call .begin() on the BLEService before calling .begin() on
  // any characteristic(s) within that service definition.. Calling .begin() on
  // a BLECharacteristic will cause it to be added to the last BLEService that
  // was 'begin()'ed!
  bnoSensor.begin();

  // Configure Button characteristic
  // Properties = Read + Notify
  // Permission = Open to read, cannot write
  // Max Len  = 20 bytes
  bnoReading.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  bnoReading.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  bnoReading.setMaxLen(20);
  bnoReading.begin();
  bnoReading.write((uint16_t *)&packet, sizeof(packet));

  // Set up and start advertising
  startAdv();

  // Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  // Serial.println("Once connected, enter character(s) that you wish to send");
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bnoSensor);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
    
}

/* Initialise the sensor */
void bno_init(){
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  delay(1000);
    
  bno.setExtCrystalUse(true);
}

void sensor_reading(){
  // sensors_event_t event;  // create a sensor event instance
  // bno.getEvent(&event);   // get sensor readings for that instance via I2C from the BNO module

  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);


 uint16_t x_angle = euler.x();
 if (x_angle >= 0 and x_angle < 45){
   strcpy(direction, "N");
 }
 else if (x_angle >= 45 and x_angle < 90) {
   strcpy(direction, "NE");
 }
 else if (x_angle >= 90 and x_angle < 135) {
   strcpy(direction, "E");
 }
 else if (x_angle >= 135 and x_angle < 180) {
    strcpy(direction, "SE");
 }
 else if (x_angle >= 180 and x_angle < 225) {
    strcpy(direction, "S");
 }
 else if (x_angle >= 225 and x_angle < 270) {
    strcpy(direction, "SW");
 }
 else if (x_angle >= 270 and x_angle < 315) {
   strcpy(direction, "W");
 }
 else if (x_angle >= 315 and x_angle < 360) {
   strcpy(direction, "NW");
 }
 Serial.print("X: ");
 Serial.println(packet.x); 
 Serial.print("Direction: ");
 Serial.println(direction);
 packet.y = (euler.y());
 Serial.print("Y: ");
 Serial.println(packet.y);
 packet.z = (euler.z());
 Serial.print("Z: ");
 Serial.println(packet.z);

//  packet.x = (event.orientation.x);
//  Serial.print("X: ");
//  Serial.println(packet.x); 
//  packet.y = (event.orientation.y);
//  Serial.print("Y: ");
//  Serial.println(packet.y);
//  packet.z = (event.orientation.z);
//  Serial.print("Z: ");
//  Serial.println(packet.z);

}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
  conn_flag = true;
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  conn_flag = false;
}

void loop(void) 
{
  while (conn_flag)
  {
    // Delay to wait for enough input, since we have a limited transmission buffer
    sensor_reading();
    // bnoReading.write((uint16_t *)&packet, sizeof(packet));
    bnoReading.write(direction);
    Serial.println("packet transmitted");    
    delay(1000);
  }
  /* Get a new sensor event */ 
}