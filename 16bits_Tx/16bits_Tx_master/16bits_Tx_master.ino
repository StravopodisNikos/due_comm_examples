#include <SPI.h>

#define SERIAL_BAUD1 250000
#define CSPIN_EXT   10
#define SPI0_MISO   25
#define SPI0_MOSI   26
#define SPI0_SPCK   27
#define SPI0_NPCS0  10
#define SPI0_NPCS0  10
#define CMD_LED_ON  7978 // 1F2A (HEX) // 00011111-00101010 (BIN)
#define CMD_LED_OFF 4598 // 11F6 (HEX) // 00010001-11110110 (BIN)
#define ST_LED_ON   8797 // 225D (HEX) // 00100010-01011101 (BIN)
#define ST_LED_OFF  8954 // 22FA (HEX) // 00100010-11111010 (BIN)

/*
 * DUE acts as Master, sends a 16bit length msg, no response from slave.
 * Msg is split. MSB is sent first. Using the advanced SPI capabilities
 * of the DUE, provided by the Arduino SPI library for SAM.
 */

 /*
  * debugging:
  * I get for 4000000, MSBFIRST, SPI_MODE0 -> 11110101 01010010
  */
SPISettings spi_setting(4000000, MSBFIRST, SPI_MODE0);
// With SPI_CLOCK_DIV2 -> pulse width of SCK is 140ns (High), 120ns(Low) ~ 250ns, so 4MHz!!
// 21000000 -> pulse width of SCK is 20ns, still DUE loses pulses!!

uint8_t array2send[2];
uint16_t MERGED_CMD;

void setupMaster(uint8_t cs_pin)
{
  pinMode(cs_pin,OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin(cs_pin);
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
  //SPI.beginTransaction(cs_pin,spi_setting);
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  split_uint16(cmd, array2send_temp);  // BREAK THE BYTES
  //SPI.transfer(cs_pin,array2send_temp[0],SPI_CONTINUE);   // FIRST SENDS THE HIGH BYTE!
  SPI.transfer(cs_pin,array2send_temp[0]);
  // NO NEED TO WAIT BETWEEN TRANSFERS // 1.25μs default delay!! (observed in analyzer)
  //SPI.transfer(cs_pin,array2send_temp[1],SPI_LAST);   // THEN SENDS THE LOW BYTE!
  SPI.transfer(cs_pin,array2send_temp[1]);
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
  delay(1000);
  Serial.println("--SENDING--");
  sendCmd2Slave16( (uint8_t)  SPI0_NPCS0, (uint16_t) CMD_LED_ON);
  Serial.println("MASTER SENT: CMD_LED_ON"); delay(500);
  sendCmd2Slave16( (uint8_t)  SPI0_NPCS0, (uint16_t) CMD_LED_OFF);
  Serial.println("MASTER SENT: CMD_LED_OFF"); delay(500);  
  delay(1000);
}
