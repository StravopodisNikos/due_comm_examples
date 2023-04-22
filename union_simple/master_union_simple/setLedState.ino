void setLedState(uint8_t cs_pin, PacketUnion& setCmd)
{
  uint8_t response;
  
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  //for (int i=0; i<SIZEOF_PACKET; i++) {
  //  Serial.print("Sent byte["); Serial.print(i);Serial.print("]:");
  //  Serial.println(setCmd.bytes[i],BIN);
  //}
  time_start_micros = micros();
  Serial.print("Master set  [  CMD_ID]: "); Serial.println(setCmd.bytes[0],BIN);
  do {
    response = SPI.transfer(95);delay(10);
    Serial.println(response,BIN);
  } while( !(response == (uint8_t) CMD_RECEIVED) );
  response = SPI.transfer(setCmd.bytes[1]); delayMicroseconds(100);
  response = SPI.transfer(setCmd.bytes[2]); delayMicroseconds(100);
  response = SPI.transfer(setCmd.bytes[3]); delayMicroseconds(100);
  response = SPI.transfer(setCmd.bytes[4]); delayMicroseconds(100);
  Serial.print("Transfer duration [us]: "); Serial.println(micros()-time_start_micros);
  Serial.print("Slave  gave [STATE_ID]: "); Serial.println(setCmd.bytes[0],BIN);
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();  
  return;
}
