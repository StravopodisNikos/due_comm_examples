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
 * When MASTER (DUE1) sends a union data ty  
 * The SLAVE(DUE2) sets the LED ON (CMD_ID=7978) and prints the  
 * corresponding data (float) value (LED_ON_F=2.1298f) or OFF 
 * (CMD_ID=4598) with data (float) value (LED_OFF_F=8.4782f) .
 * 
 * [4-4-23] Author: Stravopodis Nikos
 */
#define SERIAL_BAUD1 250000
#define CSPIN_EXT   10
#define SPI0_MISO   25
#define SPI0_MOSI   26
#define SPI0_SPCK   27
#define SPI0_NPCS0  10

#define CMD_LED_ON  7978 // 1F2A (HEX)
#define CMD_LED_OFF 4598 // 11F6 (HEX)

#define LEDpin      4


volatile uint8_t pos;
volatile bool process_msg;
volatile bool  __data_received_flag;
volatile bool __BytesKeepComing;
volatile uint8_t __ByteReceived;
volatile bool __overs_flag;
volatile bool __modfault_flag;
volatile uint32_t __slave_status;

typedef struct customCmd {
  uint8_t cmd_id;
  float   data;
} cmd_msg;
typedef union customCmdUnion {
  cmd_msg message;
  uint8_t bytes[sizeof(cmd_msg)];
} cmd_msg_u;
uint8_t bytes_received_buffer[sizeof(cmd_msg)+1];

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
  
  __BytesKeepComing     = true;
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
  if (SPI_SR_OVRES == HIGH) // Check if OVERES flag is risen
  {
    __slave_status = SPI0->SPI_SR;
    __overs_flag = true;
  }
  if ( (!__overs_flag) && (SPI0->SPI_SR & SPI_SR_RDRF) ) // There are unread data in the Receive Data Register
  {  
    if (__BytesKeepComing) 
    {
      __ByteReceived = SPI0->SPI_RDR;
      bytes_received_buffer[pos] = __ByteReceived;
      pos++;
      if (pos >= sizeof(customCmdUnion)) 
      {
        __data_received_flag = true;
      }
      else
      {
        __BytesKeepComing = true;
        __data_received_flag = false;
      }
    }
    else 
    {
      __ByteReceived = SPI0->SPI_RDR; 
      __BytesKeepComing = true;
      __data_received_flag = false;
    }
    
    if (__data_received_flag)
    {
      __BytesKeepComing = false;
      SPI0->SPI_RDR; // Clear the RDR
    }
  }
  else
  {
    Serial.println("UNREAD DATA");
  }
  return;
}

void react2Cmd(customCmd ArrivedCmd)
{
  if (ArrivedCmd.cmd_id == (uint8_t) CMD_LED_ON )
  {
    ChangeLEDstate2ON(); Serial.println("LED ON");
    Serial.print("CMD DATA = "); Serial.println(ArrivedCmd.data,4);    
  }
  else if (ArrivedCmd.cmd_id == (uint8_t) CMD_LED_OFF )
  {
    ChangeLEDstate2OFF(); Serial.println("LED OFF");
    Serial.print("CMD DATA = "); Serial.println(ArrivedCmd.data,4);    
  }
  else
  {
    Serial.println("RECEIVED CORRUPTED DATA!");
    Serial.print("CMD  = "); Serial.println(ArrivedCmd.cmd_id,DEC);
    Serial.print("DATA = "); Serial.println(ArrivedCmd.data,4);  
  }
  return;
}

customCmd newArrivedCmd;
bool react2ReceivedCmd;

void setup() {
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


void loop() {
  if (__overs_flag)
  {
    Serial.println("OVERES FLAG IS SET!");
  }
  if (__data_received_flag)
  {
    bytes_received_buffer[pos] = 0; //why??
    customCmdUnion newArrivedCmdUn;
    for (size_t i = 0; i < sizeof(customCmdUnion); i++)
    {
      newArrivedCmdUn.bytes[i] = bytes_received_buffer[i];
    }
    newArrivedCmd = newArrivedCmdUn.message;

    pos = 0;
    __data_received_flag = false;
    react2ReceivedCmd = true;
  }

  if (react2ReceivedCmd)
  {
    react2Cmd(newArrivedCmd);
    react2ReceivedCmd = false;
    __BytesKeepComing = true;
  }
  delay(10);
}
