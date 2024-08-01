#pragma once

#include <Arduino.h>

class TaskCallbacks;

enum class TaskMode
{
  Normal,
  RepeatUntilStopped
};

class Task 
{
  public:
    Task(const char *id);
    
    bool run();
    void tick();
    void stop();

    void setEnabled(bool enabled);

    bool isRunning() const;
    bool isSuccessful() const;

    void setCallbacks(TaskCallbacks *callbacks);

    const char *id;

    TaskMode mode = TaskMode::Normal;

    uint32_t minAttemptDuration = 0;
    uint32_t maxAttemptDuration = 10000;
    
    uint16_t minAttempts = 1;
    uint16_t maxAttempts = 1;

    uint32_t lastAttemptMillis = 0;
    uint32_t lastFinishMillis = 0;
    
    bool yetToRun = true;

  protected:
    virtual void preRun();
    virtual void initiateAttempt() = 0;
    virtual void process() = 0;
    virtual void endAttempt(bool success);

    bool enabled = true;
    bool running = false;
    bool attemptInProgress;
    bool lastAttemptWasSuccessful;
    bool lastRunWasSuccessful;
    uint16_t attemptCount;

    TaskCallbacks *callbacks = nullptr;

  private:
    void nextAttempt();
};

class TaskCallbacks
{
  public:
    virtual void onTaskRun(Task *task) = 0;
    virtual void onPollResponse(Task *task, uint8_t **frames) = 0;
};