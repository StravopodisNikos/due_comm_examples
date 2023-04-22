#include <Dynamixel2Arduino.h>

#define SERIAL0_BAUD1 115200
#define SERIAL3_BAUD  115200
#define SERIAL3_BAUD1 1000000

#define MAX_PACKET_BITS 10

const uint8_t DXL_DIR_PIN = 2;

/*
 * DUE(master) -> Serial0 -> PC
 * DUE(master) -> Serial3 -> DUE(slave)
 * Master sends 8 and 16 bit packets and slave
 * reacts and responds.
 */

#define CMD_LED_ON  7978 // 1F2A (HEX) // 00011111-00101010 (BIN)
#define CMD_LED_OFF 4598 // 11F6 (HEX) // 00010001-11110110 (BIN)
#define ST_LED_ON   8797 // 225D (HEX) // 00100010-01011101 (BIN)
#define ST_LED_OFF  8954 // 22FA (HEX) // 00100010-11111010 (BIN)

#define CMD_LED_ON_1  0b00101010
#define CMD_LED_OFF_1 0b11110110
#define ST_LED_ON_1   0b01011101
#define ST_LED_OFF_1  0b11111010

uint8_t data2send;
uint8_t response;
uint16_t data2send16;

uint16_t merge2Bytes(uint8_t HighB, uint8_t LowB) {
  return ((uint16_t) HighB << 8 ) | LowB;
}

void setByte(uint8_t SetByteCmd) {
  Serial3.write(SetByteCmd);
  return;
}

void set2Bytes(uint16_t SetByteCmd16) {
  uint8_t LowByte  = SetByteCmd16 & 0xFF;
  uint8_t HighByte = (SetByteCmd16 >> 8) & 0xFF;
  
  Serial3.write(HighByte);
  while(Serial3.availableForWrite()<MAX_PACKET_BITS);
  Serial3.write(LowByte);
  return;
}

uint8_t getByte(uint8_t GetByteCmd) {
    Serial3.write(GetByteCmd);
    while(!Serial3.available());
    return Serial3.read();
}

uint16_t get2Byte(uint8_t Get2BytesCmd) {
    while(Serial3.availableForWrite()<MAX_PACKET_BITS);
    Serial3.write(Get2BytesCmd);
    while(!Serial3.available());
    uint8_t HighByteReceived = Serial3.read();
    while(!Serial3.available());
    uint8_t LowByteReceived = Serial3.read();
    uint16_t Packet16Received = merge2Bytes(HighByteReceived, LowByteReceived);
    return Packet16Received;
}

Dynamixel2Arduino dxl(Serial, DXL_DIR_PIN);

void setup() {
  SerialUSB.begin(SERIAL0_BAUD1);
  Serial3.begin(SERIAL3_BAUD);

  while (!SerialUSB); //wait for the Serial Monitor to pop
  SerialUSB.println("MASTER STARTED");

}

void loop() {
  setByte(CMD_LED_ON_1); delay(1000);
  response = getByte(CMD_LED_OFF_1); delay(1000);
  SerialUSB.print("SLAVE RESPONDED: "); SerialUSB.println(response,BIN);

  
}
