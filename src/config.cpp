#include "config.h"
#include "metric/metric_manager.h"
#include "wireless/bluetooth/bluetooth_manager.h"
#include <esp_mac.h>

void Config::begin()
{
  hostname = new StringMetric<1>("mcu", "hostname", MetricType::Parameter, 16);
  GlobalMetricManager.registerMetric(hostname);

  if (!hostname->isValid())
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

    hostname->setValueFromString(generatedHostname);
    hostname->save();
  }

  vehicleId = new StringMetric<1>("mcu", "vehicle_id", MetricType::Parameter, 16);
  GlobalMetricManager.registerMetric(vehicleId);

  blePin = new IntMetric<1>("ble", "pin", MetricType::Parameter);
  blePin->redact();
  GlobalMetricManager.registerMetric(blePin);

  wifiNetworks = new StringMetric<4>("wifi", "networks", MetricType::Parameter, 32);
  GlobalMetricManager.registerMetric(wifiNetworks);

  wifiPasswords = new StringMetric<4>("wifi", "passwords", MetricType::Parameter, 32);
  wifiPasswords->redact();
  GlobalMetricManager.registerMetric(wifiPasswords);
}

uint8_t Config::getBluetoothMode()
{
  return blePin->isValid() ? BLE_MODE_ENCRYPTED : BLE_MODE_OPEN;
}

Config GlobalConfig;