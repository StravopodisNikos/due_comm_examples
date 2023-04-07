void setModeSlave() {
  if (__slave_packet.bytes[0] == (uint8_t ) CMD_LED_ON) {
    ChangeLEDstate2ON();
    Serial.print("LED STATE       : [ON]"); printTimeNow();
    __slave_packet.packet.data = STATE_LED_ON_F;
  } else if (__slave_packet.bytes[0] == (uint8_t )CMD_LED_OFF){
    ChangeLEDstate2OFF();
    Serial.print("LED STATE       : [OFF]"); printTimeNow();
    __slave_packet.packet.data = STATE_LED_OFF_F;    
  }
}
