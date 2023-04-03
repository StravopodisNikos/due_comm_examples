#include <SPI.h>
/*
 * ***********************************************************
 * ********************       MASTER       *******************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Master for SPI Communication. Summary:
 * 
 * When MASTER (DUE1) sends a specific (uint8_t) command ID (CMD_ID),
 *  the SLAVE(DUE2) sets the LED ON (CMD_ID=95) or OFF (CMD_ID=96).
 *  
 * [2-4-23] Author: Stravopodis Nikos
 */
#define SERIAL_BAUD1 250000
#define CSPIN_EXT   10
#define SPI0_MISO   25
#define SPI0_MOSI   26
#define SPI0_SPCK   27
#define SPI0_NPCS0  10

#define CMD_LED_ON  95
#define CMD_LED_OFF 96

SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);

void setupMaster(uint8_t cs_pin)
{
  pinMode(cs_pin,OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin();
  return;
}

void sendCmd2Slave(uint8_t cs_pin, uint8_t cmd)
{
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  SPI.transfer(cmd);
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
  sendCmd2Slave( (uint8_t)  SPI0_NPCS0, (uint8_t) CMD_LED_ON);
  Serial.println("MASTER SENT: CMD_LED_ON"); delay(500);
  sendCmd2Slave( (uint8_t)  SPI0_NPCS0, (uint8_t) CMD_LED_OFF);
  Serial.println("MASTER SENT: CMD_LED_OFF"); delay(500);  

}
