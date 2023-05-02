#define SERIAL0_BAUD1 115200
#define SERIAL3_BAUD  115200
#define SERIAL3_BAUD1 1000000
#define LED_pin 4
#define MAX_PACKET_BITS  10
#define BIT32_ARRAY_SIZE 4
#define CMD_LED_ON_1  10 // 00001010 (BIN)
#define CMD_LED_OFF_1 20 // 00010100 (BIN)
#define GIVE_STATE    30 // 00011110 (BIN)
#define SYNCED        111// 01101111 (BIN)
#define CMD_LED_ON_L  79787978 // 00000100-11000001-01110111-11001010 (BIN)
#define CMD_LED_OFF_L 45984598 // 00000010-10111101-10101011-01010110 (BIN) 
#define ST_LED_ON_L   87978797 // 00000101-00111110-01110011-00101101 (BIN)
#define ST_LED_OFF_L  89548954 // 00000101-01010110-01101000-10011010 (BIN)

template<typename R>
void merge_bytes_to_32bits(R & val, uint8_t *byte_array) {
  uint8_t *ptr = reinterpret_cast<uint8_t*>(&val);
  for (int i = 0; i < BIT32_ARRAY_SIZE; i++) {
    ptr[i] = byte_array[i];
  }
  return;
}
template void merge_bytes_to_32bits<float&>(float&, uint8_t *byte_array);
template void merge_bytes_to_32bits<long&>(long&, uint8_t *byte_array);

uint8_t cmd_received;
float data_received_f;
long data_received_l;
long cur_state_l;
uint8_t bit32_array[BIT32_ARRAY_SIZE];

void print4Bytes(uint8_t * Byte4Array) {
  for (int i = 0; i < BIT32_ARRAY_SIZE; i++){
    while(!Serial3.available());
    *(Byte4Array + i) = Serial3.read();
    Serial.print("Received Byte["); Serial.print(i); Serial.print("]:");
    Serial.println(Byte4Array[i],BIN);
  }
  merge_bytes_to_32bits(data_received_l, Byte4Array);
  Serial.print("Received Data: "); Serial.println(data_received_l,DEC);
}

void returnCurrentState() {
  Serial.println("UNDER DEVEL... SORRY...");
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
    cmd_received = Serial3.read();
    if (cmd_received == CMD_LED_ON_1) {
      Serial3.write((uint8_t) SYNCED);
      digitalWrite(LED_pin,HIGH);
      print4Bytes(bit32_array);
      cur_state_l = ST_LED_ON_L;
    } else if (cmd_received == CMD_LED_OFF_1) {
      Serial3.write((uint8_t) SYNCED);
      digitalWrite(LED_pin,LOW);
      print4Bytes(bit32_array);
      cur_state_l = ST_LED_OFF_L;
    } else if (cmd_received == GIVE_STATE){
      Serial3.write((uint8_t) SYNCED);
      returnCurrentState();
    }
    else {
      Serial.println("UNKNOWN COMMAND RECEIVED!");
    }
  }
  
  delayMicroseconds(100);
}
