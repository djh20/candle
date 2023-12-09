#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>
#include "../metric/metric.h"

class Gps
{
  public:
    Gps(uint8_t rxPin, MetricFloat *latMetric, MetricFloat *lngMetric, 
        MetricInt *lockMetric, MetricFloat *distanceMetric);

    void update(bool moving);
    void resetDistance();

    MetricFloat *latMetric;
    MetricFloat *lngMetric;
    MetricInt *lockMetric;
    MetricFloat *distanceMetric;

  private:
    TinyGPSPlus *tinyGps;
    HardwareSerial* serial;

    double lastLat;
    double lastLng;
};
