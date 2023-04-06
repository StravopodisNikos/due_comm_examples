#include <Arduino.h>
#include <SPI.h>
#include <sam.h>
#include "core_cm3.h"
/*
 * ***********************************************************
 * ********************       SLAVE       ********************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Slave for SPI Communication. Summary:
 * 
 *  When MASTER (DUE1) sends a specific (float) value (32bit),
 *  the SLAVE(DUE2) responds accordingly. Main difference is
 *  that the transaction is executed by defining a Union and
 *  passing the 2 16bit packets of the float to be transfered.
 * 
 * [6-4-23] Author: Stravopodis Nikos
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
#define IMVALID_CMD   66.6699f // 0x428556fd [HEX] 01000010/10000101/01010110/11111101 [BIN]
#define LEDpin      4

volatile uint32_t __slave_status;
volatile long unsigned int * data_in;
volatile bool __data_received_flag;
volatile bool __overs_flag;
volatile bool __modfault_flag;
volatile bool __DataKeepComing;
uint16_t receivedData = 0;
volatile uint8_t pos;
bool react2ReceivedCmd;
volatile int cnt;
// Globally define the type of the msgs exchanged between master-slave
typedef union{
  float data;
  uint8_t packets[4];
} msg_t;
volatile msg_t cmd2receive;
volatile msg_t res2send;

//void respond2cmd(volatile msg_t cmd_received , volatile msg_t & state_response)
void respond2cmd()
{
  // Here the float data value of the response is given, based on
  // the received command. The byte packets of this value that are
  // to be sent are actually sent in sendPackets() 
  if (cmd2receive.data == CMD_LED_ON_F)
  {
    ChangeLEDstate2ON();
    Serial.println("4");
    res2send.data = STATE_LED_ON;
    Serial.println("LED ON");
  }
  else if (cmd2receive.data == CMD_LED_OFF_F)
  {
    ChangeLEDstate2OFF();
    Serial.println("5");
    res2send.data = STATE_LED_OFF;
    Serial.println("LED OFF");
  }  
  else
  {
    res2send.data = IMVALID_CMD;
    Serial.println("NO VALID CMD RECEIVED");
  }
  return;
}

void sendPackets()
{
  cnt = 0;
  while ( ( cnt < 4 )) { 
    SPI0->SPI_TDR = res2send.packets[cnt];    // send each byte
    //while (!SPI_SR_TXEMPTY) {Serial.println("1");}; // wait until byte is sent
    Serial.print("Packet SENT:"); Serial.println(res2send.packets[cnt]);
    cnt++;
  }
  return;
}

void setupSlave(uint8_t cs_pin)
{
  pinMode(SPI0_NPCS0, INPUT_PULLUP);
  pinMode(SPI0_MISO, OUTPUT);
  // SPI Control Register(P.32.8.1/p.693): Enable the SPI to transfer and receive data
  SPI0->SPI_CR  = 0x00000001; //SPI_CR_SPIEN  
  // SPI Mode Register(P.32.8.2/p.694): Enable the SPI in Slave Mode
  SPI0->SPI_WPMR = 0x00000000; // Write Protection Disabled - NO KEY -> Must be Disabled to write in MR!
  SPI0->SPI_MR  = 0x00000000; //
  // SPI Interrupt Enable Register(P.32.8.6/p.700): Enable the Interrupts
  SPI0->SPI_IER = 0x00000001; // Enable RDRF(Receive Data Register Full Interrupt Enable) 
  NVIC_EnableIRQ(SPI0_IRQn); // FP.10.20.10.1 p.164
  NVIC_SetPriority(SPI0_IRQn, 2 << 4);
  // SPI Chip Select Register (P.32.8.9/p.703)
  SPI0->SPI_CSR[0] = SPI_CSR_NCPHA | SPI_DLYBCT(10,8) | SPI_CSR_BITS_8_BIT| SPI_CSR_SCBR(1);
  __DataKeepComing      = true;
  __data_received_flag  = false;
  __overs_flag          = false;
  __modfault_flag       = false;
  pos = 0;
  return;
}

void ChangeLEDstate2ON()
{
  //__led_state = !__led_state;
  digitalWrite(LEDpin, HIGH); delay(500);
  return;
}
void ChangeLEDstate2OFF()
{
  //__led_state = !__led_state;
  digitalWrite(LEDpin, LOW); delay(500);
  return;
}

void SPI0_Handler() 
{
  // [DEBUG] Check the counters!!!
  cnt = 3;
  Serial.println(cnt);
  while ( (cnt>=0) )
  {   
    Serial.println("SPI0_Handler is dead...");
    //while ((SPI0->SPI_SR & SPI_SR_RDRF)) {Serial.println("2");};
    cmd2receive.packets[cnt] = SPI0->SPI_RDR;
    Serial.print("Packet[");Serial.print(cnt);Serial.print("] received:"); Serial.println(cmd2receive.packets[cnt],BIN);
    cnt--;
    Serial.println(cnt);
    if (cnt == 0)
    {
      __data_received_flag = true;
      //respond2cmd(cmd2receive, res2send);
  
    }   
  }

  if (__data_received_flag)
  {
    respond2cmd();
    sendpackets();
  }
  return;
}

void setup() 
{
  Serial.begin(SERIAL_BAUD1);
  SPI.begin(SPI0_NPCS0);
  SPI.setBitOrder(MSBFIRST);
  setupSlave((uint8_t) SPI0_NPCS0);
  SPI.attachInterrupt();
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW); // Make sure Led is OFF
  ChangeLEDstate2ON(); delay(500); ChangeLEDstate2OFF(); delay(500);
 
  Serial.println("Slave starts listening to master...");
  react2ReceivedCmd = false;
}

void loop() 
{
  if (__modfault_flag)
  {
    // Do nothing...
    __modfault_flag = false;
  }
  if (__overs_flag)
  {
    // Do nothing...
    __overs_flag = false;
  }  
  if (__data_received_flag)
  {
    //respond2cmd(cmd2receive.data, res2send.data);
    //sendPackets();
    __data_received_flag = false;
    //react2ReceivedCmd = true;
  }

  delay(10);
}
