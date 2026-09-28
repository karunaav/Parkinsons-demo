/**
 * ble_service.h
 * BLE GATT Service Interface
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 * 
 * Provides BLE connectivity to transmit detection results
 * to mobile devices (iOS/Android)
 */

#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include "config.h"
#include <stdint.h>

/**
 * Initialize BLE service
 * Sets up GATT server with custom characteristics
 * Starts advertising as "ParkinsonDet"
 */
void ble_service_init(void);

/**
 * Process BLE events
 * Must be called regularly in main loop
 */
void ble_service_process(void);

/**
 * Update BLE characteristic values
 * @param state Current detection state
 * @param tremor_intensity Tremor intensity (0-100%)
 * @param dyskinesia_intensity Dyskinesia intensity (0-100%)
 */
void ble_service_update(DetectionState state, int tremor_intensity, int dyskinesia_intensity);

/**
 * Check if a device is connected
 * @return true if connected, false otherwise
 */
bool ble_service_is_connected(void);

#endif // BLE_SERVICE_H
