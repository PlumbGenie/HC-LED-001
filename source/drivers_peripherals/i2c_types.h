/* 
 * File:   i2c_types.h
 * Author: c10948
 *
 * Created on July 1, 2015, 4:47 PM
 */

#ifndef I2C_TYPES_H
#define	I2C_TYPES_H


typedef enum {
    I2C_WRITE_COLLISION,
    I2C_MESSAGE_FAIL,
    I2C_MESSAGE_PENDING,
    I2C_STUCK_START,
    I2C_ADDRESS_NO_ACK,
    I2C_DATA_NO_ACK,
    I2C_LOST_STATE,
    I2C_BUSY,
    I2C_NOERR
} i2c_error_t;

typedef uint8_t i2c_address_t;

#endif	/* I2C_TYPES_H */

