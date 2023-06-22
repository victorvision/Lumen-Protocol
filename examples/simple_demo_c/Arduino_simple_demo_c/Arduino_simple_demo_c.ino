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

lumen_package_t valor_2 = { 20, kS32};
const int valor_1_address = 10;
lumen_package_t *currentPackage;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("\r\n Begin");

  delay(2000);
}

void loop() {

  while (lumen_available() > 0) {
    currentPackage = lumen_get_first_package();

    if (currentPackage != NULL) {
      if (currentPackage->address == valor_1_address){
        valor_2.data._s32 = currentPackage->data._s32;
        lumen_write_package(&valor_2);
      }
    }
  }
}
