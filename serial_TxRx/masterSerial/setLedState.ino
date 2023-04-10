void setLedState(uint8_t cs_pin, uint8_t cmd) {
  uint8_t response;
  // Split the decoy value, we sent the decoy since no response is to be sent back 
  split_float_to_bytes((float) DECOY_F, txBuf);
  Serial.println("SET LED PACKAGE: ");
  Serial.println(txBuf[0],BIN);
  Serial.println(txBuf[1],BIN);
  Serial.println(txBuf[2],BIN);
  Serial.println(txBuf[3],BIN);
  time_start_micros = micros();
  
  // TRANSFER SECTION
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  Serial.print("Master set  [  CMD_ID]: "); Serial.println(cmd,BIN);
  // 1. send the sync byte and wait until SYNCED
  //do {
    response = SPI.transfer(cmd); delayMicroseconds(50); Serial.print("Slave responded:"); Serial.println(response,BIN);
  //} while (!(response == (uint8_t) SYNCED));
  // 2. send the decoy packets
  SPI.transfer(txBuf[0]); delayMicroseconds(50);
  SPI.transfer(txBuf[1]); delayMicroseconds(50);
  SPI.transfer(txBuf[2]); delayMicroseconds(50);
  SPI.transfer(txBuf[3]); delayMicroseconds(50);
  // 3. Send the info byte
  //do {
    response = SPI.transfer(0); delayMicroseconds(50); Serial.print("Slave responded:"); Serial.println(response,BIN);
  //} while (!(response == (uint8_t) DONE));
  Serial.print("Transfer duration [us]: "); Serial.println(micros()-time_start_micros);
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();  
  return;
}
