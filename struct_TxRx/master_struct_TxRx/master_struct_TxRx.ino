#include <SPI.h>

#define SERIAL_BAUD1 250000
#define CSPIN_EXT   10
#define SPI0_MISO   25
#define SPI0_MOSI   26
#define SPI0_SPCK   27
#define SPI0_NPCS0  10

#define CMD_READ_POS      7978    // 1F2A (HEX)
#define READ_POS_F        2.1298f // 0x40084ea5 (HEX)
#define RESP_ID_READ_POS  111
#define CMD_READ_VEL      4598    // 11F6 (HEX)
#define READ_VEL_F        8.4782f // 0x4107a6b5 (HEX)
#define RESP_ID_READ_VEL  222

SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);

typedef struct customResponseCmd {
  uint8_t cmd_id;
  float   data;
} cmd_res_msg; // 5 bytes

typedef union customResponseCmdUnion {
  cmd_res_msg message;
  unsigned char bytes[sizeof(cmd_res_msg)];
} cmd_res_msg_u; // 5 bytes

void DemandResponseFromSlave(uint8_t cs_pin, customResponseCmd & cmd_cdm2send)
{
  customResponseCmdUnion cmd_res_msg_u2send;
  cmd_res_msg_u2send.message = cmd_cdm2send;
  
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  for (size_t i = 0; i < sizeof(customResponseCmdUnion); i++)
  {
    cmd_res_msg_u2send.bytes[i] = SPI.transfer();
  }
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();

  //
  return;
}

void setupMaster(uint8_t cs_pin)
{
  pinMode(cs_pin,OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin();
  return;
}

void buildMsgLedOn(customResponseCmd & cmdLedOn)
{
  cmdLedOn.cmd_id = (uint8_t) CMD_LED_ON;
  cmdLedOn.data   = LED_ON_F;
  return;
}
void buildMsgLedOff(customResponseCmd & cmdLedOff)
{
  cmdLedOff.cmd_id = (uint8_t) CMD_LED_OFF;
  cmdLedOff.data   = LED_OFF_F;
  return;
}

void setup() {
  Serial.begin(SERIAL_BAUD1);
  setupMaster((uint8_t) SPI0_NPCS0);
  
  Serial.println("MASTER started sending data...");
  delay(500);
}

void loop() {
  customResponseCmd custom_CMD;
  
  Serial.println("--SENDING: LED ON--");
  buildMsgLedOn(custom_CMD);
  sendcustomResponseCmd2Slave( (uint8_t) SPI0_NPCS0, custom_CMD);
  delay(1000);
  Serial.println("--SENDING: LED OFF--");
  buildMsgLedOff(custom_CMD);
  sendcustomResponseCmd2Slave( (uint8_t) SPI0_NPCS0, custom_CMD);
  delay(1000);
}
