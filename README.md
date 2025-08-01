# Getting Started

## Using the C library

1. Include the files from [C Library Directory](src/c) into your project and reference [LumenProtocol.h](src/c/LumenProtocol.h):

``` Arduino
// Arduino Sketch example
#include "LumenProtocol.h"
```

2. Provide your platform-specific implementation of `lumen_write_bytes` and `lumen_get_byte` functions:

```cpp
// Arduino Sketch example
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

3. Implement receiving logic:

```cpp
// ...

void loop() {
 while (lumen_available() > 0) {
    currentPacket = lumen_get_first_packet();

    if (currentPacket == NULL) {
        break;
    }

    // Do something with the new packet
}

// ...
```

4. Function to edit basic text list and integer list:

```cpp
// ...

uint32_t lumen_write_variable_list(uint16_t address, uint16_t index, uint8_t *data, uint32_t length);

// Example
// Change the text at index 7 of a basic text list configured at address 125 to "New Text"

lumen_write_variable_list(125, 7, "New Text", 9);

// ...
```



# Samples

See usage examples in the [Examples Directory](./examples).