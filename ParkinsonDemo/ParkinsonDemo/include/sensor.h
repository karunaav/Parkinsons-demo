/**
 * sensor.h
 * LSM6DSL Accelerometer/Gyroscope Driver Interface
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 */

#ifndef SENSOR_H
#define SENSOR_H

#include "mbed.h"

/**
 * Initialize the LSM6DSL sensor
 * Configures: 52Hz sampling rate, +/-2G range
 * @return true if successful, false otherwise
 */
bool sensor_init(void);

/**
 * Read acceleration magnitude (combined from all 3 axes)
 * @return Magnitude value scaled to reasonable range
 */
int16_t sensor_read_accel_magnitude(void);

/**
 * Read raw acceleration data from all 3 axes
 * @param ax Pointer to store X-axis value
 * @param ay Pointer to store Y-axis value
 * @param az Pointer to store Z-axis value
 */
void sensor_read_accel_raw(int16_t* ax, int16_t* ay, int16_t* az);

#endif // SENSOR_H
