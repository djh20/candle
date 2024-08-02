#include "task.h"

Task::Task(const char *id)
{
  this->id = id;
}

bool Task::run()
{
  if (!enabled || running) return false;

  running = true;
  lastRunWasSuccessful = false;
  lastAttemptWasSuccessful = false;
  attemptCount = 0;

  if (callbacks) callbacks->onTaskRun(this);

  preRun();
  nextAttempt();

  return true;
}

void Task::tick() 
{
  if (!running) return;

  uint32_t now = millis();

  if (attemptInProgress && (!enabled || now - lastAttemptMillis >= maxAttemptDuration))
  {
    endAttempt(false);
  }

  if (!attemptInProgress && now - lastAttemptMillis >= minAttemptDuration)
  {
    nextAttempt();
  }

  if (attemptInProgress) process();
}

void Task::stop()
{
  if (!running) return;

  endAttempt(false);
  lastRunWasSuccessful = lastAttemptWasSuccessful;
  lastFinishMillis = millis();
  running = false;
  yetToRun = false;
}

void Task::setEnabled(bool enabled)
{
  if (this->enabled == enabled) return;
  if (enabled) yetToRun = true;
  this->enabled = enabled;

  log_i("Task [%s] is now %s", id, enabled ? "enabled" : "disabled");
}

bool Task::isRunning() const
{
  return running;
}

bool Task::isSuccessful() const
{
  return lastRunWasSuccessful;
}

void Task::endAttempt(bool success)
{
  if (!attemptInProgress) return;

  lastAttemptWasSuccessful = success;
  attemptInProgress = false;
}

void Task::nextAttempt()
{
  if (
    attemptCount >= minAttempts && 
    (lastAttemptWasSuccessful || attemptCount >= maxAttempts)
  ) {
    stop();
    return;
  }

  if (mode != TaskMode::RepeatUntilStopped)
  {
    attemptCount++;
  }

  lastAttemptWasSuccessful = false;
  lastAttemptMillis = millis();
  attemptInProgress = true;

  initiateAttempt();
}

void Task::setCallbacks(TaskCallbacks *callbacks)
{
  this->callbacks = callbacks;
}

void Task::preRun() {}