#include "metric.h"

Metric::Metric(const char* id, uint16_t cooldown) 
{
  this->id = id;
  this->cooldown = cooldown;
}

bool Metric::update(uint32_t &now) {
  return false;
}
bool Metric::canUpdate(uint32_t &now) {
  return (now - lastUpdateMillis) >= cooldown;
}

void Metric::reset() {}
void Metric::addToJsonDoc(DynamicJsonDocument &doc) {}


MetricInt::MetricInt(const char* id, int32_t defaultValue, uint16_t cooldown) : Metric(id, cooldown) 
{
  this->defaultValue = defaultValue;
  this->value = defaultValue;
  this->lastUpdateValue = defaultValue;
}

void MetricInt::setValue(int32_t newValue) 
{
  if (value == newValue) return;
  value = newValue;
}

bool MetricInt::update(uint32_t &now)
{
  if (!canUpdate(now)) return false;
  if (lastUpdateValue == value) return false;
  
  lastUpdateMillis = now;
  lastUpdateValue = value;
  return true;
}

void MetricInt::reset() { setValue(defaultValue); }
void MetricInt::addToJsonDoc(DynamicJsonDocument &doc) { doc[id] = value; }


MetricFloat::MetricFloat(const char* id, float defaultValue, uint16_t cooldown) : Metric(id, cooldown) 
{
  this->defaultValue = defaultValue;
  this->value = defaultValue;
  this->lastUpdateValue = defaultValue;
}

void MetricFloat::setValue(float newValue) 
{
  if (value == newValue) return;
  value = newValue;
}

bool MetricFloat::update(uint32_t &now)
{
  if (!canUpdate(now)) return false;
  if (lastUpdateValue == value) return false;
  
  lastUpdateMillis = now;
  lastUpdateValue = value;
  return true;
}

void MetricFloat::reset() { setValue(defaultValue); }
void MetricFloat::addToJsonDoc(DynamicJsonDocument &doc) { doc[id] = value; }