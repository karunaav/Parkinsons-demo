/**
 * main.cpp
 * Parkinson's Disease Symptom Detector - Main Program
 * 
 * Board: B-L475E-IOT01A
 * Framework: Mbed OS
 * 
 * Features:
 * - Tremor Detection (3-5 Hz): Rhythmic oscillations
 * - Dyskinesia Detection (5-7 Hz): Dance-like movements
 * - Freezing of Gait (FOG): Sudden stop after walking
 * - BLE transmission to mobile device
 * 
 * Author: [Your Name]
 * Date: 2025
 */

#include "mbed.h"
#include "config.h"
#include "sensor.h"
#include "dft.h"
#include "detection.h"
#include "ble_service.h"

// ============== Hardware Objects ==============
DigitalOut led1(LED1);          // Status indicator LED
DigitalOut led2(LED2);          // BLE connection / detection LED
InterruptIn button(BUTTON1);    // User button for pause/resume

// ============== Global Variables ==============
static int16_t accel_buffer[SAMPLE_COUNT];  // Accelerometer sample buffer
static int sample_index = 0;                 // Current sample index
static bool system_running = true;           // System running flag
static Timer sampleTimer;                    // Sampling timer

// ============== LED Indication Functions ==============

/**
 * Indicate tremor detection with fast LED1 blinking
 */
void indicate_tremor(void) {
    for (int i = 0; i < 6; i++) {
        led1 = !led1;
        ThisThread::sleep_for(80ms);
    }
    led1 = 1;
}

/**
 * Indicate dyskinesia detection with slow LED2 blinking
 */
void indicate_dyskinesia(void) {
    for (int i = 0; i < 4; i++) {
        led2 = !led2;
        ThisThread::sleep_for(150ms);
    }
    if (!ble_service_is_connected()) led2 = 0;
}

/**
 * Indicate FOG detection with both LEDs alternating
 */
void indicate_fog(void) {
    for (int i = 0; i < 10; i++) {
        led1 = !led1;
        led2 = !led2;
        ThisThread::sleep_for(100ms);
    }
    led1 = 1;
    if (!ble_service_is_connected()) led2 = 0;
}

// ============== Button Callback ==============

/**
 * Toggle system pause/resume on button press
 */
void button_pressed(void) {
    system_running = !system_running;
    printf("System %s\n", system_running ? "RESUMED" : "PAUSED");
}

// ============== Result Display ==============

/**
 * Print detection result to serial console
 */
void print_result(const DetectionResult& result) {
    printf("T:%4d(%2d%%) D:%4d(%2d%%) W:%4d A:%4d | ",
           result.tremor_energy, result.tremor_intensity,
           result.dyskinesia_energy, result.dyskinesia_intensity,
           result.walking_energy, result.activity);
    
    switch (result.state) {
        case STATE_TREMOR:
            printf(">>> TREMOR <<<\n");
            break;
        case STATE_DYSKINESIA:
            printf(">>> DYSKINESIA <<<\n");
            break;
        case STATE_WALKING:
            printf("Walking...\n");
            break;
        case STATE_FOG:
            printf(">>> FOG DETECTED <<<\n");
            break;
        default:
            printf("Normal\n");
            break;
    }
}

// ============== Main Program ==============

int main() {
    // Print startup banner
    printf("\n\n");
    printf("================================================\n");
    printf("    Parkinson's Disease Symptom Detector\n");
    printf("    Board: B-L475E-IOT01A\n");
    printf("================================================\n\n");
    
    // Initialize sensor
    printf("[1/3] Initializing sensor...\n");
    if (!sensor_init()) {
        printf("FATAL: Sensor initialization failed!\n");
        while (true) {
            led1 = !led1;
            ThisThread::sleep_for(100ms);
        }
    }
    
    // Initialize BLE service
    printf("[2/3] Initializing BLE...\n");
    ble_service_init();
    
    // Initialize button
    printf("[3/3] Initializing controls...\n");
    button.fall(&button_pressed);
    
    // Print configuration
    printf("\n--- Configuration ---\n");
    printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
    printf("Window: %d seconds (%d samples)\n", WINDOW_SECONDS, SAMPLE_COUNT);
    printf("Tremor: %d-%d Hz (threshold: %d)\n", TREMOR_LOW, TREMOR_HIGH, TREMOR_THRESHOLD);
    printf("Dyskinesia: %d-%d Hz (threshold: %d)\n", DYSKINESIA_LOW, DYSKINESIA_HIGH, DYSKINESIA_THRESHOLD);
    printf("Walking: %d-%d Hz (threshold: %d)\n", WALKING_LOW, WALKING_HIGH, WALKING_THRESHOLD);
    printf("FOG: Walking then sudden stop\n");
    printf("\n--- Controls ---\n");
    printf("USER Button: Pause/Resume detection\n");
    printf("LED1: Status indicator\n");
    printf("LED2: BLE connection / Detection indicator\n");
    printf("\nConnect with LightBlue app to 'ParkinsonDet'\n");
    printf("\n=============================================\n");
    printf("System ready. Starting detection...\n\n");
    
    // System ready
    led1 = 1;
    
    // Main loop
    sampleTimer.start();
    int lastSampleTime = 0;
    int sampleInterval_us = 1000000 / SAMPLE_RATE;  // ~19230 us for 52 Hz
    
    while (true) {
        // Process BLE events
        ble_service_process();
        
        // Check if paused
        if (!system_running) {
            led1 = 0;
            led2 = 0;
            ThisThread::sleep_for(100ms);
            continue;
        }
        
        // Sampling control
        int currentTime = sampleTimer.elapsed_time().count();
        
        if (currentTime - lastSampleTime >= sampleInterval_us) {
            lastSampleTime = currentTime;
            
            // Read accelerometer sample
            accel_buffer[sample_index] = sensor_read_accel_magnitude();
            sample_index++;
            
            // Sampling indicator (toggle every ~0.5 seconds)
            if (sample_index % 26 == 0) {
                led2 = !led2;
            }
            
            // Window complete - perform analysis
            if (sample_index >= SAMPLE_COUNT) {
                sample_index = 0;
                
                // Run detection algorithm
                DetectionResult result = detection_analyze(accel_buffer, SAMPLE_COUNT);
                
                // Print result to serial
                print_result(result);
                
                // Update BLE characteristics
                ble_service_update(result.state, result.tremor_intensity, result.dyskinesia_intensity);
                
                // LED indication based on state
                switch (result.state) {
                    case STATE_TREMOR:
                        indicate_tremor();
                        break;
                    case STATE_DYSKINESIA:
                        indicate_dyskinesia();
                        break;
                    case STATE_FOG:
                        indicate_fog();
                        break;
                    default:
                        led1 = 1;
                        if (!ble_service_is_connected()) led2 = 0;
                        break;
                }
                
                // Reset timer for next window
                sampleTimer.reset();
                lastSampleTime = 0;
            }
        }
    }
}
