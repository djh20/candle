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

MetricInt::MetricInt(const char* id, int32_t defaultValue) : Metric(id) 
{
  this->defaultValue = defaultValue;
  this->value = defaultValue;
}

void MetricInt::setValue(int32_t newValue)
{
  if (newValue == value) return;
  value = newValue;
  lastUpdateMillis = millis();
  if (updateHandler) updateHandler();
}

void MetricInt::setValueFromString(String str) {
  setValue(str.toInt());
}

void MetricInt::reset() { setValue(defaultValue); }
void MetricInt::addToJsonDoc(DynamicJsonDocument &doc) { doc[id] = value; }


MetricFloat::MetricFloat(const char* id, float defaultValue) : Metric(id) 
{
  this->defaultValue = defaultValue;
  this->value = defaultValue;
}

void MetricFloat::setValue(float newValue)
{
  if (newValue == value) return;
  value = newValue;
  lastUpdateMillis = millis();
  if (updateHandler) updateHandler();
}

void MetricFloat::setValueFromString(String str) {
  setValue(str.toFloat());
}

void MetricFloat::reset() { setValue(defaultValue); }
void MetricFloat::addToJsonDoc(DynamicJsonDocument &doc) { doc[id] = value; }