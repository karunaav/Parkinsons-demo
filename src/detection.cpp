/**
 * detection.cpp
 * Parkinson's Symptom Detection Algorithm Implementation
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 * 
 * This module implements detection algorithms for:
 * - Tremor: Detected by high energy in 3-5 Hz band
 * - Dyskinesia: Detected by high energy in 5-7 Hz band
 * - FOG: Detected by walking pattern followed by sudden stillness
 */

#include "detection.h"
#include "dft.h"

// FOG detection state variables
static int walking_count = 0;       // Consecutive walking window counter
static bool was_walking = false;    // Flag indicating previous walking state

DetectionResult detection_analyze(int16_t* samples, int sample_count) {
    DetectionResult result;
    
    // Perform DFT frequency analysis
    dft_compute(samples, sample_count);
    
    // Get energy in each frequency band
    result.tremor_energy = dft_get_band_energy(TREMOR_LOW, TREMOR_HIGH);
    result.dyskinesia_energy = dft_get_band_energy(DYSKINESIA_LOW, DYSKINESIA_HIGH);
    result.walking_energy = dft_get_band_energy(WALKING_LOW, WALKING_HIGH);
    result.activity = dft_get_total_activity(samples, sample_count);
    
    // Calculate intensity percentages (capped at 100%)
    result.tremor_intensity = result.tremor_energy * 100 / 2000;
    result.dyskinesia_intensity = result.dyskinesia_energy * 100 / 2000;
    if (result.tremor_intensity > 100) result.tremor_intensity = 100;
    if (result.dyskinesia_intensity > 100) result.dyskinesia_intensity = 100;
    
    // Check if currently still (low activity)
    bool is_still = (result.activity < STILLNESS_THRESHOLD);
    
    // FOG Detection: Previously walking, now suddenly still
    if (was_walking && is_still) {
        was_walking = false;
        walking_count = 0;
        result.state = STATE_FOG;
        return result;
    }
    
    // Tremor Detection: High energy in 3-5 Hz band
    // Must be greater than walking energy to avoid false positives
    if (result.tremor_energy > TREMOR_THRESHOLD && 
        result.tremor_energy > result.walking_energy) {
        walking_count = 0;
        was_walking = false;
        result.state = STATE_TREMOR;
        return result;
    }
    
    // Dyskinesia Detection: High energy in 5-7 Hz band
    // Must be greater than walking energy to avoid false positives
    if (result.dyskinesia_energy > DYSKINESIA_THRESHOLD && 
        result.dyskinesia_energy > result.walking_energy) {
        walking_count = 0;
        was_walking = false;
        result.state = STATE_DYSKINESIA;
        return result;
    }
    
    // Walking Detection: High energy in 1-3 Hz band
    // Must dominate over tremor and dyskinesia bands
    bool is_walking = (result.walking_energy > WALKING_THRESHOLD) &&
                      (result.walking_energy > result.tremor_energy) &&
                      (result.walking_energy > result.dyskinesia_energy);
    
    if (is_walking) {
        walking_count++;
        if (walking_count >= FOG_WALKING_WINDOWS) {
            was_walking = true;
        }
        result.state = STATE_WALKING;
        return result;
    }
    
    // Reset walking counter if still
    if (is_still) {
        walking_count = 0;
    }
    
    // Default: Normal state
    result.state = STATE_NORMAL;
    return result;
}

void detection_reset(void) {
    walking_count = 0;
    was_walking = false;
}
