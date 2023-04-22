
#define SERIAL0_BAUD1 250000
#define SERIAL3_BAUD  115200

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

#define CMD_LED_ON_1 0b00101010;
#define CMD_LED_ON_2 0b00011111;

uint8_t data2send;

void setup() {
  Serial.begin(SERIAL0_BAUD1);
  Serial3.begin(SERIAL3_BAUD);

  while (!Serial); //wait for the Serial Monitor to pop
  Serial.println("MASTER STARTED");

}

void loop() {
  data2send = CMD_LED_ON_1;
  Serial3.write(data2send);
  Serial.println("SENT BYTE 1");
  delay(2000);
  data2send = CMD_LED_ON_2;
  Serial3.write(data2send);
  Serial.println("SENT BYTE 2");
  delay(2000);
}
