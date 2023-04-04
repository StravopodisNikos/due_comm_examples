#include <SPI.h>
/*
 * ***********************************************************
 * ********************       MASTER       *******************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Master for SPI Communication. Summary:
 * 
 * MASTER (DUE1) sends a specific (uint16_t) command ID (CMD_ID),
 * but in 2 parts, each of a length of a single byte.
 * The SLAVE(DUE2) handles the 2 bytes received and sets the LED 
 * ON (CMD_ID=7978) or OFF (CMD_ID=4598).
 *  
 * [4-4-23] Author: Stravopodis Nikos
 */
 
#define SERIAL_BAUD1 250000
#define CSPIN_EXT   10
#define SPI0_MISO   25
#define SPI0_MOSI   26
#define SPI0_SPCK   27
#define SPI0_NPCS0  10

#define CMD_LED_ON  7978 // 1F2A (HEX)
#define CMD_LED_OFF 4598 // 11F6 (HEX) 

SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);

uint8_t array2send[2];
uint16_t MERGED_CMD;

void setupMaster(uint8_t cs_pin)
{
  pinMode(cs_pin,OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin();
  return;
}

void split_uint16(uint16_t value2half, uint8_t *array2store)
{
  * array2store   = value2half >> 8; // HIGH BYTE
  *(array2store+1)= value2half;      // LOW  BYTE
  return;
}

void merge_uint16(uint16_t &merged_value, uint8_t *store_array)
{
  merged_value = ((uint16_t)*store_array << 8) | *(store_array+1);
  return;
}

void sendCmd2Slave16(uint8_t cs_pin, uint16_t cmd)
{
  uint8_t array2send_temp[2];
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  split_uint16(cmd, array2send_temp);  // BREAK THE BYTES
  SPI.transfer(array2send_temp[0]);   // FIRST SENDS THE HIGH BYTE!
  // NO NEED TO WAIT BETWEEN TRANSFERS
  SPI.transfer(array2send_temp[1]);   // THEN SENDS THE LOW BYTE!
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();
  return;
}

void setup() 
{
  Serial.begin(SERIAL_BAUD1);
  setupMaster((uint8_t) SPI0_NPCS0);
  
  Serial.println("MASTER started sending data...");
  delay(500);
}

void loop() {
  Serial.println("--SPLITING--");
  split_uint16((uint16_t) CMD_LED_ON, array2send);
  Serial.print("SEND VALUE   (HEX) = "); Serial.println(CMD_LED_ON,HEX);
  Serial.print("BYTE VALUE 1 (HEX) = "); Serial.println(array2send[0],HEX);
  Serial.print("BYTE VALUE 2 (HEX) = "); Serial.println(array2send[1],HEX);
  delay(1000);
  Serial.println("--MERGING--");
  merge_uint16(MERGED_CMD, array2send);
  Serial.print("MERGED VALUE (HEX) = "); Serial.println(MERGED_CMD,HEX);
  Serial.println("--SENDING--");
  sendCmd2Slave16( (uint8_t)  SPI0_NPCS0, (uint16_t) CMD_LED_ON);
  Serial.println("MASTER SENT: CMD_LED_ON"); delay(500);
  sendCmd2Slave16( (uint8_t)  SPI0_NPCS0, (uint16_t) CMD_LED_OFF);
  Serial.println("MASTER SENT: CMD_LED_OFF"); delay(500);  
  
  delay(1000);
}
