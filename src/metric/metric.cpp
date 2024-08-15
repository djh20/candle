#include "metric.h"

#define RW_MODE false
#define RO_MODE true

Preferences Metric::prefs;

Metric::Metric(
  const char *domain, const char *localId, MetricType type, 
  MetricDataType dataType, Unit unit, uint8_t elementCount
) {
  this->domain = domain;
  this->localId = localId;
  this->type = type;
  this->dataType = dataType;
  this->unit = unit;
  this->elementCount = elementCount;

  uint8_t domainLen = strlen(domain);
  uint8_t localIdLen = strlen(localId);
  uint8_t idLen = domainLen + 1 + localIdLen; // +1 for the dot.

  // Allocate memory for the concatenated id.
  id = new char[idLen + 1]; // +1 for the null terminator.

  // Concatenate the strings with a dot separator.
  strcpy(id, domain);
  id[domainLen] = '.';
  strcpy(id + domainLen + 1, localId);
}

void Metric::begin()
{
  if (type != MetricType::Parameter) return;

  prefs.begin(domain, RO_MODE);
  if (prefs.isKey(localId))
  {
    loadState();
    log_i("Loaded state of [%s] from NVS", id);

    valid = true;
  }
  prefs.end();
}

void Metric::save()
{
  if (type != MetricType::Parameter) return;

  prefs.begin(domain, RW_MODE);

  if (valid) 
  {
    saveState();
  }
  else
  {
    prefs.remove(localId);
  }
  
  prefs.end();

  log_i("Saved state of [%s] to NVS", id);
}

void Metric::loadState() {}
void Metric::saveState() {}

void Metric::onUpdate(std::function<void()> handler)
{
  updateHandler = handler;
}

void Metric::invalidate()
{
  if (!valid) return;
  
  valid = false;
  markAsUpdated();
}

void Metric::redact()
{
  redacted = true;
}

void Metric::getState(char *buffer)
{
  for (uint8_t i = 0; i < elementCount; i++)
  {
    if (i > 0)
    {
      *buffer = ',';
      buffer++;
    }

    getValue(buffer, i);
    buffer += strlen(buffer);
  }
}

void Metric::getState(JsonDocument &json)
{
  if (redacted)
  {
    json[domain][localId] = "REDACTED";
  }
  else if (!valid)
  {
    json[domain][localId] = nullptr;
  }
  else
  {
    JsonArray arr = json[domain][localId].to<JsonArray>();
    for (uint8_t i = 0; i < elementCount; i++)
    {
      getValue(arr, i);
    }

    if (elementCount == 1) json[domain][localId] = arr[0];
  }
}

void Metric::markAsUpdated()
{
  lastUpdateMillis = millis();
  if (updateHandler) updateHandler();
}

void Metric::getStateData(uint8_t *buffer, uint8_t &bufferIndex) 
{
  buffer[bufferIndex++] = valid; // Flags (currently only used for validity)
}

uint8_t Metric::getStateDataSize()
{
  return 1;
}

void Metric::getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t stateDataIndex)
{
  size_t idLen = strlen(id);
  memcpy(buffer+bufferIndex, id, idLen);
  bufferIndex += idLen;
  
  buffer[bufferIndex++] = '\0';
  buffer[bufferIndex++] = stateDataIndex;
  buffer[bufferIndex++] = elementCount;
  buffer[bufferIndex++] = (static_cast<uint8_t>(type) << 4) | static_cast<uint8_t>(dataType);
  buffer[bufferIndex++] = static_cast<uint8_t>(unit);
}

uint8_t Metric::getDescriptorDataSize()
{
  return strlen(id) + 5;
}