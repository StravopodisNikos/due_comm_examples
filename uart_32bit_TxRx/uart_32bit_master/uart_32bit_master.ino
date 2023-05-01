#include <Dynamixel2Arduino.h>

#define SERIAL0_BAUD1 115200
#define SERIAL3_BAUD  115200
#define SERIAL3_BAUD1 1000000

#define MAX_PACKET_BITS  10
#define BIT32_ARRAY_SIZE 4

const uint8_t DXL_DIR_PIN = 2;

/*
 * DUE(master) -> Serial0 -> PC
 * DUE(master) -> Serial3 -> DUE(slave)
 * Master sends 32 bits through 4 packets and slave
 * reacts and responds.
 */
#define CMD_LED_ON_1  10 // 00001010 (BIN)
#define CMD_LED_OFF_1 20 // 00010100 (BIN)
#define GIVE_STATE    30 // 00011110 (BIN)
#define SYNCED        111// 01101111 (BIN)
#define CMD_LED_ON_L  79787978 // 00000100-11000001-01110111-11001010 (BIN)
#define CMD_LED_OFF_L 45984598 // 00000010-10111101-10101011-01010110 (BIN) 
#define ST_LED_ON_L   87978797 // 00000101-00111110-01110011-00101101 (BIN)
#define ST_LED_OFF_L  89548954 // 00000101-01010110-01101000-10011010 (BIN)

float data2send_f;
float response_f;
long data2send_l;
long response_l;
uint8_t next_cmd_flag;
uint8_t synced_response;
uint8_t bit32_array[BIT32_ARRAY_SIZE];

void set4Bytes(uint8_t * Byte4Array, uint8_t Set4BytesCmd) {
  // SEND THE FLAG BYTE - TELLS THAT A PACKET OF 4 BYTES IS INCOMING NEXT
  //while(Serial3.availableForWrite()<MAX_PACKET_BITS);
  do {
    Serial3.write(Set4BytesCmd);
    while(!Serial3.available());
    synced_response = Serial3.read();
    SerialUSB.println("6");
  } while (!(synced_response == SYNCED));
  SerialUSB.println("SYNCED");
  for (int i = 0; i < BIT32_ARRAY_SIZE; i++){
    //while(Serial3.availableForWrite() < MAX_PACKET_BITS);
    Serial3.write(*(Byte4Array + i));
  }
  return;
}

void get4Byte(uint8_t * Byte4Array, uint8_t Get4BytesCmd) {
    // SEND THE FLAG BYTE - DEMANDS A PACKET OF 4 BYTES NEXT
    //while(Serial3.availableForWrite()<MAX_PACKET_BITS);
    Serial3.write(Get4BytesCmd);
    for (int i = 0; i < BIT32_ARRAY_SIZE; i++){
      while(!Serial3.available());
      *(Byte4Array + i) = Serial3.read();
    }
}

template<typename T>
void split_32bits_to_bytes(T val, uint8_t *byte_array) {
  uint8_t *ptr = reinterpret_cast<uint8_t*>(&val);
  for (int i = 0; i < BIT32_ARRAY_SIZE; i++) {
    byte_array[i] = ptr[i];
  }
}
template void split_32bits_to_bytes<float>(float val, uint8_t *byte_array);
template void split_32bits_to_bytes<long>(long val, uint8_t *byte_array);

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


Dynamixel2Arduino dxl(Serial, DXL_DIR_PIN);

void setup() {
  SerialUSB.begin(SERIAL0_BAUD1);
  Serial3.begin(SERIAL3_BAUD);

  while (!SerialUSB); //wait for the Serial Monitor to pop
  SerialUSB.println("MASTER STARTED");

}

void loop() {
  // SET THE LED->ON
  next_cmd_flag = (uint8_t) CMD_LED_ON_1;
  split_32bits_to_bytes(CMD_LED_ON_L, bit32_array);
  for (int i = 0; i < BIT32_ARRAY_SIZE; i++) {
    SerialUSB.print("Sent Byte["); SerialUSB.print(i); SerialUSB.print("]:");
    SerialUSB.println(bit32_array[i],BIN);
  }
  set4Bytes(bit32_array, next_cmd_flag);
  delay(1000);
  /*
  // GET THE CURRENT SLAVE STATE
  next_cmd_flag = (uint8_t) GIVE_STATE;
  get4Byte(bit32_array, next_cmd_flag);
  for (int i = 0; i < BIT32_ARRAY_SIZE; i++) {
    SerialUSB.print("Received Byte["); SerialUSB.print(i); SerialUSB.print("]:");
    SerialUSB.println(bit32_array[i],BIN);
  }
  merge_bytes_to_32bits(response_l, bit32_array);
  SerialUSB.print("STATE 1 RECEIVED: ");SerialUSB.println(response_l,DEC);
  */
  // SET THE LED->OFF
  next_cmd_flag = (uint8_t) CMD_LED_OFF_1;
  split_32bits_to_bytes(CMD_LED_OFF_L, bit32_array);
  for (int i = 0; i < BIT32_ARRAY_SIZE; i++) {
    SerialUSB.print("Sent Byte["); SerialUSB.print(i); SerialUSB.print("]:");
    SerialUSB.println(bit32_array[i],BIN);
  }
  set4Bytes(bit32_array, next_cmd_flag);
  delay(1000);  
  /*
  // GET THE CURRENT SLAVE STATE
  next_cmd_flag = (uint8_t) GIVE_STATE;
  get4Byte(bit32_array, next_cmd_flag);
  for (int i = 0; i < BIT32_ARRAY_SIZE; i++) {
    SerialUSB.print("Received Byte["); SerialUSB.print(i); SerialUSB.print("]:");
    SerialUSB.println(bit32_array[i],BIN);
  }
  merge_bytes_to_32bits(response_l, bit32_array);
  SerialUSB.print("STATE 2 RECEIVED: ");SerialUSB.println(response_l,DEC);
  */
}
