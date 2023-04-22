#include <SPI.h>
/*
 * ***********************************************************
 * ********************       MASTER       *******************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Master for SPI Communication. Summary:
 * 
 * MASTER (DUE1) sends a byte chain of 5-byte length. The first byte
 * is used as sync byte. It tells the slave that the next 4bytes that
 * are sent form a float value that should be handled in a specific way.
 * Here: The value should be checked and the specific led shoulb be on
 * and a new float should be returned to master. A struct is used to
 * define the transfer packet. 
 * SLAVE (DUE) Knows beforehand the length of the Packet. So checks that
 * all 5 bytes are received. Checks the first and responds as previously
 * stated. 
 *  
 * [7-4-23] Author: Stravopodis Nikos
 */
#define SERIAL_BAUD1      250000
#define CSPIN_EXT         10
#define SPI0_MISO         25
#define SPI0_MOSI         26
#define SPI0_SPCK         27
#define SPI0_NPCS0        10
#define CMD_RECEIVED      69       // 0x45 [HEX] 1000101 [BIN]
#define CMD_GET_STATE     85       // 0x55       [HEX] 1010101 [BIN]
#define CMD_GET_STATE_F   17.1796f // 0x41896fd2 [HEX] 01000001/10001001/01101111/11010010 [BIN]
#define CMD_LED_ON        95       // 0x5F       [HEX] 1011111 [BIN]
#define CMD_LED_OFF       105      // 0x69       [HEX] 1101001 [BIN]   
#define CMD_LED_ON_F      14.3785f // 0x41660e56 [HEX] 01000001/01100110/00001110/01010110 [BIN]
#define CMD_LED_OFF_F     96.7854f // 0x42c191aa [HEX] 01000010/11000001/10010010/00100000 [BIN]
#define STATE_LED_ON      55       // 0x37       [HEX] 110111 [BIN]
#define STATE_LED_OFF     66 //65       // 0x41       [HEX] 1000001 [BIN]
#define STATE_LED_ON_F    64.7836f // 0x42819134 [HEX] 01000010/10000001/10010001/00110100 [BIN]
#define STATE_LED_OFF_F   74.2418f // 0x42947bcd [HEX] 01000010/10010100/01111011/11001101 [BIN]
#define OUT_OF_BOUNDS     39.4719f // 0x421de33a [HEX] 01000010/00011101/11100011/00111010 [BIN]
#define RESPONSE_TIMEOUT  50 // [ms]
#define SIZEOF_PACKET     5

SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);

struct Packet {
  uint8_t sync;
  float data;
};
union PacketUnion {
  Packet packet;
  uint8_t bytes[SIZEOF_PACKET];
};

unsigned long time_start_micros;
PacketUnion master_packet;

Packet CMD_LED_ON_Pck;
Packet CMD_LED_OFF_Pck;
void setup() {
  Serial.begin(SERIAL_BAUD1);
  setupMaster((uint8_t) SPI0_NPCS0);
  Serial.print("MASTER STARTED"); printTimeNow();

  /*
   *  One time execute
   */
   // Set Led state to ON
  assignUnionMembers(master_packet, (uint8_t) CMD_LED_ON, (float) CMD_LED_ON_F);
  Serial.print("master_packet.sync_byte_1 : "); Serial.println(master_packet.packet.sync,BIN);
  setLedState((uint8_t) SPI0_NPCS0, master_packet);
  Serial.print("master_packet.sync_byte_2 : "); Serial.println(master_packet.packet.sync,BIN);
  Serial.print("MASTER SET LED STATE: [ON]"); printTimeNow(); 
  delay(1000);
  // Set Led state to OFF
  assignUnionMembers(master_packet, (uint8_t) CMD_LED_OFF, (float) CMD_LED_OFF_F);
  Serial.print("master_packet.sync_byte_3 : "); Serial.println(master_packet.packet.sync,BIN);
  setLedState((uint8_t) SPI0_NPCS0, master_packet);
  Serial.print("master_packet.sync_byte_4 : "); Serial.println(master_packet.packet.sync,BIN);
  Serial.print("MASTER SET LED STATE: [OFF]"); printTimeNow(); 
  delay(1000);
  // Set Led state to ON
  assignUnionMembers(master_packet, (uint8_t) CMD_LED_ON, (float) CMD_LED_ON_F);
  Serial.print("master_packet.sync_byte_1 : "); Serial.println(master_packet.packet.sync,BIN);
  setLedState((uint8_t) SPI0_NPCS0, master_packet);
  Serial.print("master_packet.sync_byte_2 : "); Serial.println(master_packet.packet.sync,BIN);
  Serial.print("MASTER SET LED STATE: [ON]"); printTimeNow(); 
  delay(1000);
}

void loop() { /*
  // Set Led state to ON
  assignUnionMembers(master_packet, (uint8_t) CMD_LED_ON, (float) CMD_LED_ON_F);
  Serial.print("master_packet.sync_byte_1 : "); Serial.println(master_packet.packet.sync,BIN);
  setLedState((uint8_t) SPI0_NPCS0, master_packet);
  Serial.print("master_packet.sync_byte_2 : "); Serial.println(master_packet.packet.sync,BIN);
  Serial.print("MASTER SET LED STATE: [ON]"); printTimeNow(); 
  delay(1000);

  // Get Led state
  //master_packet.packet.sync = (uint8_t) CMD_GET_STATE;
  //master_packet.packet.data = (float) CMD_GET_STATE_F;
  //getLedState((uint8_t) SPI0_NPCS0, master_packet);
  //printState(master_packet);
  //delay(2000);
  
  // Set Led state to OFF
  assignUnionMembers(master_packet, (uint8_t) CMD_LED_OFF, (float) CMD_LED_OFF_F);
  Serial.print("master_packet.sync_byte_3 : "); Serial.println(master_packet.packet.sync,BIN);
  setLedState((uint8_t) SPI0_NPCS0, master_packet);
  Serial.print("master_packet.sync_byte_4 : "); Serial.println(master_packet.packet.sync,BIN);
  Serial.print("MASTER SET LED STATE: [OFF]"); printTimeNow(); 
  delay(1000);

  // Get Led state
  //master_packet.packet.sync = (uint8_t) CMD_GET_STATE;
  //master_packet.packet.data = (float) CMD_GET_STATE_F;
  //getLedState((uint8_t) SPI0_NPCS0, master_packet);
  //printState(master_packet);
  //delay(2000);  
*/}
