#define SERIAL0_BAUD1 115200
#define SERIAL3_BAUD  115200
#define SERIAL3_BAUD1 1000000
#define LED_pin 4

#define CMD_LED_ON_1  10 // 00001010 (BIN)
#define CMD_LED_OFF_1 20 // 00010100 (BIN)
#define GIVE_STATE    30 // 00011110 (BIN)

#define CMD_LED_ON_L  79787978 // 00000100-11000001-01110111-11001010 (BIN)
#define CMD_LED_OFF_L 45984598 // 00000010-10111101-10101011-01010110 (BIN) 
#define ST_LED_ON_L   87978797 // 00000101-00111110-01110011-00101101 (BIN)
#define ST_LED_OFF_L  89548954 // 00000101-01010110-01101000-10011010 (BIN)

uint8_t data2receive;

void respond2setByte() {
  digitalWrite(LED_pin,HIGH);
  return;
}

void respond2set2Bytes() {
  digitalWrite(LED_pin,HIGH);
  
  return;
}

void respond2getByte() {
  digitalWrite(LED_pin,LOW);
  Serial3.write(ST_LED_OFF_1);
  return;
}

void setup() {
  pinMode(LED_pin, OUTPUT);
  Serial.begin(SERIAL0_BAUD1);
  Serial3.begin(SERIAL3_BAUD);

  while(!Serial);
  Serial.println("SLAVE STARTED");
}

void loop() {
  if (Serial3.available()) {
    data2receive = Serial3.read();
    if (data2receive == CMD_LED_ON_1) {
      respond2setByte();
    } else if (data2receive == CMD_LED_OFF_1) {
      respond2getByte();
    } else {
      Serial.println("UNKNOWN COMMAND RECEIVED!");
    }
    Serial.flush();
  }
  
  delayMicroseconds(100);
}
