#include "PropV2Seeed.h"
#include <LSM6DS3.h>
#include <Wire.h>
#include <ArduinoBLE.h>

//BLE service
BLEService HIDService ("555A0002-0000-467A-9538-01F0652C74E8");
BLECharacteristic HIDCharacteristic ("555A0002-0002-467A-9538-01F0652C74E8", BLERead | BLENotify, 3 , true);


//Create an instance of class LSM6DS3
LSM6DS3 lsm6ds3(I2C_MODE, 0x6A);    //I2C device address 0x6A


uint8_t state = IDLING;
uint8_t previousState = IDLING;
int timer0 = TOUCH_RELEASE_DURATION; // timer for touch releasing
int timer1 = SWIPING_UPDATE_INTERVAL; // time slot for angle calc
int timer2 = MINIMUM_PRESS_DOWN_DURATION; // timer for press sensing

// timer references
uint32_t timerRef0; // timestamp for touch releasing
uint32_t timerRef1; // timestamp for angle calc
uint32_t timerRef2; // timestamp for pressing

// touch release
bool firstPointRelease = false;
bool secondPointRelease = false;
bool firstZoneLeft = false;
bool firstZoneRight = false;
bool secondZoneLeft = false;
bool secondZoneRight = false;

//z Axis rotation
float zAxisRotation;

int FSR0,FSR1;

void setup() {
  Serial.begin(9600);
  lsm6ds3.begin();
  //lsm6ds3.settings.gyroEnabled = 0;
  setupBLE();
}

void loop() {
  BLE.poll();
  // Check connection status
  BLEDevice XRDevice = BLE.central();
  if(XRDevice.connected()){
    byte data[3]; // {1st byte: input state, 2nd-3rd bytes: press strength}
    updateState(data);  
    if(HIDCharacteristic.subscribed())
      {
        HIDCharacteristic.writeValue(data, 3, false);
      }
  }
  //Serial.println(lsm6ds3.readFloatGyroZ());
}

void setupBLE(void){
  const char deviceName[13] = "XRController";

  // begin initialization
  if (!BLE.begin()) 
    {
      Serial.println("Unable to start BLE!");
  
      while (1);
    }

    // set adv name
    BLE.setLocalName("XRController");

    // include service uuid in adv paket
    BLE.setAdvertisedService(HIDService);

    // add characteristic to HID service
    HIDService.addCharacteristic(HIDCharacteristic);

    // add HID service
    BLE.addService(HIDService);

    // initialize the characteristic value
    HIDCharacteristic.writeValue(state);

    // start advertising
    BLE.setAdvertisingInterval(320);
    BLE.advertise();
    
    // set connInt
    BLE.setConnectionInterval(0x0006, 0x0c80); // 7.5 ms minimum, 4 s maximum

    // set handlers
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  }

void blePeripheralConnectHandler(BLEDevice device) {
  lsm6ds3.settings.gyroEnabled = 1;
}
void blePeripheralDisconnectHandler(BLEDevice device) {
  lsm6ds3.settings.gyroEnabled = 0;
}

void updateState(byte* data){
 // read the input on analog pin 0:
 FSR0 = analogRead(A0); // first force sensor
 FSR1 = analogRead(A2); // second force sensor
  
  switch (state) {

    case IDLING: // Default state, listen for change on first and second force sensors
      
      if (FSR0 >= NORMAL_TOUCH || FSR1 >= NORMAL_TOUCH) state = TOUCHING;
      break;

    case TOUCHING: // When contact is detected on at least one force sensor
      
      if (FSR0 >= NORMAL_TOUCH && FSR1 >= NORMAL_TOUCH) break; // Not yet built
      // first point sensing
      if (FSR0 >= NORMAL_TOUCH && FSR0 < HARD_PRESS) { // Light-medium pressure on 1s force sensor
        state = FIRST_ZONE_TOUCHING; // Set state to swiping
        timerRef1 = millis(); // Start time reference for calculating z Axis rotation
      }
      else if (FSR0 >= HARD_PRESS){ 
        state = FIRST_ZONE_START_PRESS; // Hard press on first force sensor
        timerRef2 = millis();
      }
      // second point sensing
      if (FSR1 >= NORMAL_TOUCH && FSR1 < HARD_PRESS) { // Light-medium pressure on 2 force sensor
        state = SECOND_ZONE_TOUCHING; // Set state to swiping
        timerRef1 = millis();
      }
      else if (FSR1 >= HARD_PRESS) {
        state = SECOND_ZONE_START_PRESS; // Hard press on first force sensor
        timerRef2 = millis();
      }
      break;

    case FIRST_ZONE_TOUCHING:   
      if (FSR0 < NORMAL_TOUCH) { 
        state = TOUCH_RELEASE;
        firstPointRelease = true;
        timerRef0 = millis(); // start timer
        if(firstZoneLeft){
            state = FIRST_ZONE_LEFT;
            firstZoneLeft = false;
            break;
          } 
        if(firstZoneRight){
            state = FIRST_ZONE_RIGHT;
            firstZoneRight = false;
            break;
          }
      }
      else if (FSR0 >= HARD_PRESS) { // pressure > 600 => pressing
        state = FIRST_ZONE_START_PRESS;
        timerRef2 = millis(); // timer ref for pressing
      }
      else {
        if (timer1 > 0) {
          // Integration from angular velocity to rotation
          zAxisRotation += lsm6ds3.readFloatGyroZ() * (millis() - timerRef1) / 1000;
          timer1 -= millis() - timerRef1;
          timerRef1 = millis();
          if (zAxisRotation >= SWIPING_ANGLE) {
            firstZoneLeft = true;
            zAxisRotation = 0;
          }
          else if (zAxisRotation <= -SWIPING_ANGLE) {
            firstZoneRight = true;
            zAxisRotation = 0;
          }
        }
        else {
          timer1 = SWIPING_UPDATE_INTERVAL;
          zAxisRotation = 0;
          firstZoneLeft = false;
          firstZoneRight = false;
        }
      }
      break;

    case FIRST_ZONE_LEFT:
      state = IDLING;
      break;

    case FIRST_ZONE_RIGHT:
      state = IDLING;
      break;

    case FIRST_ZONE_START_PRESS:
      // If pressure down to 10 and below, just switch to touch release
      if (FSR0 <= NONE_CONTACT){ 
        state = IDLING;
        break;
      }
      // The timer count down from 500
      if (timer2 > 0) {
        timer2 -= millis() - timerRef2; //On first time, the timerRef2 was initialized on TOUCHING
        timerRef2 = millis(); //Re-initialized for next loop
        if (FSR0 < HARD_PRESS) state = FIRST_ZONE_QUICK_PRESS; //if pressing shorter than 500ms => press once
      }
      else { 
          timer2 = 0;
          state = FIRST_ZONE_PRESS_DOWN;
      }
      break;

    case FIRST_ZONE_QUICK_PRESS:
      // Do something with single tap
      timer2 = MINIMUM_PRESS_DOWN_DURATION; // reset timer 2
      state = TOUCHING;
      break;

    case FIRST_ZONE_PRESS_DOWN:
      // keep state until force below 600
      if(FSR0 < HARD_PRESS) {
        state = TOUCHING;
        timer2 = MINIMUM_PRESS_DOWN_DURATION; // reset timer 2
        }
      break;

    case SECOND_ZONE_TOUCHING:
      
      if (FSR1 <= NONE_CONTACT) {
        state = TOUCH_RELEASE;
        secondPointRelease = true;
        timerRef0 = millis();
      }
      else if (FSR1 >= HARD_PRESS) {
        state = SECOND_ZONE_START_PRESS;
        timerRef2 = millis();
      }
      else {
        if (timer1 > 0) {
           zAxisRotation += lsm6ds3.readFloatGyroZ() * (millis() - timerRef1) / 1000;
          timer1 -= millis() - timerRef1;
          timerRef1 = millis();
          if (zAxisRotation >= SWIPING_ANGLE) {
            secondZoneLeft = true;
            //state = SECOND_ZONE_LEFT;
            zAxisRotation = 0;
          }
          else if (zAxisRotation <= -SWIPING_ANGLE) {
            secondZoneRight = true;
            //state = SECOND_ZONE_RIGHT;
            zAxisRotation = 0;
          }
        }
        else {
          timer1 = SWIPING_UPDATE_INTERVAL;
          zAxisRotation = 0;
        }
      }
      break;

    case SECOND_ZONE_LEFT:
      //Do sth
      state = TOUCHING;
      break;

    case SECOND_ZONE_RIGHT:
      //Do sth
      state = TOUCHING;
      break;

    case SECOND_ZONE_START_PRESS:
//      // If pressure down to 10 and below, just switch to switch release
      if (FSR1 <= NONE_CONTACT){ 
        state = IDLING;
        break;
      }
//      // The timer count down for the duration of defined minimum press down duration
      if (timer2 > 0) {
        timer2 -= millis() - timerRef2; //On first time, the timerRef2 was initialized on TOUCHING
        timerRef2 = millis(); //Re-initialized for next loop
        if (FSR1 < HARD_PRESS) state = SECOND_ZONE_QUICK_PRESS; //if pressing shorter than 100ms => press once
      }
      else { 
        timer2 = 0;
        state = SECOND_ZONE_PRESS_DOWN;
      }
      break;

    case SECOND_ZONE_QUICK_PRESS:
      // Do something with single tap
      timer2 = MINIMUM_PRESS_DOWN_DURATION; // reset timer 2
      state = TOUCHING;
      break;

    case SECOND_ZONE_PRESS_DOWN:
      // keep state until force below 600
      if(FSR1 < HARD_PRESS) {
        state = TOUCHING;
        timer2 = MINIMUM_PRESS_DOWN_DURATION; // reset timer 2
        }
      break;
      
    case TOUCH_RELEASE:
      timer0 -= (millis() - timerRef0);
      timerRef0 = millis();
      if (timer0 <= 0){
          timer0 = TOUCH_RELEASE_DURATION;
          state = IDLING;
          // After touch release interval without any touch, reset first/secondTouchRelease bool
          firstPointRelease = false; 
          secondPointRelease = false;
      }
      else{
        if(secondZoneLeft){
            state = SECOND_ZONE_LEFT;
            secondZoneLeft = false;
            timer0 = TOUCH_RELEASE_DURATION;
            break;
          } 
        else if(secondZoneRight){
            state = SECOND_ZONE_RIGHT;
            secondZoneRight = false;
            timer0 = TOUCH_RELEASE_DURATION;
            break;
          }
         
         
        if(firstPointRelease && FSR1 > NONE_CONTACT){
          firstPointRelease = false;
          timer0 = TOUCH_RELEASE_DURATION;
          state = UP;
          break;
          }
          
        if(secondPointRelease && FSR0 >= NONE_CONTACT){
          secondPointRelease = false;
          timer0 = TOUCH_RELEASE_DURATION;
          state = DOWN;
          break;
          }       
      }
      break;

    case UP:
        state = IDLING;
        break;

    case DOWN:
        state = IDLING;
        break;
  }

  previousState = state;

  int pressure = (FSR0>FSR1)?FSR0:FSR1;
  data[0] = state;
  data[1] = pressure & 0xff;
  data[2] = (pressure >> 8) & 0xff; 
}
