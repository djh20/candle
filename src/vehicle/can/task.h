#pragma once

#include <Arduino.h>

class Task 
{
  public:
    Task(const char *id);
    
    bool run();
    void stop();

    void tick();

    void repeat();

    bool isRunning() const;
    bool isSuccessful() const;

    const char *id;
    bool enabled = true;

    uint32_t minAttemptDuration = 0;
    uint32_t maxAttemptDuration = 10000;
    
    uint16_t minAttempts = 1;
    uint16_t maxAttempts = 1;

    std::function<void()> onFinish;

  protected:
    virtual void initiateAttempt();
    virtual void process();
    virtual void endAttempt(bool success);

    bool running = false;
    bool attemptInProgress;
    bool lastAttemptWasSuccessful;
    bool lastRunWasSuccessful;

    uint32_t lastAttemptMillis = 0;
    uint32_t lastSuccessMillis = 0;

    uint16_t attemptCount;

  private:
    void nextAttempt();
};
