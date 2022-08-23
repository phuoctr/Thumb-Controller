//Debug
//#define HID_DEBUG_OUTPUT
//#define FORCE_SENSORS_DEBUG

//IMUs
//#define IMU_DEBUG


// 17 input states
enum TouchState
{
  IDLING, // 0
  TOUCHING, // 1
  FIRST_ZONE_TOUCHING, // 2
  FIRST_ZONE_LEFT, // 3
  FIRST_ZONE_RIGHT, // 4
  FIRST_ZONE_START_PRESS, // 5
  FIRST_ZONE_QUICK_PRESS, // 6
  FIRST_ZONE_PRESS_DOWN, // 7
  SECOND_ZONE_TOUCHING, //8
  SECOND_ZONE_LEFT, // 9
  SECOND_ZONE_RIGHT, // 10
  SECOND_ZONE_START_PRESS, // 11
  SECOND_ZONE_QUICK_PRESS, // 12
  SECOND_ZONE_PRESS_DOWN, // 13
  TOUCH_RELEASE, // 14
  UP, // 15
  DOWN, // 16
};

// Pressure 
#define NONE_CONTACT 30 // <30: none contact, just ground noise
#define NORMAL_TOUCH 31 // 31-299: normal touch
#define HARD_PRESS 450 // >= 700: hard press

// Interval for touch release and swipe
#define SWIPING_UPDATE_INTERVAL 350
#define TOUCH_RELEASE_DURATION 200
#define MINIMUM_PRESS_DOWN_DURATION 400 // miliseconds

//Based on the angle measured (with center is thumb lower joint)
#define SWIPING_ANGLE 4 // degrees
