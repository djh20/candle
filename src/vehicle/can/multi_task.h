#pragma once

#include <Arduino.h>
#include "task.h"

#define MULTI_TASK_MAX_TASKS 8

class MultiTask: public Task
{
  public:
    MultiTask(const char *id);

    void add(uint8_t stage, Task *task, bool mustFinish = true);

  protected:
    void initiateAttempt() override;
    void process() override;
    void endAttempt(bool success) override;

  private:
    void runTasksForCurrentStage();

    Task *tasks[MULTI_TASK_MAX_TASKS];
    uint8_t totalTasks = 0;

    uint8_t flags[MULTI_TASK_MAX_TASKS];
  
    uint8_t currentStage = 0;
};
