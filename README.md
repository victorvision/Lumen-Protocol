# About the Lumen Protocol library
The Lumen Protocol library makes it very easy to establish serial communication between any microcontrolller and your Smart + Display!

It is written in C, so it is compatible with any MCU architecture from any manufacturer (PIC, ARM, ATMega, STM, etc.)!

Most examples in this documentation are written for Arduino, but you can use on absolutely any microcontroller with a UART port.

To make things even easier, you can export all the variable definitions directly from your UnicView Studio project!

# Getting Started

## C library setup

1. Include the files from [C Library Directory](src/c) into your project and reference [LumenProtocol.h](src/c/LumenProtocol.h):

``` cpp
#include "LumenProtocol.h"
```

2. Provide your platform-specific implementation of `lumen_write_bytes` and `lumen_get_byte` functions:

``` cpp
#include "LumenProtocol.h"

extern "C" void lumen_write_bytes(uint8_t *data, uint32_t length) {
  Serial.write(data, length);
}

extern "C" uint16_t lumen_get_byte() {
  if (Serial.available()) {
    return Serial.read();
  }
  return DATA_NULL;
}
```

## Communication setup

1. Copy the variable definitions from UnicView Studio:

``` cpp
// User Variable Address
const uint16_t engine_temperature_setpointAddress = 121;
const uint16_t fire_alarm_statusAddress = 122;

// User Variable Packets
lumen_packet_t engine_temperature_setpointPacket = { engine_temperature_setpointAddress, kS32 };
lumen_packet_t fire_alarm_statusPacket = { fire_alarm_statusAddress, kBool };
```

2. Implement reception and transmission logic according to your application:

``` cpp
// User Variable Address
const uint16_t engine_temperature_setpointAddress = 121;
const uint16_t fire_alarm_statusAddress = 122;

// User Variable Packets
lumen_packet_t engine_temperature_setpointPacket = { engine_temperature_setpointAddress, kS32 };
lumen_packet_t fire_alarm_statusPacket = { fire_alarm_statusAddress, kBool };

void loop() {

  // Display data reception logic:
  while (lumen_available() > 0) {
    currentPacket = lumen_get_first_packet();

    if (currentPacket != NULL) {

      // Checking if currentPacket is from fire_alarm_status
      if (currentPacket->address == fire_alarm_statusAddress) {
        bool isFireAlarmOn = currentPacket->data._bool;

        if (isFireAlarmOn == true) {
          // Alarm on! Set temperature set point to 20°C
          engine_temperature_setpointPacket->data.__s32 = 20;
        }

        // Send the new value (20) to the Display
        lumen_write_packet(&engine_temperature_setpointPacket);
      }

    }
  }

  // Other application logic goes here 👇

}
```

# Functions
--- Section under construction 🛠 ---

## Function to edit 'basic text list' and 'integer list' variables:

``` cpp
uint32_t lumen_write_variable_list(uint16_t address, uint16_t index, uint8_t *data, uint32_t length);

// Usage example:
// Change the text at index 7 of a 'basic text list' variable at address 125 to "New Text".
lumen_write_variable_list(125, 7, "New Text", 9);
```

# Usage examples
See all usage examples in the [Examples Directory](./examples).

## Updating the Display Project by UART (using ESP32 WiFi)
This repository contains a demonstration project showcasing how to transfer a compiled UnicView Studio project to a display via serial communication using the Lumen Protocol library: https://github.com/victorvision/serial-project-transfer-demo

## Basic Display Project update by UART
See this example project for a bsic implementation of Project update by UART: [project_transfer_demo_c](./examples/project_transfer_demo_c).