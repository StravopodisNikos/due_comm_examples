float merge_bytes_to_float(const uint8_t* bytes)
/*
{
    union {
        uint32_t i;
        float f;
    } u;
    
    u.i = (static_cast<uint32_t>(bytes[0]) << 24) |
          (static_cast<uint32_t>(bytes[1]) << 16) |
          (static_cast<uint32_t>(bytes[2]) << 8) |
          static_cast<uint32_t>(bytes[3]);
    
    return u.f;
} */
{
    // Create a uint32_t by merging the individual bytes in the array
    uint32_t value = static_cast<uint32_t>(bytes[0]) |
                     static_cast<uint32_t>(bytes[1]) << 8 |
                     static_cast<uint32_t>(bytes[2]) << 16 |
                     static_cast<uint32_t>(bytes[3]) << 24;

    // Cast the uint32_t pointer to a float pointer
    return *reinterpret_cast<float*>(&value);
}
