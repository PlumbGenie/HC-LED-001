/* 
 * File:   at30ts74_temperature_sensor.h
 * Author: C16490
 *
 * Created on May 5, 2017, 1:26 PM
 */

#ifndef AT30TS74_TEMPERATURE_SENSOR_H
#define	AT30TS74_TEMPERATURE_SENSOR_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

uint8_t AT30TS74_Initialize(void);

int16_t AT30TS74_ReadTemperature(void);

int16_t ConvertTemperatureToFahrenheit(int16_t temperatureC);


#ifdef	__cplusplus
}
#endif

#endif	/* AT30TS74_TEMPERATURE_SENSOR_H */

