/* 
 * File:   light_sensor.h
 * Author: C16490
 *
 * Created on May 5, 2017, 1:04 PM
 */

#ifndef LIGHT_SENSOR_H
#define	LIGHT_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

// return 0 if conversion is complete and result is ready, or 1 for conversion in progress
uint8_t ReadLightSensor(void); 


#ifdef	__cplusplus
}
#endif

#endif	/* LIGHT_SENSOR_H */

