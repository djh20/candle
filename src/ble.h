#pragma once

#include <BLEUUID.h>

// Custom UUID Format (Randomly Generated):
// XXXXXXXX-XXXX-4D91-B049-E2828E6DA1A0

#define BLE_STD_SERVICE_DEVICE_INFO 0x180A
#define BLE_STD_CHARACTERISTIC_MODEL_NUMBER 0x2A24
#define BLE_STD_CHARACTERISTIC_FIRMWARE_VERSION 0x2A26

#define BLE_SERVICE_OTA 0x0000
#define BLE_CHARACTERISTIC_OTA_COMMAND 0x0001
#define BLE_CHARACTERISTIC_OTA_DATA 0x0002

#define BLE_SERVICE_DEBUG 0x0100
#define BLE_CHARACTERISTIC_DEBUG_COMMAND 0x0101
#define BLE_CHARACTERISTIC_DEBUG_LOG 0x0102

#define BLE_SERVICE_CONFIG 0x0200
#define BLE_CHARACTERISTIC_CONFIG_STATUS 0x0201
#define BLE_CHARACTERISTIC_CONFIG_NAME 0x0202
#define BLE_CHARACTERISTIC_CONFIG_PIN 0x0203
#define BLE_CHARACTERISTIC_CONFIG_VEHICLE 0x0204

#define BLE_SERVICE_VEHICLE_CATALOG 0x0300
#define BLE_CHARACTERISTIC_SUPPORTED_VEHICLE 0x0301

#define BLE_SERVICE_METRICS 0x0400
#define BLE_CHARACTERISTIC_GROUPED_METRIC_DATA 0x0401
#define BLE_DESCRIPTOR_GROUPED_METRIC_INFO 0x0402
#define BLE_CHARACTERISTIC_METRIC_MUTATOR 0x0403

#define BLE_SIZE_GROUPED_METRIC_DATA 64
#define BLE_SIZE_GROUPED_METRIC_INFO 96

BLEUUID generateUUID(uint16_t id, uint16_t discriminator = 0x0000);