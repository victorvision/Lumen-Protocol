#include "LumenProtocol.h"
#include <Arduino.h>

extern void lumen_write_bytes(uint8_t *data, uint32_t length);
extern uint16_t lumen_get_byte();

typedef union {
  struct
  {
    uint8_t low;
    uint8_t high;
  } byte;
  uint16_t value;
} u16_union_t;

typedef union {
  struct
  {
    uint8_t low;
    uint8_t high;
  } byte;
  int16_t value;
} s16_union_t;

typedef union {
  struct
  {
    uint8_t lowest;
    uint8_t low;
    uint8_t high;
    uint8_t highest;
  } byte;
  uint32_t value;
} u32_union_t;

typedef union {
  struct
  {
    uint8_t lowest;
    uint8_t low;
    uint8_t high;
    uint8_t highest;
  } byte;
  int32_t value;
} s32_union_t;

typedef union {
  struct
  {
    uint8_t lowest;
    uint8_t low;
    uint8_t high;
    uint8_t highest;
  } byte;
  float value;
} float_union_t;

typedef union {
  struct
  {
    uint8_t _7;
    uint8_t _6;
    uint8_t _5;
    uint8_t _4;
    uint8_t _3;
    uint8_t _2;
    uint8_t _1;
    uint8_t _0;
  } byte;
  double value;
} double_union_t;

typedef enum {
  kCommand,
  kAddressHigh,
  kAddressLow,
  kData,
  kPayloadNull
} PayloadIndex_t;

typedef enum {
  kCommandNull,
  kWrite,
  kRead,
  kReadMultiplesVariables
} Command_t;

static uint8_t quantityOfPacketsAvailable = 0;

static lumen_packet_t packets[QUANTITY_OF_PACKETS];
static bool ocupiedSlots[QUANTITY_OF_PACKETS];

#if USE_CRC
static u16_union_t _crc;
#endif

uint8_t _dataOut[255];

#define kDataLength 255
static uint32_t _dataIndex;
static uint16_t receivedData;
static uint8_t _dataIn[kDataLength];
static uint32_t _command;
static u16_union_t _address;
static PayloadIndex_t _payloadIndex;

lumen_packet_t *readingPacket;
static bool reading = false;

#if USE_CRC
void CalculateCRC(uint16_t data) {
  static uint32_t pos;
  static uint32_t i;
  static uint32_t z = 0;
  static uint32_t u = 0x0001;
  static uint32_t o = 8;
  static uint32_t p = 0xA001;

  _crc.value ^= data;
  ++data;

  for (i = o; z != i; --i) {
    if (z != (u & _crc.value)) {
      _crc.value >>= u;
      _crc.value ^= p;
    } else {
      _crc.value >>= u;
    }
  }
}
#endif

static uint8_t writeTempData;

uint32_t lumen_write(uint16_t address, uint8_t *data, uint32_t length) {
  static uint32_t outDataindex;
  outDataindex = 0;

  _dataOut[outDataindex] = START_FLAG;
  ++outDataindex;

  _dataOut[outDataindex] = WRITE_FLAG;
#if USE_CRC
  _crc.value = 0xFFFF;
  CalculateCRC(_dataOut[outDataindex]);
#endif
  ++outDataindex;

  writeTempData = address & 0xFF;
#if USE_CRC
  CalculateCRC(tempData);
#endif
  if (writeTempData == START_FLAG || writeTempData == END_FLAG || writeTempData == ESCAPE_FLAG) {
    _dataOut[outDataindex] = ESCAPE_FLAG;
    ++outDataindex;
    _dataOut[outDataindex] = writeTempData ^ XOR_FLAG;
  } else {
    _dataOut[outDataindex] = writeTempData;
  }
  ++outDataindex;

  writeTempData = address >> 8;
#if USE_CRC
  CalculateCRC(writeTempData);
#endif
  if (writeTempData == START_FLAG || writeTempData == END_FLAG || writeTempData == ESCAPE_FLAG) {
    _dataOut[outDataindex] = ESCAPE_FLAG;
    ++outDataindex;
    _dataOut[outDataindex] = writeTempData ^ XOR_FLAG;
  } else {
    _dataOut[outDataindex] = writeTempData;
  }
  ++outDataindex;

#if USE_CRC
  for (uint16_t i = 0; i < length; i++) {
    CalculateCRC(data[i]);
  }
#endif

  for (uint16_t i = 0; i < length; i++) {
    if (data[i] == START_FLAG || data[i] == END_FLAG || data[i] == ESCAPE_FLAG) {
      _dataOut[outDataindex] = ESCAPE_FLAG;
      ++outDataindex;
      _dataOut[outDataindex] = data[i] ^ XOR_FLAG;
      ++outDataindex;
    } else {
      _dataOut[outDataindex] = data[i];
      ++outDataindex;
    }
  }

#if USE_CRC
  if ((_crc.byte.low == (uint8_t)START_FLAG) || (_crc.byte.low == (uint8_t)END_FLAG)
      || (_crc.byte.low == (uint8_t)ESCAPE_FLAG)) {
    _dataOut[outDataindex] = ESCAPE_FLAG;
    ++outDataindex;
    _dataOut[outDataindex] = _crc.byte.low ^ XOR_FLAG;
    ++outDataindex;
  } else {
    _dataOut[outDataindex] = _crc.byte.low;
    ++outDataindex;
  }

  if ((_crc.byte.high == (uint8_t)START_FLAG) || (_crc.byte.high == (uint8_t)END_FLAG)
      || (_crc.byte.high == (uint8_t)ESCAPE_FLAG)) {
    _dataOut[outDataindex] = ESCAPE_FLAG;
    ++outDataindex;
    _dataOut[outDataindex] = _crc.byte.high ^ XOR_FLAG;
    ++outDataindex;
  } else {
    _dataOut[outDataindex] = _crc.byte.high;
    ++outDataindex;
  }

#endif

  _dataOut[outDataindex] = END_FLAG;
  ++outDataindex;

  lumen_write_bytes(_dataOut, outDataindex);

  return outDataindex;
}

uint32_t lumen_write_packet(lumen_packet_t *packet) {
  switch (packet->type) {
    case kBool:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 1);
      }
      break;
    case kString:
      {
        uint8_t length = 0;
        for (; length < MAX_STRING_SIZE; ++length) {
          if (packet->data._string[length] == '\0') {
            break;
          }
        }

        lumen_write(packet->address, (uint8_t *)packet->data._string, length);
      }
      break;
    case kChar:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 1);
      }
      break;
    case kU8:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 1);
      }
      break;
    case kS8:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 1);
      }
      break;
    case kU16:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 2);
      }
      break;
    case kS16:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 2);
      }
      break;
    case kU32:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 4);
      }
      break;
    case kS32:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 4);
      }
      break;
    case kFloat:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 4);
      }
      break;
    case kDouble:
      {
        lumen_write(packet->address, (uint8_t *)packet->data._string, 8);
      }
      break;
    default:
      {
      }
      break;
  }
}

void ParsePayload() {
  switch (_payloadIndex) {
    case kCommand:
      {
        _dataIn[_dataIndex] = receivedData;
        ++_dataIndex;
        _command = receivedData;
        _payloadIndex = kAddressLow;
      }
      break;
    case kAddressLow:
      {
        _dataIn[_dataIndex] = receivedData;
        ++_dataIndex;
        _address.byte.low = receivedData;
        _payloadIndex = kAddressHigh;
      }
      break;
    case kAddressHigh:
      {
        _dataIn[_dataIndex] = receivedData;
        ++_dataIndex;
        _address.byte.high = receivedData;
        _payloadIndex = kData;
      }
      break;
    case kData:
      {
        if (_dataIndex < kDataLength) {
          _dataIn[_dataIndex] = receivedData;
          ++_dataIndex;
        }
      }
      break;
    default:
      break;
  }
}

void Empack() {
  Command_t command;

  if (_command == READ_FLAG) {

    if (reading == true) {
      if (_address.value == readingPacket->address) {

        uint8_t dataSize = _dataIndex - kData;

        for (uint8_t i = 0; i < dataSize; ++i) {
          readingPacket->data._string[i] = _dataIn[i + kData];
        }
        reading = false;
        return;
      }
    }

    for (uint8_t packetIndex = 0; packetIndex < QUANTITY_OF_PACKETS; ++packetIndex) {
      if (ocupiedSlots[packetIndex] == true) {
        if (packets[packetIndex].address == _address.value) {
          uint8_t dataSize = _dataIndex - kData;

          for (uint8_t i = 0; i < dataSize; ++i) {
            packets[packetIndex].data._string[i] = _dataIn[i + kData];
          }
          break;
        }
      }
    }

    for (uint8_t packetIndex = 0; packetIndex < QUANTITY_OF_PACKETS; ++packetIndex) {
      if (ocupiedSlots[packetIndex] == false) {

        packets[packetIndex].address = _address.value;

        uint8_t dataSize = _dataIndex - kData;

        for (uint8_t i = 0; i < dataSize; ++i) {
          packets[packetIndex].data._string[i] = _dataIn[i + kData];
        }
        ocupiedSlots[packetIndex] = true;

        ++quantityOfPacketsAvailable;
        break;
      }
    }
  }
}

uint32_t lumen_available() {
  static bool _started;
  static bool _scaped;
#if USE_CRC
  static bool _crcStarted;
  static uint16_t _crcIndex;
  static uint16_t _crcIndexDelayed;
  static uint16_t _crcData[3];
#endif

  receivedData = lumen_get_byte();

  while (receivedData != 0xFFFF) {
    if (receivedData == START_FLAG) {

#if USE_CRC
      _crc.value = 0xFFFF;
      _crcIndex = 0;
      _crcStarted = false;
      _crcIndexDelayed = 0;
#endif
      _started = true;
      _scaped = false;
      _dataIndex = 0;
      _payloadIndex = kCommand;

    } else if (receivedData == END_FLAG) {

#if USE_CRC
      _crc.value = 0xFFFF;
      uint16_t _dataIndexOffseted = _dataIndex - 2;

      for (uint16_t pos = 0; pos < _dataIndexOffseted; ++pos) {
        CalculateCRC(_dataIn[pos]);
      }

      if ((_dataIn[_dataIndex - 1] == (_crc.byte.high)) && (_dataIn[_dataIndex - 2] == (_crc.byte.low))) {

        Empack();
      }
      _crcStarted = false;

#else
      Empack();

#endif
      _started = false;
      _payloadIndex = kPayloadNull;
    } else if (_started) {
      if (_scaped) {
        receivedData ^= XOR_FLAG;
        _scaped = false;
        ParsePayload();
      } else if (receivedData == ESCAPE_FLAG) {

        _scaped = true;
      } else {
        ParsePayload();
      }
    }
    receivedData = lumen_get_byte();
  }

  return quantityOfPacketsAvailable;
}

lumen_packet_t *lumen_get_first_packet() {
  for (uint8_t i = 0; i < QUANTITY_OF_PACKETS; ++i) {
    if (ocupiedSlots[i]) {
      ocupiedSlots[i] = false;
      --quantityOfPacketsAvailable;
      return &packets[i];
    }
  }
  return NULL;
}

bool lumen_read(lumen_packet_t *packet) {
  static uint32_t outDataindex;

  readingPacket = packet;
  outDataindex = 0;

  _dataOut[outDataindex] = START_FLAG;
  ++outDataindex;

  _dataOut[outDataindex] = READ_FLAG;
#if USE_CRC
  _crc.value = 0xFFFF;
  CalculateCRC(_dataOut[outDataindex]);
#endif
  ++outDataindex;

  writeTempData = readingPacket->address & 0xFF;
#if USE_CRC
  CalculateCRC(writeTempData);
#endif
  if (writeTempData == START_FLAG || writeTempData == END_FLAG || writeTempData == ESCAPE_FLAG) {
    _dataOut[outDataindex] = ESCAPE_FLAG;
    ++outDataindex;
    _dataOut[outDataindex] = writeTempData ^ XOR_FLAG;
  } else {
    _dataOut[outDataindex] = writeTempData;
  }
  ++outDataindex;

  writeTempData = readingPacket->address >> 8;
#if USE_CRC
  CalculateCRC(writeTempData);
#endif
  if (writeTempData == START_FLAG || writeTempData == END_FLAG || writeTempData == ESCAPE_FLAG) {
    _dataOut[outDataindex] = ESCAPE_FLAG;
    ++outDataindex;
    _dataOut[outDataindex] = writeTempData ^ XOR_FLAG;
  } else {
    _dataOut[outDataindex] = writeTempData;
  }
  ++outDataindex;

  _dataOut[outDataindex] = 1;
#if USE_CRC
  CalculateCRC(_dataOut[outDataindex]);
#endif
  ++outDataindex;

#if USE_CRC
  if ((_crc.byte.low == (uint8_t)START_FLAG) || (_crc.byte.low == (uint8_t)END_FLAG)
      || (_crc.byte.low == (uint8_t)ESCAPE_FLAG)) {
    _dataOut[outDataindex] = ESCAPE_FLAG;
    ++outDataindex;
    _dataOut[outDataindex] = _crc.byte.low ^ XOR_FLAG;
    ++outDataindex;
  } else {
    _dataOut[outDataindex] = _crc.byte.low;
    ++outDataindex;
  }

  if ((_crc.byte.high == (uint8_t)START_FLAG) || (_crc.byte.high == (uint8_t)END_FLAG)
      || (_crc.byte.high == (uint8_t)ESCAPE_FLAG)) {
    _dataOut[outDataindex] = ESCAPE_FLAG;
    ++outDataindex;
    _dataOut[outDataindex] = _crc.byte.high ^ XOR_FLAG;
    ++outDataindex;
  } else {
    _dataOut[outDataindex] = _crc.byte.high;
    ++outDataindex;
  }

#endif

  _dataOut[outDataindex] = END_FLAG;
  ++outDataindex;

  reading = true;
  lumen_write_bytes(_dataOut, outDataindex);

  uint32_t elapsedTickTimeOut = 0;

  while (reading == true) {
    lumen_available();

    ++elapsedTickTimeOut;

    if (elapsedTickTimeOut >= TICK_TIME_OUT) {
      return false;
    }
  }

  return true;
}
