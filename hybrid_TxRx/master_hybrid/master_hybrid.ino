#include <SPI.h>

#define SLAVE_SELECT_PIN 10

typedef union {
  uint8_t cmd_bytes[1];
  float cmd_f;
} cmd_t;
cmd_t cmd_data;

float response_f;

SPISettings spi_setting(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE2);

void setup() {
  Serial.begin(250000);
  SPI.begin();
  pinMode(SLAVE_SELECT_PIN, OUTPUT);
  digitalWrite(SLAVE_SELECT_PIN, HIGH); // Deselect slave at startup
}

void loop() {
  cmd_data.cmd_bytes[0] = 0x0A; // Send command to slave
  sendReadCmd2SlaveWait(SLAVE_SELECT_PIN, cmd_data, response_f);
  Serial.print("Response from slave: ");
  Serial.println(response_f);
  delay(1000);
}

void sendReadCmd2SlaveWait(uint8_t cs_pin, cmd_t  cmd, float &response_f) {
  bool response_received = false;
  uint32_t timeout = millis() + 1000; // Timeout after 1 second
  
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);

  // Send command byte to slave
  while (!(SPI0->SPI_SR & SPI_SR_TDRE)); // Wait for transmit buffer to be empty
  SPI0->SPI_TDR = cmd.cmd_bytes[0];
  
  // Wait for response from slave
  while (millis() < timeout) {
    if (SPI0->SPI_SR & SPI_SR_RDRF) {
      union { uint8_t response_bytes[4]; float response_f; } response;
      response.response_bytes[0] = SPI0->SPI_RDR;
      response.response_bytes[1] = SPI0->SPI_RDR;
      response.response_bytes[2] = SPI0->SPI_RDR;
      response.response_bytes[3] = SPI0->SPI_RDR;
      response_f = response.response_f;
      response_received = true;
      break;
    }
  }

  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();

  if (response_received) {
    Serial.println("Valid slave response received!");
    Serial.println(response_f);
  } else {
    Serial.println("Timeout waiting for response from slave!");
  }
}
