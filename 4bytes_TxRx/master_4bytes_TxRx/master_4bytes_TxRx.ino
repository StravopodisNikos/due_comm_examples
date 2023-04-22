#include <SPI.h>
/*
 * ***********************************************************
 * ********************       MASTER       *******************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Master for SPI Communication. Summary:
 * 
 *  When MASTER (DUE1) sends a specific (float) value (32bit),
 *  the SLAVE(DUE2) responds accordingly. Main difference is
 *  that the transaction is executed by defining a Union and
 *  passing the 2 16bit parts of the float to be transfered.
 *  
 * [5-4-23] Author: Stravopodis Nikos
 */
#define SERIAL_BAUD1 250000
#define CSPIN_EXT   10
#define SPI0_MISO   25
#define SPI0_MOSI   26
#define SPI0_SPCK   27
#define SPI0_NPCS0  10
#define CMD_LED_ON_F  14.3785f // 0x41660e56 [HEX]
#define CMD_LED_OFF_F 96.7854f // 0x42c191aa [HEX]
#define STATE_LED_ON  64.7836f // 0x42819134 [HEX]
#define STATE_LED_OFF 74.2418f // 0x42947bcd [HEX]
#define RESPONSE_TIMEOUT 100 // [ms]

typedef union{
  float cmd_f;
  uint8_t cmd_parts[4];
} cmd_un_t;

cmd_un_t cmd2send;

unsigned long time_start_millis;
bool timeout_error;
SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);
float SlaveResponseReceived_f;

void setupMaster(uint8_t cs_pin)
{
  pinMode(cs_pin,OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin();

  timeout_error = false;
  SlaveResponseReceived_f = 0;
  return;
}

void sendReadCmd2SlaveWait(uint8_t cs_pin, cmd_un_t cmd, uint8_t * response, float & response_f)
{
  //begins Transaction and waits for the full response, or timeouts
  bool response_received = false;
  timeout_error = false;  
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  time_start_millis = millis();
  while ( (!response_received) && (!timeout_error) )
  {
    *response     = SPI.transfer(cmd.cmd_parts[0]); while (!SPI_SR_TDRE);
    *(response+1) = SPI.transfer(cmd.cmd_parts[1]); while (!SPI_SR_TDRE);
    *(response+2) = SPI.transfer(cmd.cmd_parts[2]); while (!SPI_SR_TDRE);
    *(response+3) = SPI.transfer(cmd.cmd_parts[3]); while (!SPI_SR_TDRE);
    if ( (millis() - time_start_millis) > RESPONSE_TIMEOUT)
    {
      timeout_error = true;
      Serial.println("TIMEOUT ERROR!");
    }
    if ( (*response) > 0 )
    {
      response_f = cmd.cmd_f;
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
  uint16_t slaveResponseAr[2];
  cmd2send.cmd_f = CMD_LED_ON_F;
  sendReadCmd2SlaveWait( (uint8_t)  SPI0_NPCS0, cmd2send, slaveResponseAr, SlaveResponseReceived_f);
  Serial.println("MASTER SENT: CMD_LED_ON_F"); 
  Serial.print("MASTER RECEIVED: "); Serial.println(SlaveResponseReceived_f,DEC);
  delay(1000);

  cmd2send.cmd_f = CMD_LED_OFF_F;
  sendReadCmd2SlaveWait( (uint8_t)  SPI0_NPCS0, cmd2send, slaveResponseAr, SlaveResponseReceived_f);
  Serial.println("MASTER SENT: CMD_LED_OFF_F"); 
  Serial.print("MASTER RECEIVED: "); Serial.println(SlaveResponseReceived_f,DEC);
  delay(1000);  

  delay(1000);
}
