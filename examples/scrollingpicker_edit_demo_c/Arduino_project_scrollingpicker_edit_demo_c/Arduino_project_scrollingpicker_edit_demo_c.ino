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

void AddToList(uint32_t quantity);
void RemoveFromList();

// User Variable Address
const uint16_t add_1Address = 123;
const uint16_t add_5Address = 124;
const uint16_t listAddress = 125;
const uint16_t input_indexAddress = 126;
const uint16_t list_indexAddress = 127;
const uint16_t removeAddress = 128;
const uint16_t input_item_nameAddress = 129;

// User Variable Packets
lumen_packet_t add_1Packet = { add_1Address, kBool };
lumen_packet_t add_5Packet = { add_5Address, kBool };
lumen_packet_t input_indexPacket = { input_indexAddress, kS32 };
lumen_packet_t list_indexPacket = { list_indexAddress, kS32 };
lumen_packet_t removePacket = { removeAddress, kBool };
lumen_packet_t input_item_namePacket = { input_item_nameAddress, kString };

lumen_packet_t *currentPacket;

uint32_t selectedInputIndex = 0;
uint32_t selectedListIndex = 0;

uint32_t NextInputToListIndex = 0;

const uint32_t kInputSize = 38;
uint8_t inputToListMap[kInputSize];
uint8_t listQuantityMap[kInputSize];
uint8_t listToBasicTextListMap[kInputSize];

const uint32_t kListUnusedValue = 255;

String NewListText = "";

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("\r\n Begin");

  for (int i = 0; i < kInputSize; ++i) inputToListMap[i] = kListUnusedValue;
  for (int i = 0; i < kInputSize; ++i) listQuantityMap[i] = 0;
  for (int i = 0; i < kInputSize; ++i) listToBasicTextListMap[i] = kListUnusedValue;

  delay(2000);
}

void loop() {

  while (lumen_available() > 0) {
    currentPacket = lumen_get_first_packet();

    if (currentPacket != NULL) {
      switch (currentPacket->address) {
        case add_1Address:
          AddToList(1);
          break;
        case add_5Address:
          AddToList(5);
          break;
        case input_indexAddress:
          selectedInputIndex = currentPacket->data._s32;
          break;
        case list_indexAddress:
          selectedListIndex = currentPacket->data._s32;
          break;
        case removeAddress:
          RemoveFromList();
          break;
        default:
          break;
      }
    }
  }
}

void AddToList(uint32_t quantity) {
  if (inputToListMap[selectedInputIndex] == kListUnusedValue) {
    inputToListMap[selectedInputIndex] = NextInputToListIndex;
    ++NextInputToListIndex;
  }
  listQuantityMap[selectedInputIndex] += quantity;

  if (lumen_read(&input_item_namePacket)) {
    NewListText = String(listQuantityMap[selectedInputIndex]);
    NewListText += "x ";
    NewListText += input_item_namePacket.data._string;

    lumen_write_variable_list(listAddress, inputToListMap[selectedInputIndex], NewListText.c_str(), NewListText.length() + 1);

    selectedListIndex = inputToListMap[selectedInputIndex];
    list_indexPacket.data._s32 = inputToListMap[selectedInputIndex];
    lumen_write_packet(&list_indexPacket);

    listToBasicTextListMap[selectedListIndex] = selectedListIndex;
  }
}

void RemoveFromList() {
  lumen_write_variable_list(listAddress, listToBasicTextListMap[selectedListIndex], "\0", 1);
  for (uint8_t i = selectedListIndex; i < kInputSize; ++i) {
    listToBasicTextListMap[i] = listToBasicTextListMap[i + 1];
    if (listToBasicTextListMap[i] == kListUnusedValue) {
      break;
    }
  }
}