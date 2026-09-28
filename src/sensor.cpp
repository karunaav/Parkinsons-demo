/**
 * sensor.cpp
 * LSM6DSL Accelerometer/Gyroscope Driver Implementation
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 */

#include "sensor.h"
#include <cstdlib>

// ============== Hardware Definitions ==============
#define SENSOR_I2C_SDA PB_11
#define SENSOR_I2C_SCL PB_10
#define LSM6DSL_ADDR   (0x6A << 1)  // 7-bit address shifted left

// LSM6DSL Register Addresses
#define LSM6DSL_WHO_AM_I     0x0F
#define LSM6DSL_CTRL1_XL     0x10   // Accelerometer control
#define LSM6DSL_CTRL2_G      0x11   // Gyroscope control
#define LSM6DSL_OUTX_L_XL    0x28   // Accelerometer data start

// I2C Instance
static I2C i2c(SENSOR_I2C_SDA, SENSOR_I2C_SCL);

// ============== Internal Functions ==============

/**
 * Write a value to a register
 */
static bool writeRegister(uint8_t reg, uint8_t value) {
    char data[2] = {(char)reg, (char)value};
    return i2c.write(LSM6DSL_ADDR, data, 2) == 0;
}

/**
 * Read a single register
 */
static uint8_t readRegister(uint8_t reg) {
    char data[1] = {(char)reg};
    i2c.write(LSM6DSL_ADDR, data, 1, true);
    i2c.read(LSM6DSL_ADDR, data, 1);
    return (uint8_t)data[0];
}

/**
 * Read multiple consecutive registers
 */
static void readRegisters(uint8_t reg, uint8_t* buffer, int length) {
    char r[1] = {(char)reg};
    i2c.write(LSM6DSL_ADDR, r, 1, true);
    i2c.read(LSM6DSL_ADDR, (char*)buffer, length);
}

// ============== Public Functions ==============

bool sensor_init(void) {
    // Set I2C frequency to 400kHz
    i2c.frequency(400000);
    
    // Verify device ID
    uint8_t whoami = readRegister(LSM6DSL_WHO_AM_I);
    printf("LSM6DSL WHO_AM_I: 0x%02X (expected 0x6A)\n", whoami);
    
    if (whoami != 0x6A) {
        printf("ERROR: LSM6DSL not found!\n");
        return false;
    }
    
    // Configure accelerometer: 52Hz ODR, +/-2G full scale
    // CTRL1_XL: ODR_XL[3:0]=0011 (52Hz), FS_XL[1:0]=00 (+/-2G)
    writeRegister(LSM6DSL_CTRL1_XL, 0x30);
    
    // Configure gyroscope: 52Hz ODR, 250dps full scale
    // CTRL2_G: ODR_G[3:0]=0011 (52Hz), FS_G[1:0]=00 (250dps)
    writeRegister(LSM6DSL_CTRL2_G, 0x30);
    
    printf("LSM6DSL initialized: 52Hz, +/-2G\n");
    return true;
}

int16_t sensor_read_accel_magnitude(void) {
    uint8_t buffer[6];
    readRegisters(LSM6DSL_OUTX_L_XL, buffer, 6);
    
    // Combine low and high bytes (little-endian)
    int16_t ax = (int16_t)(buffer[1] << 8 | buffer[0]);
    int16_t ay = (int16_t)(buffer[3] << 8 | buffer[2]);
    int16_t az = (int16_t)(buffer[5] << 8 | buffer[4]);
    
    // Calculate magnitude using sum of absolute values (fast approximation)
    int32_t mag = abs(ax) + abs(ay) + abs(az);
    return (int16_t)(mag / 16);
}

void sensor_read_accel_raw(int16_t* ax, int16_t* ay, int16_t* az) {
    uint8_t buffer[6];
    readRegisters(LSM6DSL_OUTX_L_XL, buffer, 6);
    
    *ax = (int16_t)(buffer[1] << 8 | buffer[0]);
    *ay = (int16_t)(buffer[3] << 8 | buffer[2]);
    *az = (int16_t)(buffer[5] << 8 | buffer[4]);
}
