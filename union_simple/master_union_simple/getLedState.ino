void getLedState(uint8_t cs_pin, PacketUnion & getRes)
{
  SPI.beginTransaction(spi_setting);
  digitalWrite(cs_pin, LOW);
  time_start_micros = micros();
  SPI.transfer(getRes.bytes, SIZEOF_PACKET);
  Serial.print("Transfer duration [us]: "); Serial.println(micros()-time_start_micros);  
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();   
  return;
}
