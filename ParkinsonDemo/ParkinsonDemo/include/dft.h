/**
 * dft.h
 * Discrete Fourier Transform (DFT) Analysis Interface
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 */

#ifndef DFT_H
#define DFT_H

#include "config.h"
#include <stdint.h>

/**
 * Compute DFT on the sample buffer
 * Analyzes frequency content in the 1-10 Hz range
 * @param samples Pointer to sample buffer
 * @param sample_count Number of samples
 */
void dft_compute(int16_t* samples, int sample_count);

/**
 * Get energy in a specific frequency band
 * @param freq_low Lower frequency bound (Hz)
 * @param freq_high Upper frequency bound (Hz)
 * @return Energy value (scaled)
 */
int dft_get_band_energy(int freq_low, int freq_high);

/**
 * Get total activity level from samples
 * @param samples Pointer to sample buffer
 * @param sample_count Number of samples
 * @return Average activity level
 */
int dft_get_total_activity(int16_t* samples, int sample_count);

#endif // DFT_H
