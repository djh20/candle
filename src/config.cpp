#include "config.h"
#include "metric/metric_manager.h"
#include "bluetooth/bluetooth.h"

static const char *domain = "mcu";

void Config::begin()
{
  hostname = new StringMetric(domain, "hostname", MetricType::Parameter, 16);
  GlobalMetricManager.registerMetric(hostname);

  if (!hostname->valid)
  {
    uint8_t mac[8];
    esp_efuse_mac_get_default(mac); // Read MAC address from eFuse.

    char generatedHostname[12] = "CANDLE-";

    // ASCII:
    // Number characters from 48 to 57.
    // Letter characters from 65 to 90.

    // Calculate letters using 5th byte of MAC address.
    generatedHostname[7] = ((mac[4] >> 4) & 0x0F) + 65;
    generatedHostname[8] = (mac[4] & 0x0F) + 65;

    // Calculate numbers using 6th byte of MAC address.
    generatedHostname[9] = ((mac[5] >> 3) & 0x07) + 48;
    generatedHostname[10] = (mac[5] & 0x07) + 48;

    log_i("Generated hostname: %s", generatedHostname);

    hostname->setValue(generatedHostname);
  }

  vehicleId = new StringMetric(domain, "vehicle_id", MetricType::Parameter, 16);
  GlobalMetricManager.registerMetric(vehicleId);

  blePin = new IntMetric(domain, "ble_pin", MetricType::Parameter);
  GlobalMetricManager.registerMetric(blePin);

  // // 128 bytes = 4 SSIDs
  // wifiNetworks = new StringMetric(domain, "wifi_networks", MetricType::Parameter, 128);
  // GlobalMetricManager.registerMetric(wifiNetworks);

  // // 128 bytes = 4 passwords
  // wifiPasswords = new StringMetric(domain, "wifi_passwords", MetricType::Parameter, 128);
  // wifiPasswords->redact();
  // GlobalMetricManager.registerMetric(wifiPasswords);
}

uint8_t Config::getBluetoothMode()
{
  return blePin->valid ? BLE_MODE_ENCRYPTED : BLE_MODE_OPEN;
}

Config GlobalConfig;