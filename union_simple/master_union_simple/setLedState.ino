void setLedState(uint8_t cs_pin, PacketUnion setCmd)
{
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  //for (int i=0; i<SIZEOF_PACKET; i++) {
  //  Serial.print("Sent byte["); Serial.print(i);Serial.print("]:");
  //  Serial.println(setCmd.bytes[i],BIN);
  //}
  time_start_micros = micros();
  SPI.transfer(setCmd.bytes, SIZEOF_PACKET);
  Serial.print("Transfer duration [us]: "); Serial.println(micros()-time_start_micros);
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();  
  return;
}
