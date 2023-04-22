#define SERIAL0_BAUD1 250000
#define SERIAL3_BAUD  115200

uint8_t reverseByte(uint8_t b) {
  uint8_t reversed = 0;
  for (int i = 0; i < 8; i++) {
    reversed = (reversed << 1) | (b & 1);
    b >>= 1;
  }
  return reversed;
}

void setup() {
  Serial.begin(SERIAL0_BAUD1);
  Serial3.begin(SERIAL3_BAUD);

  while(!Serial);
  Serial.println("SLAVE STARTED");
}

void loop() {
  if (Serial3.available()) {
    uint8_t b = Serial3.read();
    uint8_t reversed = reverseByte(b);
    int decimal = (int)reversed;
    Serial.print("Reversed byte: "); Serial.println(reversed, BIN);
    Serial.print(" Decimal value: "); Serial.println(decimal);
  }
}
