#include <Arduino.h>
#include <SPI.h>
//#include <sam.h>
//#include <cstdint>
/*
 * ***********************************************************
 * ********************       SLAVE        *******************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Master for SPI Communication. Summary:
 * 
 * MASTER (DUE1) sends a byte chain of 6-byte length. The first byte
 * is the syncByte. It tells the slave that the next 4bytes that
 * are sent form a float value that should be handled in a specific way.
 * Here: The value should be checked and the specific led shoulb be on
 * and a new float should be returned to master.
 * SLAVE (DUE) understands the length of the Packet based on the
 * received command code.
 *  
 * [10-4-23] Author: Stravopodis Nikos
 */
#define SERIAL_BAUD1      250000
#define LEDpin            4
#define CSPIN_EXT         10
#define SPI0_MISO         25
#define SPI0_MOSI         26
#define SPI0_SPCK         27
#define SPI0_NPCS0        10

#define CMD_RECEIVED      69       // 0x45 [HEX] 1000101 [BIN]
#define CMD_GET_STATE     85       // 0x55       [HEX] 1010101 [BIN]
#define CMD_GET_STATE_F   17.1796f // 0x41896fd2 [HEX] 01000001/10001001/01101111/11010010 [BIN]
#define CMD_LED_ON        95       // 0x5F       [HEX] 1011111 [BIN]
#define CMD_LED_OFF       105      // 0x69       [HEX] 1101001 [BIN]   
#define CMD_LED_ON_F      14.3785f // 0x41660e56 [HEX] 01000001/01100110/00001110/01010110 [BIN]
#define CMD_LED_OFF_F     96.7854f // 0x42c191aa [HEX] 01000010/11000001/10010010/00100000 [BIN]
#define STATE_LED_ON      55       // 0x37       [HEX] 110111  [BIN]
#define STATE_LED_OFF     66       // 0x42       [HEX] 1000010 [BIN]
#define STATE_LED_ON_F    64.7836f // 0x42819134 [HEX] 01000010/10000001/10010001/00110100 [BIN]
#define STATE_LED_OFF_F   74.2418f // 0x42947bcd [HEX] 01000010/10010100/01111011/11001101 [BIN]
#define OUT_OF_BOUNDS     39.4719f // 0x421de33a [HEX] 01000010/00011101/11100011/00111010 [BIN]
#define SYNCED            99       // 0X63       [HRX] 1100011
#define DONE              98       // 0X62       [HRX] 1100010
#define DECOY_F           999.999; // 0x4479fff0 [HEX] 01000100/01111001/11111111/11110000 [BIN]
#define WAIT4SYNC         75       // 0X4b       [HRX] 1001011
#define WAIT4DATA         76       // 0X4c       [HRX] 1001100
#define WAIT4INFO         77       // 0X4d       [HRX] 1001101

#define RESPONSE_TIMEOUT  10       // [ms]

// Define Global slave vars
//***********************
uint8_t rxBuf[sizeof(float)];
uint8_t txBuf[sizeof(float)];
int buf_cnt;
bool transferCompleted;
float TxResponse;
//***********************

// DEFINE ALL VOLATILES
//***********************
volatile bool TIME2READ;
volatile uint8_t __transferState;
volatile uint8_t __syncByte;
volatile uint8_t __infoByte;
volatile uint8_t __newByte;
volatile bool __syncByteReceived          = false; 
volatile bool __newByteReceived           = false;
volatile bool __infoByteReceived          = false;
volatile bool __setModeTriggered          = false;
volatile bool __getModeTriggered          = false;
//***********************

void SPI0_Handler() {
  //if ( (SPI0->SPI_SR & SPI_SR_RDRF) && (TIME2READ) ) { // byte from master arrived and it is also time to read
  if (( SPI_SR_RDRF==HIGH) && (TIME2READ) ) {
    TIME2READ = false; // reverse the flag in order not to read again, before responding
    //Serial.println("READING BUFFER");
    if ( __transferState == (uint8_t )WAIT4SYNC) {
      __syncByte = SPI0->SPI_RDR;
      __syncByteReceived = true;
      __newByteReceived  = false;
      __infoByteReceived = false;
      SPI0->SPI_TDR = (uint8_t) SYNCED;
      SPI0->SPI_RDR;
    } else if ( __transferState == (uint8_t )WAIT4DATA) {
      __newByte = SPI0->SPI_RDR; 
      __syncByteReceived = false;
      __newByteReceived = true; // guides loop to handle data 
      __infoByteReceived = false;   
      SPI0->SPI_RDR;  
    } else if (__transferState == (uint8_t )WAIT4INFO) {
      __infoByte = SPI0->SPI_RDR;
      __syncByteReceived = false;
      __newByteReceived  = false;
      __infoByteReceived = true;
      SPI0->SPI_RDR;
    }
  }
} // ISR END

void setup() {
  Serial.begin(SERIAL_BAUD1);
  SPI.begin(SPI0_NPCS0);
  SPI.setBitOrder(MSBFIRST);
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
  SPI.attachInterrupt();
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW); // Make sure Led is OFF
  //ChangeLEDstate2ON(); delay(500); ChangeLEDstate2OFF(); delay(500);
  Serial.print("["); Serial.print(millis()); Serial.print("] "); Serial.println("SLAVE STARTED "); 
  transferCompleted = false;
  TIME2READ = true;
  buf_cnt = 4;
  __transferState = WAIT4SYNC;
  setupGetMode();
  SPI0->SPI_RDR;
  SPI0->SPI_TDR;
}

void loop() {
  // ISR-GUIDANCE SECTION
  if (__syncByteReceived) { // Here slave returnes SYNCED and the mode is set
     TIME2READ = true;
    Serial.println("1");
    Serial.print("["); Serial.print(millis()); Serial.print("] "); Serial.print("SYNC BYTE: ");
    Serial.println(__syncByte,BIN);
    if (__syncByte == CMD_GET_STATE) {
      SPI0->SPI_TDR = (uint8_t) SYNCED;
      __setModeTriggered = false;
      __getModeTriggered = true;
      __transferState = WAIT4DATA;
      buf_cnt = 4;
    }
    else if ( (__syncByte == CMD_LED_ON) || (__syncByte == CMD_LED_OFF) ) {
      SPI0->SPI_TDR = (uint8_t) SYNCED;
      __setModeTriggered = true;
      __getModeTriggered = false;
      __transferState = WAIT4DATA;
      //buf_cnt = 0;    
    }
    else { // Although used for sync, not known command, so again waits for sync
      __setModeTriggered = false;
      __getModeTriggered = false;
      __syncByteReceived = false;
      __transferState = WAIT4SYNC;
    }
  }
  else if (__newByteReceived) { // byte operation depens on the mode set by sync byte
    Serial.print("["); Serial.print(millis()); Serial.print("] "); Serial.print("NEW BYTE: ");
    Serial.println(__newByte,BIN);
    if (__setModeTriggered) { // Here only reads 0 since action is determined by the sync byte
      //rxBuf[buf_cnt] = __newByte; 
      TIME2READ = true;
      Serial.println("2");
    } else if (__getModeTriggered) {
      //rxBuf[buf_cnt] = __newByte;
      SPI0->SPI_TDR = (uint8_t) txBuf[buf_cnt-1];
      TIME2READ = true;
      Serial.println("3");
    }
    else{ // wrong Mode request
      Serial.println("X");
      TIME2READ = true;
    }
    Serial.println(buf_cnt);
    buf_cnt--;
    if (buf_cnt == 0) {
      __transferState = WAIT4INFO;
    } else {
      __transferState = WAIT4DATA; // remains unchanged
    }
  }
  else if (__infoByteReceived) {
    Serial.println(__infoByte,BIN);
    SPI0->SPI_TDR = (uint8_t) DONE; 
    TIME2READ = true;
    __transferState = WAIT4SYNC;
    transferCompleted = true;
    Serial.println("4");
  }

  // ACTION AFTER TRANSFER SECTION
  if (transferCompleted) {
    if (__setModeTriggered) { // Again action after transfer depends on the Mode
      // Sets LED ON-OFF and changes the current state
      setModeAction();
      Serial.println("5");
    } else if (__getModeTriggered) {
      // Already responded with value during transfer...
      // Prints the value sent to master, so merges last txBuf to float and prints it
      TxResponse = merge_bytes_to_float(txBuf);
      Serial.print("["); Serial.print(millis()); Serial.print("] Slave sent: "); Serial.println(TxResponse,4);
      Serial.println("6");
    }
    transferCompleted = false; // resets flag
    Serial.println("7");
  }

  delayMicroseconds(10);
}
