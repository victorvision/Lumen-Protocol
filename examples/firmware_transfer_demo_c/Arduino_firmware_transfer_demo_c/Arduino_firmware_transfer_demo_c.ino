#include "LumenProtocol.h"
#include "lumen_fw.h"

extern "C" void lumen_write_bytes(uint8_t* data, uint32_t length) {
  Serial2.write(data, length);
  Serial.write(data, length);
}

extern "C" uint16_t lumen_get_byte() {
  if (Serial2.available()) {
    return Serial2.read();
  }
  return DATA_NULL;
}

const uint16_t rebootAddress = 102;
lumen_packet_t rebootPacket = { rebootAddress, kBool };

const uint32_t block_length = 1660;

uint8_t BlkToSend[block_length];

void PrepareBlock(const uint8_t* blk, uint32_t length) {

  for (int i = 0; i < length; ++i) {
    BlkToSend[i] = pgm_read_byte_near(blk + i);
  }
}

uint32_t sended_bytes = 0;
uint32_t sending_bytes = block_length;

enum SendProjectState {
  kSending,
  kFinishing,
  kFinished
};

SendProjectState sendProjectState = kFinished;
SendProjectState lastSendProjectState = kFinished;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  delay(2000);
  rebootPacket.data._bool = true;
  lumen_write_packet(&rebootPacket);

  delay(1000);
  PrepareBlock(&lumen_fw_data[sended_bytes], block_length);
  sendProjectState = kSending;
}

const uint32_t tickIntervalInMs = 10;
uint32_t MillisAux = 0;

uint32_t tempoTeste = 0;
uint32_t tempoTesteIntervalo = 200;

void loop() {

  if (millis() > tempoTeste) {
    tempoTeste = millis() + tempoTesteIntervalo;

    Serial.print("L");
  }

  switch (sendProjectState) {
    case kSending:
      {

        if (millis() > tempoTeste) {
          tempoTeste = millis() + tempoTesteIntervalo;

          Serial.print("S");
        }

        if (sended_bytes < lumen_fw_array_size) {
          if (lumen_firmware_update_send_data(BlkToSend, sending_bytes)) {
            Serial.print("1");
            sended_bytes += sending_bytes;
            sending_bytes = sended_bytes - lumen_fw_array_size;

            if (sending_bytes > block_length) {
              sending_bytes = block_length;
            }

            PrepareBlock(&lumen_fw_data[sended_bytes], sending_bytes);
          }
        } else {
          sendProjectState = kFinishing;
        }
      }
      break;
    case kFinishing:
      {
        if (millis() > tempoTeste) {
          tempoTeste = millis() + tempoTesteIntervalo;

          Serial.print("f");
        }

        lumen_project_update_finish();
        sendProjectState = kFinished;
      }
      break;
    case kFinished:
      {
        if (millis() > tempoTeste) {
          tempoTeste = millis() + tempoTesteIntervalo;

          Serial.print("F");
        }
      }
      break;
    default: break;
  }

  if (millis() > MillisAux) {
    lumen_project_update_tick(tickIntervalInMs);
    MillisAux = millis() + tickIntervalInMs;
  }
}
