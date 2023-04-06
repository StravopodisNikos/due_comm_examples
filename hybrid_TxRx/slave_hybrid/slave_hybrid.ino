#include <SPI.h>

#define SLAVE_SELECT_PIN 10

void setup() {
  // Set up SPI pins
  pinMode(SLAVE_SELECT_PIN, OUTPUT);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
  
  // Set up SPI as slave
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
}

void loop() {
  if (SPI0->SPI_SR & SPI_SR_SPIENS) {
    // Check if SPI is enabled
    if (SPI0->SPI_SR & SPI_SR_RDRF) {
      // Check if data is received
      uint8_t byte = SPI0->SPI_RDR; // Read received byte
      float response = byte / 2.0; // Calculate response
      
      // Send response back to master
      SPI0->SPI_TDR = *((uint32_t*)(&response)); // Write response to transmit data register
      while (!(SPI0->SPI_SR & SPI_SR_TDRE)); // Wait until response is sent
    }
  }
}
