Tutorial: Thumb-mounted controller

This thumb-mounted controller is an input device for use in extended reality (XR) environments, it operates by sensing its user’s thumb gestures in which thumb contact and movements are processed using various output data. This data is gathered from 2 or more force sensitive resistors and a gyroscope. Recognised gestures are then sent to an XR device via Bluetooth Low Energy (BLE). In this version, the Thumb-mounted controller uses Adafruit CircuitPython libraries for creating a BLE keyboard profile

Materials

Seeed Xiao BLE Sense

This is a BLE development board which comes with a built-in accelerator and gyroscope. The Xiao BLE Sense can be found on the SEEED website.

Force-Sensitive Resistors (FSRs)

Two 30g minimum sensitivity resistors are recommended for better gesture recognition. The FSRs can be purchased quite readily but a 0-2kg sensitive FSR on AliExpress is a good example.

Resistors

Two 10,000Ω resistors are required for the controller to function properly. Wattage is not a concern for this device as the board has relatively low power consumption. The resistors can be purchased online fairly easily. However, for beginners, this guide shows the color code for the resistors.

Li-Po Battery

For this controller, a 3.3V 25mAH battery is soldered on the board. For a more advanced setup, the controller can also be powered by a USB-C port. Without it, however, the battery will last for ~2 hours before it needs to be replaced. So it is up to the user's discretion to decide whether to use the USB-C port or the 25mAH battery.

Note: The power consumption is not very significant, so some battery banks will only work for a few seconds before the controller loses power.
3D Printed Housing

This housing should be printed with TPU filament (link is an example only) for better fitting. The .stl file for this housing can be found here.

Controller Architecture

FSRs play an important role on this controller for touch recognition (i.e. swiping and pressing). To use the force sensors, we need to make a voltage divider circuit as shown in the figure below.

Force-Sensitive Resistors connected to a voltage divider

A 3V power contact is connected to one pin on each FSR while the other pin (the analog input and ground) is connected to the resistor.
