
#define SERIAL0_BAUD1 250000
#define SERIAL3_BAUD  115200
#define SERIAL3_BAUD1 1000000

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

void setByte(uint8_t SetByteCmd) {
  Serial3.write(SetByteCmd);
  return;
}

uint8_t getByte(uint8_t GetByteCmd) {
    Serial3.write(GetByteCmd);
    while(!Serial3.available());
    return Serial3.read();
}

void setup() {
  Serial.begin(SERIAL0_BAUD1);
  Serial3.begin(SERIAL3_BAUD1);

  while (!Serial); //wait for the Serial Monitor to pop
  Serial.println("MASTER STARTED");

}

void loop() {
  setByte(CMD_LED_ON_1); delay(1000);
  response = getByte(CMD_LED_OFF_1); delay(1000);
  Serial.print("SLAVE RESPONDED: "); Serial.println(response,BIN);
}
