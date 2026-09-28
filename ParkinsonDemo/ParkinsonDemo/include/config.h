/**
 * config.h
 * Global configuration parameters
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============== Sampling Configuration ==============
#define SAMPLE_RATE     52      // Sampling rate in Hz
#define WINDOW_SECONDS  3       // Window duration in seconds
#define SAMPLE_COUNT    156     // 52 * 3 = 156 samples
#define FFT_SIZE        256     // FFT size (power of 2)

// ============== Frequency Ranges ==============
#define TREMOR_LOW      3       // Tremor: 3-5 Hz
#define TREMOR_HIGH     5
#define DYSKINESIA_LOW  5       // Dyskinesia: 5-7 Hz
#define DYSKINESIA_HIGH 7
#define WALKING_LOW     1       // Walking: 1-3 Hz
#define WALKING_HIGH    3

// ============== Detection Thresholds ==============
#define TREMOR_THRESHOLD     500
#define DYSKINESIA_THRESHOLD 500
#define WALKING_THRESHOLD    300
#define STILLNESS_THRESHOLD  1100

// ============== FOG Parameters ==============
#define FOG_WALKING_WINDOWS  2  // Required consecutive walking windows

// ============== State Enumeration ==============
typedef enum {
    STATE_NORMAL,
    STATE_TREMOR,
    STATE_DYSKINESIA,
    STATE_WALKING,
    STATE_FOG
} DetectionState;

#endif // CONFIG_H
