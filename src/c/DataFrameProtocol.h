#ifndef DATAFRAMEPROTOCOL_H_
#define DATAFRAMEPROTOCOL_H_

#if defined(__cplusplus)
extern "C"
{
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "DataFrameProtocolConfiguration.h"

#define DATA_NULL 0xFFFF

  typedef union {
    bool _bool;
    char _char;
    uint8_t _u8;
    int8_t _s8;
    uint16_t _u16;
    int16_t _s16;
    uint32_t _u32;
    int32_t _s32;
    float _float;
    double _double;
    char _string[MAX_STRING_SIZE];
  } data_t;

  typedef enum data_type {
    kBool,
    kChar,
    kU8,
    kS8,
    kU16,
    kS16,
    kU32,
    kS32,
    kFloat,
    kDouble,
    kString,
  } data_type_t;

  typedef struct {
    uint16_t address;
    data_type_t type;
    data_t data;
  } package_t;

  uint32_t data_frame_write(uint16_t address, uint8_t *data, uint32_t length);
  uint32_t data_frame_write_package(package_t *package);
  uint32_t data_frame_available();
  bool data_frame_read(package_t *package);
  package_t *data_frame_get_first_package();
  
#if defined(__cplusplus)
}
#endif

#endif /* DATAFRAMEPROTOCOL_H_ */