void setModeAction() {
  if (__syncByte == CMD_LED_ON) {
    digitalWrite(LEDpin, HIGH);
    split_float_to_bytes((float) STATE_LED_ON_F, txBuf); //sets the transmit buffer when get Mode is executed
  } else if (__syncByte == CMD_LED_OFF) {
    digitalWrite(LEDpin, LOW);
    split_float_to_bytes((float) STATE_LED_OFF_F, txBuf);
  } else {
  }
  return;
}
