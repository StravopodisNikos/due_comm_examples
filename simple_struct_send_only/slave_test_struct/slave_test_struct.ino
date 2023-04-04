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

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
