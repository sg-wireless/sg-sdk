/*!
 * \file      CayenneLpp.c
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
#include "CayenneLpp.h"

#include <stdint.h>
#include <stdio.h>

uint8_t CayenneLpp_Add_DigitalInput(uint8_t *buffer, int cursor,
                                    uint8_t channel, uint8_t value) {
    if ((cursor + LPP_DIGITAL_INPUT_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_DIGITAL_INPUT;
    buffer[cursor++] = value;

    return cursor;
}

uint8_t CayenneLpp_Add_DigitalOutput(uint8_t *buffer, int cursor,
                                     uint8_t channel, uint8_t value) {
    if ((cursor + LPP_DIGITAL_OUTPUT_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_DIGITAL_OUTPUT;
    buffer[cursor++] = value;

    return cursor;
}

uint8_t CayenneLpp_Add_AnalogInput(uint8_t *buffer, int cursor, uint8_t channel,
                                   float value) {
    if ((cursor + LPP_ANALOG_INPUT_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }

    int16_t val = (int16_t)(value * 100);
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_ANALOG_INPUT;
    buffer[cursor++] = val >> 8;
    buffer[cursor++] = val;

    return cursor;
}

uint8_t CayenneLpp_Add_AnalogOutput(uint8_t *buffer, int cursor,
                                    uint8_t channel, float value) {
    if ((cursor + LPP_ANALOG_OUTPUT_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    int16_t val = (int16_t)(value * 100);
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_ANALOG_OUTPUT;
    buffer[cursor++] = val >> 8;
    buffer[cursor++] = val;

    return cursor;
}

uint8_t CayenneLpp_Add_Luminosity(uint8_t *buffer, int cursor, uint8_t channel,
                                  uint16_t lux) {
    if ((cursor + LPP_LUMINOSITY_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_LUMINOSITY;
    buffer[cursor++] = lux >> 8;
    buffer[cursor++] = lux;

    return cursor;
}

uint8_t CayenneLpp_Add_Presence(uint8_t *buffer, int cursor, uint8_t channel,
                                uint8_t value) {
    if ((cursor + LPP_PRESENCE_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_PRESENCE;
    buffer[cursor++] = value;

    return cursor;
}

uint8_t CayenneLpp_Add_Temperature(uint8_t *buffer, int cursor, uint8_t channel,
                                   float celsius) {
    if ((cursor + LPP_TEMPERATURE_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    int16_t val = (int16_t)(celsius * 10);
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_TEMPERATURE;
    buffer[cursor++] = val >> 8;
    buffer[cursor++] = val;
    return cursor;
}

uint8_t CayenneLpp_Add_RelativeHumidity(uint8_t *buffer, int cursor,
                                        uint8_t channel, float rh) {
    if ((cursor + LPP_RELATIVE_HUMIDITY_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_RELATIVE_HUMIDITY;
    buffer[cursor++] = (uint8_t)(rh * 2);

    return cursor;
}

uint8_t CayenneLpp_Add_Accelerometer(uint8_t *buffer, int cursor,
                                     uint8_t channel, float x, float y,
                                     float z) {
    if ((cursor + LPP_ACCELEROMETER_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    int16_t vx = (int16_t)(x * 1000);
    int16_t vy = (int16_t)(y * 1000);
    int16_t vz = (int16_t)(z * 1000);

    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_ACCELEROMETER;
    buffer[cursor++] = vx >> 8;
    buffer[cursor++] = vx;
    buffer[cursor++] = vy >> 8;
    buffer[cursor++] = vy;
    buffer[cursor++] = vz >> 8;
    buffer[cursor++] = vz;

    return cursor;
}

uint8_t CayenneLpp_Add_BarometricPressure(uint8_t *buffer, int cursor,
                                          uint8_t channel, float hpa) {
    if ((cursor + LPP_BAROMETRIC_PRESSURE_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    int16_t val = (int16_t)(hpa * 10);

    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_BAROMETRIC_PRESSURE;
    buffer[cursor++] = val >> 8;
    buffer[cursor++] = val;

    return cursor;
}

uint8_t CayenneLpp_Add_Gyrometer(uint8_t *buffer, int cursor, uint8_t channel,
                                 float x, float y, float z) {
    if ((cursor + LPP_GYROMETER_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    int16_t vx = (int16_t)(x * 100);
    int16_t vy = (int16_t)(y * 100);
    int16_t vz = (int16_t)(z * 100);

    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_GYROMETER;
    buffer[cursor++] = vx >> 8;
    buffer[cursor++] = vx;
    buffer[cursor++] = vy >> 8;
    buffer[cursor++] = vy;
    buffer[cursor++] = vz >> 8;
    buffer[cursor++] = vz;

    return cursor;
}

uint8_t CayenneLpp_Add_Gps(uint8_t *buffer, int cursor, uint8_t channel,
                           float latitude, float longitude, float meters) {
    if ((cursor + LPP_GPS_SIZE) > CAYENNE_LPP_MAXBUFFER_SIZE) {
        return 0;
    }
    int32_t lat = (int32_t)(latitude * 10000);
    int32_t lon = (int32_t)(longitude * 10000);
    int32_t alt = (int32_t)(meters * 100);

    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_GPS;

    buffer[cursor++] = lat >> 16;
    buffer[cursor++] = lat >> 8;
    buffer[cursor++] = lat;
    buffer[cursor++] = lon >> 16;
    buffer[cursor++] = lon >> 8;
    buffer[cursor++] = lon;
    buffer[cursor++] = alt >> 16;
    buffer[cursor++] = alt >> 8;
    buffer[cursor++] = alt;

    return cursor;
}