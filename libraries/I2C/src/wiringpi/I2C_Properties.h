#pragma once

// Was following the pattern of ../samd/I2C_Properties.h, but noticed none of the names
// defined there are referenced elsewhere, so why bother with an extra layer of namespace?
namespace wiringpi
{
    struct TI2CProperties
    {
       int    m_fileDescriptor;
       bool   m_issuedTransactionUnsupportedWarning;
    };
}
