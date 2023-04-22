#define SERIAL0_BAUD1 250000
#define SERIAL3_BAUD  115200
#define SERIAL3_BAUD1 1000000
#define LED_pin 4

#define CMD_LED_ON_1  0b00101010
#define CMD_LED_OFF_1 0b11110110
#define ST_LED_ON_1   0b01011101
#define ST_LED_OFF_1  0b11111010

uint8_t data2receive;

void respond2setByte() {
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
  Serial3.begin(SERIAL3_BAUD1);

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
