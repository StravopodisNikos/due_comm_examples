#include <Arduino.h>
#include <SPI.h>
#include <sam.h>

#define SERIAL_BAUD1 250000
#define CSPIN_EXT   10
#define SPI0_MISO   25
#define SPI0_MOSI   26
#define SPI0_SPCK   27
#define SPI0_NPCS0  10

#define CMD_LED_ON  7978 // 1F2A (HEX)
#define CMD_LED_OFF 4598 // 11F6 (HEX)

#define LEDpin      4

typedef union {
  struct {
    uint8_t cmd;
    float data;
  } fields;
  uint8_t raw[sizeof(float) + sizeof(uint8_t)];
} cmd_msg;

typedef union {
  struct {
    uint8_t cmd;
    uint8_t data[6];
  } fields;
  uint8_t raw[sizeof(uint8_t) * 7];
} data_msg;

volatile bool __overs_flag = false;
volatile bool __BytesKeepComing = false;
volatile bool __data_received_flag = false;
volatile uint8_t __ByteReceived = 0;
volatile uint8_t bytes_received_buffer[sizeof(cmd_msg)];
volatile uint8_t pos = 0;

void* operator new(size_t size, void* ptr) { return ptr; }

void setup() {
  Serial.begin(9600);

  pinMode(LEDpin, OUTPUT);

  SPI.begin(SPI0_NPCS0);
  SPI.attachInterrupt();
}

void loop() {
  if (__data_received_flag) {
    if (bytes_received_buffer[0] == 0x01) {
      cmd_msg* cmd = reinterpret_cast<cmd_msg*>(bytes_received_buffer);
      if (cmd->fields.cmd == 0x02) {
        digitalWrite(LEDpin, HIGH);
        Serial.print("CMD DATA = ");
        Serial.println(cmd->fields.data, 4);
      } else {
        Serial.println("INVALID CMD");
      }
    } else if (bytes_received_buffer[0] == 0x03) {
      data_msg* data = reinterpret_cast<data_msg*>(bytes_received_buffer);
      if (data->fields.cmd == 0x04) {
        digitalWrite(LEDpin, HIGH);
        Serial.print("DATA: ");
        for (int i = 0; i < 6; i++) {
          Serial.print(data->fields.data[i]);
          Serial.print(" ");
        }
        Serial.println();
      } else {
        Serial.println("INVALID CMD");
      }
    } else {
      Serial.println("INVALID FORMAT");
    }

    // reset the flags and buffer
    __data_received_flag = false;
    __BytesKeepComing = false;
    pos = 0;
  }
}

void SPI0_Handler() {
  if (SPI_SR_OVRES == HIGH) {
    __overs_flag = true;
  }

  if ((!__overs_flag) && (SPI0->SPI_SR & SPI_SR_RDRF)) {
    if (__BytesKeepComing) {
      __ByteReceived = SPI0->SPI_RDR;
      bytes_received_buffer[pos] = __ByteReceived;
      pos++;

      if (pos >= sizeof(cmd_msg)) {
        __data_received_flag = true;
      } else {
        __BytesKeepComing = true;
        __data_received_flag = false;
      }
    } else {
      __ByteReceived = SPI0->SPI_RDR;
      __BytesKeepComing = true;
      __data_received_flag = false;
    }

    if (__data_received_flag) {
      __BytesKeepComing = false;
      SPI0->SPI_RDR;
    }
  } else {
    Serial.println("UNREAD DATA");
  }

  return;
}
