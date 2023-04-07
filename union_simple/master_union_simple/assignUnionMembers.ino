void assignUnionMembers(PacketUnion & packet, uint8_t sync_value, float data_value)
{
  packet.bytes[0] = sync_value;
  byte* data_ptr = (byte*)&data_value;
  for (int i = 0; i < sizeof(float); i++) {
    packet.bytes[i+1] = *(data_ptr + i);
  }
  return;
}
