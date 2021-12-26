#ifndef __BYTEORDER_HPP__
#define __BYTEORDER_HPP__

enum BYTE_ORDER {
    ELITTLE_ENDIAN,
    EBIG_ENDIAN
};

// Reference: https://stackoverflow.com/questions/1001307/detecting-endianness-programmatically-in-a-c-program
inline BYTE_ORDER get_byte_order() {
    const int value = 1;
    const void * address = static_cast<const void *>(&value);
    const unsigned char * least_significant_address = static_cast<const unsigned char *>(address);
    if (*least_significant_address == 0x01) {
        return ELITTLE_ENDIAN;
    } else {
        return EBIG_ENDIAN;
    }
}

#endif // __BYTEORDER_HPP__
