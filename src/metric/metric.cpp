#include "metric.h"

#define RW_MODE false
#define RO_MODE true

Preferences Metric::prefs;

Metric::Metric(
  const char *domain, const char *localId, MetricType type, 
  MetricDataType dataType, Unit unit
) {
  this->domain = domain;
  this->localId = localId;
  this->type = type;
  this->dataType = dataType;
  this->unit = unit;

  uint8_t domainLen = strlen(domain);
  uint8_t localIdLen = strlen(localId);
  uint8_t idLen = domainLen + 1 + localIdLen; // +1 for the dot.

  // Allocate memory for the concatenated id.
  id = new char[idLen + 1]; // +1 for the null terminator.

  // Create the id by Concatenate the strings with a dot separator.
  strcpy(id, domain);
  id[domainLen] = '.';
  strcpy(id + domainLen + 1, localId);
}

void Metric::begin()
{
  if (type != MetricType::Parameter) return;

  prefs.begin(domain, RO_MODE);
  if (prefs.isKey(localId)) loadValue();
  prefs.end();
}

void Metric::save()
{
  if (type != MetricType::Parameter) return;

  prefs.begin(domain, RW_MODE);

  if (valid) 
  {
    saveValue();
  }
  else
  {
    prefs.remove(localId);
  }
  
  prefs.end();
}

void Metric::loadValue() {}
void Metric::saveValue() {}

void Metric::onUpdate(std::function<void()> handler)
{
  updateHandler = handler;
}

void Metric::invalidate()
{
  valid = false;
  markAsUpdated();
  save();
}

void Metric::redact()
{
  redacted = true;
}

void Metric::markAsUpdated()
{
  lastUpdateMillis = millis();
  if (updateHandler) updateHandler();
}

void Metric::getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t valueDataIndex)
{
  size_t idLen = strlen(localId);
  
  memcpy(buffer+bufferIndex, localId, idLen);
  bufferIndex += idLen + 1; // Extra byte for null terminator.

  buffer[bufferIndex++] = valueDataIndex;
  buffer[bufferIndex++] = static_cast<uint8_t>(unit);
  buffer[bufferIndex++] = static_cast<uint8_t>(dataType);
}