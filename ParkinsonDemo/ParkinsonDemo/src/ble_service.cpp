/**
 * ble_service.cpp
 * BLE GATT Service Implementation
 * 
 * Parkinson's Disease Symptom Detector
 * Board: B-L475E-IOT01A
 * 
 * Custom BLE Service with 4 characteristics:
 * - Tremor status and intensity
 * - Dyskinesia status and intensity
 * - FOG status
 * - Overall status (text summary)
 */

#include "ble_service.h"
#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"
#include <cstring>

// ============== BLE UUIDs ==============
// Custom service and characteristic UUIDs
static const char* PARKINSON_SERVICE_UUID = "12340000-1234-1234-1234-123456789abc";
static const char* TREMOR_CHAR_UUID       = "12340001-1234-1234-1234-123456789abc";
static const char* DYSKINESIA_CHAR_UUID   = "12340002-1234-1234-1234-123456789abc";
static const char* FOG_CHAR_UUID          = "12340003-1234-1234-1234-123456789abc";
static const char* STATUS_CHAR_UUID       = "12340004-1234-1234-1234-123456789abc";

// ============== BLE Data Buffers ==============
// Text format for easy reading in LightBlue app
static uint8_t tremor_data[20] = {0};      // e.g., "TREMOR:75%"
static uint8_t dyskinesia_data[20] = {0};  // e.g., "DYSKINESIA:60%"
static uint8_t fog_data[20] = {0};         // e.g., "FOG DETECTED!"
static uint8_t status_data[32] = {0};      // Overall status summary

// ============== BLE Objects ==============
static BLE& bleInstance = BLE::Instance();
static bool ble_connected = false;

// GATT Characteristics
static ReadOnlyArrayGattCharacteristic<uint8_t, 20>* tremorChar;
static ReadOnlyArrayGattCharacteristic<uint8_t, 20>* dyskinesiaChar;
static ReadOnlyArrayGattCharacteristic<uint8_t, 20>* fogChar;
static ReadOnlyArrayGattCharacteristic<uint8_t, 32>* statusChar;

static GattCharacteristic* characteristics[4];
static GattService* parkinsonService;

// ============== BLE Callbacks ==============

/**
 * Called when BLE initialization completes
 */
static void onBleInitComplete(BLE::InitializationCompleteCallbackContext* context) {
    if (context->error != BLE_ERROR_NONE) {
        printf("BLE init error: %d\n", context->error);
        return;
    }
    printf("BLE initialized.\n");
}

/**
 * GAP Event Handler for connection events
 */
class BLEGapEventHandler : public ble::Gap::EventHandler {
public:
    void onConnectionComplete(const ble::ConnectionCompleteEvent& event) override {
        ble_connected = true;
        printf("BLE Connected!\n");
    }
    
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent& event) override {
        ble_connected = false;
        printf("BLE Disconnected. Restarting advertising...\n");
        bleInstance.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    }
};

static BLEGapEventHandler gapEventHandler;

// ============== Public Functions ==============

void ble_service_init(void) {
    printf("Initializing BLE...\n");
    
    // Initialize BLE stack
    bleInstance.init(onBleInitComplete);
    
    // Wait for initialization to complete
    while (!bleInstance.hasInitialized()) {
        bleInstance.processEvents();
        ThisThread::sleep_for(10ms);
    }
    
    // Register GAP event handler
    bleInstance.gap().setEventHandler(&gapEventHandler);
    
    // Create UUIDs
    UUID serviceUUID(PARKINSON_SERVICE_UUID);
    UUID tremorUUID(TREMOR_CHAR_UUID);
    UUID dyskinesiaUUID(DYSKINESIA_CHAR_UUID);
    UUID fogUUID(FOG_CHAR_UUID);
    UUID statusUUID(STATUS_CHAR_UUID);
    
    // Create GATT characteristics with read and notify properties
    tremorChar = new ReadOnlyArrayGattCharacteristic<uint8_t, 20>(
        tremorUUID, tremor_data,
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
    );
    
    dyskinesiaChar = new ReadOnlyArrayGattCharacteristic<uint8_t, 20>(
        dyskinesiaUUID, dyskinesia_data,
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
    );
    
    fogChar = new ReadOnlyArrayGattCharacteristic<uint8_t, 20>(
        fogUUID, fog_data,
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
    );
    
    statusChar = new ReadOnlyArrayGattCharacteristic<uint8_t, 32>(
        statusUUID, status_data,
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
    );
    
    // Add characteristics to service
    characteristics[0] = tremorChar;
    characteristics[1] = dyskinesiaChar;
    characteristics[2] = fogChar;
    characteristics[3] = statusChar;
    
    // Create and register GATT service
    parkinsonService = new GattService(serviceUUID, characteristics, 4);
    bleInstance.gattServer().addService(*parkinsonService);
    
    // Configure advertising
    const char DEVICE_NAME[] = "ParkinsonDet";
    
    ble::AdvertisingParameters advParams(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(100))
    );
    bleInstance.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, advParams);
    
    // Build advertising data
    uint8_t adv_buffer[31];
    ble::AdvertisingDataBuilder advBuilder(adv_buffer, sizeof(adv_buffer));
    advBuilder.setFlags(ble::adv_data_flags_t::LE_GENERAL_DISCOVERABLE | 
                        ble::adv_data_flags_t::BREDR_NOT_SUPPORTED);
    advBuilder.setName(DEVICE_NAME);
    
    bleInstance.gap().setAdvertisingPayload(
        ble::LEGACY_ADVERTISING_HANDLE,
        advBuilder.getAdvertisingData()
    );
    
    // Start advertising
    ble_error_t err = bleInstance.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if (err == BLE_ERROR_NONE) {
        printf("BLE advertising: %s\n", DEVICE_NAME);
    } else {
        printf("BLE advertising failed: %d\n", err);
    }
}

void ble_service_process(void) {
    bleInstance.processEvents();
}

void ble_service_update(DetectionState state, int tremor_intensity, int dyskinesia_intensity) {
    // Clear all data buffers
    memset(tremor_data, 0, sizeof(tremor_data));
    memset(dyskinesia_data, 0, sizeof(dyskinesia_data));
    memset(fog_data, 0, sizeof(fog_data));
    memset(status_data, 0, sizeof(status_data));
    
    // Set text values based on current state
    switch (state) {
        case STATE_TREMOR:
            sprintf((char*)tremor_data, "TREMOR:%d%%", tremor_intensity);
            sprintf((char*)status_data, "TREMOR %d%%", tremor_intensity);
            strcpy((char*)dyskinesia_data, "OK");
            strcpy((char*)fog_data, "OK");
            break;
            
        case STATE_DYSKINESIA:
            sprintf((char*)dyskinesia_data, "DYSKINESIA:%d%%", dyskinesia_intensity);
            sprintf((char*)status_data, "DYSKINESIA %d%%", dyskinesia_intensity);
            strcpy((char*)tremor_data, "OK");
            strcpy((char*)fog_data, "OK");
            break;
            
        case STATE_FOG:
            strcpy((char*)fog_data, "FOG DETECTED!");
            strcpy((char*)status_data, "FOG DETECTED!");
            strcpy((char*)tremor_data, "OK");
            strcpy((char*)dyskinesia_data, "OK");
            break;
            
        case STATE_WALKING:
            strcpy((char*)status_data, "WALKING...");
            strcpy((char*)tremor_data, "OK");
            strcpy((char*)dyskinesia_data, "OK");
            strcpy((char*)fog_data, "OK");
            break;
            
        default:  // STATE_NORMAL
            strcpy((char*)status_data, "NORMAL");
            strcpy((char*)tremor_data, "OK");
            strcpy((char*)dyskinesia_data, "OK");
            strcpy((char*)fog_data, "OK");
            break;
    }
    
    // Update GATT characteristic values
    bleInstance.gattServer().write(tremorChar->getValueHandle(), 
                                   tremor_data, strlen((char*)tremor_data));
    bleInstance.gattServer().write(dyskinesiaChar->getValueHandle(), 
                                   dyskinesia_data, strlen((char*)dyskinesia_data));
    bleInstance.gattServer().write(fogChar->getValueHandle(), 
                                   fog_data, strlen((char*)fog_data));
    bleInstance.gattServer().write(statusChar->getValueHandle(), 
                                   status_data, strlen((char*)status_data));
}

bool ble_service_is_connected(void) {
    return ble_connected;
}
