#include <Arduino.h>
#include <SPI.h>
#include <sam.h>
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
#define LEDpin      4

uint8_t CMD_Received;
uint8_t last_CMD_Transit;
float __CMD_Received_f;
volatile uint32_t __slave_status;
volatile long unsigned int * data_in;
volatile bool __data_received_flag;
volatile bool __overs_flag;
volatile bool __modfault_flag;
volatile bool __DataKeepComing;
uint16_t receivedData = 0;
volatile uint8_t pos;
volatile uint16_t __PacketReceived[4];
bool react2ReceivedCmd;

typedef union{
  float res_f;
  uint16_t res_parts[4];
} res_un_t;
res_un_t res2send;

// [1-4-223] Author Stravopodis N.
void setupSlave(uint8_t cs_pin)
{
  pinMode(SPI0_MISO, OUTPUT);
  // SPI Control Register(P.32.8.1/p.693): Enable the SPI to transfer and receive data
  SPI0->SPI_CR  = 0x00000001; //SPI_CR_SPIEN  
  // SPI Mode Register(P.32.8.2/p.694): Enable the SPI in Slave Mode
  SPI0->SPI_WPMR = 0x00000000; // Write Protection Disabled - NO KEY -> Must be Disabled to write in MR!
  SPI0->SPI_MR  = 0x00000000; //
  // SPI Interrupt Enable Register(P.32.8.6/p.700): Enable the Interrupts
  SPI0->SPI_IER = 0x00000001; // Enable RDRF(Receive Data Register Full Interrupt Enable) 
  NVIC_EnableIRQ(SPI0_IRQn); // FP.10.20.10.1 p.164
  // SPI Chip Select Register (P.32.8.9/p.703)
  SPI0->SPI_CSR[0] = SPI_CSR_NCPHA | SPI_DLYBCT(10,8) | SPI_CSR_BITS_8_BIT| SPI_CSR_SCBR(1);
  __DataKeepComing      = true;
  __data_received_flag  = false;
  __overs_flag          = false;
  __modfault_flag       = false;
  pos = 0;
  return;
}

void respond2master(res_un_t res)
{
  res.res_f = __CMD_Received_f;
  REG_SPI0_TDR = res.res_parts[0]; while (!SPI_SR_TDRE);
  REG_SPI0_TDR = res.res_parts[1]; while (!SPI_SR_TDRE);
  REG_SPI0_TDR = res.res_parts[2]; while (!SPI_SR_TDRE);
  REG_SPI0_TDR = res.res_parts[3]; while (!SPI_SR_TDRE);  
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
  if (SPI_SR_OVRES == HIGH) // Check if OVERES flag is risen
  {
    __slave_status = SPI0->SPI_SR;
    __overs_flag = true;
  }
  if (SPI_SR_MODF == HIGH) // Check if Mode Fault Error flag is risen
  {
    __modfault_flag = true;
  }  
  if ((!__overs_flag) && (SPI0->SPI_SR & SPI_SR_RDRF)) 
  {  
    __PacketReceived[pos] = SPI0->SPI_RDR; // pos = 0

    if (__data_received_flag)
    {
      __DataKeepComing = false;
      SPI0->SPI_RDR; // Clear the RDR
    }
  }
  else
  {
    Serial.println("NO UNREAD DATA");
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
    for (size_t i = 0; i < sizeof(res2send); i++)
    {
      res2send.res_parts[i] = __PacketReceived[i];
    }
    __CMD_Received_f = res2send.res_f;

    pos = 0;    
    __data_received_flag = false;
    react2ReceivedCmd = true;
  }

  if (react2ReceivedCmd)
  {
    if (__CMD_Received_f == CMD_LED_ON_F)
    {
      ChangeLEDstate2ON();
      respond2master(res2send);
    }
    if (__CMD_Received_f == CMD_LED_OFF_F)
    {
      ChangeLEDstate2OFF();
      respond2master(res2send);
    }
    __DataKeepComing = true;
    react2ReceivedCmd = false;
  }

  delay(10);
}
