#include "gps.h"
#include "../../utils/logger.h"

Gps::Gps(uint8_t rxPin, MetricFloat *latMetric, MetricFloat *lngMetric,
        MetricInt *lockMetric, MetricFloat *distanceMetric)
{
  tinyGps = new TinyGPSPlus();
  
  serial = new SoftwareSerial(rxPin);
  serial->begin(9600);

  this->latMetric = latMetric;
  this->lngMetric = lngMetric;
  this->lockMetric = lockMetric;
  this->distanceMetric = distanceMetric;
}

void Gps::update(bool moving)
{
  while (serial->available() > 0) 
  {
    tinyGps->encode(serial->read());
  }

  lockMetric->setValue((tinyGps->location.age() < 5000) ? 1 : 0);

  if (tinyGps->location.isValid() && tinyGps->location.isUpdated())
  {
    double lat = tinyGps->location.lat();
    double lng = tinyGps->location.lng();

    latMetric->setValue(lat);
    lngMetric->setValue(lng);

    if (lastLat != 0 && lastLng != 0 && lastLat != lat && lastLng != lng && moving) 
    {
      float distance = TinyGPSPlus::distanceBetween(lat, lng, lastLat, lastLng);
      if (distance >= 0.1 && distance <= 100000) 
      {
        distanceMetric->setValue(distanceMetric->value + distance);
      }

      Logger.log(Debug, "gps", "Moved %fm", distance);
    }

    lastLat = lat;
    lastLng = lng;
  }
}

void Gps::resetDistance()
{
  distanceMetric->reset();
}
