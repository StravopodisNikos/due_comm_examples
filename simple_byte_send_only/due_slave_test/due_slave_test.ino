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

#define LEDpin      4

uint8_t CMD_Received;
volatile uint8_t __CMD_Received;
volatile uint32_t __slave_status;
volatile long unsigned int * data_in;
volatile bool __data_received_flag;
volatile bool __overs_flag;
volatile bool __modfault_flag;
uint16_t receivedData = 0;

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
  
  __data_received_flag  = false;
  __overs_flag          = false;
  __modfault_flag       = false;
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
  if (SPI0->SPI_SR & SPI_SR_RDRF) // There are unread data in the Receive Data Register
  {  
    __CMD_Received = SPI0->SPI_RDR; 
    //Serial.print("__CMD_Received= "); Serial.println(__CMD_Received,HEX);
    __data_received_flag = true;
    SPI0->SPI_RDR; // Clear the buffer
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
}

void loop() 
{
  if (__data_received_flag)
  {
    __data_received_flag = false;
  }
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
  if ( (__CMD_Received) == CMD_LED_ON)
  {
    ChangeLEDstate2ON();
    Serial.println("LED ON");
  }
  if ( (__CMD_Received) == CMD_LED_OFF)
  {
    ChangeLEDstate2OFF();
    Serial.println("LED OFF");
  }  
  delay(10);
}
