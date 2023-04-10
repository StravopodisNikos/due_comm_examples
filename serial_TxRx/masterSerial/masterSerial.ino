#include <SPI.h>
/*
 * ***********************************************************
 * ********************       MASTER       *******************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Master for SPI Communication. Summary:
 * 
 * MASTER (DUE1) sends a byte chain of 6-byte length. The first byte
 * is used as syncByte. It tells the slave that the next 4bytes that
 * are sent form a float value that should be handled in a specific way.
 * Here: The value should be checked and the specific led shoulb be on
 * and a new float should be returned to master.
 * SLAVE (DUE) Knows beforehand the length of the Packet. So checks that
 * all 5 bytes are received. Checks the first and responds as previously
 * stated. The final byte (infoByte) is used to make sure the transfer aof all packets 
 * was completed
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
#define RESPONSE_TIMEOUT  10 // [ms]
#define SYNCED            99
#define DONE              98
#define DECOY_F           999.999 // 0x4479fff0 [HEX] 01000100/01111001/11111111/11110000 [BIN]

SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);

unsigned long time_start_micros;
float SlaveStateReceived;
uint8_t txBuf[sizeof(float)]; // each var passed to slave must be transfered here before transfer() call!
uint8_t rxBuf[sizeof(float)]; // received packets from slave after each single transfer call

void setup() {
  float test_f;
  Serial.begin(SERIAL_BAUD1);
  setupMaster((uint8_t) SPI0_NPCS0);
  Serial.print("["); Serial.print(millis()); Serial.print("] "); Serial.println("MASTER STARTED");

  // TEST CONVERSION FLOAT<--> BYTES
  /*
  split_float_to_bytes((float) DECOY_F, txBuf);
  Serial.println(txBuf[3],BIN); rxBuf[0] = txBuf[3];
  Serial.println(txBuf[2],BIN); rxBuf[1] = txBuf[2];
  Serial.println(txBuf[1],BIN); rxBuf[2] = txBuf[1];
  Serial.println(txBuf[0],BIN); rxBuf[3] = txBuf[0];
  test_f =  merge_bytes_to_float(txBuf);
  Serial.print("FORWARD PUSH: "); Serial.println(test_f,4); //-> THIS WORKS
  */
  
  // TEST CODE 
  // /*
  getLedState((uint8_t) SPI0_NPCS0,(uint8_t) CMD_GET_STATE);
  delay(1000);
  setLedState((uint8_t) SPI0_NPCS0,(uint8_t) CMD_LED_ON);
  delay(1000);
  getLedState((uint8_t) SPI0_NPCS0,(uint8_t) CMD_GET_STATE);
  delay(1000);
  /*
  setLedState((uint8_t) SPI0_NPCS0,(uint8_t) CMD_LED_OFF);
  delay(5000);
  getLedState((uint8_t) SPI0_NPCS0,(uint8_t) CMD_GET_STATE);
  delay(1000); */
}

void loop() {
  // put your main code here, to run repeatedly:

}
