#ifndef LUMEN_PROTOCOL_CONFIGURATION_H_
#define LUMEN_PROTOCOL_CONFIGURATION_H_

#define MAX_STRING_SIZE 11
#define QUANTITY_OF_PACKETS 10

#define TICK_TIME_OUT 0xFFFFFF

#define USE_CRC false
#define USE_ACK false

#if USE_ACK
#define QUANTITY_OF_DATABUFFER_FOR_RETRY 100
#define ELAPSED_TIME_TO_RETRY 500
#define QUANTITY_OF_RETRIES 3
#else
#define QUANTITY_OF_DATABUFFER_FOR_RETRY 1
#endif

// DO NOT MODIFY THESE 👇
#define START_FLAG 0x12
#define END_FLAG 0x13
#define ESCAPE_FLAG 0x7D
#define XOR_FLAG 0x20
#define WRITE_FLAG 0xA0
#define READ_FLAG 0xA1
#define ACK_FLAG 0xA2

#endif /* LUMEN_PROTOCOL_CONFIGURATION_H_ */
