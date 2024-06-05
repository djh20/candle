#pragma once

#include <Arduino.h>

// Generic Metrics
#define METRIC_AWAKE 0x8B51
#define METRIC_GEAR 0x2C96
#define METRIC_SPEED 0x75AE
#define METRIC_STEERING_ANGLE 0x35F8
#define METRIC_FAN_SPEED 0x3419
#define METRIC_RANGE 0xC3E8
#define METRIC_TURN_SIGNAL 0x13CA
#define METRIC_HEADLIGHTS 0xF20B
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
#define METRIC_REMAINING_CHARGE_TIME 0x31D1
#define METRIC_RANGE_LAST_CHARGE 0x5E38
#define METRIC_QUICK_CHARGES 0x90FB
#define METRIC_SLOW_CHARGES 0x18AB

// GPS Metrics
#define METRIC_TRIP_DISTANCE 0x912F

enum MetricType
{
  Int,
  Float
};

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
  PSI = 14,
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
    virtual void setValueFromRawData(uint8_t *data);
    virtual void setValueFromString(String str);
    virtual void getDescriptorData(uint8_t buffer[], uint8_t &bufferIndex, uint8_t valueDataIndex);
    virtual void getValueData(uint8_t buffer[], uint8_t &bufferIndex);
    virtual uint8_t getValueDataLength();
    
    bool initialized = false;
    uint16_t id;
    uint32_t lastUpdateMillis = 0;
    Unit unit;
    std::function<void()> updateHandler;
};

class MetricInt: public Metric 
{
  public:
    MetricInt(uint16_t id, Unit unit);

    void setValue(int32_t newValue);
    void setValueFromRawData(uint8_t *data);
    void setValueFromString(String str);
    void getDescriptorData(uint8_t buffer[], uint8_t &bufferIndex, uint8_t valueDataIndex);
    void getValueData(uint8_t buffer[], uint8_t &bufferIndex);
    uint8_t getValueDataLength();

    int32_t value = 0;
};

class MetricFloat: public Metric 
{
  public:
    MetricFloat(uint16_t id, Unit unit, Precision precision);

    void setValue(float newValue);
    void setValueFromRawData(uint8_t *data);
    void setValueFromString(String str);
    void getDescriptorData(uint8_t buffer[], uint8_t &bufferIndex, uint8_t valueDataIndex);
    void getValueData(uint8_t buffer[], uint8_t &bufferIndex);
    uint8_t getValueDataLength();
 
    float value = 0;
    Precision precision;
};