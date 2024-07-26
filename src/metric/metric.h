#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>

enum class MetricType
{
  Statistic,
  Parameter
};

enum class MetricDataType
{
  Int,
  Float,
  String
};

enum class Unit
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

enum class Precision
{
  Low = 1,
  Medium = 2,
  High = 4,
};

class Metric
{
  public:
    Metric(
      const char *domain, const char *localId, MetricType type, 
      MetricDataType dataType, Unit unit, uint8_t elementCount
    );

    void begin();
    void save();

    void onUpdate(std::function<void()> handler);
    void invalidate();
    void redact();

    virtual void setValue(const char *value, uint8_t elementIndex = 0) = 0;
    virtual void getValue(char *buffer, uint8_t elementIndex = 0) = 0;

    void getState(char *buffer);
    void getState(JsonDocument &json);

    virtual void getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t valueDataIndex);
    virtual void getValueData(uint8_t *buffer, uint8_t &bufferIndex) = 0;
    virtual uint8_t getValueDataLength() = 0;
    
    char *id;
    const char *domain;
    const char *localId;
    Unit unit;
    MetricType type;
    MetricDataType dataType;

    bool valid = false;
    bool redacted = false;
    uint32_t lastUpdateMillis = 0;
    uint32_t lastSaveMillis = 0;

    uint8_t elementCount = 0;

  protected:
    virtual void getValue(JsonArray &json, uint8_t elementIndex = 0) = 0;

    void markAsUpdated();
    virtual void loadState();
    virtual void saveState();

    std::function<void()> updateHandler;

    static Preferences prefs;
};