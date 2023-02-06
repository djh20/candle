#include "metric.h"

Metric::Metric(const char* id, uint16_t cooldown) 
{
  this->id = id;
  this->_cooldown = cooldown;
}

/*
void Metric::onUpdate(MetricUpdateHandler updateHandler) 
{
  _updateHandler = updateHandler;
}
*/

bool Metric::isCooldownActive(uint32_t &now) 
{
  return (now - _lastApplyMillis) < _cooldown;
}

bool Metric::applyValue(bool ignoreCooldown) { return false; }
void Metric::reset() {}
void Metric::addToJsonDoc(DynamicJsonDocument &doc) {}


MetricInt::MetricInt(const char* id, int32_t defaultValue, uint16_t cooldown) : Metric(id, cooldown) 
{
  this->defaultValue = defaultValue;
  this->value = defaultValue;
  this->pendingValue = defaultValue;
}

void MetricInt::setValue(int32_t newValue, bool applyInstantly)
{
  pendingValue = newValue;
  if (applyInstantly) applyValue(true);
}

bool MetricInt::applyValue(bool ignoreCooldown)
{
  if (value == pendingValue) return false;

  uint32_t now = millis();
  if (!ignoreCooldown && isCooldownActive(now)) return false;

  value = pendingValue;
  _lastApplyMillis = now;
  //if (_updateHandler != NULL) _updateHandler(this);

  return true;
}

void MetricInt::reset() { setValue(defaultValue); }
void MetricInt::addToJsonDoc(DynamicJsonDocument &doc) { doc[id] = value; }


MetricFloat::MetricFloat(const char* id, float defaultValue, uint16_t cooldown) : Metric(id, cooldown) 
{
  this->defaultValue = defaultValue;
  this->value = defaultValue;
  this->pendingValue = defaultValue;
}

void MetricFloat::setValue(float newValue, bool applyInstantly)
{
  pendingValue = newValue;
  if (applyInstantly) applyValue(true);
}

bool MetricFloat::applyValue(bool ignoreCooldown)
{
  if (value == pendingValue) return false;

  uint32_t now = millis();
  if (!ignoreCooldown && isCooldownActive(now)) return false;

  value = pendingValue;
  _lastApplyMillis = now;
  
  return true;
}

void MetricFloat::reset() { setValue(defaultValue); }
void MetricFloat::addToJsonDoc(DynamicJsonDocument &doc) { doc[id] = value; }