void printState(PacketUnion res_packet)
{

  if (res_packet.packet.sync == (uint8_t) STATE_LED_ON)
  {
    Serial.print("LED STATE       : [ON]"); printTimeNow();
    Serial.print("STATE_F RECEIVED:"); Serial.print(res_packet.packet.data); printTimeNow();
  }
  else if (res_packet.packet.sync == (uint8_t) STATE_LED_OFF)
  {
    Serial.print("LED STATE       : [OFF]"); printTimeNow();
    Serial.print("STATE_F RECEIVED:"); Serial.print(res_packet.packet.data); printTimeNow();
  }   
  else
  {
    Serial.print("LED STATE       : [XXX]"); printTimeNow();
    Serial.print("STATE_F RECEIVED:"); Serial.print(res_packet.packet.data); printTimeNow();    
  }
  return;
}
