#include <stdint.h>
#include "adc.h"

uint8_t ReadLightSensor(void)
{
    uint8_t result;
    
    result = ADC_GetConversion(channel_AN4);
    
    return result;
}
