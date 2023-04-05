#include <SPI.h>
#include <math.h>
/*
 * ***********************************************************
 * ********************       MASTER       *******************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Master for SPI Communication. Summary:
 * 
 *  When MASTER (DUE1) sends a specific (uint8_t) command ID (CMD_ID),
 *  the SLAVE(DUE2) sets the LED ON (CMD_ID=95) or OFF (CMD_ID=96)
 *  and responds accordingly
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
#define STATE_LED_ON  105
#define STATE_LED_OFF 106
#define BYTE_RECEIVED  96
#define EMPTY_RESPONSE 
#define RESPONSE_TIMEOUT 1 // [ms]

unsigned long time_start_millis;
bool timeout_error;
SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);

void setupMaster(uint8_t cs_pin)
{
  pinMode(cs_pin,OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin();

  timeout_error = false;
  return;
}

void sendReadCmd2SlaveWait(uint8_t cs_pin, uint8_t cmd, uint8_t & response)
{
  //begins Transaction and waits for response, or timeouts
  bool response_received = false;
  timeout_error = false;  
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  time_start_millis = millis();
  while ( (!response_received) && (!timeout_error) )
  {
    response = SPI.transfer(cmd);
    if ( (millis() - time_start_millis) > RESPONSE_TIMEOUT)
    {
      timeout_error = true;
      Serial.println("TIMEOUT ERROR!");
    }
    if (response > 0)
    {
      response_received = true;
      digitalWrite(cs_pin, HIGH);
      SPI.endTransaction();
    }
  }

  if (!response_received)
  {
      digitalWrite(cs_pin, HIGH);
      SPI.endTransaction();    
  }

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
  uint8_t slaveResponse;
  sendReadCmd2SlaveWait( (uint8_t)  SPI0_NPCS0, (uint8_t) CMD_LED_ON, slaveResponse);
  Serial.println("MASTER SENT: CMD_LED_ON"); 
  Serial.print("MASTER RECEIVED: "); Serial.println(slaveResponse,DEC);
  delay(500);

  sendReadCmd2SlaveWait( (uint8_t)  SPI0_NPCS0, (uint8_t) CMD_LED_OFF, slaveResponse);
  Serial.println("MASTER SENT: CMD_LED_OFF"); 
  Serial.print("MASTER RECEIVED: "); Serial.println(slaveResponse,DEC);
  delay(500);  

}
