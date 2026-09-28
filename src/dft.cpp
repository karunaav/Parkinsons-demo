/**
 * dft.cpp
 * Discrete Fourier Transform (DFT) Analysis Implementation
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 * 
 * This module performs frequency analysis on accelerometer data
 * to detect characteristic frequencies of Parkinson's symptoms.
 */

#include "dft.h"
#include <cmath>
#include <cstdlib>

// FFT magnitude array (stores frequency bin magnitudes)
static int32_t fft_magnitude[FFT_SIZE / 2];

void dft_compute(int16_t* samples, int sample_count) {
    // Clear magnitude array
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        fft_magnitude[i] = 0;
    }
    
    // Calculate mean value (for DC removal)
    int32_t mean = 0;
    for (int i = 0; i < sample_count; i++) {
        mean += samples[i];
    }
    mean /= sample_count;
    
    // DFT computation for frequency bins 3-49 (approximately 0.6-10 Hz)
    // Frequency resolution: SAMPLE_RATE / FFT_SIZE = 52/256 = 0.2 Hz per bin
    // Bin k corresponds to frequency: k * SAMPLE_RATE / FFT_SIZE
    for (int k = 3; k <= 49; k++) {
        int32_t real_sum = 0;
        int32_t imag_sum = 0;
        
        for (int n = 0; n < sample_count; n++) {
            // Calculate angle: 2 * PI * k * n / FFT_SIZE
            float angle = 2.0f * 3.14159f * k * n / FFT_SIZE;
            
            // Remove DC component
            int32_t sample = samples[n] - mean;
            
            // Fixed-point arithmetic (multiply by 1000 for precision)
            int32_t cos_val = (int32_t)(cosf(angle) * 1000);
            int32_t sin_val = (int32_t)(sinf(angle) * 1000);
            
            real_sum += sample * cos_val / 1000;
            imag_sum -= sample * sin_val / 1000;
        }
        
        // Calculate magnitude using sum of absolute values (fast approximation)
        // This avoids expensive sqrt() computation
        fft_magnitude[k] = abs(real_sum) + abs(imag_sum);
    }
}

int dft_get_band_energy(int freq_low, int freq_high) {
    // Convert frequency to bin index
    // bin = freq * FFT_SIZE / SAMPLE_RATE
    int bin_low = freq_low * FFT_SIZE / SAMPLE_RATE;
    int bin_high = freq_high * FFT_SIZE / SAMPLE_RATE;
    
    // Sum energy in the frequency band
    int32_t energy = 0;
    for (int i = bin_low; i <= bin_high; i++) {
        energy += fft_magnitude[i];
    }
    
    // Scale down for reasonable output range
    return (int)(energy / 100);
}

int dft_get_total_activity(int16_t* samples, int sample_count) {
    // Calculate average absolute acceleration
    int32_t total = 0;
    for (int i = 0; i < sample_count; i++) {
        total += abs(samples[i]);
    }
    return (int)(total / sample_count);
}
