#if( WIRINGPI )

#include <I2C.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>

using namespace i2c;

#include <wiringPi.h>
#include <wiringPiI2C.h>


#define ENABLE_I2C_DEBUG 1

// Pass in this option to enable error counting
#ifdef ENABLE_I2C_DEBUG
        #define HANDLE_RESULT( result ) ( m_statistics.AddResult( result ) )
#else
        #define HANDLE_RESULT( result ) ( result )
#endif

I2C::I2C()
{
        m_customProperties.m_fileDescriptor = open("/dev/i2c-1", O_RDWR);
        m_customProperties.m_issuedTransactionUnsupportedWarning = false;
}

// Enable is a noop on this platform, but preserve enable/disable logic
// to avoid a portability error.
EI2CResult I2C::Enable()
{
        if( m_isEnabled )
        {
                // Already enabled
                return HANDLE_RESULT( EI2CResult::RESULT_ERR_ALREADY_INITIALIZED );
        }

       // Mark as enabled
        m_isEnabled = true;

        // Success
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

EI2CResult I2C::Disable()
{
        // Disable the interface
        m_isEnabled = false;

        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

// Noop.
EI2CResult I2C::Reset()
{
        printf( "I2C::Reset() is not supported.\n" );
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

bool I2C::LockAddress( uint8_t addressIn )
{
        // Validate address
        if( addressIn >= 127 )
        {
                return false;
        }

        if( m_addressLocks[ addressIn ] == 1 )
        {
                // Address already locked
                return false;
        }
        else
        {
                // Lock address
                m_addressLocks[ addressIn ] = 1;
                return true;
        }
}

void I2C::FreeAddress( uint8_t addressIn )
{
        // Validate address
        if( addressIn >= 127 )
        {
                return;
        }

        // Free the lock
        m_addressLocks[ addressIn ] = 0;
}

bool I2C::IsAvailable()
{
        return m_isEnabled;
}

EI2CResult I2C::SetBaudRate( EI2CBaudRate baudRateIn )
{
        printf( "I2C::SetBaudRate() is not supported. Use modprobe instead.\n" );
        return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED );
}

// -----------------------
// Transaction Methods
// -----------------------

EI2CResult I2C::WriteByte( uint8_t slaveAddressIn, uint8_t dataIn, bool issueRepeatedStart )
{
        if ( ioctl( m_customProperties.m_fileDescriptor, I2C_SLAVE, slaveAddressIn ) < 0 )
        {
                printf( "Unable to select I2C device: %s\n", strerror( errno ) );
                return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED );
        }
        int result = wiringPiI2CWrite( m_customProperties.m_fileDescriptor, dataIn );
        if (result < 0)
        {
                printf( "Unable to write I2C device: %s\n", strerror(errno) );
                return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED );
        }
        // Send stop, if requested
        if( !issueRepeatedStart )
        {
                m_result = StopTransaction( );
                if( m_result ){ return HANDLE_RESULT( m_result ); }
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

EI2CResult I2C::StartTransaction()
{
        if (!m_customProperties.m_issuedTransactionUnsupportedWarning)
        {
                printf( "StartTransaction() is not supported.\n" );
                m_customProperties.m_issuedTransactionUnsupportedWarning = true;
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

EI2CResult I2C::StopTransaction()
{
        if (!m_customProperties.m_issuedTransactionUnsupportedWarning)
        {
                printf( "StopTransaction() is not supported.\n" );
                m_customProperties.m_issuedTransactionUnsupportedWarning = true;
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

EI2CResult I2C::WriteRegisterByte( uint8_t slaveAddressIn, uint8_t registerIn, uint8_t dataIn, bool issueRepeatedStart )
{
        // Start transaction
        m_result = StartTransaction();
        if( m_result ){ return HANDLE_RESULT( m_result ); }

        // Send address
        if ( ioctl( m_customProperties.m_fileDescriptor, I2C_SLAVE, slaveAddressIn ) < 0 )
        {
                return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED );
        }

        // Write Register Address
        int result = wiringPiI2CWriteReg8( m_customProperties.m_fileDescriptor, registerIn, dataIn );
        if( result ){ return HANDLE_RESULT( m_result ); }

        // Send stop, if requested
        if( !issueRepeatedStart )
        {
                m_result = StopTransaction();
                if( m_result ){ return HANDLE_RESULT( m_result ); }
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

EI2CResult I2C::WriteRegisterWord( uint8_t slaveAddressIn, uint8_t registerIn, uint16_t dataIn, bool issueRepeatedStart )
{
        // Start transaction
        m_result = StartTransaction();
        if( m_result ){ return HANDLE_RESULT( m_result ); }

        // Send address
        if ( ioctl( m_customProperties.m_fileDescriptor, I2C_SLAVE, slaveAddressIn ) < 0 )
        {
                return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED );
        }

        // Write Register Address
        int result = wiringPiI2CWriteReg16( m_customProperties.m_fileDescriptor, registerIn, dataIn );
        if( result ){ return HANDLE_RESULT( m_result ); }

        // Send stop, if requested
        if( !issueRepeatedStart )
        {
                m_result = StopTransaction();
                if( m_result ){ return HANDLE_RESULT( m_result ); }
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

EI2CResult I2C::WriteBytes( uint8_t slaveAddressIn, uint8_t *dataIn, uint8_t numberBytesIn,  bool issueRepeatedStart )
{
        // Start transaction
        m_result = StartTransaction();
        if( m_result ){ return HANDLE_RESULT( m_result ); }

        // Send address
        int result = ioctl( m_customProperties.m_fileDescriptor, I2C_SLAVE, slaveAddressIn );
        if( result ){ return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED ); }

        // Write multiple bytes
        for( size_t i = 0; i < numberBytesIn; ++i )
        {
                int result = wiringPiI2CWrite( m_customProperties.m_fileDescriptor, dataIn[ i ] );
                if( result ){ return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED ); }
        }

        // Send stop, if requested
        if( !issueRepeatedStart )
        {
                m_result = StopTransaction();
                if( m_result ){ return HANDLE_RESULT( m_result ); }
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

EI2CResult I2C::WriteRegisterBytes( uint8_t slaveAddressIn, uint8_t registerIn, uint8_t *dataIn, uint8_t numberBytesIn, bool issueRepeatedStart )
{
        // Start transaction
        m_result = StartTransaction();
        if( m_result ){ return HANDLE_RESULT( m_result ); }

        // Send address
        int result = ioctl( m_customProperties.m_fileDescriptor, I2C_SLAVE, slaveAddressIn );
        if( result ){ return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED ); }

        // Write Register Address
        result = wiringPiI2CWrite( m_customProperties.m_fileDescriptor, registerIn );
        if( result ){ return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED ); }

        // Write multiple bytes
        for( size_t i = 0; i < numberBytesIn; ++i )
        {
                int result = wiringPiI2CWrite( m_customProperties.m_fileDescriptor, dataIn [ i ] );
                if( result ){ return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED ); }
        }

        // Send stop, if requested
        if( !issueRepeatedStart )
        {
                m_result = StopTransaction();
                if( m_result ){ return HANDLE_RESULT( m_result ); }
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

// Read operations
EI2CResult I2C::ReadRegisterByte( uint8_t slaveAddressIn, uint8_t registerIn, uint8_t *dataOut, bool issueRepeatedStart )
{
        // Start transaction
        m_result = StartTransaction();
        if( m_result ){ return HANDLE_RESULT( m_result ); }

        // Send address
        int result = ioctl( m_customProperties.m_fileDescriptor, I2C_SLAVE, slaveAddressIn );
        if( result ){ return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED ); }

        *dataOut = wiringPiI2CReadReg8( m_customProperties.m_fileDescriptor, registerIn );

        // Send stop, if requested
        if( !issueRepeatedStart )
        {
                m_result = StopTransaction();
                if( m_result ){ return HANDLE_RESULT( m_result ); }
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

// Read operations
EI2CResult I2C::ReadRegisterWord( uint8_t slaveAddressIn, uint8_t registerIn, uint16_t *dataOut, bool issueRepeatedStart )
{
        // Start transaction
        m_result = StartTransaction();
        if( m_result ){ return HANDLE_RESULT( m_result ); }

        // Send address
        int result = ioctl( m_customProperties.m_fileDescriptor, I2C_SLAVE, slaveAddressIn );
        if( result ){ return HANDLE_RESULT( EI2CResult::RESULT_ERR_FAILED ); }

        *dataOut = wiringPiI2CReadReg16( m_customProperties.m_fileDescriptor, registerIn );

        // Send stop, if requested
        if( !issueRepeatedStart )
        {
                m_result = StopTransaction();
                if( m_result ){ return HANDLE_RESULT( m_result ); }
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

EI2CResult I2C::ReadRegisterBytes( uint8_t slaveAddressIn, uint8_t registerIn, uint8_t *dataOut, uint8_t numberBytesIn, bool issueRepeatedStart )
{
        // Start write transaction
        m_result = StartTransaction();
        if( m_result ){ return HANDLE_RESULT( m_result ); }

        // Read in the requested amount of bytes
        for( size_t i = 0; i < numberBytesIn; ++i )
        {
                m_result = ReadRegisterByte( slaveAddressIn, registerIn, dataOut + i , issueRepeatedStart );
                if ( m_result ){ return HANDLE_RESULT( m_result ); }
        }

        // Send stop, if requested
        if( !issueRepeatedStart )
        {
                m_result = StopTransaction();
                if( m_result ){ return HANDLE_RESULT( m_result ); }
        }
        return HANDLE_RESULT( EI2CResult::RESULT_SUCCESS );
}

// -------------------------------------------------------
// Instantiation - Only support one I2C interface

I2C I2C0;

#endif
