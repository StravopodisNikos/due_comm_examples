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
#define CMD_LED_ON_F  14.3785f // 0x41660e56 [HEX] 01000001/01100110/00001110/01010110 [BIN]
#define CMD_LED_OFF_F 96.7854f // 0x42c191aa [HEX] 01000010/11000001/10010010/00100000 [BIN]
#define STATE_LED_ON  64.7836f // 0x42819134 [HEX] 01000010/10000001/10010001/00110100 [BIN]
#define STATE_LED_OFF 74.2418f // 0x42947bcd [HEX] 01000010/10010100/01111011/11001101 [BIN]
#define OUT_OF_BOUNDS 39.4719f // 0x421de33a [HEX] 01000010/00011101/11100011/00111010 [BIN]
#define RESPONSE_TIMEOUT 50 // [ms]

// Globally define the type of the msgs exchanged between master-slave
typedef union{
  float data;
  uint8_t packets[4];
} msg_t;
msg_t cmd2send;
msg_t res2receive;

unsigned long trans_time_millis;
unsigned long time_start_millis;
bool timeout_error;
SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);
float SlaveResponseReceived_f;
float val_lim = 10.0f;

void setupMaster(uint8_t cs_pin)
{
  pinMode(cs_pin,OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin();

  timeout_error = false;
  SlaveResponseReceived_f = 0;
  return;
}

void sendReadCmd2SlaveWait(uint8_t cs_pin, msg_t & cmd, msg_t & res)
{
  //begins Transaction and waits for the full response, or timeouts
  bool response_received = false;
  timeout_error = false; 
  uint32_t timeout = millis() + RESPONSE_TIMEOUT; 
  int i =0;
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  // Sends data
  Serial.print("Sending Packet[0]: "); Serial.println(cmd.packets[0],BIN);
  //SPI0->SPI_TDR = cmd.packets[0]; while (!SPI_SR_TXEMPTY) {Serial.println("1");}; // => SPI.transfer(cmd.packets[0]); //while (!SPI_SR_TDRE);
  SPI.transfer(cmd.packets[0]); //while (!SPI_SR_TXEMPTY) {Serial.println("1");};
  Serial.print("Sending Packet[1]: "); Serial.println(cmd.packets[1],BIN);  
  //SPI0->SPI_TDR = cmd.packets[1]; while (!SPI_SR_TXEMPTY);
  SPI.transfer(cmd.packets[1]); //while (!SPI_SR_TXEMPTY) {};
  Serial.print("Sending Packet[2]: "); Serial.println(cmd.packets[2],BIN); 
  //SPI0->SPI_TDR = cmd.packets[2]; while (!SPI_SR_TXEMPTY);
  SPI.transfer(cmd.packets[2]); //while (!SPI_SR_TXEMPTY){};
  Serial.print("Sending Packet[3]: "); Serial.println(cmd.packets[3],BIN); 
  SPI.transfer(cmd.packets[3]); //while (!SPI_SR_TXEMPTY){};
  //SPI0->SPI_TDR = cmd.packets[3]; while (!SPI_SR_TXEMPTY);

  uint8_t bytes[4];
  time_start_millis = millis();
  while ( (i < 4) && (millis() < timeout) ) {
    if (millis() > timeout) {
      SPI.endTransaction();
      digitalWrite(cs_pin, HIGH);
      Serial.println("TIMEOUT_ERROR");
    }
    
    
    for (int j = 0; j < 4; j++) {
      bytes[j] = SPI.transfer(0x00); delayMicroseconds (100);
    }    
    res.packets[i] = *(float*)bytes; 
    Serial.print("Packet["); Serial.print(i); Serial.print("] received: "); Serial.println(res.packets[i],BIN);
    i++;
  }
  trans_time_millis = millis() - time_start_millis;
  Serial.print("Reading Total Time: "); Serial.print(trans_time_millis); Serial.println(" [ms]");
  float received_value = res.data;
  if (abs(received_value) > val_lim) 
  {
    res.data = cmd.data;
    response_received = true;
    Serial.println("Valid Slave responce received!");
    Serial.print("Slave sent: "); Serial.println(received_value,DEC);
  }
  else
  {
    res.data = OUT_OF_BOUNDS;
    response_received = false;
    Serial.println("Slave responce is OUT_OF_BOUNDS!");
  }
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
  //uint8_t slaveResponseAr[4];
  cmd2send.data = CMD_LED_ON_F;
  sendReadCmd2SlaveWait( (uint8_t)  SPI0_NPCS0, cmd2send, res2receive);
  Serial.println("MASTER SENT: CMD_LED_ON_F"); 
  Serial.print("MASTER RECEIVED: "); Serial.println(res2receive.data,DEC);
  delay(5000);

  cmd2send.data = CMD_LED_OFF_F;
  sendReadCmd2SlaveWait( (uint8_t)  SPI0_NPCS0, cmd2send, res2receive);
  Serial.println("MASTER SENT: CMD_LED_OFF_F"); 
  Serial.print("MASTER RECEIVED: "); Serial.println(res2receive.data,DEC);
  delay(5000);  
}
