#pragma once

#include <Arduino.h>
#include <BLECharacteristic.h>
#include <BLEDescriptor.h>

enum MetricType 
{
  Int,
  Float
};

// Generic Metrics
#define METRIC_GEAR 0x2C96
#define METRIC_IGNITION 0x8B51
#define METRIC_SPEED 0x75AE
#define METRIC_FAN_SPEED 0x3419
#define METRIC_RANGE 0xC3E8
#define METRIC_TURN_SIGNAL 0x13CA
#define METRIC_PARK_BRAKE 0x8BFE
#define METRIC_AMBIENT_TEMPERATURE 0x2842

// EV Metrics
#define METRIC_SOC 0x3DEE
#define METRIC_SOH 0x22DA
#define METRIC_HV_BATT_VOLTAGE 0xC549
#define METRIC_HV_BATT_CURRENT 0xE27C
#define METRIC_HV_BATT_POWER 0xE2DB
#define METRIC_HV_BATT_CAPACITY 0x5F5F
#define METRIC_HV_BATT_TEMPERATURE 0x87C9
#define METRIC_CHARGE_STATUS 0x2DA6
#define METRIC_REMAIN_CHARGE_TIME 0x31D1
#define METRIC_RANGE_LAST_CHARGE 0x5E38
#define METRIC_QUICK_CHARGES 0x90FB
#define METRIC_SLOW_CHARGES 0x18AB

enum Unit
{
  None = 0,
  Percent = 1,
  Meters = 2,
  Kilometers = 3,
  KilometersPerHour = 4,
  Volts = 5,
  Amps = 6,
  Watts = 7,
  Kilowatts = 8,
  KilowattHours = 9,
  Celsius = 10,
  Seconds = 11,
  Minutes = 12,
  Hours = 13,
};

enum Precision
{
  Low = 1,
  Medium = 2,
  High = 4,
};

class Metric
{
  public:
    Metric(uint16_t id, Unit unit);

    void onUpdate(std::function<void()> handler);
    virtual void setValueFromString(String str);
    
    bool initialized = false;
    uint16_t id;
    uint32_t lastUpdateMillis = 0;
    Unit unit;
    std::function<void()> updateHandler;
    BLECharacteristic *bleCharacteristic;
    BLEDescriptor *bleDescriptor;
};

class MetricInt: public Metric 
{
  public:
    MetricInt(uint16_t id, Unit unit, int32_t minValue, int32_t maxValue);

    void setValue(int32_t newValue, bool force = false);
    void setValueFromString(String str);

    int32_t value = 0;
    int32_t minValue;
    int32_t maxValue;
};

class MetricFloat: public Metric 
{
  public:
    MetricFloat(uint16_t id, Unit unit, Precision precision, float minValue, float maxValue);

    void setValue(float newValue, bool force = false);
    void setValueFromString(String str);
 
    float value = 0;
    float minValue;
    float maxValue;
    Precision precision;
};