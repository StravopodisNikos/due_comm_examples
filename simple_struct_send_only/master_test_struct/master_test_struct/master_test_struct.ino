#include <SPI.h>

#define SERIAL_BAUD1 250000
#define CSPIN_EXT   10
#define SPI0_MISO   25
#define SPI0_MOSI   26
#define SPI0_SPCK   27
#define SPI0_NPCS0  10

#define CMD_LED_ON  7978    // 1F2A (HEX)
#define LED_ON_F    2.1298f // 0x40084ea5 (HEX)
#define CMD_LED_OFF 4598    // 11F6 (HEX)
#define LED_OFF_F   8.4782f // 0x4107a6b5 (HEX)

SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);

typedef struct customCmd {
  uint8_t cmd_id;
  float   data;
} cmd_msg;

typedef union customCmdUnion {
  cmd_msg message;
  unsigned char bytes[sizeof(cmd_msg)];
} cmd_msg_u;

void sendsendCmd2Slave16(uint8_t cs_pin, customCmd & cmd_cdm2send)
{
  customCmdUnion cmd_msg_u2send;
  cmd_msg_u2send.message = cmd_cdm2send;
  
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  for (size_t i = 0; i < sizeof(cmd_msg); i++)
  {
    SPI.transfer(cmd_msg_u2send.bytes[i]);
  }
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();
  return;
}

void setupMaster(uint8_t cs_pin)
{
  pinMode(cs_pin,OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin();
  return;
}

void buildMsgLedOn(customCmd & cmdLedOn)
{
  cmdLedOn.cmd_id = (uint8_t) CMD_LED_ON;
  cmdLedOn.data   = LED_ON_F;
  return;
}
void buildMsgLedOff(customCmd & cmdLedOff)
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
  Serial.println("--SENDING: LED ON--");
  customCmd CMC_LED_ON;
  buildMsgLedOn(CMC_LED_ON);
  sendsendCmd2Slave16( (uint8_t) SPI0_NPCS0, CMC_LED_ON);
  delay(2000);
  Serial.println("--SENDING: LED OFF--");
  customCmd CMC_LED_OFF;
  buildMsgLedOn(CMC_LED_OFF);
  sendsendCmd2Slave16( (uint8_t) SPI0_NPCS0, CMC_LED_OFF);
  delay(2000);
}
