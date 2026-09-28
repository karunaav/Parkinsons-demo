/**
 * detection.h
 * Parkinson's Symptom Detection Algorithm Interface
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 * 
 * Detects three symptoms:
 * - Tremor (3-5 Hz oscillations)
 * - Dyskinesia (5-7 Hz movements)
 * - Freezing of Gait (FOG)
 */

#ifndef DETECTION_H
#define DETECTION_H

#include "config.h"
#include <stdint.h>

/**
 * Detection result structure
 * Contains all analysis results from one window
 */
typedef struct {
    DetectionState state;           // Current detected state
    int tremor_energy;              // Energy in tremor frequency band
    int dyskinesia_energy;          // Energy in dyskinesia frequency band
    int walking_energy;             // Energy in walking frequency band
    int activity;                   // Overall activity level
    int tremor_intensity;           // Tremor intensity (0-100%)
    int dyskinesia_intensity;       // Dyskinesia intensity (0-100%)
} DetectionResult;

/**
 * Analyze samples and detect symptoms
 * @param samples Pointer to accelerometer sample buffer
 * @param sample_count Number of samples in buffer
 * @return DetectionResult containing state and energy values
 */
DetectionResult detection_analyze(int16_t* samples, int sample_count);

/**
 * Reset detection state
 * Clears FOG tracking variables
 */
void detection_reset(void);

#endif // DETECTION_H
