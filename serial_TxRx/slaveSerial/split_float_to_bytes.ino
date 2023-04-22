void split_float_to_bytes(float f, uint8_t* bytes)
{
    // Cast the float pointer to uint32_t to access individual bytes
    uint32_t* p = reinterpret_cast<uint32_t*>(&f);

    // Copy the individual bytes to the output array
    bytes[0] = static_cast<uint8_t>(*p);
    bytes[1] = static_cast<uint8_t>(*p >> 8);
    bytes[2] = static_cast<uint8_t>(*p >> 16);
    bytes[3] = static_cast<uint8_t>(*p >> 24);
}
