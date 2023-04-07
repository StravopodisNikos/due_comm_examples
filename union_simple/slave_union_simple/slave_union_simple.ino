#include <Arduino.h>
#include <SPI.h>
#include <sam.h>
/*
 * ***********************************************************
 * ********************       SLAVE        *******************
 * ***********************************************************
 * Simple TEST code for Ovidius SMM Project. Arduino DUE is con-
 * figured as Master for SPI Communication. Summary:
 * 
 * MASTER (DUE1) sends a byte chain of 5-byte length. The first byte
 * is used as sync byte. It tells the slave that the next 4bytes that
 * are sent form a float value that should be handled in a specific way.
 * Here: The value should be checked and the specific led shoulb be on
 * and a new float should be returned to master. A struct is used to
 * define the transfer packet. 
 * SLAVE (DUE) Knows beforehand the length of the Packet. So checks that
 * all 5 bytes are received. Checks the first and responds as previously
 * stated. 
 *  
 * [7-4-23] Author: Stravopodis Nikos
 */
#define SERIAL_BAUD1      250000
#define CSPIN_EXT         10
#define SPI0_MISO         25
#define SPI0_MOSI         26
#define SPI0_SPCK         27
#define SPI0_NPCS0        10
#define CMD_GET_STATE     85
#define CMD_GET_STATE_F   17.1796f
#define CMD_LED_ON        95
#define CMD_LED_OFF       105
#define CMD_LED_ON_F      14.3785f // 0x41660e56 [HEX] 01000001/01100110/00001110/01010110 [BIN]
#define CMD_LED_OFF_F     96.7854f // 0x42c191aa [HEX] 01000010/11000001/10010010/00100000 [BIN]
#define STATE_LED_ON      55
#define STATE_LED_OFF     65
#define STATE_LED_ON_F      64.7836f // 0x42819134 [HEX] 01000010/10000001/10010001/00110100 [BIN]
#define STATE_LED_OFF_F     74.2418f // 0x42947bcd [HEX] 01000010/10010100/01111011/11001101 [BIN]
#define OUT_OF_BOUNDS     39.4719f // 0x421de33a [HEX] 01000010/00011101/11100011/00111010 [BIN]
#define RESPONSE_TIMEOUT  50 // [ms]
#define SIZEOF_PACKET     5
#define LEDpin            4

struct Packet {
  uint8_t sync;
  float data;
};
union PacketUnion {
  Packet packet;
  uint8_t bytes[SIZEOF_PACKET];
};

unsigned long set_time_millis;

// DEFINE ALL VOLATILES
//***********************
volatile PacketUnion __slave_packet;
volatile bool __PacketReceived            = false; // All bytes of the union have arrived
volatile bool __receivedNewByte           = false; // 1 new byte read from RDR
volatile bool __transferStillInProgress   = false; // still waiting for bytes
volatile uint8_t __newByte;
volatile bool   __breakTransfer           = false; // Demands to stop reading because corrupted data occured
volatile bool __setModeTriggered          = false;
volatile bool __getModeTriggered          = false;
volatile bool __waitSyncByte              = true;  // First time demands 1st byte to sync
volatile bool __wrongModeRequested = false;
//***********************

void setup() 
{
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
  ChangeLEDstate2ON(); delay(500); ChangeLEDstate2OFF(); delay(500);
  
  Serial.print("SLAVE STARTED "); printTimeNow();
  __waitSyncByte = true;
}

void SPI0_Handler() {
  static int cnt = 0;
  __breakTransfer = false;
  //Serial.println("1");
  // check if Overrun(SPI_SR_OVRES) error has occured. Abandon Transfer!
  if (SPI_SR_OVRES == HIGH) {
    __breakTransfer = true;
    //Serial.println("2");
  }
  if ( (__waitSyncByte) && (!__breakTransfer) )
  {
    //Serial.println("3");
    while (!SPI_SR_RDRF);
    __newByte = SPI0->SPI_RDR;
    //__slave_packet.packet.sync = __newByte;
    __slave_packet.bytes[cnt] = __newByte;
    Serial.print("bytes[0]-1="); Serial.println(__slave_packet.bytes[cnt]);
    //Serial.print("__newByte-1="); Serial.println(__newByte);
    if (__newByte == (uint8_t) CMD_GET_STATE) {
      Serial.print("bytes[0]-2="); Serial.println(__slave_packet.bytes[cnt]);
      //Serial.print("__newByte-2="); Serial.println(__newByte);
      __getModeTriggered = true;
      __setModeTriggered = false;
      __transferStillInProgress = true;
      __waitSyncByte = false;
      Serial.println("4");
    }
    else if ( (__newByte == (uint8_t) CMD_LED_ON) || (__newByte == (uint8_t) CMD_LED_OFF) ) {
      Serial.print("bytes[0]-3="); Serial.println(__slave_packet.bytes[cnt]);
      //Serial.print("__newByte="); Serial.println(__newByte);
      __getModeTriggered = false;
      __setModeTriggered = true;
      __transferStillInProgress = true;
      __waitSyncByte = false;
      Serial.println("5");
    }
    else {
      __transferStillInProgress = false;
      __waitSyncByte = true;
      __wrongModeRequested = true;
      SPI0->SPI_RDR;
      Serial.println("6");
    }
  }
  if ( (__transferStillInProgress) && (!__breakTransfer)  ){
    Serial.println("7");
    Serial.print("cnt="); Serial.println(cnt);
    cnt++;
    while (!SPI_SR_RDRF);
    __slave_packet.bytes[cnt] = SPI0->SPI_RDR;
    if (cnt > SIZEOF_PACKET) {
      cnt = 0;
      __transferStillInProgress = false;
      __waitSyncByte = true;
      SPI0->SPI_RDR;
    }
  }
  
}

void loop() {
  if (__setModeTriggered && (!__transferStillInProgress)) {
    setModeSlave();
    __setModeTriggered = false;
    __waitSyncByte = true;    
  }

  if (__wrongModeRequested) {
    __wrongModeRequested = false;
  }
  
  delayMicroseconds(50);
}
