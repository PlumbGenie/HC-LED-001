#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "BME280_Defs.h"
#include "BME280_Driver.h"
#include "mssp_spi1_master.h"
#include "hardware.h"
#include "log.h"

char tmp_data[20];
bme280_calibration_param cal_param;
long adc_t, adc_p, adc_h, t_fine;

long Pressure;
unsigned long Temperature;
unsigned int Humidity;

void BME280_DeviceInit()
{
    /**MSSP INIT is done using MCC*/
    BME280_SCK1_TRIS = 0;               //BME280 SCK1 - o/p   
    BME280_SDI1_TRIS = 1;               //BME280 SDI1 - i/p    
    BME280_SDO1_TRIS = 0;               //BME280 SDO1 - o/p  
    
    /**To be done by user*/
    BME280_CS_TRSIS = 0;                //BME280 CS - o/p
    
    /**Calibration init**/
    BME280_OversamplingInit();
    
    while(BME280_IsMeasuring());
    BME280_ReadMeasurements();
    
    /**Oversampling init**/
    BME280_INIT();
}

uint8_t readBME280(uint8_t addr)
{
    uint8_t ret;
    
    BME280_CS_LOW();
    
    BME280_SPI_WRITE(addr);    
    ret = BME280_SPI_READ();
    
    BME280_CS_HIGH();
    return ret;
}

void readBlockBME280(uint8_t addr, uint8_t *data, uint8_t len)
{    
    
    BME280_CS_LOW();
    
    BME280_SPI_WRITE(addr);   
    while(len--)
    {
        *data++ = BME280_SPI_READ();
    }
    
    BME280_CS_HIGH();
   
}

void writeBME280(uint8_t addr, uint8_t data)
{
    BME280_CS_LOW();
    
    BME280_SPI_WRITE(addr & WRITE_MASK);    
    BME280_SPI_WRITE(data);    
    
    BME280_CS_HIGH();
}




void BME280_ReadMeasurements() {
  tmp_data[0] = BME280_PRESSURE_MSB_REG;
  memset(tmp_data,0,sizeof(tmp_data));
readBlockBME280(BME280_PRESSURE_MSB_REG,tmp_data,8);
  adc_h = tmp_data[BME280_DATA_FRAME_HUMIDITY_LSB_BYTE];
  adc_h |= (unsigned long)tmp_data[BME280_DATA_FRAME_HUMIDITY_MSB_BYTE] << 8;

  adc_t  = (unsigned long)tmp_data[BME280_DATA_FRAME_TEMPERATURE_XLSB_BYTE] >> 4;
  adc_t |= (unsigned long)tmp_data[BME280_DATA_FRAME_TEMPERATURE_LSB_BYTE] << 4;
  adc_t |= (unsigned long)tmp_data[BME280_DATA_FRAME_TEMPERATURE_MSB_BYTE] << 12;

  adc_p  = (unsigned long)tmp_data[BME280_DATA_FRAME_PRESSURE_XLSB_BYTE] >> 4;
  adc_p |= (unsigned long)tmp_data[BME280_DATA_FRAME_PRESSURE_LSB_BYTE] << 4;
  adc_p |= (unsigned long)tmp_data[BME280_DATA_FRAME_PRESSURE_MSB_BYTE] << 12;
}


char BME280_GetID() {
 return readBME280(BME280_CHIP_ID_REG);
}

void BME280_SoftReset() {
  writeBME280(BME280_RST_REG, BME280_SOFT_RESET);
}

char BME280_GetStatus() {
  return readBME280(BME280_STAT_REG);
}

char BME280_GetCtrlMeasurement() {
  return readBME280(BME280_CTRL_MEAS_REG);
}

char BME280_GetCtrlHumidity() {
  return readBME280(BME280_CTRL_HUMIDITY_REG);
}

char BME280_GetConfig() {
  return readBME280(BME280_CONFIG_REG);
}


//Read factory calibration parameters
void BME280_ReadCalibrationParams() {
   char lsb, msb;
   

   msb = readBME280(BME280_TEMPERATURE_CALIB_DIG_T1_MSB_REG);
   cal_param.dig_T1 = (unsigned int) msb;
   lsb = readBME280(BME280_TEMPERATURE_CALIB_DIG_T1_LSB_REG);
   cal_param.dig_T1 = (cal_param.dig_T1 << 8) + lsb;
   

   msb = readBME280(BME280_TEMPERATURE_CALIB_DIG_T2_MSB_REG);
   cal_param.dig_T2 = (int) msb;
   lsb = readBME280(BME280_TEMPERATURE_CALIB_DIG_T2_LSB_REG);
   cal_param.dig_T2 = (cal_param.dig_T2 << 8) + lsb;
   

   msb = readBME280(BME280_TEMPERATURE_CALIB_DIG_T3_MSB_REG);
   cal_param.dig_T3 = (int) msb;
   lsb = readBME280(BME280_TEMPERATURE_CALIB_DIG_T3_LSB_REG);
   cal_param.dig_T3 = (cal_param.dig_T3 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P1_MSB_REG);
   cal_param.dig_P1 = (unsigned int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P1_LSB_REG);
   cal_param.dig_P1 = (cal_param.dig_P1 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P2_MSB_REG);
   cal_param.dig_P2 = (int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P2_LSB_REG);
   cal_param.dig_P2 = (cal_param.dig_P2 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P3_MSB_REG);
   cal_param.dig_P3 = (int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P3_LSB_REG);
   cal_param.dig_P3 = (cal_param.dig_P3 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P4_MSB_REG);
   cal_param.dig_P4 = (int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P4_LSB_REG);
   cal_param.dig_P4 = (cal_param.dig_P4 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P5_MSB_REG);
   cal_param.dig_P5 = (int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P5_LSB_REG);
   cal_param.dig_P5 = (cal_param.dig_P5 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P6_MSB_REG);
   cal_param.dig_P6 = (int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P6_LSB_REG);
   cal_param.dig_P6 = (cal_param.dig_P6 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P7_MSB_REG);
   cal_param.dig_P7 = (int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P7_LSB_REG);
   cal_param.dig_P7 = (cal_param.dig_P7 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P8_MSB_REG);
   cal_param.dig_P8 = (int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P8_LSB_REG);
   cal_param.dig_P8 = (cal_param.dig_P8 << 8) + lsb;
   

   msb = readBME280(BME280_PRESSURE_CALIB_DIG_P9_MSB_REG);
   cal_param.dig_P9 = (int) msb;
   lsb = readBME280(BME280_PRESSURE_CALIB_DIG_P9_LSB_REG);
   cal_param.dig_P9 = (cal_param.dig_P9 << 8) + lsb;
   
   lsb = readBME280(BME280_HUMIDITY_CALIB_DIG_H1_REG);
   cal_param.dig_H1 = (char) lsb;
   

   msb = readBME280(BME280_HUMIDITY_CALIB_DIG_H2_MSB_REG);
   cal_param.dig_H2 = (int) msb;
   lsb = readBME280(BME280_HUMIDITY_CALIB_DIG_H2_LSB_REG);
   cal_param.dig_H2 = (cal_param.dig_H2 << 8) + lsb;
   
   lsb = readBME280(BME280_HUMIDITY_CALIB_DIG_H3_REG);
   cal_param.dig_H3 = (char) lsb;
   

   msb = readBME280(BME280_HUMIDITY_CALIB_DIG_H4_MSB_REG);
   cal_param.dig_H4 = (int) msb;
   lsb = readBME280(BME280_HUMIDITY_CALIB_DIG_H4_LSB_REG);
   cal_param.dig_H4 = (cal_param.dig_H4 << 4) | (lsb & 0xF);
   
   msb = readBME280(BME280_HUMIDITY_CALIB_DIG_H5_MSB_REG);
   cal_param.dig_H5 = (int) msb;
   cal_param.dig_H5 = (cal_param.dig_H5 << 4) | (lsb >> 4);
   
   lsb = readBME280(BME280_HUMIDITY_CALIB_DIG_H6_REG);
   cal_param.dig_H6 = (short) lsb;
}

void BME280_SetOversamplingPressure(char Value) {
  char ctrlm;
  ctrlm = BME280_GetCtrlMeasurement();
  ctrlm &= ~BME280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__MSK;
  ctrlm |= Value << BME280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__POS;
  
  writeBME280(BME280_CTRL_MEAS_REG, ctrlm);
}

void BME280_SetOversamplingTemperature(char Value) {
  char ctrlm;
  ctrlm = BME280_GetCtrlMeasurement();
  ctrlm &= ~BME280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__MSK;
  ctrlm |= Value << BME280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__POS;

  writeBME280(BME280_CTRL_MEAS_REG, ctrlm);
}

void BME280_SetOversamplingHumidity(char Value) {

  writeBME280(BME280_CTRL_HUMIDITY_REG, Value );
}

void BME280_SetOversamplingMode(char Value) {
  char ctrlm;
  ctrlm = BME280_GetCtrlMeasurement();
  ctrlm |= Value;

  writeBME280(BME280_CTRL_MEAS_REG, ctrlm);
}

void BME280_SetFilterCoefficient(char Value) {
  char cfgv;
  cfgv = BME280_GetConfig();
  cfgv &= ~BME280_CONFIG_REG_FILTER__MSK;
  cfgv |= Value << BME280_CONFIG_REG_FILTER__POS;
}

void BME280_SetStandbyTime(char Value) {
  char cfgv;
  cfgv = BME280_GetConfig();
  cfgv &= ~BME280_CONFIG_REG_TSB__MSK;
  cfgv |= Value << BME280_CONFIG_REG_TSB__POS;
}

char BME280_IsMeasuring() {
  char output;
  output = BME280_GetStatus();
  return (output & BME280_STAT_REG_MEASURING__MSK);
}

void BME280_INIT() {
  BME280_SetStandbyTime(BME280_STANDBY_TIME_1_MS);                              // Standby time 1ms
  BME280_SetFilterCoefficient(BME280_FILTER_COEFF_16);                          // IIR Filter coefficient 16
  BME280_SetOversamplingPressure(BME280_OVERSAMP_16X);                          // Pressure x16 oversampling
  BME280_SetOversamplingTemperature(BME280_OVERSAMP_2X);                        // Temperature x2 oversampling
  BME280_SetOversamplingHumidity(BME280_OVERSAMP_1X);                           // Humidity x1 oversampling
  BME280_SetOversamplingMode(BME280_NORMAL_MODE);
}

void BME280_OversamplingInit(void)
{
    BME280_ReadCalibrationParams();                                               //Read calibration parameters
    BME280_SetOversamplingPressure(BME280_OVERSAMP_1X);
    BME280_SetOversamplingTemperature(BME280_OVERSAMP_1X);
    BME280_SetOversamplingHumidity(BME280_OVERSAMP_1X);
    BME280_SetOversamplingMode(BME280_FORCED_MODE);
    
    while(BME280_IsMeasuring());
    BME280_ReadMeasurements();

}

/****************************************************************************************************/
/* Returns temperature in DegC, resolution is 0.01 DegC. Output value of ?5123? equals 51.23 DegC.  */
/***************************************************************************************************/

static long BME280_Compensate_T() {
  long temp1, temp2, T;

  temp1 = ((((adc_t>>3) -((long)cal_param.dig_T1<<1))) * ((long)cal_param.dig_T2)) >> 11;
  temp2 = (((((adc_t>>4) - ((long)cal_param.dig_T1)) * ((adc_t>>4) - ((long)cal_param.dig_T1))) >> 12) * ((long)cal_param.dig_T3)) >> 14;
  t_fine = temp1 + temp2;
  T = (t_fine * 5 + 128) >> 8;
  return T;
}

/************************************************************************************************************/
/* Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits). */
/* Output value of ?47445? represents 47445/1024 = 46.333 %RH */
/************************************************************************************************************/

static unsigned long BME280_Compensate_H() {
  long h1;
  h1 = (t_fine - ((long)76800));
  h1 = (((((adc_h << 14) - (((long)cal_param.dig_H4) << 20) - (((long)cal_param.dig_H5) * h1)) +
    ((long)16384)) >> 15) * (((((((h1 * ((long)cal_param.dig_H6)) >> 10) * (((h1 *
    ((long)cal_param.dig_H3)) >> 11) + ((long)32768))) >> 10) + ((long)2097152)) *
    ((long)cal_param.dig_H2) + 8192) >> 14));
  h1 = (h1 - (((((h1 >> 15) * (h1 >> 15)) >> 7) * ((long)cal_param.dig_H1)) >> 4));
  h1 = (h1 < 0 ? 0 : h1);
  h1 = (h1 > 419430400 ? 419430400 : h1);
  return (unsigned long)(h1>>12);
}

/***********************************************************************************************************/
/* Returns pressure in Pa as unsigned 32 bit integer. Output value of ?96386? equals 96386 Pa = 963.86 hPa */
/***********************************************************************************************************/

static unsigned long BME280_Compensate_P() {
  long press1, press2;
  unsigned long P;
  
  press1 = (((long)t_fine)>>1) - (long)64000;
  press2 = (((press1>>2) * (press1>>2)) >> 11 ) * ((long)cal_param.dig_P6);
  press2 = press2 + ((press1*((long)cal_param.dig_P5))<<1);
  press2 = (press2>>2)+(((long)cal_param.dig_P4)<<16);
  press1 = (((cal_param.dig_P3 * (((press1>>2) * (press1>>2)) >> 13 )) >> 3) + ((((long)cal_param.dig_P2) * press1)>>1))>>18;
  press1 =((((32768+press1))*((long)cal_param.dig_P1))>>15);
  if (press1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
  P = (((unsigned long)(((long)1048576)-adc_p)-(press2>>12)))*3125;
  if (P < 0x80000000) {
    P = (P << 1) / ((unsigned long)press1);
  } else {
    P = (P / (unsigned long)press1) * 2;
  }
  press1 = (((long)cal_param.dig_P9) * ((long)(((P>>3) * (P>>3))>>13)))>>12;
  press2 = (((long)(P>>2)) * ((long)cal_param.dig_P8))>>13;
  P = (unsigned long)((long)P + ((press1 + press2 + cal_param.dig_P7) >> 4));
  return P;
}


float BME280_GetTemperature() {
    uint16_t temp;
    while(BME280_IsMeasuring());
    BME280_ReadMeasurements(); 
    NOP();
    NOP();
    readBlockBME280(BME280_TEMPERATURE_MSB_REG,&temp,2);
    NOP();
    NOP();
    BME280_INIT();
  return (float)BME280_Compensate_T() / 100;
}

float BME280_GetHumidity() {
    while(BME280_IsMeasuring());
    BME280_ReadMeasurements(); 
    BME280_INIT();
  return (float)BME280_Compensate_H() / 1024;
  
}

float BME280_GetPressure() {
    while(BME280_IsMeasuring());
    BME280_ReadMeasurements(); 
    BME280_INIT();    
  return (float)BME280_Compensate_P() / 100;
}
