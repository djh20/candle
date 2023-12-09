#include "metric.h"

Metric::Metric(const char* id) 
{
  this->id = id;
}

void Metric::onUpdate(std::function<void()> handler)
{
  updateHandler = handler;
}

void Metric::setValueFromString(String str) {}
void Metric::reset() {}
void Metric::addToJsonDoc(DynamicJsonDocument &doc) {}

MetricInt::MetricInt(const char* id, int32_t defaultValue, int32_t minValue, int32_t maxValue) : Metric(id) 
{
  this->defaultValue = defaultValue;
  this->minValue = minValue;
  this->maxValue = maxValue;
  this->value = defaultValue;
}

void MetricInt::setValue(int32_t newValue, bool force)
{
  if (newValue == value) return;
  if (!force && (newValue < minValue || newValue > maxValue)) return;

  value = newValue;
  lastUpdateMillis = millis();
  if (updateHandler) updateHandler();
}

void MetricInt::setValueFromString(String str) {
  setValue(str.toInt(), true);
}

void MetricInt::reset() { setValue(defaultValue, true); }
void MetricInt::addToJsonDoc(DynamicJsonDocument &doc) { doc[id] = value; }


MetricFloat::MetricFloat(const char* id, float defaultValue, float minValue, float maxValue) : Metric(id) 
{
  this->defaultValue = defaultValue;
  this->minValue = minValue;
  this->maxValue = maxValue;
  this->value = defaultValue;
}

void MetricFloat::setValue(float newValue, bool force)
{
  if (newValue == value) return;
  if (!force && (newValue < minValue || newValue > maxValue)) return;

  value = newValue;
  lastUpdateMillis = millis();
  if (updateHandler) updateHandler();
}

void MetricFloat::setValueFromString(String str) {
  setValue(str.toFloat(), true);
}

void MetricFloat::reset() { setValue(defaultValue, true); }
void MetricFloat::addToJsonDoc(DynamicJsonDocument &doc) { doc[id] = value; }