#pragma once

#ifndef CSerial_h
#define CSerial_h

#include <Print.h>

class CustomSerial: public Print {
    using Print::Print;
    public:
        size_t write(uint8_t c) override;
        size_t write(const uint8_t *buffer, size_t size) override;
        // virtual size_t write(uint8_t val);
        //virtual size_t write(const char *str);
        // virtual size_t write(const uint8_t *buffer, size_t size);
        void begin();
        // virtual size_t write(uint8_t c);
        // virtual size_t write(const uint8_t *buffer, size_t size);
        String readString();
        String getSerialInput();
};

extern CustomSerial CSerial;
#endif