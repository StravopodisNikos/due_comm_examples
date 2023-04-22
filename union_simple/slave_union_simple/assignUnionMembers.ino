void assignUnionMembers(uint8_t sync_value, float data_value)
{
  __slave_packet.bytes[0] = sync_value;
  byte* data_ptr = (byte*)&data_value;
  for (int i = 0; i < sizeof(float); i++) {
    __slave_packet.bytes[i+1] = *(data_ptr + i);
  }
  return;
}
