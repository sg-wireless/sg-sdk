/*!
 * \file      CayenneLpp.h
 *
 * \brief     Implements the Cayenne Low Power Protocol
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */
#ifndef __CAYENNE_LPP_H__
#define __CAYENNE_LPP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define LPP_DIGITAL_INPUT 0          // 1 byte
#define LPP_DIGITAL_OUTPUT 1         // 1 byte
#define LPP_ANALOG_INPUT 2           // 2 bytes, 0.01 signed
#define LPP_ANALOG_OUTPUT 3          // 2 bytes, 0.01 signed
#define LPP_LUMINOSITY 101           // 2 bytes, 1 lux unsigned
#define LPP_PRESENCE 102             // 1 byte, 1
#define LPP_TEMPERATURE 103          // 2 bytes, 0.1°C signed
#define LPP_RELATIVE_HUMIDITY 104    // 1 byte, 0.5% unsigned
#define LPP_ACCELEROMETER 113        // 2 bytes per axis, 0.001G
#define LPP_BAROMETRIC_PRESSURE 115  // 2 bytes 0.1 hPa Unsigned
#define LPP_GYROMETER 134            // 2 bytes per axis, 0.01 °/s
#define LPP_GPS 136  // 3 byte lon/lat 0.0001 °, 3 bytes alt 0.01m

// Data ID + Data Type + Data Size
#define LPP_DIGITAL_INPUT_SIZE 3
#define LPP_DIGITAL_OUTPUT_SIZE 3
#define LPP_ANALOG_INPUT_SIZE 4
#define LPP_ANALOG_OUTPUT_SIZE 4
#define LPP_LUMINOSITY_SIZE 4
#define LPP_PRESENCE_SIZE 3
#define LPP_TEMPERATURE_SIZE 4
#define LPP_RELATIVE_HUMIDITY_SIZE 3
#define LPP_ACCELEROMETER_SIZE 8
#define LPP_BAROMETRIC_PRESSURE_SIZE 4
#define LPP_GYROMETER_SIZE 8
#define LPP_GPS_SIZE 11
#define CAYENNE_LPP_MAXBUFFER_SIZE 242

uint8_t CayenneLpp_Add_DigitalInput(uint8_t* buffer, int cursor,
                                    uint8_t channel, uint8_t value);
uint8_t CayenneLpp_Add_DigitalOutput(uint8_t* buffer, int cursor,
                                     uint8_t channel, uint8_t value);

uint8_t CayenneLpp_Add_AnalogInput(uint8_t* buffer, int cursor, uint8_t channel,
                                   float value);
uint8_t CayenneLpp_Add_AnalogOutput(uint8_t* buffer, int cursor,
                                    uint8_t channel, float value);

uint8_t CayenneLpp_Add_Luminosity(uint8_t* buffer, int cursor, uint8_t channel,
                                  uint16_t lux);
uint8_t CayenneLpp_Add_Presence(uint8_t* buffer, int cursor, uint8_t channel,
                                uint8_t value);
uint8_t CayenneLpp_Add_Temperature(uint8_t* buffer, int cursor, uint8_t channel,
                                   float celsius);
uint8_t CayenneLpp_Add_RelativeHumidity(uint8_t* buffer, int cursor,
                                        uint8_t channel, float rh);
uint8_t CayenneLpp_Add_Accelerometer(uint8_t* buffer, int cursor,
                                     uint8_t channel, float x, float y,
                                     float z);
uint8_t CayenneLpp_Add_BarometricPressure(uint8_t* buffer, int cursor,
                                          uint8_t channel, float hpa);
uint8_t CayenneLpp_Add_Gyrometer(uint8_t* buffer, int cursor, uint8_t channel,
                                 float x, float y, float z);
uint8_t CayenneLpp_Add_Gps(uint8_t* buffer, int cursor, uint8_t channel,
                           float latitude, float longitude, float meters);

#ifdef __cplusplus
}
#endif

#endif  // __CAYENNE_LPP_H__