#include <stdint.h>
#include "at30ts74_temperature_sensor.h"
#include "m_i2c.h"

// i2c address for at30ts74
#define AT30TS74_ADDRESS    0x48

// Registers for at30ts74
#define AT30TS74_READ_TEMP  0x00
#define AT30TS74_CONFIG     0x01
#define AT30TS74_TEMP_LOW   0x02
#define AT30TS74_TEMP_HIGH  0x03

// Other
#define CONVERSION_9BIT     0x18    // Set the resolution in Config register
#define TEMP_LOW_UPPER      0xC9    // Upper and lower bytes
#define TEMP_LOW_LOWER      0x00    //   set low temp limit to -55 degrees C
#define TEMP_HIGH_UPPER     0x7D    // Upper and lower bytes
#define TEMP_HIGH_LOWER     0x00    //   set high temp limit to +125 degrees C
#define TEMP_READ_MAX_BYTES    2    // Max number of bytes for temp reading
#define CONFIG_MAX_BYTES       2    // Number of bytes to configure device
#define TEMP_SET_MAX_BYTES     3    // Number of bytes to set temp limits

// Command arrays
uint8_t configCmd[]        = {AT30TS74_CONFIG, CONVERSION_9BIT};
uint8_t setLowTempLimit[]  = {AT30TS74_TEMP_LOW, TEMP_LOW_UPPER, TEMP_LOW_LOWER};
uint8_t setHighTempLimit[] = {AT30TS74_TEMP_HIGH, TEMP_HIGH_UPPER, TEMP_HIGH_LOWER};
uint8_t setToReadTemp[]    = {AT30TS74_READ_TEMP};

uint8_t AT30TS74_Initialize(void)
{
    uint8_t returnCode;;
    
    // Bit banged init I2C
    i2cInit();
    
    // Setup pointer register to configure device and
    // send configuration: Norm op; 9 bit; 6 fault; low fault;Comp mode; Active
    returnCode = i2cWrite(AT30TS74_ADDRESS, configCmd, CONFIG_MAX_BYTES);
    
    // Setup pointer register to temp low limit and
    // set to -55 max low limit
    returnCode = i2cWrite(AT30TS74_ADDRESS, setLowTempLimit, TEMP_SET_MAX_BYTES);
    
    // Setup pointer register to temp high limit and
    // set to +125 max high limit
    returnCode = i2cWrite(AT30TS74_ADDRESS, setHighTempLimit, TEMP_SET_MAX_BYTES);
    
    // Setup pointer register to read from temperature register
    returnCode = i2cWrite(AT30TS74_ADDRESS, setToReadTemp, 1);
    return returnCode;      //jira: CAE_MCU8-5647
}

int16_t AT30TS74_ReadTemperature(void)
{
    uint8_t readTemp[TEMP_READ_MAX_BYTES];
    uint8_t returnCode;
    int16_t combined;
    
    // Read from temperature register
    returnCode = i2cRead(AT30TS74_ADDRESS, readTemp, TEMP_READ_MAX_BYTES);
    
    combined = (readTemp[0] << 8) + readTemp[1];
//    combined = (int16_t)combined * 0.00390625;
    combined = combined >> 8 ;        // 1/0.00390625=256, shifting the value back and forth is to get the sign of negative value.  jira:CAE_MCU8-5647
    if(combined <= 0)
    {
        combined = 0;
    }
    
    // Return degrees C in signed int16 format (MSB is sign, the rest is full integer)
    return combined;
}

int16_t ConvertTemperatureToFahrenheit(int16_t temperatureC)
{
    int16_t fahrenheit;
    
    // Take in an signed int16 format(MSB is sign, the rest is full integer)
    // and convert to fahrenheit
    fahrenheit = (int16_t)(temperatureC * 1.8) + 32;            //jira: CAE_MCU8-5647
    
    // return degrees F in signed int16 format (MSB is sign, the rest is full integer)
    return fahrenheit;
}
